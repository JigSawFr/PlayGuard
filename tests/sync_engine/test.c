// Host tests for source/sync/sync_engine.c and mqtt_client.c against a fake
// MQTT 5.0 / 3.1.1 broker kept in memory (retained messages, subscriptions, last
// will, keep-alive), with a scripted clock: the connection and its last
// will, the discovery, the state and activity, orders applied, refused,
// merged, taken by the host or kept waiting, Home Assistant restarting,
// the broker refusing or dropping the connection, oversized messages.
#include "check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_client.h"
#include "mqtt_packet.h"
#include "sync_engine.h"

// ---------------------------------------------------------------- clock

static uint64_t now = 1000000;
static uint64_t clock_ms(void) { return now; }

// ---------------------------------------------------------------- broker

#define MAX_RETAINED 64
#define MAX_LOG      512

typedef struct {
    char    topic[160];
    char    payload[32768];   // the discovery: 18.5 KiB
    size_t  len;
    bool    retain;
} Message;

static struct {
    bool     up;              // accepts connections
    uint8_t  refuse_code;     // CONNACK return code (3.1.1's; mapped for a 5.0 client)
    uint8_t  min_version, max_version;   // the protocol levels it speaks (4: 3.1.1, 5: 5.0)
    bool     close_on_unknown;           // another level: closes the stream instead of answering
    unsigned refused_versions;
    uint8_t  version;         // the connected client's
    uint32_t client_max_packet;          // what a 5.0 CONNECT said
    uint16_t server_keepalive;           // 5.0 CONNACK properties (0: not sent)
    uint32_t max_packet;
    bool     no_retain;
    char     reason[64];
    bool     connected;       // a client is connected (CONNECT accepted)
    bool     open;
    bool     answer_pings;
    char     client_id[64], username[64], password[64];
    Message  will;
    bool     has_will;
    char     subs[4][160];
    uint8_t  sub_qos[4];
    size_t   n_subs;
    char     deny[160];       // a filter its access list refuses ("" none)
    Message  retained[MAX_RETAINED];
    size_t   n_retained;
    Message *log;             // every PUBLISH the client sent
    size_t   n_log;
    unsigned pings, disconnects, connects;
    uint8_t  in[65536];       // client -> broker, not parsed yet
    size_t   in_len;
    uint8_t  out[262144];     // broker -> client
    size_t   out_len;
    uint16_t next_id;
} broker;

static void broker_reset(void)
{
    Message *log = broker.log;
    memset(&broker, 0, sizeof(broker));
    broker.log = log;
    broker.up = true;
    broker.answer_pings = true;
    broker.next_id = 1;
    broker.min_version = 4;
    broker.max_version = 5;
}

static void emit(const uint8_t *b, size_t n)
{
    REQUIRE(broker.out_len + n <= sizeof(broker.out));
    memcpy(broker.out + broker.out_len, b, n);
    broker.out_len += n;
}

static void emit_publish(const char *topic, const char *payload, size_t len, bool retain, uint8_t qos)
{
    uint8_t head[256];
    const size_t h = mqtt_encode_publish_head(head, sizeof(head), broker.version ? broker.version : MQTT_V311, topic, len,
                                              qos, retain, false, qos ? broker.next_id++ : 0);
    REQUIRE(h);
    emit(head, h);
    emit((const uint8_t *)payload, len);
}

static bool subscribed(const char *topic, uint8_t *qos)
{
    for (size_t i = 0; i < broker.n_subs; i++)
        if (mqtt_topic_matches(broker.subs[i], topic, strlen(topic))) {
            *qos = broker.sub_qos[i];
            return true;
        }
    return false;
}

static Message *find_retained(const char *topic)
{
    for (size_t i = 0; i < broker.n_retained; i++)
        if (!strcmp(broker.retained[i].topic, topic)) return &broker.retained[i];
    return NULL;
}

// Stores (or deletes, empty payload) a retained message, and delivers it to
// the client when it is subscribed (RETAIN 0: an established subscription).
static void broker_publish(const char *topic, const char *payload, size_t len, bool retain)
{
    if (retain) {
        Message *m = find_retained(topic);
        if (len == 0) {
            if (m) *m = broker.retained[--broker.n_retained];
        } else {
            if (!m) {
                REQUIRE(broker.n_retained < MAX_RETAINED);
                m = &broker.retained[broker.n_retained++];
            }
            snprintf(m->topic, sizeof(m->topic), "%s", topic);
            memcpy(m->payload, payload, len);
            m->payload[len] = '\0';
            m->len = len;
            m->retain = true;
        }
    }
    uint8_t qos;
    if (broker.connected && subscribed(topic, &qos)) emit_publish(topic, payload, len, false, qos);
}

static void broker_drop(bool send_will)
{
    if (broker.connected && send_will && broker.has_will)
        broker_publish(broker.will.topic, broker.will.payload, broker.will.len, broker.will.retain);
    broker.connected = false;
    broker.open = false;
    broker.n_subs = 0;
    broker.in_len = broker.out_len = 0;
}

// 5.0: the broker ends the connection itself, saying why.
static void broker_kick(uint8_t code, const char *reason)
{
    const size_t n = strlen(reason);
    uint8_t d[64] = { 0xE0, (uint8_t)(2 + 3 + n), code, (uint8_t)(3 + n), 0x1F, 0x00, (uint8_t)n };
    memcpy(d + 7, reason, n);
    emit(d, 7 + n);
}

static void read_str(const uint8_t **p, char *out, size_t cap)
{
    const size_t n = ((size_t)(*p)[0] << 8) | (*p)[1];
    REQUIRE(n < cap);
    memcpy(out, *p + 2, n);
    out[n] = '\0';
    *p += 2 + n;
}

static void broker_handle(const MqttPacket *pk)
{
    switch (pk->type) {
    case MQTT_CONNECT: {
        const uint8_t version = pk->body[6];
        const uint8_t *p = pk->body + 7;
        const uint8_t flags = *p++;
        p += 2;   // keep-alive
        if (version == MQTT_V5) {
            // The client's properties: only its maximum packet size.
            const uint8_t n = *p++;
            broker.client_max_packet = 0;
            if (n == 5 && p[0] == 0x27)
                broker.client_max_packet = ((uint32_t)p[1] << 24) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 8) | p[4];
            p += n;
        }
        if (version < broker.min_version || version > broker.max_version) {
            // A broker of another version: "unacceptable protocol version"
            // in 3.1.1's words, or nothing at all.
            broker.refused_versions++;
            if (broker.close_on_unknown) {
                broker_drop(false);
            } else {
                const uint8_t ack[4] = { 0x20, 0x02, 0x00, 0x01 };
                emit(ack, 4);
            }
            break;
        }
        broker.version = version;
        read_str(&p, broker.client_id, sizeof(broker.client_id));
        broker.has_will = flags & 0x04;
        if (broker.has_will) {
            if (version == MQTT_V5) p += 1 + p[0];   // will properties
            read_str(&p, broker.will.topic, sizeof(broker.will.topic));
            char tmp[64];
            read_str(&p, tmp, sizeof(tmp));
            snprintf(broker.will.payload, sizeof(broker.will.payload), "%s", tmp);
            broker.will.len = strlen(tmp);
            broker.will.retain = flags & 0x20;
        }
        broker.username[0] = broker.password[0] = '\0';
        if (flags & 0x80) read_str(&p, broker.username, sizeof(broker.username));
        if (flags & 0x40) read_str(&p, broker.password, sizeof(broker.password));
        if (version == MQTT_V5) {
            uint8_t props[96];
            size_t np = 0;
            if (broker.server_keepalive) {
                props[np++] = 0x13;
                props[np++] = (uint8_t)(broker.server_keepalive >> 8);
                props[np++] = (uint8_t)broker.server_keepalive;
            }
            if (broker.max_packet) {
                props[np++] = 0x27;
                for (int i = 3; i >= 0; i--) props[np++] = (uint8_t)(broker.max_packet >> (8 * i));
            }
            if (broker.no_retain) {
                props[np++] = 0x25;
                props[np++] = 0;
            }
            if (broker.reason[0]) {
                const size_t n = strlen(broker.reason);
                props[np++] = 0x1F;
                props[np++] = 0;
                props[np++] = (uint8_t)n;
                memcpy(props + np, broker.reason, n);
                np += n;
            }
            const uint8_t rc = broker.refuse_code == 0 ? 0 : broker.refuse_code == 4 ? 0x86
                             : broker.refuse_code == 5 ? 0x87 : 0x80;
            uint8_t ack[128] = { 0x20, (uint8_t)(3 + np), 0x00, rc, (uint8_t)np };
            memcpy(ack + 5, props, np);
            emit(ack, 5 + np);
        } else {
            const uint8_t ack[4] = { 0x20, 0x02, 0x00, broker.refuse_code };
            emit(ack, 4);
        }
        broker.connects++;
        broker.connected = broker.refuse_code == 0;
        break;
    }
    case MQTT_PUBLISH: {
        MqttPublish m;
        CHECK(mqtt_decode_publish(pk, broker.version, &m) == MQTT_OK);
        CHECK(m.qos == 0);   // the client publishes QoS 0 only
        char topic[160];
        memcpy(topic, m.topic, m.topic_len);
        topic[m.topic_len] = '\0';
        REQUIRE(broker.n_log < MAX_LOG);
        Message *l = &broker.log[broker.n_log++];
        snprintf(l->topic, sizeof(l->topic), "%s", topic);
        memcpy(l->payload, m.payload, m.payload_len);
        l->payload[m.payload_len] = '\0';
        l->len = m.payload_len;
        l->retain = m.retain;
        broker_publish(topic, (const char *)m.payload, m.payload_len, m.retain);
        break;
    }
    case MQTT_SUBSCRIBE: {
        const uint8_t *p = pk->body;
        const uint16_t id = (uint16_t)((p[0] << 8) | p[1]);
        p += 2;
        if (broker.version == MQTT_V5) p += 1 + p[0];   // properties
        uint8_t codes[4];
        size_t n = 0;
        char filters[4][160];
        while (p < pk->body + pk->body_len) {
            read_str(&p, filters[n], sizeof(filters[n]));
            const uint8_t q = *p++;
            if (broker.deny[0] && !strcmp(filters[n], broker.deny)) {
                codes[n++] = broker.version == MQTT_V5 ? 0x87 : 0x80;   // not authorised / failure
                continue;
            }
            // Replace an identical filter, as the spec says.
            size_t k = 0;
            while (k < broker.n_subs && strcmp(broker.subs[k], filters[n])) k++;
            if (k == broker.n_subs) broker.n_subs++;
            snprintf(broker.subs[k], sizeof(broker.subs[k]), "%s", filters[n]);
            broker.sub_qos[k] = q;
            codes[n++] = q;
        }
        if (broker.version == MQTT_V5) {
            uint8_t ack[9] = { 0x90, (uint8_t)(3 + n), (uint8_t)(id >> 8), (uint8_t)id, 0x00 };
            memcpy(ack + 5, codes, n);
            emit(ack, 5 + n);
        } else {
            uint8_t ack[8] = { 0x90, (uint8_t)(2 + n), (uint8_t)(id >> 8), (uint8_t)id };
            memcpy(ack + 4, codes, n);
            emit(ack, 4 + n);
        }
        // Retained messages matching a new subscription, RETAIN 1.
        for (size_t f = 0; f < n; f++)
            for (size_t i = 0; codes[f] < 0x80 && i < broker.n_retained; i++)
                if (mqtt_topic_matches(filters[f], broker.retained[i].topic, strlen(broker.retained[i].topic)))
                    emit_publish(broker.retained[i].topic, broker.retained[i].payload, broker.retained[i].len, true,
                                 codes[f]);
        break;
    }
    case MQTT_PUBACK:
        break;
    case MQTT_PINGREQ:
        broker.pings++;
        if (broker.answer_pings) {
            const uint8_t r[2] = { 0xD0, 0x00 };
            emit(r, 2);
        }
        break;
    case MQTT_DISCONNECT:
        broker.disconnects++;
        broker_drop(false);
        break;
    default:
        CHECK(!"unexpected packet from the client");
    }
}

static void broker_feed(void)
{
    for (;;) {
        MqttPacket pk;
        if (mqtt_parse(broker.in, broker.in_len, sizeof(broker.in), &pk) != MQTT_OK) return;
        const size_t size = pk.size;
        broker_handle(&pk);
        if (!broker.open) return;
        memmove(broker.in, broker.in + size, broker.in_len - size);
        broker.in_len -= size;
    }
}

// ---------------------------------------------------------------- the client's stream

static int io_open(void *ctx, const char *host, uint16_t port, int timeout_ms, char *err, size_t err_size)
{
    (void)ctx;
    (void)timeout_ms;
    CHECK(!strcmp(host, "broker.lan") || !strcmp(host, "other.lan"));
    CHECK(port == 1883);
    if (!broker.up) {
        snprintf(err, err_size, "connection refused");
        return -1;
    }
    broker.open = true;
    broker.in_len = broker.out_len = 0;
    return 0;
}

static int io_write(void *ctx, const void *buf, size_t len, int timeout_ms)
{
    (void)ctx;
    (void)timeout_ms;
    if (!broker.open) return -1;
    REQUIRE(broker.in_len + len <= sizeof(broker.in));
    memcpy(broker.in + broker.in_len, buf, len);
    broker.in_len += len;
    broker_feed();
    return 0;
}

static long io_read(void *ctx, void *buf, size_t len, int timeout_ms)
{
    (void)ctx;
    if (!broker.open) return -1;
    if (broker.out_len == 0) {
        now += (uint64_t)(timeout_ms > 0 ? timeout_ms : 0);   // waited that long
        return 0;
    }
    const size_t n = broker.out_len < len ? broker.out_len : len;
    memcpy(buf, broker.out, n);
    memmove(broker.out, broker.out + n, broker.out_len - n);
    broker.out_len -= n;
    return (long)n;
}

static void io_close(void *ctx)
{
    (void)ctx;
    if (broker.open) broker_drop(true);
}

// ---------------------------------------------------------------- the host

static SyncEngine engine;

static struct {
    unsigned states, activities, orders, reports, conf_changes;
    SyncIntent last_intent;
    char last_entity[64];
    bool last_retained;
    bool async;               // order() takes the order (returns false)
    uint32_t taken_id;
    SyncReason answer;        // the outcome order() gives
    bool read_only;
    SyncConf saved;
    unsigned profiles_asked;
    unsigned integrations;    // integration() calls
    char integration[SYNC_INTEGRATION_MAX + 1];
    size_t integration_len;
    unsigned refused_logs;    // "the broker refused <base>/integration" lines
    unsigned systems;         // system() calls
    uint64_t posix;           // the wall clock (0: 1791000000)
    SyncDevice device;        // what device() gives
    unsigned devices;         // device() calls
    SyncNotice notice;        // what state() finds, as the agent's sync_watch would (event_type NULL: nothing)
} host;

static size_t h_state(void *ctx, char *out, size_t cap)
{
    (void)ctx;
    host.states++;
    if (host.notice.event_type) sync_engine_notices(&engine, &host.notice, 1);
    return (size_t)snprintf(out, cap, "{\"schema\":1,\"n\":%u}", host.states);
}

static size_t h_activity(void *ctx, char *out, size_t cap)
{
    (void)ctx;
    host.activities++;
    return (size_t)snprintf(out, cap, "{\"total_min\":%u}", host.activities);
}

static bool h_order(void *ctx, uint32_t id, const SyncIntent *in, const char *entity, const char *payload, bool retained,
                    SyncOutcome *out)
{
    (void)ctx;
    (void)payload;
    host.orders++;
    host.last_intent = *in;
    snprintf(host.last_entity, sizeof(host.last_entity), "%s", entity);
    host.last_retained = retained;
    if (host.async) {
        host.taken_id = id;
        return false;
    }
    out->applied = host.answer == SyncReason_None;
    out->reason = host.answer;
    return true;
}

static size_t h_report(void *ctx, char *out, size_t cap)
{
    (void)ctx;
    host.reports++;
    return (size_t)snprintf(out, cap, "=== PlayGuard diagnostic ===\n");
}

static size_t h_profiles(void *ctx, char (*names)[SYNC_PROFILE_MAX], size_t max)
{
    (void)ctx;
    host.profiles_asked++;
    REQUIRE(max >= 2);
    strcpy(names[0], "School week");
    strcpy(names[1], "Holidays");
    return 2;
}

static void h_conf_changed(void *ctx, const SyncConf *conf)
{
    (void)ctx;
    host.conf_changes++;
    host.saved = *conf;
}

static bool h_read_only(void *ctx)
{
    (void)ctx;
    return host.read_only;
}

static uint64_t h_now_posix(void *ctx)
{
    (void)ctx;
    return host.posix ? host.posix : 1791000000;
}

static void h_log(void *ctx, const char *line)
{
    (void)ctx;
    if (strstr(line, "refused playguard/a1b2c3d4/integration, the integration's version stays unknown"))
        host.refused_logs++;
}

static size_t h_system(void *ctx, char *out, size_t cap)
{
    (void)ctx;
    host.systems++;
    return (size_t)snprintf(out, cap, "{\"schema\":1,\"n\":%u}", host.systems);
}

static void h_device(void *ctx, SyncDevice *out)
{
    (void)ctx;
    host.devices++;
    *out = host.device;
}

static void h_integration(void *ctx, const char *doc, size_t len)
{
    (void)ctx;
    REQUIRE(len <= SYNC_INTEGRATION_MAX);
    host.integrations++;
    memcpy(host.integration, doc, len);
    host.integration[len] = '\0';
    host.integration_len = len;
}

// ---------------------------------------------------------------- helpers

static SyncConf conf;

static void setup(bool timer_writes, SyncPolicy policy)
{
    broker_reset();
    memset(&host, 0, sizeof(host));
    sync_conf_defaults(&conf);
    conf.enabled = true;
    strcpy(conf.host, "broker.lan");
    strcpy(conf.username, "parent");
    strcpy(conf.password, "secret");
    strcpy(conf.console_id, "a1b2c3d4");
    strcpy(conf.console_name, "Salon");
    conf.remote_timer_writes = timer_writes;
    conf.policy = policy;
    SyncIo io = { NULL, io_open, io_write, io_read, io_close };
    SyncHost h = { NULL,           h_state,     h_activity,  h_order, h_report,      h_profiles,
                   h_conf_changed, h_read_only, h_now_posix, h_log,   h_integration, h_system,
                   h_device };
    sync_engine_init(&engine, &conf, io, &h, clock_ms, "app", "1.1.0");
}

static void turns(int n)
{
    for (int i = 0; i < n; i++) {
        const int sleep = sync_engine_step(&engine, 50);
        now += (uint64_t)sleep;
    }
}

static const Message *last_on(const char *topic)
{
    for (size_t i = broker.n_log; i-- > 0;)
        if (!strcmp(broker.log[i].topic, topic)) return &broker.log[i];
    return NULL;
}

static size_t count_on(const char *topic)
{
    size_t n = 0;
    for (size_t i = 0; i < broker.n_log; i++) n += !strcmp(broker.log[i].topic, topic);
    return n;
}

static void ha_order(const char *entity, const char *payload)
{
    char topic[160];
    snprintf(topic, sizeof(topic), "playguard/a1b2c3d4/%s/set", entity);
    broker_publish(topic, payload, strlen(payload), true);
}

static bool retained_order(const char *entity)
{
    char topic[160];
    snprintf(topic, sizeof(topic), "playguard/a1b2c3d4/%s/set", entity);
    return find_retained(topic) != NULL;
}

static const char *last_event(void)
{
    const Message *m = last_on("playguard/a1b2c3d4/event");
    return m ? m->payload : "";
}

// ---------------------------------------------------------------- tests

static void test_connect_and_publish(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    CHECK(sync_engine_online(&engine) && engine.status.state == SyncLink_Online);
    CHECK(!strcmp(broker.client_id, "pg-a1b2c3d4-app"));
    CHECK(!strcmp(broker.username, "parent") && !strcmp(broker.password, "secret"));
    CHECK(broker.has_will && !strcmp(broker.will.topic, "playguard/a1b2c3d4/availability") &&
           !strcmp(broker.will.payload, "offline") && broker.will.retain);
    const Message *avail = find_retained("playguard/a1b2c3d4/availability");
    CHECK(avail && !strcmp(avail->payload, "online"));
    CHECK(broker.n_subs == 3 && !strcmp(broker.subs[0], "playguard/a1b2c3d4/+/set") && broker.sub_qos[0] == 1 &&
           !strcmp(broker.subs[1], "homeassistant/status") && !strcmp(broker.subs[2], "playguard/a1b2c3d4/integration") &&
           broker.sub_qos[2] == 1);
    const Message *disc = find_retained("homeassistant/device/playguard_a1b2c3d4/config");
    CHECK(disc && strstr(disc->payload, "\"cmps\":{") && strstr(disc->payload, "\"limit_mon\":{\"p\":\"number\"}") &&
          !strstr(disc->payload, "limit_mon/set"));
    CHECK(host.profiles_asked >= 1);
    const Message *st = find_retained("playguard/a1b2c3d4/state");
    CHECK(st && strstr(st->payload, "\"schema\":1"));
    // The system document with it, retained.
    const Message *sy = find_retained("playguard/a1b2c3d4/system");
    CHECK(sy && sy->retain && strstr(sy->payload, "\"schema\":1") && host.systems >= 1);
    CHECK(find_retained("playguard/a1b2c3d4/activity"));

    // The state again after poll_s, not before.
    const unsigned states = host.states, systems = host.systems;
    now += 10000;
    turns(1);
    CHECK(host.states == states && host.systems == systems);
    now += 25000;
    turns(1);
    CHECK(host.states == states + 1 && host.systems == systems + 1);
    CHECK(strstr(find_retained("playguard/a1b2c3d4/system")->payload, "\"n\":") != NULL);

    // Today's activity: every minute, or at once when the host says it changed.
    const size_t acts = count_on("playguard/a1b2c3d4/activity");
    turns(1);
    CHECK(count_on("playguard/a1b2c3d4/activity") == acts);
    sync_engine_activity_changed(&engine);
    turns(1);
    CHECK(count_on("playguard/a1b2c3d4/activity") == acts + 1);
}

// What the host's sync_watch finds while building the state: on the event
// topic, not retained, after that state, once.
static void test_notices(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    const size_t events = count_on("playguard/a1b2c3d4/event");
    host.notice = (SyncNotice){ "time_warning", 0x0100000000010000ULL, 10, NULL };
    sync_engine_state_changed(&engine);
    turns(1);
    host.notice.event_type = NULL;
    CHECK(count_on("playguard/a1b2c3d4/event") == events + 1);
    const Message *m = last_on("playguard/a1b2c3d4/event");
    CHECK(m && !m->retain && strstr(m->payload, "\"event_type\":\"time_warning\"") &&
          strstr(m->payload, "\"source\":\"app\",\"ts\":1791000000") &&
          strstr(m->payload, "\"app_id\":\"0100000000010000\",\"minutes_left\":10"));
    CHECK(m > last_on("playguard/a1b2c3d4/state"));   // after the state that found it
    sync_engine_state_changed(&engine);
    turns(1);
    CHECK(count_on("playguard/a1b2c3d4/event") == events + 1);
    // The discovery's event entity knows these events.
    const Message *disc = find_retained("homeassistant/device/playguard_a1b2c3d4/config");
    CHECK(disc && strstr(disc->payload, "\"command_waiting\",\"play_started\",\"play_stopped\",\"time_warning\","
                                        "\"play_blocked\""));
}

static void test_orders_applied_and_refused(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    // Applied: an event, the retained order cleared, the state again.
    const unsigned states = host.states;
    ha_order("vr_restricted", "ON");
    turns(2);
    CHECK(host.orders == 1 && host.last_intent.kind == SyncIntent_Vr && host.last_intent.on);
    CHECK(strstr(last_event(), "\"event_type\":\"command_applied\"") && strstr(last_event(), "\"entity\":\"vr_restricted\""));
    CHECK(!retained_order("vr_restricted"));
    now += 400;
    turns(1);
    CHECK(host.states > states);
    CHECK(!strcmp(engine.status.last_result, "vr_restricted: applied"));

    // Out of range: refused without asking the host.
    ha_order("restriction_level", "adult");
    turns(2);
    CHECK(host.orders == 1 && strstr(last_event(), "command_rejected") && strstr(last_event(), "\"reason\":\"invalid\""));
    CHECK(!retained_order("restriction_level"));

    // A play-timer order while remote_timer_writes is off.
    ha_order("limit_mon", "90");
    now += 2000;
    turns(2);
    CHECK(host.orders == 1 && strstr(last_event(), "timer_writes_disabled") && !retained_order("limit_mon"));

    // Refused by the host (the console said no).
    host.answer = SyncReason_NotCustom;
    ha_order("sns_post_restricted", "OFF");
    turns(2);
    CHECK(host.orders == 2 && strstr(last_event(), "\"reason\":\"not_custom\"") && !retained_order("sns_post_restricted"));
    CHECK(engine.status.rejected == 3);

    // Read-only: refused before the host.
    host.read_only = true;
    host.answer = SyncReason_None;
    ha_order("vr_restricted", "OFF");
    turns(2);
    CHECK(host.orders == 2 && strstr(last_event(), "read_only"));
    host.read_only = false;

    // An order that was there before the console came back: RETAIN 1.
    sync_engine_stop(&engine);
    ha_order("vr_restricted", "ON");
    now += 10000;
    turns(3);
    CHECK(host.orders == 3 && host.last_retained && !retained_order("vr_restricted"));
}

static void test_policy_off(void)
{
    setup(true, SyncPolicy_Off);
    turns(3);
    ha_order("lock_now", "PRESS");
    turns(2);
    CHECK(host.orders == 0 && strstr(last_event(), "policy_off"));
    // Harmless orders still work.
    const size_t before = count_on("homeassistant/device/playguard_a1b2c3d4/config");
    ha_order("sync_now", "PRESS");
    turns(2);
    CHECK(strstr(last_event(), "command_applied"));
    CHECK(count_on("homeassistant/device/playguard_a1b2c3d4/config") == before + 1);
    ha_order("export_report", "PRESS");
    turns(2);
    CHECK(host.reports == 1 && strstr(last_event(), "command_applied"));
    CHECK(!find_retained("playguard/a1b2c3d4/report"));   // publish_report is off
}

static void test_limits_merged(void)
{
    setup(true, SyncPolicy_Auto);
    turns(3);
    // Two sliders moved together: one console call after the pause.
    ha_order("limit_mon", "90");
    ha_order("limit_tue", "60");
    turns(1);
    CHECK(host.orders == 0);
    ha_order("limit_mon", "100");   // the newest replaces the queued one, and the pause starts again
    now += 1000;
    turns(1);
    CHECK(host.orders == 0);
    now += 600;
    turns(1);
    CHECK(host.orders == 0);
    now += 1000;
    turns(2);
    CHECK(host.orders == 1);
    const SyncIntent *in = &host.last_intent;
    CHECK(in->kind == SyncIntent_LimitsWeek && in->mask == 0x06 && in->days[1] == 100 && in->days[2] == 60);
    CHECK(!retained_order("limit_mon") && !retained_order("limit_tue"));
    // Two events, one per order.
    size_t applied = 0;
    for (size_t i = 0; i < broker.n_log; i++)
        if (!strcmp(broker.log[i].topic, "playguard/a1b2c3d4/event") && strstr(broker.log[i].payload, "command_applied"))
            applied++;
    CHECK(applied == 2);
    // The discovery announces the timer entities and the profiles.
    const Message *disc = find_retained("homeassistant/device/playguard_a1b2c3d4/config");
    CHECK(strstr(disc->payload, "limit_mon") && strstr(disc->payload, "\"ops\":[\"School week\",\"Holidays\"]"));
}

static void test_async_and_waiting(void)
{
    setup(true, SyncPolicy_Ask);
    turns(3);
    host.async = true;
    ha_order("console_lock", "ON");
    turns(2);
    CHECK(host.orders == 1 && host.taken_id != 0 && sync_engine_pending(&engine) == 1);
    CHECK(retained_order("console_lock"));   // nothing reported yet
    SyncOutcome out;
    sync_outcome_init(&out);
    out.applied = true;
    sync_engine_order_done(&engine, host.taken_id, &out);
    CHECK(sync_engine_pending(&engine) == 0 && !retained_order("console_lock"));
    CHECK(strstr(last_event(), "command_applied"));

    // Kept waiting (nobody at the console to confirm): stays on the broker,
    // reported once even when the broker sends it again.
    host.async = false;
    host.answer = SyncReason_Waiting;
    ha_order("stop_today", "PRESS");
    turns(2);
    CHECK(retained_order("stop_today") && strstr(last_event(), "command_waiting"));
    const size_t events = count_on("playguard/a1b2c3d4/event");
    sync_engine_replay_orders(&engine);
    turns(2);
    CHECK(host.orders == 3 && count_on("playguard/a1b2c3d4/event") == events);
    // Confirmed later.
    host.answer = SyncReason_None;
    sync_engine_replay_orders(&engine);
    turns(2);
    CHECK(!retained_order("stop_today") && strstr(last_event(), "command_applied"));
}

// An order stays retained until it is done, so the broker sends it again
// while it is carried out: at a replay (PlayGuard opens or closes beside the
// agent) and at a new connection. Seen on a console: +15 min given twice.
static void test_replay_in_flight(void)
{
    setup(true, SyncPolicy_Auto);
    turns(3);
    host.async = true;
    ha_order("console_lock", "ON");
    turns(2);
    CHECK(host.orders == 1 && sync_engine_pending(&engine) == 1);
    const uint32_t first = host.taken_id;
    sync_engine_replay_orders(&engine);
    turns(3);
    CHECK(host.orders == 1 && sync_engine_pending(&engine) == 1);
    broker_drop(false);
    turns(1);
    now += 6000;
    turns(3);
    CHECK(sync_engine_online(&engine) && host.orders == 1 && sync_engine_pending(&engine) == 1);

    // Another value while it runs is another order, and so is a press that
    // comes live (RETAIN 0) while one with the same payload runs.
    ha_order("console_lock", "OFF");
    turns(2);
    CHECK(host.orders == 2 && !strcmp(host.last_entity, "console_lock") && !host.last_intent.on);
    const uint32_t second = host.taken_id;
    broker_publish("playguard/a1b2c3d4/console_lock/set", "OFF", 3, false);
    turns(2);
    CHECK(host.orders == 3);
    const uint32_t third = host.taken_id;

    SyncOutcome out;
    sync_outcome_init(&out);
    out.applied = true;
    sync_engine_order_done(&engine, first, &out);
    sync_engine_order_done(&engine, second, &out);
    sync_engine_order_done(&engine, third, &out);
    CHECK(sync_engine_pending(&engine) == 0 && !retained_order("console_lock"));
}

// The device is the console as Nintendo names it, its system as its software;
// the discovery follows when the system changes (an update, then the agent
// sees the new one).
static void test_device(void)
{
    setup(false, SyncPolicy_Auto);
    host.device.model = ConsoleModel_Oled;
    host.device.hos_version = (23u << 16) | (0u << 8) | 1u;
    host.device.ams_version = (1u << 16) | (10u << 8) | 1u;
    turns(3);
    const char *topic = "homeassistant/device/playguard_a1b2c3d4/config";
    CHECK(count_on(topic) == 1 && host.devices >= 1);
    const Message *disc = last_on(topic);
    CHECK(strstr(disc->payload, "\"mf\":\"Nintendo\",\"mdl\":\"Nintendo Switch \xe2\x80\x93 OLED Model\","
                                "\"mdl_id\":\"HEG-001\",\"sw\":\"23.0.1 \xc2\xb7 Atmosph\xc3\xa8re 1.10.1\"}"));
    CHECK(strstr(disc->payload, "\"o\":{\"name\":\"PlayGuard\",\"sw\":\"1.1.0\""));
    // The same console at the next state: no new discovery.
    now += (uint64_t)conf.poll_s * 1000ULL;
    turns(2);
    CHECK(count_on(topic) == 1 && host.systems >= 2);
    // Another system: published again.
    host.device.hos_version = (24u << 16);
    now += (uint64_t)conf.poll_s * 1000ULL;
    turns(3);
    CHECK(count_on(topic) == 2 && strstr(last_on(topic)->payload, "\"sw\":\"24.0.0 \xc2\xb7 Atmosph"));
    // Its nickname changed, sync.conf naming none: published again, under it.
    strcpy(engine.conf.console_name, "");
    strcpy(host.device.nickname, "Switch de Léa");
    now += (uint64_t)conf.poll_s * 1000ULL;
    turns(3);
    CHECK(count_on(topic) == 3 && strstr(last_on(topic)->payload, "\"name\":\"Switch de Léa\""));
}

// What is not published leaves nothing on the broker: cleared at each
// connection and when the settings change.
static void test_unpublished_cleared(void)
{
    setup(false, SyncPolicy_Auto);   // the report is off by default
    turns(3);
    const Message *m = last_on("playguard/a1b2c3d4/report");
    CHECK(m && m->retain && m->len == 0);
    CHECK(!last_on("playguard/a1b2c3d4/games") && !last_on("playguard/a1b2c3d4/week"));
    // Play time and the games turned off: their documents go.
    SyncConf next = conf;
    next.publish_activity = next.publish_games = false;
    sync_engine_reconfigure(&engine, &next);
    turns(2);
    static const char *const gone[] = { "activity", "week", "names", "games" };
    for (size_t i = 0; i < sizeof(gone) / sizeof(gone[0]); i++) {
        char topic[96];
        snprintf(topic, sizeof(topic), "playguard/a1b2c3d4/%s", gone[i]);
        m = last_on(topic);
        CHECK(m && m->retain && m->len == 0);
    }
    // No activity published meanwhile.
    const size_t activities = host.activities;
    now += 120000;
    turns(3);
    CHECK(host.activities == activities);
}

static void test_home_assistant_restart_and_discovery_switch(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    const size_t before = count_on("homeassistant/device/playguard_a1b2c3d4/config");
    broker_publish("homeassistant/status", "online", 6, false);
    turns(2);
    CHECK(count_on("homeassistant/device/playguard_a1b2c3d4/config") == before + 1);
    // "offline" changes nothing.
    broker_publish("homeassistant/status", "offline", 7, false);
    turns(2);
    CHECK(count_on("homeassistant/device/playguard_a1b2c3d4/config") == before + 1);

    // The integration takes over: the native discovery is cleared and saved off.
    ha_order("discovery", "off");
    turns(2);
    CHECK(!find_retained("homeassistant/device/playguard_a1b2c3d4/config"));
    CHECK(host.conf_changes == 1 && !host.saved.ha_discovery);
    broker_publish("homeassistant/status", "online", 6, false);
    turns(2);
    CHECK(!find_retained("homeassistant/device/playguard_a1b2c3d4/config"));
    ha_order("discovery", "on");
    turns(2);
    CHECK(find_retained("homeassistant/device/playguard_a1b2c3d4/config") && host.saved.ha_discovery);
}

// The integration's version: handed over as it came, at the subscription
// (retained) and when it changes; cleared; too large, dropped. Optional: a
// broker refusing the topic keeps the link up.
static void test_integration_version(void)
{
    const char *topic = "playguard/a1b2c3d4/integration";
    const char *doc = "{\"schema\":1,\"version\":\"0.2.0\",\"homeassistant\":\"2026.10.1\"}";
    setup(false, SyncPolicy_Auto);
    broker_publish(topic, doc, strlen(doc), true);
    turns(3);
    CHECK(sync_engine_online(&engine));
    CHECK(host.integrations == 1 && host.integration_len == strlen(doc) && !strcmp(host.integration, doc));
    // Not an order: no event, left on the broker.
    CHECK(!last_on("playguard/a1b2c3d4/event") && find_retained(topic));

    const char *newer = "{\"schema\":1,\"version\":\"0.3.0\"}";
    broker_publish(topic, newer, strlen(newer), true);
    turns(2);
    CHECK(host.integrations == 2 && !strcmp(host.integration, newer));
    broker_publish(topic, "", 0, true);
    turns(2);
    CHECK(host.integrations == 3 && host.integration_len == 0);
    char big[SYNC_INTEGRATION_MAX + 2];
    memset(big, 'x', sizeof(big) - 1);
    big[sizeof(big) - 1] = '\0';
    broker_publish(topic, big, sizeof(big) - 1, true);
    turns(2);
    CHECK(host.integrations == 3 && sync_engine_online(&engine));
    // <base>/integration/set is an order like the others: no entity of that name.
    ha_order("integration", "0.2.0");
    turns(2);
    CHECK(host.integrations == 3 && strstr(last_event(), "command_rejected") && !retained_order("integration"));

    // A broker whose access list leaves it out, in 3.1.1 and 5.0: still online.
    for (uint8_t v = 4; v <= 5; v++) {
        setup(false, SyncPolicy_Auto);
        broker.min_version = broker.max_version = v;
        snprintf(broker.deny, sizeof(broker.deny), "%s", topic);
        broker_publish(topic, doc, strlen(doc), true);
        turns(3);
        CHECK(sync_engine_online(&engine) && broker.n_subs == 2 && host.integrations == 0);
        CHECK(host.refused_logs == 1);
    }
}

// Another discovery prefix: the device under it, and Home Assistant's
// restart still seen on homeassistant/status (its birth topic does not follow
// the prefix).
static void test_custom_discovery_prefix(void)
{
    setup(false, SyncPolicy_Auto);
    strcpy(conf.discovery_prefix, "ha");
    SyncIo io = { NULL, io_open, io_write, io_read, io_close };
    SyncHost h = { NULL,           h_state,     h_activity,  h_order, h_report,      h_profiles,
                   h_conf_changed, h_read_only, h_now_posix, h_log,   h_integration, h_system,
                   h_device };
    sync_engine_init(&engine, &conf, io, &h, clock_ms, "app", "1.1.0");
    turns(3);
    CHECK(sync_engine_online(&engine) && find_retained("ha/device/playguard_a1b2c3d4/config"));
    CHECK(broker.n_subs == 3 && !strcmp(broker.subs[1], "homeassistant/status"));
    const size_t before = count_on("ha/device/playguard_a1b2c3d4/config");
    broker_publish("homeassistant/status", "online", 6, false);
    turns(2);
    CHECK(count_on("ha/device/playguard_a1b2c3d4/config") == before + 1);
    broker_publish("ha/status", "online", 6, false);
    turns(2);
    CHECK(count_on("ha/device/playguard_a1b2c3d4/config") == before + 1);
}

static void test_connection_failures(void)
{
    // Refused credentials: the reason, then a pause before the next attempt.
    setup(false, SyncPolicy_Auto);
    broker.refuse_code = 4;
    turns(1);
    CHECK(!sync_engine_online(&engine) && engine.status.state == SyncLink_Waiting);
    CHECK(strstr(engine.status.error, "bad user name or password"));
    const unsigned attempts = broker.connects;
    sync_engine_step(&engine, 50);
    CHECK(broker.connects == attempts);   // not before the pause
    broker.refuse_code = 0;
    now += 5000;
    turns(2);
    CHECK(sync_engine_online(&engine));

    // The broker drops the connection: its last will says offline, then the
    // engine reconnects and says online again.
    broker_drop(true);
    turns(1);
    const Message *avail = find_retained("playguard/a1b2c3d4/availability");
    CHECK(avail && !strcmp(avail->payload, "offline"));
    CHECK(!sync_engine_online(&engine));
    now += 6000;
    turns(3);
    CHECK(sync_engine_online(&engine));
    avail = find_retained("playguard/a1b2c3d4/availability");
    CHECK(!strcmp(avail->payload, "online"));

    // Unreachable: the pause doubles, up to 5 minutes.
    broker_drop(false);
    broker.up = false;
    turns(1);
    for (int i = 0; i < 12; i++) {
        now += 400000;
        turns(1);
    }
    CHECK(engine.backoff_ms == 300000);
    broker.up = true;
    now += 400000;
    turns(3);
    CHECK(sync_engine_online(&engine) && engine.backoff_ms == 0);

    // A clean stop: offline, DISCONNECT, no last will.
    const unsigned disconnects = broker.disconnects;
    sync_engine_stop(&engine);
    CHECK(broker.disconnects == disconnects + 1);
    avail = find_retained("playguard/a1b2c3d4/availability");
    CHECK(!strcmp(avail->payload, "offline"));
}

static void test_keepalive(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    const unsigned pings = broker.pings;
    // Nothing sent for 3/4 of the keep-alive: a PINGREQ.
    for (int i = 0; i < 30 && broker.pings == pings; i++) {
        now += 1000;
        sync_engine_step(&engine, 0);
    }
    CHECK(broker.pings == pings + 1 && sync_engine_online(&engine));
    // The broker stops answering: closed after the keep-alive.
    broker.answer_pings = false;
    for (int i = 0; i < 90 && sync_engine_online(&engine); i++) {
        now += 1000;
        sync_engine_step(&engine, 0);
    }
    CHECK(!sync_engine_online(&engine) && strstr(engine.status.error, "stopped answering"));

    // The broker says nothing while this client keeps publishing (a
    // connection dropped while the console slept takes what is written): a
    // PINGREQ all the same, and the connection closed when it goes unanswered.
    setup(false, SyncPolicy_Auto);
    turns(3);
    const unsigned before = broker.pings;
    for (int i = 0; i < 30 && broker.pings == before; i++) {
        now += 1000;
        if (i % 5 == 0) CHECK(sync_engine_publish(&engine, "names", "{}", 2, true) == 0);
        sync_engine_step(&engine, 0);
    }
    CHECK(broker.pings == before + 1 && sync_engine_online(&engine));
    broker.answer_pings = false;
    for (int i = 0; i < 90 && sync_engine_online(&engine); i++) {
        now += 1000;
        if (i % 5 == 0) sync_engine_publish(&engine, "names", "{}", 2, true);
        sync_engine_step(&engine, 0);
    }
    CHECK(!sync_engine_online(&engine) && strstr(engine.status.error, "stopped answering"));
}

// The console slept: its clock moved on by more than the broker keeps a
// silent connection. The connection is closed without a word before anything
// is written to it, and the next step connects again.
static void test_woke(void)
{
    setup(false, SyncPolicy_Auto);
    host.posix = 1791000000;
    turns(3);
    CHECK(sync_engine_online(&engine) && sync_engine_publish(&engine, "names", "{}", 2, true) == 0);
    const unsigned connects = broker.connects, disconnects = broker.disconnects;
    const size_t sent = broker.n_log;
    // A clock that moved on within the keep-alive: nothing to do.
    host.posix += 40;
    CHECK(sync_engine_publish(&engine, "names", "{}", 2, true) == 0 && sync_engine_online(&engine));
    // Ten minutes asleep (the tick barely moved).
    host.posix += 600;
    now += 100;
    CHECK(sync_engine_publish(&engine, "names", "{}", 2, true) < 0);
    CHECK(!sync_engine_online(&engine) && strstr(engine.status.error, "slept"));
    CHECK(broker.n_log == sent + 1 && broker.disconnects == disconnects);   // not written, no DISCONNECT
    turns(2);
    CHECK(sync_engine_online(&engine) && broker.connects == connects + 1);
    const Message *avail = find_retained("playguard/a1b2c3d4/availability");
    CHECK(avail && !strcmp(avail->payload, "online"));
    // Asleep while offline: nothing to close, the step connects as before.
    sync_engine_stop(&engine);
    host.posix += 3600;
    turns(2);
    CHECK(sync_engine_online(&engine) && broker.connects == connects + 2);
}

static void test_oversized_and_reconfigure(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    // A retained order larger than the receive buffer is dropped, the
    // connection stays.
    static char big[SYNC_RX_SIZE + 512];
    memset(big, 'x', sizeof(big) - 1);
    big[sizeof(big) - 1] = '\0';
    ha_order("vr_restricted", big);
    turns(3);
    CHECK(sync_engine_online(&engine) && engine.mqtt.dropped == 1 && host.orders == 0);
    ha_order("vr_restricted", "ON");
    turns(2);
    CHECK(host.orders == 1);

    // The same sync.conf on an emuMMC: a device of its own (its id's first
    // digit's top bit flipped), so the console's device is cleared and left
    // offline under its own id.
    SyncConf emu = conf;
    emu.emummc = true;
    unsigned connects = broker.connects;
    sync_engine_reconfigure(&engine, &emu);
    turns(3);
    CHECK(sync_engine_online(&engine) && broker.connects == connects + 1);
    CHECK(!strcmp(broker.client_id, "pg-21b2c3d4-app"));
    const Message *cleared = find_retained("homeassistant/device/playguard_a1b2c3d4/config");
    CHECK(!cleared || !cleared->payload[0]);
    CHECK(find_retained("homeassistant/device/playguard_21b2c3d4/config"));
    const Message *avail = find_retained("playguard/21b2c3d4/availability");
    CHECK(avail && !strcmp(avail->payload, "online"));
    avail = find_retained("playguard/a1b2c3d4/availability");
    CHECK(avail && !strcmp(avail->payload, "offline"));
    CHECK(find_retained("playguard/21b2c3d4/state"));
    sync_engine_reconfigure(&engine, &conf);
    turns(3);
    CHECK(sync_engine_online(&engine) && !strcmp(broker.client_id, "pg-a1b2c3d4-app"));

    // Another broker: reconnects there.
    SyncConf other = conf;
    strcpy(other.host, "other.lan");
    connects = broker.connects;
    sync_engine_reconfigure(&engine, &other);
    turns(3);
    CHECK(sync_engine_online(&engine) && broker.connects == connects + 1);
    // Turned off: disconnects and stays off.
    other.enabled = false;
    sync_engine_reconfigure(&engine, &other);
    turns(2);
    CHECK(!sync_engine_online(&engine) && engine.status.state == SyncLink_Off);
    // Not configured: off, with the reason.
    other.enabled = true;
    other.host[0] = '\0';
    sync_engine_reconfigure(&engine, &other);
    turns(1);
    CHECK(engine.status.state == SyncLink_Off && strstr(engine.status.error, "host"));
}

// MQTT 5.0 by default; "auto" falls back to 3.1.1 when the broker refuses
// 5.0 (answering in 3.1.1, or closing the stream), and keeps it; a version
// asked for is not changed.
static void test_mqtt_versions(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    CHECK(sync_engine_online(&engine) && broker.version == MQTT_V5 && engine.status.protocol == MQTT_V5);
    CHECK(broker.client_max_packet == SYNC_RX_SIZE);
    CHECK(find_retained("playguard/a1b2c3d4/state") && find_retained("homeassistant/device/playguard_a1b2c3d4/config"));
    // Orders and their answers work the same.
    ha_order("vr_restricted", "ON");
    turns(2);
    CHECK(host.orders == 1 && strstr(last_event(), "command_applied") && !retained_order("vr_restricted"));

    // A 3.1.1 broker: refused, then 3.1.1 at once.
    for (int closes = 0; closes < 2; closes++) {
        setup(false, SyncPolicy_Auto);
        broker.max_version = MQTT_V311;
        broker.close_on_unknown = closes;
        turns(3);
        CHECK(sync_engine_online(&engine) && broker.version == MQTT_V311 && engine.status.protocol == MQTT_V311);
        CHECK(broker.refused_versions == 1 && broker.connects == 1);
        // Reconnecting: straight to 3.1.1.
        broker_drop(true);
        turns(1);
        now += 6000;
        turns(3);
        CHECK(sync_engine_online(&engine) && broker.refused_versions == 1 && broker.version == MQTT_V311);
    }

    // 5.0 asked for: no fallback, the reason said.
    setup(false, SyncPolicy_Auto);
    broker.max_version = MQTT_V311;
    conf.mqtt_version = SyncMqtt_V5;
    sync_engine_reconfigure(&engine, &conf);
    turns(1);
    CHECK(!sync_engine_online(&engine) && strstr(engine.status.error, "does not speak this MQTT version"));
    CHECK(broker.refused_versions == 1 && broker.connects == 0);
    // "auto" again: the fallback.
    conf.mqtt_version = SyncMqtt_Auto;
    sync_engine_reconfigure(&engine, &conf);
    turns(3);
    CHECK(sync_engine_online(&engine) && broker.version == MQTT_V311);

    // A 5.0-only broker: 3.1.1 asked for is refused; "auto" uses 5.0 again
    // once the settings change (what worked is forgotten).
    setup(false, SyncPolicy_Auto);
    broker.min_version = MQTT_V5;
    conf.mqtt_version = SyncMqtt_V311;
    sync_engine_reconfigure(&engine, &conf);
    turns(1);
    CHECK(!sync_engine_online(&engine) && strstr(engine.status.error, "does not speak this MQTT version"));
    conf.mqtt_version = SyncMqtt_Auto;
    sync_engine_reconfigure(&engine, &conf);
    turns(3);
    CHECK(sync_engine_online(&engine) && broker.version == MQTT_V5 && engine.proto_ok == MQTT_V5);
}

// What a 5.0 broker says in its CONNACK and DISCONNECT.
static void test_mqtt5_properties(void)
{
    // Its keep-alive replaces the client's.
    setup(false, SyncPolicy_Auto);
    broker.server_keepalive = 10;
    turns(3);
    CHECK(sync_engine_online(&engine) && engine.mqtt.keepalive_s == 10);
    const unsigned pings = broker.pings;
    for (int i = 0; i < 9 && broker.pings == pings; i++) {
        now += 1000;
        sync_engine_step(&engine, 0);
    }
    CHECK(broker.pings == pings + 1);

    // A refusal with its reason string.
    setup(false, SyncPolicy_Auto);
    broker.refuse_code = 5;
    strcpy(broker.reason, "acl denies pg-a1b2c3d4-app");
    turns(1);
    CHECK(!sync_engine_online(&engine) && strstr(engine.status.error, "not authorised (acl denies pg-a1b2c3d4-app)"));
    CHECK(broker.refused_versions == 0);   // not a version problem: no 3.1.1 attempt

    // No retained messages: the link cannot work.
    setup(false, SyncPolicy_Auto);
    broker.no_retain = true;
    turns(1);
    CHECK(!sync_engine_online(&engine) && strstr(engine.status.error, "retained messages"));

    // A small maximum packet size: the discovery (about 13 KiB) is not sent,
    // the rest is.
    setup(false, SyncPolicy_Auto);
    broker.max_packet = 2048;
    turns(3);
    CHECK(sync_engine_online(&engine) && engine.mqtt.too_large >= 1);
    CHECK(!find_retained("homeassistant/device/playguard_a1b2c3d4/config") && find_retained("playguard/a1b2c3d4/state"));

    // The broker ends the session, saying why.
    setup(false, SyncPolicy_Auto);
    turns(3);
    broker_kick(0x8E, "taken");
    turns(1);
    CHECK(!sync_engine_online(&engine));
    CHECK(strstr(engine.status.error, "session taken over by another client (taken)"));
}

// A file published a piece at a time (the agent's icon of the game played).
typedef struct {
    const char *data;
    size_t      len, at, piece;
} Reader;

static size_t read_piece(void *ctx, void *buf, size_t cap)
{
    Reader *r = (Reader *)ctx;
    size_t n = r->len - r->at;
    if (n > cap) n = cap;
    if (n > r->piece) n = r->piece;
    memcpy(buf, r->data + r->at, n);
    r->at += n;
    return n;
}

static void test_publish_stream(void)
{
    setup(false, SyncPolicy_Auto);
    turns(3);
    CHECK(sync_engine_online(&engine));
    static char icon[20000];
    for (size_t i = 0; i < sizeof(icon); i++) icon[i] = (char)(i * 7 + 1);
    const uint32_t publishes = sync_engine_status(&engine)->publishes;
    Reader r = { icon, sizeof(icon), 0, 1500 };
    CHECK(sync_engine_publish_stream(&engine, "icon/0100000000010000", sizeof(icon), true, read_piece, &r) == 0);
    const Message *m = last_on("playguard/a1b2c3d4/icon/0100000000010000");
    CHECK(m && m->retain && m->len == sizeof(icon) && !memcmp(m->payload, icon, sizeof(icon)));
    CHECK(sync_engine_status(&engine)->publishes == publishes + 1 && sync_engine_online(&engine));
    // A file that comes short: half a packet went out, so the connection is
    // closed, and the next step connects again; nothing was published.
    Reader cut = { icon, 5000, 0, 1500 };
    CHECK(sync_engine_publish_stream(&engine, "icon/0100000000020000", 6000, true, read_piece, &cut) < 0);
    CHECK(!sync_engine_online(&engine));
    CHECK(strstr(sync_engine_status(&engine)->error, "cut short") != NULL);
    turns(3);
    CHECK(sync_engine_online(&engine));
    CHECK(!last_on("playguard/a1b2c3d4/icon/0100000000020000"));
    // Offline: refused at once.
    sync_engine_stop(&engine);
    Reader none = { icon, 10, 0, 10 };
    CHECK(sync_engine_publish_stream(&engine, "icon/0100000000030000", 10, true, read_piece, &none) < 0 && none.at == 0);
}

int main(void)
{
    static Message log[MAX_LOG];
    broker.log = log;
    test_connect_and_publish();
    test_notices();
    test_orders_applied_and_refused();
    test_policy_off();
    test_limits_merged();
    test_async_and_waiting();
    test_replay_in_flight();
    test_device();
    test_unpublished_cleared();
    test_home_assistant_restart_and_discovery_switch();
    test_custom_discovery_prefix();
    test_integration_version();
    test_connection_failures();
    test_keepalive();
    test_woke();
    test_oversized_and_reconfigure();
    test_mqtt_versions();
    test_mqtt5_properties();
    test_publish_stream();
    return CHECK_DONE("sync_engine: all tests passed");
}

// Host tests for the remote link's console-free parts (source/sync/): the
// JSON writer, the MQTT 3.1.1 codec, sync.conf, the order parser and its
// agreement with the entity table, the Home Assistant discovery and the
// state / activity / event documents. Writes the discovery and a state to
// files that `make check` validates as JSON (tools/check_sync_json.py).
#include "check.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_packet.h"
#include "sync_apply.h"
#include "sync_conf.h"
#include "sync_discovery.h"
#include "sync_engine.h"   // SYNC_SCRATCH_SIZE: what the discovery must fit in
#include "sync_entities.h"
#include "sync_games.h"
#include "sync_system.h"
#include "sync_exec.h"
#include "sync_json.h"
#include "sync_state.h"

static const char *out_dir = ".";

static bool contains(const char *hay, const char *needle)
{
    return strstr(hay, needle) != NULL;
}

static void save(const char *name, const char *text)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", out_dir, name);
    // Owner-writable only (fopen would create it 0666 under the umask).
    const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    REQUIRE(fd >= 0);
    FILE *f = fdopen(fd, "wb");
    REQUIRE(f);
    fputs(text, f);
    fclose(f);
}

// ---------------------------------------------------------------- JSON

static void test_json(void)
{
    char buf[256];
    SyncJson j;
    sync_json_init(&j, buf, sizeof(buf));
    sync_json_obj(&j);
    sync_json_kstr(&j, "a", "x\"y\\z\n\t\x01");
    sync_json_kint(&j, "n", -42);
    sync_json_kbool(&j, "t", true);
    sync_json_knull(&j, "z");
    sync_json_key(&j, "arr");
    sync_json_arr(&j);
    sync_json_int(&j, 1);
    sync_json_obj(&j);
    sync_json_obj_end(&j);
    sync_json_hex64(&j, 0x0100000000010000ULL);
    sync_json_arr_end(&j);
    sync_json_kint_or_null(&j, "maybe", false, 5);
    sync_json_members(&j, "\"m\":1,\"k\":2");
    sync_json_obj_end(&j);
    const size_t n = sync_json_end(&j);
    CHECK(n == strlen(buf));
    CHECK(!strcmp(buf, "{\"a\":\"x\\\"y\\\\z\\n\\t\\u0001\",\"n\":-42,\"t\":true,\"z\":null,"
                        "\"arr\":[1,{},\"0100000000010000\"],\"maybe\":null,\"m\":1,\"k\":2}"));

    // Too small: 0, and the buffer stays a valid C string.
    char small[8];
    sync_json_init(&j, small, sizeof(small));
    sync_json_obj(&j);
    sync_json_kstr(&j, "long", "value");
    sync_json_obj_end(&j);
    CHECK(sync_json_end(&j) == 0);
    CHECK(strlen(small) < sizeof(small));

    // Unbalanced: 0.
    sync_json_init(&j, buf, sizeof(buf));
    sync_json_obj(&j);
    CHECK(sync_json_end(&j) == 0);
    sync_json_init(&j, buf, sizeof(buf));
    sync_json_obj_end(&j);
    CHECK(sync_json_end(&j) == 0);
}

// ---------------------------------------------------------------- MQTT

static void test_connect_encoding(void)
{
    uint8_t buf[256];
    MqttConnect c = { 0 };
    c.client_id = "pg-a1b2c3d4-app";
    c.username = "user";
    c.password = "pass";
    c.will_topic = "playguard/a1b2c3d4/availability";
    c.will_payload = "offline";
    c.will_len = 7;
    c.will_retain = true;
    c.keepalive_s = 30;
    c.clean_session = true;
    const size_t n = mqtt_encode_connect(buf, sizeof(buf), &c);
    CHECK(n > 0);
    CHECK(buf[0] == 0x10);
    MqttPacket p;
    CHECK(mqtt_parse(buf, n, sizeof(buf), &p) == MQTT_OK);
    CHECK(p.type == MQTT_CONNECT && p.size == n);
    const uint8_t *b = p.body;
    CHECK(b[0] == 0 && b[1] == 4 && !memcmp(b + 2, "MQTT", 4) && b[6] == 4);
    // user, password, will retain, will QoS 0, will, clean session
    CHECK(b[7] == (0x80 | 0x40 | 0x20 | 0x04 | 0x02));
    CHECK(b[8] == 0 && b[9] == 30);
    size_t at = 10;
    CHECK(b[at] == 0 && b[at + 1] == strlen(c.client_id) && !memcmp(b + at + 2, c.client_id, strlen(c.client_id)));
    at += 2 + strlen(c.client_id);
    CHECK(b[at + 1] == strlen(c.will_topic));
    at += 2 + strlen(c.will_topic);
    CHECK(b[at + 1] == 7 && !memcmp(b + at + 2, "offline", 7));
    at += 9;
    CHECK(b[at + 1] == 4 && !memcmp(b + at + 2, "user", 4));
    at += 6;
    CHECK(b[at + 1] == 4 && !memcmp(b + at + 2, "pass", 4));
    at += 6;
    CHECK(at == p.body_len);

    // No user: no password either; buffer too small: 0.
    c.username = NULL;
    const size_t n2 = mqtt_encode_connect(buf, sizeof(buf), &c);
    CHECK(n2 > 0 && (buf[9] & 0xC0) == 0);   // connect flags: no user, no password
    CHECK(mqtt_encode_connect(buf, 10, &c) == 0);
    // A will topic with a wildcard is refused.
    c.will_topic = "a/#";
    CHECK(mqtt_encode_connect(buf, sizeof(buf), &c) == 0);
}

static void test_publish_roundtrip(void)
{
    uint8_t buf[512];
    const char *payload = "{\"x\":1}";
    const size_t h = mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "playguard/id/state", strlen(payload), 0, true, false, 0);
    CHECK(h > 0);
    memcpy(buf + h, payload, strlen(payload));
    MqttPacket p;
    CHECK(mqtt_parse(buf, h + strlen(payload), sizeof(buf), &p) == MQTT_OK);
    MqttPublish m;
    CHECK(mqtt_decode_publish(&p, MQTT_V311, &m) == MQTT_OK);
    CHECK(m.retain && m.qos == 0 && !m.dup);
    CHECK(m.topic_len == strlen("playguard/id/state") && !memcmp(m.topic, "playguard/id/state", m.topic_len));
    CHECK(m.payload_len == strlen(payload) && !memcmp(m.payload, payload, m.payload_len));

    // QoS 1 carries a packet id (and refuses 0); QoS 2 is not sent.
    CHECK(mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "t", 0, 1, false, false, 0) == 0);
    const size_t h1 = mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "t", 0, 1, false, true, 0x1234);
    CHECK(h1 > 0 && (buf[0] & 0x0F) == 0x0A);
    CHECK(mqtt_parse(buf, h1, sizeof(buf), &p) == MQTT_OK && mqtt_decode_publish(&p, MQTT_V311, &m) == MQTT_OK);
    CHECK(m.qos == 1 && m.dup && m.packet_id == 0x1234 && m.payload_len == 0);
    CHECK(mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "t", 0, 2, false, false, 1) == 0);
    // Wildcards and empty topics cannot be published to.
    CHECK(mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "a/+/b", 0, 0, false, false, 0) == 0);
    CHECK(mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "a/#", 0, 0, false, false, 0) == 0);
    CHECK(mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "", 0, 0, false, false, 0) == 0);
    // An empty retained payload (clears a retained message).
    const size_t he = mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V311, "a/b/set", 0, 0, true, false, 0);
    CHECK(he == 2 + 2 + 7 && buf[1] == 9);
}

static void test_remaining_length(void)
{
    // The boundaries of the variable length: 127/128, 16383/16384, 2097151/2097152.
    static uint8_t big[2100000 + 16];
    const size_t sizes[] = { 0, 1, 127 - 4, 128 - 4, 16383 - 4, 16384 - 4, 2097151 - 4, 2097152 - 4 };
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        const size_t h = mqtt_encode_publish_head(big, sizeof(big), MQTT_V311, "ab", sizes[i], 0, false, false, 0);
        CHECK(h > 0);
        const size_t remaining = 2 + 2 + sizes[i];
        const size_t len_bytes = remaining < 128 ? 1 : remaining < 16384 ? 2 : remaining < 2097152 ? 3 : 4;
        CHECK(h == 1 + len_bytes + 4);
        memset(big + h, 'x', sizes[i]);
        MqttPacket p;
        CHECK(mqtt_parse(big, h + sizes[i], sizeof(big), &p) == MQTT_OK);
        CHECK(p.body_len == remaining && p.size == h + sizes[i]);
        // One byte short: incomplete.
        if (h + sizes[i] > 2) CHECK(mqtt_parse(big, h + sizes[i] - 1, sizeof(big), &p) == MQTT_INCOMPLETE);
        // Over the caller's limit: too large (decided from the header alone).
        if (h + sizes[i] > 64) CHECK(mqtt_parse(big, h, 64, &p) == MQTT_TOO_LARGE);
    }
    // A fifth length byte is malformed; reserved types 0 and 15 too.
    const uint8_t five[] = { 0x30, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 };
    MqttPacket p;
    CHECK(mqtt_parse(five, sizeof(five), 1u << 30, &p) == MQTT_MALFORMED);
    const uint8_t zero[] = { 0x00, 0x00 };
    CHECK(mqtt_parse(zero, 2, 64, &p) == MQTT_MALFORMED);
    const uint8_t fifteen[] = { 0xF0, 0x00 };
    CHECK(mqtt_parse(fifteen, 2, 64, &p) == MQTT_MALFORMED);
    // Fewer than two bytes, or a length still running: incomplete.
    CHECK(mqtt_parse(zero, 1, 64, &p) == MQTT_INCOMPLETE);
    const uint8_t running[] = { 0x30, 0x80 };
    CHECK(mqtt_parse(running, 2, 64, &p) == MQTT_INCOMPLETE);
}

static void test_other_packets(void)
{
    uint8_t buf[256];
    const char *topics[] = { "playguard/id/+/set", "homeassistant/status" };
    const uint8_t qos[] = { 1, 0 };
    const size_t n = mqtt_encode_subscribe(buf, sizeof(buf), MQTT_V311, 7, topics, qos, 2);
    CHECK(n > 0 && buf[0] == 0x82);
    MqttPacket p;
    CHECK(mqtt_parse(buf, n, sizeof(buf), &p) == MQTT_OK && p.type == MQTT_SUBSCRIBE);
    CHECK(p.body[0] == 0 && p.body[1] == 7);
    CHECK(p.body[2] == 0 && p.body[3] == strlen(topics[0]));
    CHECK(p.body[4 + strlen(topics[0])] == 1);
    CHECK(mqtt_encode_subscribe(buf, sizeof(buf), MQTT_V311, 0, topics, qos, 2) == 0);   // id 0
    CHECK(mqtt_encode_subscribe(buf, sizeof(buf), MQTT_V311, 1, topics, qos, 0) == 0);   // nothing

    const uint8_t suback[] = { 0x90, 0x04, 0x00, 0x07, 0x01, 0x80 };
    CHECK(mqtt_parse(suback, sizeof(suback), 64, &p) == MQTT_OK);
    uint16_t id = 0;
    uint8_t codes[4];
    size_t count = 0;
    CHECK(mqtt_decode_suback(&p, MQTT_V311, &id, codes, 4, &count) == MQTT_OK);
    CHECK(id == 7 && count == 2 && codes[0] == 1 && codes[1] == 0x80);
    const uint8_t bad_suback[] = { 0x90, 0x03, 0x00, 0x07, 0x03 };
    CHECK(mqtt_parse(bad_suback, sizeof(bad_suback), 64, &p) == MQTT_OK);
    CHECK(mqtt_decode_suback(&p, MQTT_V311, &id, codes, 4, &count) == MQTT_MALFORMED);

    const uint8_t connack[] = { 0x20, 0x02, 0x01, 0x05 };
    MqttConnack ca;
    CHECK(mqtt_parse(connack, 4, 64, &p) == MQTT_OK && mqtt_decode_connack(&p, MQTT_V311, &ca) == MQTT_OK);
    CHECK(ca.session_present && ca.return_code == 5);
    CHECK(!strcmp(mqtt_connack_text(5), "not authorised"));
    CHECK(!strcmp(mqtt_connack_text(4), "bad user name or password"));
    const uint8_t bad_connack[] = { 0x20, 0x02, 0x02, 0x00 };
    CHECK(mqtt_parse(bad_connack, 4, 64, &p) == MQTT_OK && mqtt_decode_connack(&p, MQTT_V311, &ca) == MQTT_MALFORMED);

    CHECK(mqtt_encode_puback(buf, sizeof(buf), 0xBEEF) == 4);
    CHECK(mqtt_parse(buf, 4, 64, &p) == MQTT_OK);
    CHECK(mqtt_decode_puback(&p, MQTT_V311, &id) == MQTT_OK && id == 0xBEEF);
    CHECK(mqtt_encode_empty(buf, sizeof(buf), MQTT_PINGREQ) == 2 && buf[0] == 0xC0 && buf[1] == 0);
    CHECK(mqtt_encode_empty(buf, sizeof(buf), MQTT_DISCONNECT) == 2 && buf[0] == 0xE0);
    CHECK(mqtt_encode_empty(buf, sizeof(buf), MQTT_PUBLISH) == 0);

    // A PUBLISH whose topic runs past the packet, or holds a wildcard / NUL.
    const uint8_t cut[] = { 0x30, 0x03, 0x00, 0x05, 'a' };
    CHECK(mqtt_parse(cut, sizeof(cut), 64, &p) == MQTT_OK);
    MqttPublish m;
    CHECK(mqtt_decode_publish(&p, MQTT_V311, &m) == MQTT_MALFORMED);
    const uint8_t wild[] = { 0x30, 0x04, 0x00, 0x02, 'a', '#' };
    CHECK(mqtt_parse(wild, sizeof(wild), 64, &p) == MQTT_OK && mqtt_decode_publish(&p, MQTT_V311, &m) == MQTT_MALFORMED);
    const uint8_t nul[] = { 0x30, 0x04, 0x00, 0x02, 'a', 0 };
    CHECK(mqtt_parse(nul, sizeof(nul), 64, &p) == MQTT_OK && mqtt_decode_publish(&p, MQTT_V311, &m) == MQTT_MALFORMED);
    // QoS 1 with packet id 0.
    const uint8_t qos1_zero[] = { 0x32, 0x05, 0x00, 0x01, 'a', 0x00, 0x00 };
    CHECK(mqtt_parse(qos1_zero, sizeof(qos1_zero), 64, &p) == MQTT_OK && mqtt_decode_publish(&p, MQTT_V311, &m) == MQTT_MALFORMED);
}

// MQTT 5.0: the CONNECT's properties, the empty property lists the client
// writes, and what it reads in the broker's CONNACK, SUBACK, PUBLISH and
// DISCONNECT.
static void test_mqtt5(void)
{
    uint8_t buf[512];
    MqttConnect c;
    memset(&c, 0, sizeof(c));
    c.client_id = "pg-a1b2c3d4-app";
    c.username = "user";
    c.password = "pass";
    c.will_topic = "playguard/a1b2c3d4/availability";
    c.will_payload = "offline";
    c.will_len = 7;
    c.will_retain = true;
    c.keepalive_s = 30;
    c.clean_session = true;
    c.version = MQTT_V5;
    c.max_packet = 4096;
    size_t n = mqtt_encode_connect(buf, sizeof(buf), &c);
    MqttPacket p;
    CHECK(n > 0 && mqtt_parse(buf, n, sizeof(buf), &p) == MQTT_OK && p.type == MQTT_CONNECT);
    const uint8_t *b = p.body;
    CHECK(b[6] == 5 && b[7] == (0x80 | 0x40 | 0x20 | 0x04 | 0x02) && b[8] == 0 && b[9] == 30);
    // Properties: the maximum packet size only.
    CHECK(b[10] == 5 && b[11] == 0x27 && b[12] == 0 && b[13] == 0 && b[14] == 0x10 && b[15] == 0);
    size_t at = 16;
    CHECK(b[at + 1] == strlen(c.client_id));
    at += 2 + strlen(c.client_id);
    CHECK(b[at] == 0);   // will properties: none
    at += 1;
    CHECK(b[at + 1] == strlen(c.will_topic));
    at += 2 + strlen(c.will_topic) + 2 + 7 + 6 + 6;
    CHECK(at == p.body_len);
    // No maximum packet size: an empty property list.
    c.max_packet = 0;
    n = mqtt_encode_connect(buf, sizeof(buf), &c);
    CHECK(mqtt_parse(buf, n, sizeof(buf), &p) == MQTT_OK && p.body[10] == 0);

    // PUBLISH and SUBSCRIBE carry an empty property list.
    n = mqtt_encode_publish_head(buf, sizeof(buf), MQTT_V5, "a/b", 2, 0, true, false, 0);
    CHECK(n == 2 + 2 + 3 + 1 && buf[1] == 2 + 3 + 1 + 2 && buf[n - 1] == 0);
    memcpy(buf + n, "on", 2);
    MqttPublish m;
    CHECK(mqtt_parse(buf, n + 2, sizeof(buf), &p) == MQTT_OK && mqtt_decode_publish(&p, MQTT_V5, &m) == MQTT_OK);
    CHECK(m.topic_len == 3 && m.payload_len == 2 && !memcmp(m.payload, "on", 2) && m.retain);
    const char *topics[] = { "playguard/id/+/set" };
    const uint8_t qos[] = { 1 };
    n = mqtt_encode_subscribe(buf, sizeof(buf), MQTT_V5, 9, topics, qos, 1);
    CHECK(n > 0 && mqtt_parse(buf, n, sizeof(buf), &p) == MQTT_OK);
    CHECK(p.body[2] == 0 && p.body[4] == strlen(topics[0]) && p.body[p.body_len - 1] == 1);

    // A PUBLISH from the broker with properties (message expiry, a user
    // property): skipped, the payload found.
    const uint8_t pub[] = { 0x31, 0x16, 0x00, 0x01, 't', 0x0D, 0x02, 0x00, 0x00, 0x00, 0x3C,
                            0x26, 0x00, 0x01, 'k', 0x00, 0x02, 'v', 'v', 'O', 'F', 'F', '!', '!' };
    CHECK(mqtt_parse(pub, sizeof(pub), 64, &p) == MQTT_OK && mqtt_decode_publish(&p, MQTT_V5, &m) == MQTT_OK);
    CHECK(m.retain && m.payload_len == 5 && !memcmp(m.payload, "OFF!!", 5));
    // Property lengths that run past the packet, or an unknown property.
    const uint8_t pub_cut[] = { 0x30, 0x06, 0x00, 0x01, 't', 0x09, 0x02, 0x00 };
    CHECK(mqtt_parse(pub_cut, sizeof(pub_cut), 64, &p) == MQTT_OK && mqtt_decode_publish(&p, MQTT_V5, &m) == MQTT_MALFORMED);
    const uint8_t pub_unknown[] = { 0x30, 0x06, 0x00, 0x01, 't', 0x02, 0x7E, 0x00 };
    CHECK(mqtt_parse(pub_unknown, sizeof(pub_unknown), 64, &p) == MQTT_OK &&
           mqtt_decode_publish(&p, MQTT_V5, &m) == MQTT_MALFORMED);

    // CONNACK: accepted, with a server keep-alive, a maximum packet size and
    // retained messages available.
    const uint8_t ack[] = { 0x20, 0x0D, 0x00, 0x00, 0x0A, 0x13, 0x00, 0x3C, 0x27, 0x00, 0x01, 0x00, 0x00, 0x25, 0x01 };
    MqttConnack ca;
    CHECK(mqtt_parse(ack, sizeof(ack), 64, &p) == MQTT_OK && mqtt_decode_connack(&p, MQTT_V5, &ca) == MQTT_OK);
    CHECK(ca.return_code == 0 && !ca.v311 && ca.has_server_keepalive && ca.server_keepalive == 60);
    CHECK(ca.max_packet == 65536 && !ca.retain_unavailable && !ca.wildcard_unavailable && !ca.reason[0]);
    // Refused, with a reason string; retained messages and wildcards unavailable.
    const uint8_t refused[] = { 0x20, 0x14, 0x00, 0x87, 0x11, 0x25, 0x00, 0x28, 0x00, 0x1F, 0x00, 0x0A,
                                'n', 'o', ' ', 'a', 'c', 'c', 'e', 's', 's', '\n' };
    CHECK(mqtt_parse(refused, sizeof(refused), 64, &p) == MQTT_OK && mqtt_decode_connack(&p, MQTT_V5, &ca) == MQTT_OK);
    CHECK(ca.return_code == 0x87 && ca.retain_unavailable && ca.wildcard_unavailable);
    CHECK(!strcmp(ca.reason, "no access "));   // control characters blanked
    CHECK(!strcmp(mqtt_reason_text(0x87), "not authorised"));
    CHECK(!strcmp(mqtt_reason_text(MQTT_RC_UNSUPPORTED_VERSION), "the broker does not speak this MQTT version"));
    // A 3.1.1 broker's answer to a 5.0 CONNECT.
    const uint8_t old[] = { 0x20, 0x02, 0x00, 0x01 };
    CHECK(mqtt_parse(old, sizeof(old), 64, &p) == MQTT_OK && mqtt_decode_connack(&p, MQTT_V5, &ca) == MQTT_OK);
    CHECK(ca.v311 && ca.return_code == 1);
    // A property list longer than the packet.
    const uint8_t ack_cut[] = { 0x20, 0x04, 0x00, 0x00, 0x05, 0x13 };
    CHECK(mqtt_parse(ack_cut, sizeof(ack_cut), 64, &p) == MQTT_OK && mqtt_decode_connack(&p, MQTT_V5, &ca) == MQTT_MALFORMED);
    // 3.1.1 never has properties.
    CHECK(mqtt_parse(ack, sizeof(ack), 64, &p) == MQTT_OK && mqtt_decode_connack(&p, MQTT_V311, &ca) == MQTT_MALFORMED);

    // SUBACK: properties, then a reason code per topic (0x87: not authorised).
    const uint8_t suback[] = { 0x90, 0x05, 0x00, 0x09, 0x00, 0x01, 0x87 };
    uint16_t id = 0;
    uint8_t codes[4];
    size_t count = 0;
    CHECK(mqtt_parse(suback, sizeof(suback), 64, &p) == MQTT_OK);
    CHECK(mqtt_decode_suback(&p, MQTT_V5, &id, codes, 4, &count) == MQTT_OK);
    CHECK(id == 9 && count == 2 && codes[0] == 1 && codes[1] == 0x87);
    CHECK(mqtt_decode_suback(&p, MQTT_V311, &id, codes, 4, &count) == MQTT_MALFORMED);   // 0x87 is 5.0's
    // PUBACK with a reason code and no properties.
    const uint8_t puback[] = { 0x40, 0x03, 0x12, 0x34, 0x10 };
    CHECK(mqtt_parse(puback, sizeof(puback), 64, &p) == MQTT_OK && mqtt_decode_puback(&p, MQTT_V5, &id) == MQTT_OK);
    CHECK(id == 0x1234 && mqtt_decode_puback(&p, MQTT_V311, &id) == MQTT_MALFORMED);

    // DISCONNECT from the broker: a reason code and a reason string.
    const uint8_t disc[] = { 0xE0, 0x0A, 0x8E, 0x08, 0x1F, 0x00, 0x05, 't', 'a', 'k', 'e', 'n' };
    uint8_t code = 0;
    char reason[32];
    CHECK(mqtt_parse(disc, sizeof(disc), 64, &p) == MQTT_OK);
    CHECK(mqtt_decode_disconnect(&p, &code, reason, sizeof(reason)) == MQTT_OK);
    CHECK(code == 0x8E && !strcmp(reason, "taken") && !strcmp(mqtt_reason_text(code), "session taken over by another client"));
    const uint8_t disc_plain[] = { 0xE0, 0x00 };
    CHECK(mqtt_parse(disc_plain, sizeof(disc_plain), 64, &p) == MQTT_OK);
    CHECK(mqtt_decode_disconnect(&p, &code, reason, sizeof(reason)) == MQTT_OK && code == 0 && !reason[0]);
}

static void test_topic_matching(void)
{
    CHECK(mqtt_topic_matches("a/+/set", "a/limit_mon/set", 15));
    CHECK(!mqtt_topic_matches("a/+/set", "a/x/y/set", 9));
    CHECK(mqtt_topic_matches("a/#", "a", 1));
    CHECK(mqtt_topic_matches("a/#", "a/b/c", 5));
    CHECK(mqtt_topic_matches("#", "x/y", 3));
    CHECK(!mqtt_topic_matches("a/b", "a/bc", 4));
    CHECK(mqtt_topic_matches("a/+", "a/", 2));
    CHECK(!mqtt_topic_matches("a/+", "a", 1));
    CHECK(mqtt_topic_valid("a/b") && !mqtt_topic_valid("a/+") && !mqtt_topic_valid(""));
}

// ---------------------------------------------------------------- sync.conf

static void test_conf(void)
{
    SyncConf c;
    sync_conf_defaults(&c);
    CHECK(c.port == 1883 && c.policy == SyncPolicy_Ask && c.ha_discovery && !c.remote_timer_writes);
    CHECK(!strcmp(c.topic_prefix, "playguard") && !strcmp(c.discovery_prefix, "homeassistant"));
    CHECK(c.mqtt_version == SyncMqtt_Auto);
    CHECK(sync_conf_problem(&c) != NULL);   // no host

    const char *text = "\xEF\xBB\xBF# comment\r\n"
                       "enabled=1\r\n"
                       "  host = broker.lan\n"           // spaces around the key, not the value
                       "port=8883\n"
                       "tls=on\n"
                       "mqtt_version=3.1.1\n"
                       "username=parent\n"
                       "password= p=a ss#word\n"         // kept verbatim after '='
                       "console_id=a1b2c3d4\n"
                       "console_name=Salon\n"
                       "policy=auto\n"
                       "remote_timer_writes=yes\n"
                       "poll_s=5\n"                      // out of range: default kept
                       "topic_prefix=bad/prefix\n"       // refused
                       "future_key=keep me\n"
                       "noequals\n"
                       "=novalue\n";
    sync_conf_defaults(&c);
    CHECK(sync_conf_parse(&c, text, strlen(text)));
    CHECK(c.enabled && c.port == 8883 && c.tls && c.mqtt_version == SyncMqtt_V311);
    CHECK(!strcmp(c.host, " broker.lan"));
    CHECK(!strcmp(c.password, " p=a ss#word"));
    CHECK(!strcmp(c.console_id, "a1b2c3d4") && !strcmp(c.console_name, "Salon"));
    CHECK(c.policy == SyncPolicy_Auto && c.remote_timer_writes);
    CHECK(c.poll_s == 30 && !strcmp(c.topic_prefix, "playguard"));
    CHECK(contains(c.extra, "future_key=keep me\n"));
    CHECK(sync_conf_problem(&c) == NULL);

    // Round trip, unknown keys kept.
    char out[2048];
    const size_t n = sync_conf_write(&c, out, sizeof(out));
    CHECK(n > 0 && contains(out, "future_key=keep me\n") && contains(out, "policy=auto\n"));
    CHECK(contains(out, "mqtt_version=3.1.1\n"));
    SyncConf back;
    sync_conf_defaults(&back);
    CHECK(sync_conf_parse(&back, out, n));
    CHECK(!memcmp(&back, &c, sizeof(c)));
    CHECK(sync_conf_write(&c, out, 64) == 0);
    // The MQTT version's spellings; anything else keeps the setting.
    const struct { const char *text; SyncMqttVersion v; } versions[] = {
        { "mqtt_version=5\n", SyncMqtt_V5 }, { "mqtt_version=5.0\n", SyncMqtt_V5 },
        { "mqtt_version=auto\n", SyncMqtt_Auto }, { "mqtt_version=4\n", SyncMqtt_V311 },
        { "mqtt_version=6\n", SyncMqtt_V311 },
    };
    SyncConf v = c;
    for (size_t i = 0; i < sizeof(versions) / sizeof(versions[0]); i++) {
        CHECK(sync_conf_parse(&v, versions[i].text, strlen(versions[i].text)));
        CHECK(v.mqtt_version == versions[i].v);
    }
    CHECK(!strcmp(sync_mqtt_version_name(SyncMqtt_V5), "5") && !strcmp(sync_mqtt_version_name(SyncMqtt_Auto), "auto"));

    // What is still missing.
    SyncConf m = c;
    m.console_id[0] = '\0';
    CHECK(sync_conf_problem(&m) && contains(sync_conf_problem(&m), "console id"));
    m = c;
    m.username[0] = '\0';
    CHECK(sync_conf_problem(&m));
    m.allow_anonymous = true;
    CHECK(sync_conf_problem(&m) == NULL);
    m = c;
    strcpy(m.console_name, "a\nb");
    CHECK(sync_conf_problem(&m));

    CHECK(sync_conf_id_valid("0123abcd") && !sync_conf_id_valid("0123ABCD") && !sync_conf_id_valid("123"));
    uint32_t u;
    CHECK(sync_parse_uint("300", 10, 300, &u) && u == 300);
    CHECK(!sync_parse_uint("301", 10, 300, &u) && !sync_parse_uint("-1", 0, 9, &u) && !sync_parse_uint("", 0, 9, &u));
    CHECK(!sync_parse_uint("99999999999", 0, 0xFFFFFFFFu, &u));
    bool b;
    CHECK(sync_parse_bool("TRUE", &b) && b && sync_parse_bool("off", &b) && !b && !sync_parse_bool("maybe", &b));
}

static void test_records(void)
{
    SyncRecords r;
    sync_records_clear(&r);
    CHECK(r.extra_weekday == -1);
    r.extra_weekday = 3;
    strcpy(r.extra_date, "2026-10-08");
    r.extra_base = 120;
    r.extra_value = 150;
    r.console_lock = true;
    r.console_lock_prev_ok = true;
    const uint16_t prev[7] = { 60, 90, 0xFFFF, 120, 0, 30, 45 };
    memcpy(r.console_lock_prev, prev, sizeof(prev));
    r.relock_pending = true;
    char text[512];
    const size_t n = sync_records_write(&r, text, sizeof(text));
    CHECK(n > 0);
    SyncRecords back;
    sync_records_parse(&back, text, n);
    CHECK(!memcmp(&back, &r, sizeof(r)));
    // A date without its weekday, or a broken list of limits.
    sync_records_parse(&back, "extra_weekday=2\nconsole_lock_prev=1,2,3\n", 40);
    CHECK(back.extra_weekday == -1 && !back.console_lock_prev_ok);
}

// ---------------------------------------------------------------- orders

static SyncReason parse(const char *entity, const char *payload, SyncIntent *in)
{
    return sync_apply_parse(entity, payload, strlen(payload), in);
}

static void test_orders(void)
{
    SyncIntent in;
    CHECK(parse("limit_mon", "90", &in) == SyncReason_None && in.kind == SyncIntent_LimitDay && in.day == 1 &&
           in.minutes == 90 && in.mask == 2 && in.days[1] == 90);
    CHECK(parse("limit_sun", " 90.0 ", &in) == SyncReason_None && in.minutes == 90);   // HA's number entity
    CHECK(parse("limit_sun", "1440", &in) == SyncReason_None && in.minutes == 0xFFFF);  // no limit
    CHECK(parse("limit_sun", "0", &in) == SyncReason_None && in.minutes == 0);
    CHECK(parse("limit_sun", "1441", &in) == SyncReason_OutOfRange);
    CHECK(parse("limit_sun", "90.5", &in) == SyncReason_Invalid);
    CHECK(parse("limit_sun", "-5", &in) == SyncReason_Invalid);
    CHECK(parse("limit_sun", "", &in) == SyncReason_Invalid);
    CHECK(parse("limit_uniform", "120", &in) == SyncReason_None && in.mask == 0x7F && in.days[6] == 120);
    CHECK(parse("max_screentime_today", "45", &in) == SyncReason_None && in.kind == SyncIntent_LimitToday);
    CHECK(parse("limits_week", "60, 90,120,120,120,180,1440", &in) == SyncReason_None);
    CHECK(in.mask == 0x7F && in.days[0] == 60 && in.days[1] == 90 && in.days[6] == 0xFFFF);
    CHECK(parse("limits_week", "60,90,120", &in) == SyncReason_Invalid);
    CHECK(parse("limits_week", "60,90,120,120,120,180,1440,5", &in) == SyncReason_Invalid);
    CHECK(parse("limits_week", "60,90,120,120,120,180,9999", &in) == SyncReason_OutOfRange);
    CHECK(parse("add_bonus_time", "30", &in) == SyncReason_None && in.kind == SyncIntent_BonusTime && in.minutes == 30);
    CHECK(parse("add_bonus_time", "4", &in) == SyncReason_OutOfRange);
    CHECK(parse("add_bonus_time", "181", &in) == SyncReason_OutOfRange);
    CHECK(parse("add_bonus_time_15", "PRESS", &in) == SyncReason_None && in.minutes == 15);
    CHECK(parse("add_bonus_time_30", "PRESS", &in) == SyncReason_None && in.minutes == 30);
    CHECK(parse("add_bonus_time_60", "PRESS", &in) == SyncReason_None && in.minutes == 60);
    CHECK(parse("add_bonus_time_60", "60", &in) == SyncReason_Invalid);
    CHECK(parse("unlocked", "ON", &in) == SyncReason_None && in.kind == SyncIntent_Unlock);
    CHECK(parse("unlocked", "OFF", &in) == SyncReason_None && in.kind == SyncIntent_LockNow);
    CHECK(parse("bedtime_alarm", "21:30:00", &in) == SyncReason_None && in.hour == 21 && in.minute == 30);
    CHECK(parse("bedtime_alarm", "21:30", &in) == SyncReason_None);
    CHECK(parse("bedtime_alarm", "15:59", &in) == SyncReason_OutOfRange);
    CHECK(parse("bedtime_alarm", "24:00", &in) == SyncReason_OutOfRange);
    CHECK(parse("bedtime_alarm", "9:30", &in) == SyncReason_Invalid);
    CHECK(parse("bedtime_end_time", "06:15", &in) == SyncReason_None);
    CHECK(parse("bedtime_end_time", "09:01", &in) == SyncReason_OutOfRange);
    CHECK(parse("bedtime_end_time", "04:59", &in) == SyncReason_OutOfRange);
    CHECK(parse("restriction_level", "young_child", &in) == SyncReason_None && in.level == 2);
    CHECK(parse("restriction_level", "adult", &in) == SyncReason_Invalid);
    // The select's option for "none", from a client without its command template.
    CHECK(parse("restriction_level", "unrestricted", &in) == SyncReason_None && in.level == 0);
    CHECK(parse("restriction_level", "none", &in) == SyncReason_None && in.level == 0);
    CHECK(parse("profile", "School week", &in) == SyncReason_None && !strcmp(in.profile, "School week"));
    CHECK(parse("profile", "0123456789012345678901234567890123", &in) == SyncReason_Invalid);
    CHECK(parse("discovery", "off", &in) == SyncReason_None && in.kind == SyncIntent_Discovery && !in.on);
    CHECK(parse("vr_restricted", "maybe", &in) == SyncReason_Invalid);
    CHECK(parse("no_such_thing", "1", &in) == SyncReason_UnknownEntity);
    CHECK(sync_apply_parse("limit_mon", "9\0" "0", 4, &in) == SyncReason_Invalid);   // a NUL inside

    // Nothing that deletes parental controls, unlinks the companion app or
    // touches the PIN is ever accepted, whatever it is called.
    const char *never[] = { "delete", "delete_parental_controls", "delete_settings", "unlink", "delete_pairing",
                            "pairing", "pin", "pin_code", "set_pin", "show_pin", "get_pin", "reset" };
    for (size_t i = 0; i < sizeof(never) / sizeof(never[0]); i++) {
        CHECK(parse(never[i], "PRESS", &in) == SyncReason_UnknownEntity);
        CHECK(parse(never[i], "ON", &in) == SyncReason_UnknownEntity);
    }
    CHECK(!strcmp(sync_reason_name(SyncReason_TimerWritesDisabled), "timer_writes_disabled"));
}

// The entity table and the order parser agree: every order HA can send
// parses, read-only entities take no order, the timer ones need
// remote_timer_writes, and every intent the parser has is an entity.
static void test_entities_match_orders(void)
{
    size_t n = 0;
    const SyncEntity *list = sync_entities(&n);
    CHECK(n > 30);
    for (size_t i = 0; i < n; i++) {
        const SyncEntity *e = &list[i];
        for (size_t k = 0; k < i; k++) CHECK(strcmp(list[k].id, e->id));   // unique ids
        CHECK(sync_entity_find(e->id) == e);
        SyncIntent in;
        const char *sample = !strcmp(e->platform, "number") ? "60"
                           : !strcmp(e->platform, "switch") ? "ON"
                           : !strcmp(e->platform, "button") ? "PRESS"
                           : !strcmp(e->platform, "time") ? (strstr(e->id, "end") ? "06:30:00" : "21:00:00")
                           : !strcmp(e->id, "restriction_level") ? "child"
                           : !strcmp(e->id, "profile") ? "School week"
                           : "1";
        const SyncReason r = parse(e->id, sample, &in);
        if (e->write == SyncWrite_None) {
            CHECK(r == SyncReason_UnknownEntity);
            continue;
        }
        CHECK(r == SyncReason_None);
        const bool timer = sync_intent_needs_timer_writes(in.kind);
        CHECK(timer == (e->write == SyncWrite_Timer));
        CHECK(e->value_template || !strcmp(e->platform, "button"));
    }
}

// ---------------------------------------------------------------- discovery

static size_t count(const char *hay, const char *needle)
{
    size_t c = 0;
    for (const char *p = strstr(hay, needle); p; p = strstr(p + 1, needle)) c++;
    return c;
}

static void test_discovery(void)
{
    SyncConf c;
    sync_conf_defaults(&c);
    strcpy(c.console_id, "a1b2c3d4");
    strcpy(c.console_name, "Salon \"Léa\"");
    char topic[128];
    CHECK(sync_discovery_topic(&c, topic, sizeof(topic)) > 0);
    CHECK(!strcmp(topic, "homeassistant/device/playguard_a1b2c3d4/config"));

    static char buf[SYNC_SCRATCH_SIZE];
    const char profiles[2][SYNC_PROFILE_MAX] = { "School week", "Holidays" };
    SyncDiscovery d = { &c, "1.1.0", false, profiles, 2 };

    // remote_timer_writes off: no limit, bedtime, lock or extra time entity,
    // each sent as its platform alone, which removes it from Home Assistant.
    size_t n = sync_discovery_build(&d, buf, sizeof(buf));
    CHECK(n > 0 && n < sizeof(buf));
    CHECK(contains(buf, "\"ids\":[\"playguard_a1b2c3d4\"]"));
    CHECK(contains(buf, "\"name\":\"Salon \\\"Léa\\\"\""));
    CHECK(contains(buf, "\"avty_t\":\"playguard/a1b2c3d4/availability\""));
    CHECK(contains(buf, "\"used_screen_time\":{\"p\":\"sensor\""));
    CHECK(contains(buf, "\"cmd_t\":\"playguard/a1b2c3d4/vr_restricted/set\""));
    CHECK(contains(buf, "\"cmd_t\":\"playguard/a1b2c3d4/lock_now/set\""));
    CHECK(contains(buf, "\"limit_mon\":{\"p\":\"number\"}"));
    CHECK(contains(buf, "\"bedtime_alarm\":{\"p\":\"time\"}"));
    CHECK(contains(buf, "\"unlocked\":{\"p\":\"switch\"}"));
    CHECK(contains(buf, "\"add_bonus_time_15\":{\"p\":\"button\"}"));
    CHECK(contains(buf, "\"profile\":{\"p\":\"select\"}"));
    CHECK(!contains(buf, "limit_mon/set"));
    // The restriction level's "none" is the option "unrestricted" (HA reads
    // a select's "None" as unknown), sent back as "none".
    CHECK(contains(buf, "\"ops\":[\"unrestricted\",\"young_child\",\"child\",\"teen\",\"custom\"]"));
    CHECK(contains(buf, "\"cmd_tpl\":\"{{ 'none' if value == 'unrestricted' else value }}\""));
    // The day's totals restart at midnight; the time left is a measurement.
    CHECK(contains(buf, "\"used_screen_time\":{\"p\":\"sensor\""));
    CHECK(contains(buf, "\"stat_cla\":\"total_increasing\""));
    CHECK(contains(buf, "\"stat_cla\":\"measurement\""));
    CHECK(contains(buf, "\"stat_t\":\"playguard/a1b2c3d4/event\",\"evt_typ\":["));
    const size_t without = n;

    // On: the timer entities appear, retained and optimistic, with the profiles.
    c.remote_timer_writes = true;
    n = sync_discovery_build(&d, buf, sizeof(buf));
    CHECK(n > without);
    CHECK(contains(buf, "\"limit_mon\":{\"p\":\"number\""));
    CHECK(contains(buf, "\"cmd_t\":\"playguard/a1b2c3d4/limit_mon/set\",\"ret\":true,\"opt\":true"));
    CHECK(contains(buf, "\"ops\":[\"School week\",\"Holidays\"]"));
    CHECK(contains(buf, "\"bedtime_alarm\":{\"p\":\"time\""));
    // The console's own entities read the system and games documents; the
    // serial number and the Wi-Fi network's name are off in Home Assistant.
    CHECK(contains(buf, "\"battery\":{\"p\":\"sensor\",\"name\":\"Battery\",\"uniq_id\":\"playguard_a1b2c3d4_battery\","
                        "\"stat_t\":\"playguard/a1b2c3d4/system\",\"val_tpl\":\"{{ value_json.power.battery_pct }}\""));
    CHECK(contains(buf, "\"stat_t\":\"playguard/a1b2c3d4/games\",\"val_tpl\":\"{{ value_json.installed }}\""));
    CHECK(contains(buf, "\"val_tpl\":\"{{ value_json.hardware.serial }}\",\"ent_cat\":\"diagnostic\",\"en\":false"));
    CHECK(contains(buf, "\"unit_of_meas\":\"\xc2\xb0" "C\",\"dev_cla\":\"temperature\""));
    CHECK(n < SYNC_SCRATCH_SIZE * 3 / 4);   // room to grow
    // Every component has a unique id and the device's availability.
    size_t total = 0;
    const SyncEntity *list = sync_entities(&total);
    CHECK(count(buf, "\"uniq_id\":\"playguard_a1b2c3d4_") == total);
    (void)list;
    save("discovery.json", buf);
    printf("discovery: %zu bytes, %zu components\n", n, total);

    // Without profiles, no profile select (nothing to choose from): removed.
    d.n_profiles = 0;
    n = sync_discovery_build(&d, buf, sizeof(buf));
    CHECK(contains(buf, "\"profile\":{\"p\":\"select\"}"));
    // Read-only: sensors only.
    d.read_only = true;
    n = sync_discovery_build(&d, buf, sizeof(buf));
    CHECK(n > 0 && !contains(buf, "cmd_t"));
    // Too small a buffer: 0.
    CHECK(sync_discovery_build(&d, buf, 512) == 0);


    // On an emuMMC (one sync.conf for every system on the SD card): a device
    // of its own, its id's first digit's top bit flipped, its name marked.
    c.emummc = true;
    d.read_only = false;
    char id[SYNC_ID_LEN + 1], name[SYNC_DEVICE_NAME_MAX];
    sync_conf_device_id(&c, id);
    CHECK(!strcmp(id, "21b2c3d4") && !strcmp(c.console_id, "a1b2c3d4"));
    sync_conf_device_name(&c, name, sizeof(name));
    CHECK(!strcmp(name, "Salon \"Léa\" (emuMMC)"));
    CHECK(sync_discovery_topic(&c, topic, sizeof(topic)) > 0);
    CHECK(!strcmp(topic, "homeassistant/device/playguard_21b2c3d4/config"));
    n = sync_discovery_build(&d, buf, sizeof(buf));
    CHECK(n > 0 && contains(buf, "\"ids\":[\"playguard_21b2c3d4\"]") && !contains(buf, "a1b2c3d4"));
    CHECK(contains(buf, "\"name\":\"Salon \\\"Léa\\\" (emuMMC)\""));
    CHECK(contains(buf, "\"cmd_t\":\"playguard/21b2c3d4/lock_now/set\""));
    strcpy(c.console_id, "f0000000");   // every digit flips both ways
    sync_conf_device_id(&c, id);
    CHECK(!strcmp(id, "70000000"));
    c.console_name[0] = '\0';
    sync_conf_device_name(&c, name, sizeof(name));
    CHECK(!strcmp(name, "Nintendo Switch (emuMMC)"));
    c.console_id[0] = '\0';   // no id yet: none
    sync_conf_device_id(&c, id);
    CHECK(!id[0]);
}

// ---------------------------------------------------------------- state

static void timer_state(PtState *pt)
{
    memset(pt, 0, sizeof(*pt));
    pt->session_valid = pt->fw_supported = pt->valid = true;
    const uint16_t days[7] = { 180, 120, 120, 120, 120, 120, 0xFFFF };
    memcpy(pt->day_min, days, sizeof(days));
    pt->enabled_valid = pt->enabled = true;
    pt->restricted_valid = true;
    pt->temporary_unlocked_valid = true;
    pt->remaining_valid = true;
    pt->remaining_ns = 45ULL * 60000000000ULL;
    pt->alarm_disabled_valid = true;
    pt->bedtime_valid = pt->bedtime_enabled = true;
    pt->bedtime_hour = 21;
    pt->bedtime_minute = 30;
    pt->bedtime_reset_valid = true;
    pt->bedtime_reset_hour = 6;
}

static void test_state(void)
{
    SyncConf c;
    sync_conf_defaults(&c);
    strcpy(c.console_id, "a1b2c3d4");
    strcpy(c.console_name, "Salon");
    PctlStatus st;
    memset(&st, 0, sizeof(st));
    st.restriction_enabled_ok = st.restriction_enabled = true;
    st.temp_unlocked_ok = true;
    st.pin_length_ok = true;
    st.pin_length = 4;
    st.safety_level_ok = true;
    st.safety_level = 3;
    st.settings_ok = true;
    st.settings.rating_age = 12;
    st.settings.sns_post_restriction = true;
    st.pairing_active_ok = true;
    PtState pt;
    timer_state(&pt);
    SysInfo sys;
    memset(&sys, 0, sizeof(sys));
    sys.hos_version = (23u << 16) | 1u;
    sys.ams_valid = true;
    sys.ams_major = 1;
    sys.ams_minor = 12;
    SyncRecords rec;
    sync_records_clear(&rec);
    rec.extra_weekday = 1;
    strcpy(rec.extra_date, "2026-10-05");
    rec.extra_base = 90;
    rec.extra_value = 120;

    SyncSnapshot s;
    memset(&s, 0, sizeof(s));
    s.source = "app";
    s.ts = 1791000000;
    s.local_date = "2026-10-05";
    s.weekday = 1;   // Monday
    s.clock_accurate_ok = s.clock_accurate = true;
    s.conf = &c;
    s.sys = &sys;
    s.app_version = "1.1.0";
    s.status = &st;
    s.timer = &pt;
    s.records = &rec;
    s.activity_ok = true;
    s.activity_s = 74 * 60;
    s.now_playing = 0x0100000000010000ULL;
    s.now_playing_name = "Super Mario Odyssey";
    s.now_playing_since = 1790990000;
    static const char profiles[2][SYNC_PROFILE_MAX] = { "School week", "Vacances d'été" };
    s.profiles = profiles;
    s.n_profiles = 2;
    static char buf[4096];
    size_t n = sync_state_build(&s, buf, sizeof(buf));
    CHECK(n > 0);
    save("state.json", buf);
    CHECK(contains(buf, "\"source\":\"app\""));
    CHECK(contains(buf, "\"firmware\":\"23.0.1\",\"atmosphere\":\"1.12.0\""));
    CHECK(contains(buf, "\"level\":\"child\""));
    CHECK(contains(buf, "\"limits\":{\"sun\":180,\"mon\":120,\"tue\":120,\"wed\":120,\"thu\":120,\"fri\":120,\"sat\":1440}"));
    CHECK(contains(buf, "\"limits_min\":[180,120,120,120,120,120,1440]"));
    CHECK(contains(buf, "\"uniform_min\":null"));
    CHECK(contains(buf, "\"limit_today_min\":120"));
    CHECK(contains(buf, "\"remaining_min\":45"));
    CHECK(contains(buf, "\"used_min\":75"));
    CHECK(contains(buf, "\"alarm_on\":true"));
    CHECK(contains(buf, "\"bedtime\":{\"enabled\":true,\"start\":\"21:30:00\",\"end\":\"06:00:00\"}"));
    CHECK(contains(buf, "\"extended_today_min\":30"));
    CHECK(contains(buf, "\"now_playing\":{\"app_id\":\"0100000000010000\",\"name\":\"Super Mario Odyssey\""));
    CHECK(contains(buf, "\"activity_today\":{\"used_min\":74"));
    CHECK(contains(buf, "\"link\":{\"policy\":\"ask\""));
    CHECK(contains(buf, "\"profiles\":[\"School week\",\"Vacances d'été\"]}"));
    CHECK(contains(buf, "\"console\":{\"id\":\"a1b2c3d4\",\"name\":\"Salon\""));
    // On an emuMMC: this system's device, its own id and name.
    c.emummc = true;
    n = sync_state_build(&s, buf, sizeof(buf));
    CHECK(contains(buf, "\"console\":{\"id\":\"21b2c3d4\",\"name\":\"Salon (emuMMC)\""));
    c.emummc = false;

    // Nothing counted yet today (1454 reads 0): the whole limit is left, the
    // time played is unknown.
    pt.remaining_ns = 0;
    n = sync_state_build(&s, buf, sizeof(buf));
    CHECK(contains(buf, "\"remaining_min\":120") && contains(buf, "\"used_min\":null"));
    // Time is up.
    pt.restricted = true;
    n = sync_state_build(&s, buf, sizeof(buf));
    CHECK(contains(buf, "\"limit_reached\":true") && contains(buf, "\"remaining_min\":0") && contains(buf, "\"used_min\":120"));

    // Nothing could be read: every object is there, its leaves null.
    SyncSnapshot empty;
    memset(&empty, 0, sizeof(empty));
    empty.weekday = -1;
    empty.conf = &c;
    n = sync_state_build(&empty, buf, sizeof(buf));
    CHECK(n > 0);
    save("state_empty.json", buf);
    CHECK(contains(buf, "\"limits\":{\"sun\":null"));
    CHECK(contains(buf, "\"limits_min\":null"));
    CHECK(contains(buf, "\"bedtime\":{\"enabled\":null,\"start\":null,\"end\":null}"));
    CHECK(contains(buf, "\"now_playing\":{\"app_id\":null,\"name\":null,\"since\":null}"));
    CHECK(contains(buf, "\"enabled\":null"));
    CHECK(contains(buf, "\"weekday\":null"));
    CHECK(contains(buf, "\"profiles\":[]}"));
}

static void test_activity_names_events(void)
{
    static char buf[4096];
    const SyncAppTime apps[] = { { 0x0100000000010000ULL, 3600 }, { 0x01000A10041EA000ULL, 840 }, { 0x1, 0 } };
    const SyncAccountTime accts[] = { { { 0x0011223344556677ULL, 0x8899AABBCCDDEEFFULL }, 4440 } };
    SyncActivity a = { "agent", 1791000000, "2026-10-05", false, apps, 3, NULL, 0, 0x0100000000010000ULL, 1790990000 };
    size_t n = sync_activity_build(&a, buf, sizeof(buf));
    CHECK(n > 0);
    CHECK(contains(buf, "\"total_s\":4440,\"total_min\":74"));
    CHECK(contains(buf, "{\"app_id\":\"0100000000010000\",\"s\":3600,\"min\":60}"));
    CHECK(contains(buf, "{\"app_id\":\"01000A10041EA000\",\"s\":840,\"min\":14}"));
    CHECK(!contains(buf, "\"0000000000000001\""));   // nothing played: left out
    CHECK(contains(buf, "\"per_account\":null"));
    CHECK(contains(buf, "\"now_playing\":{\"app_id\":\"0100000000010000\""));
    save("activity.json", buf);
    a.final = true;
    a.accounts = accts;
    a.n_accounts = 1;
    n = sync_activity_build(&a, buf, sizeof(buf));
    CHECK(contains(buf, "\"final\":true") && !contains(buf, "now_playing"));
    CHECK(contains(buf, "\"uid\":\"00112233445566778899AABBCCDDEEFF\",\"s\":4440,\"min\":74"));

    const SyncAppName names[] = { { 0x0100000000010000ULL, "Super Mario Odyssey" }, { 0x2, "" } };
    const SyncAccountName nick[] = { { { 1, 2 }, "Léa" } };
    n = sync_names_build(1791000000, names, 2, nick, 1, buf, sizeof(buf));
    CHECK(n > 0 && contains(buf, "\"0100000000010000\":\"Super Mario Odyssey\"") &&
           contains(buf, "\"00000000000000010000000000000002\":\"Léa\"") && !contains(buf, "0000000000000002\":\"\""));

    n = sync_event_build("agent", 1791000000, "command_rejected", "limit_mon", "5000", SyncReason_OutOfRange, false, 0,
                         buf, sizeof(buf));
    CHECK(n > 0 && contains(buf, "\"event_type\":\"command_rejected\",\"entity\":\"limit_mon\",\"payload\":\"5000\","
                                  "\"reason\":\"out_of_range\",\"rc\":null"));
    n = sync_event_build("app", 1, "command_applied", "lock_now", "PRESS", SyncReason_None, true, 0x1234, buf, sizeof(buf));
    CHECK(contains(buf, "\"reason\":null,\"rc\":\"0x00001234\""));
}

// The entity table as entities.json, for the Home Assistant integration's
// parity test (it carries a copy: JigSawFr/playguard-ha).
// The system document: every field, then nothing read (every key still
// there, null), and the serial number only when given.
static void test_system_document(void)
{
    SysInfo sys;
    memset(&sys, 0, sizeof(sys));
    sys.hos_version = MAKEHOSVERSION(23, 0, 1);
    sys.is_atmosphere = sys.ams_valid = true;
    sys.ams_major = 1, sys.ams_minor = 12, sys.ams_micro = 0;
    sys.emummc_valid = sys.emummc = true;
    sys.blank_valid = sys.blank = true;
    ConsoleLive l;
    console_live_init(&l);
    l.model = ConsoleModel_Oled;
    snprintf(l.nickname, sizeof(l.nickname), "Switch de L\xc3\xa9" "a");
    snprintf(l.region, sizeof(l.region), "eur");
    l.battery_pct = 87, l.charger = 1, l.docked = 0, l.temperature_c = 41;
    l.network = 1, l.wifi_bars = 3;
    snprintf(l.ip, sizeof(l.ip), "192.168.1.20");
    snprintf(l.ssid, sizeof(l.ssid), "Maison");
    l.uptime_s = 5 * 3600 + 17;
    const ConsoleSpace sd = { 21344, 59640 }, nand = { 18470, 26592 };
    SyncSystem s = { 1790990000, &sys, &l, &sd, &nand, "6.5.4", "1.2.0", "1.2.0", NULL };
    static char buf[4096];
    size_t n = sync_system_build(&s, buf, sizeof(buf));
    CHECK(n > 0);
    save("system.json", buf);
    CHECK(contains(buf, "\"versions\":{\"firmware\":\"23.0.1\",\"atmosphere\":\"1.12.0\",\"hekate\":\"6.5.4\",\"app\":\"1.2.0\",\"agent\":\"1.2.0\"}"));
    CHECK(contains(buf, "\"model\":\"oled\",\"nickname\":\"Switch de L\xc3\xa9" "a\",\"region\":\"eur\",\"system\":\"emummc\","
                        "\"prodinfo_blanked\":true,\"serial\":null"));
    CHECK(contains(buf, "\"storage\":{\"sd\":{\"free_mb\":21344,\"total_mb\":59640},\"nand\":{\"free_mb\":18470,\"total_mb\":26592}}"));
    CHECK(contains(buf, "\"power\":{\"battery_pct\":87,\"charging\":true,\"charger\":\"official\",\"docked\":false,\"temperature_c\":41}"));
    CHECK(contains(buf, "\"network\":{\"type\":\"wifi\",\"ip\":\"192.168.1.20\",\"wifi_bars\":3,\"ssid\":\"Maison\"}"));
    // Started 5 h ago, to the minute.
    CHECK(contains(buf, "\"boot_ts\":1790971980"));
    s.serial = "XAW10012345678";
    n = sync_system_build(&s, buf, sizeof(buf));
    CHECK(contains(buf, "\"serial\":\"XAW10012345678\""));
    // Handheld on a low-power charger: charging; no charger: not.
    l.charger = 2;
    n = sync_system_build(&s, buf, sizeof(buf));
    CHECK(contains(buf, "\"charging\":true,\"charger\":\"low_power\""));
    l.charger = 0;
    n = sync_system_build(&s, buf, sizeof(buf));
    CHECK(contains(buf, "\"charging\":false,\"charger\":\"none\""));
    CHECK(!sync_system_build(&s, buf, 64));   // too small: nothing

    SyncSystem empty;
    memset(&empty, 0, sizeof(empty));
    n = sync_system_build(&empty, buf, sizeof(buf));
    CHECK(n > 0);
    save("system_empty.json", buf);
    CHECK(contains(buf, "\"ts\":null") && contains(buf, "\"firmware\":null") && contains(buf, "\"model\":null"));
    CHECK(contains(buf, "\"sd\":{\"free_mb\":null,\"total_mb\":null}") && contains(buf, "\"boot_ts\":null"));
    CHECK(contains(buf, "\"battery_pct\":null,\"charging\":null,\"charger\":null,\"docked\":null,\"temperature_c\":null"));
    CHECK(contains(buf, "\"type\":null,\"ip\":null,\"wifi_bars\":null,\"ssid\":null"));
}

// The games document: the games most played first, and as many as fit.
static void test_games_document(void)
{
    static SyncGame games[200];
    static char names[200][40];
    for (size_t i = 0; i < 200; i++) {
        snprintf(names[i], sizeof(names[i]), "Game %zu \"%s\"", i, i % 2 ? "deluxe" : "edition");
        games[i] = (SyncGame){ 0x0100000000010000ULL + i * 0x1000, names[i], i < 150 ? "1.0.2" : NULL, i < 150,
                               (int64_t)(5000 - i * 20), (int64_t)(300 - i), 1700000000ULL + i, 1790000000ULL + i };
    }
    games[0].name = "Super Mario Odyssey";
    games[0].version = "1.3.0";
    SyncGames g = { 1790990000, 150, 3, 123456, games, 200 };
    static char buf[65536];
    size_t kept = 0;
    size_t n = sync_games_build(&g, buf, sizeof(buf), &kept);
    CHECK(n > 0 && kept == 200 && contains(buf, "\"complete\":true"));
    CHECK(contains(buf, "\"installed\":150,\"accounts\":3,\"total_min\":123456,\"games\":{\"0100000000010000\":"
                        "{\"name\":\"Super Mario Odyssey\",\"version\":\"1.3.0\",\"installed\":true,\"total_min\":5000,"
                        "\"launches\":300,\"first_played\":1700000000,\"last_played\":1790000000}"));
    CHECK(contains(buf, "\"Game 1 \\\"deluxe\\\"\""));
    CHECK(contains(buf, "\"version\":null,\"installed\":false"));
    save("games.json", buf);
    // 8 KiB: the most played only, said so.
    static char small[8192];
    n = sync_games_build(&g, small, sizeof(small), &kept);
    CHECK(n > 0 && n < sizeof(small) && kept > 20 && kept < 200 && contains(small, "\"complete\":false"));
    CHECK(contains(small, "\"0100000000010000\"") && !contains(small, "\"01000000000D7000\""));

    SyncGames none = { 0, -1, -1, -1, NULL, 0 };
    n = sync_games_build(&none, buf, sizeof(buf), NULL);
    CHECK(n > 0 && contains(buf, "\"ts\":null,\"installed\":null,\"accounts\":null,\"total_min\":null,\"games\":{},\"complete\":true"));
}

static void test_export_entities(void)
{
    static char buf[32768];
    size_t count = 0;
    const SyncEntity *e = sync_entities(&count);
    static const char *const WRITE[] = { "none", "free", "timer" };
    SyncJson j;
    sync_json_init(&j, buf, sizeof(buf));
    sync_json_obj(&j);
    sync_json_kint(&j, "schema", SYNC_SCHEMA);
    sync_json_key(&j, "entities");
    sync_json_arr(&j);
    for (size_t i = 0; i < count; i++) {
        sync_json_obj(&j);
        sync_json_kstr(&j, "id", e[i].id);
        sync_json_kstr(&j, "platform", e[i].platform);
        sync_json_kstr(&j, "write", WRITE[e[i].write]);
        sync_json_kstr(&j, "name", e[i].name);
        sync_json_kstr_or_null(&j, "value_template", e[i].value_template);
        sync_json_kstr(&j, "topic", e[i].topic ? e[i].topic : "state");
        sync_json_key(&j, "extra");
        sync_json_obj(&j);
        sync_json_members(&j, e[i].extra);
        sync_json_obj_end(&j);
        sync_json_obj_end(&j);
    }
    sync_json_arr_end(&j);
    sync_json_obj_end(&j);
    const size_t n = sync_json_end(&j);
    CHECK(n > 0 && contains(buf, "\"id\":\"limit_mon\",\"platform\":\"number\",\"write\":\"timer\""));
    save("entities.json", buf);
}

int main(int argc, char **argv)
{
    if (argc > 1) out_dir = argv[1];
    test_json();
    test_connect_encoding();
    test_publish_roundtrip();
    test_remaining_length();
    test_other_packets();
    test_mqtt5();
    test_topic_matching();
    test_conf();
    test_records();
    test_orders();
    test_entities_match_orders();
    test_discovery();
    test_state();
    test_activity_names_events();
    test_system_document();
    test_games_document();
    test_export_entities();
    return CHECK_DONE("sync_core: all tests passed");
}

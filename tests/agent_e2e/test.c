// The agent's link end to end on the host: the session engine and the relay
// service of the agent (what sysmodule/agent/source/main.c assembles on the
// console, without libnx) against a real MQTT broker (mosquitto), the broker
// seen and driven by its own command-line clients so that nothing but the broker
// is shared with the code under test.
//
// What it checks: the console's own documents and PlayGuard's on connection,
// a homebrew that relays (its session, its document, its order back), PlayGuard's
// orders taken, then not taken while its module is off (its documents cleared
// from the broker, `capabilities` without it), and taken again, retained order
// included, when it is back.
//
// Without mosquitto the suite says so and passes, unless REQUIRE_MOSQUITTO is
// set (CI sets it).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "agent_relay.h"
#include "bl_net.h"
#include "check.h"
#include "sync_conf.h"
#include "sync_engine.h"

#define ID   "a1b2c3d4"
#define BASE "boreale/" ID

static pid_t g_broker = -1;
static int g_port;
static SyncEngine g_engine;
static SyncConf g_conf;
static BlTcp g_tcp;
static AgentRelay g_relay;
static unsigned g_orders;            // PlayGuard's orders the engine handed to the host
static char g_last_order[64];

// ---- the broker ----

static bool have(const char *tool)
{
    char cmd[96];
    snprintf(cmd, sizeof(cmd), "command -v %s > /dev/null 2>&1", tool);
    return system(cmd) == 0;
}

static int free_port(void)
{
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK), .sin_port = 0 };
    socklen_t n = sizeof(a);
    if (fd < 0 || bind(fd, (struct sockaddr *)&a, sizeof(a)) != 0 || getsockname(fd, (struct sockaddr *)&a, &n) != 0) return 0;
    const int port = ntohs(a.sin_port);
    close(fd);
    return port;
}

static bool can_connect(int port)
{
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK), .sin_port = htons((uint16_t)port) };
    const bool ok = fd >= 0 && connect(fd, (struct sockaddr *)&a, sizeof(a)) == 0;
    if (fd >= 0) close(fd);
    return ok;
}

static bool start_broker(void)
{
    g_port = free_port();
    if (!g_port) return false;
    char conf[96];
    snprintf(conf, sizeof(conf), "/tmp/agent_e2e_%d.conf", (int)getpid());
    FILE *f = fopen(conf, "w");
    if (!f) return false;
    fprintf(f, "listener %d 127.0.0.1\nallow_anonymous true\npersistence false\n", g_port);
    fclose(f);
    char log[96];
    snprintf(log, sizeof(log), "/tmp/agent_e2e_%d.log", (int)getpid());
    g_broker = fork();
    if (g_broker == 0) {
        freopen("/dev/null", "r", stdin);
        freopen(log, "w", stdout);
        freopen(log, "w", stderr);
        execlp("mosquitto", "mosquitto", "-c", conf, (char *)NULL);
        _exit(127);
    }
    for (int i = 0; i < 100; i++) {
        if (can_connect(g_port)) return true;
        int status = 0;
        if (waitpid(g_broker, &status, WNOHANG) == g_broker) {   // gone before it listened: say why
            fprintf(stderr, "agent_e2e: the broker exited (status %d) on port %d, configuration:\n", status, g_port);
            FILE *c = fopen(conf, "r");
            for (int ch; c && (ch = fgetc(c)) != EOF;) fputc(ch, stderr);
            if (c) fclose(c);
            FILE *l = fopen(log, "r");
            fprintf(stderr, "its output:\n");
            for (int ch; l && (ch = fgetc(l)) != EOF;) fputc(ch, stderr);
            if (l) fclose(l);
            g_broker = -1;
            return false;
        }
        usleep(50000);
    }
    fprintf(stderr, "agent_e2e: nothing listens on port %d after 5 s\n", g_port);
    return false;
}

static void stop_broker(void)
{
    if (g_broker > 0) {
        kill(g_broker, SIGTERM);
        waitpid(g_broker, NULL, 0);
    }
    char path[96];
    snprintf(path, sizeof(path), "/tmp/agent_e2e_%d.conf", (int)getpid());
    remove(path);
    snprintf(path, sizeof(path), "/tmp/agent_e2e_%d.log", (int)getpid());
    remove(path);
}

// What the broker has retained under `filter` (one second of listening), one
// "topic|payload" per line. mosquitto_sub, not the code under test.
static char g_seen[8192];
static void retained(const char *filter)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mosquitto_sub -h 127.0.0.1 -p %d -t '%s' -W 1 -F '%%t|%%p' 2>/dev/null", g_port, filter);
    g_seen[0] = '\0';
    FILE *p = popen(cmd, "r");
    if (!p) return;
    size_t n = fread(g_seen, 1, sizeof(g_seen) - 1, p);
    g_seen[n] = '\0';
    pclose(p);
}

static bool seen(const char *needle)
{
    return strstr(g_seen, needle) != NULL;
}

static void put(const char *topic, const char *payload, bool keep)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mosquitto_pub -h 127.0.0.1 -p %d -q 1 %s -t '%s' -m '%s'", g_port, keep ? "-r" : "", topic, payload);
    CHECK(system(cmd) == 0);
}

// ---- what main.c does on the console, as far as the link goes ----

static size_t h_state(void *ctx, char *out, size_t cap)
{
    (void)ctx;
    return (size_t)snprintf(out, cap, "{\"schema\":1,\"controls\":{\"enabled\":true}}");
}

static size_t h_system(void *ctx, char *out, size_t cap)
{
    (void)ctx;
    return (size_t)snprintf(out, cap, "{\"schema\":1,\"platform\":\"switch\"}");
}

static bool h_order(void *ctx, uint32_t id, const SyncIntent *in, const char *entity, const char *payload, bool retained_order,
                    SyncOutcome *out)
{
    (void)ctx;
    (void)id;
    (void)in;
    (void)retained_order;
    g_orders++;
    snprintf(g_last_order, sizeof(g_last_order), "%s=%s", entity, payload);
    memset(out, 0, sizeof(*out));
    out->applied = true;
    return true;
}

static void h_relay(void *ctx, const char *rel, const void *payload, size_t len, bool retain)
{
    (void)ctx;
    agent_relay_deliver(&g_relay, rel, payload, len, retain);
}

static void engine_start(void)
{
    sync_conf_defaults(&g_conf);
    g_conf.enabled = true;
    snprintf(g_conf.host, sizeof(g_conf.host), "127.0.0.1");
    g_conf.port = (uint16_t)g_port;
    snprintf(g_conf.console_id, sizeof(g_conf.console_id), ID);
    g_conf.ha_discovery = false;
    g_conf.allow_anonymous = true;   // the test broker has no accounts
    g_conf.policy = SyncPolicy_Auto;
    g_tcp.fd = -1;
    const SyncHost host = { .state = h_state, .system = h_system, .order = h_order, .relay = h_relay };
    sync_engine_init(&g_engine, &g_conf, bl_tcp_io(&g_tcp), &host, bl_now_ms, "agent", "1.1.0");
}

// The loop of main.c: the relay's sessions and publications, then a turn of the engine.
static void pump(unsigned ms)
{
    const uint64_t until = bl_now_ms() + ms;
    do {
        SyncRelayModule open[AGENT_RELAY_SESSIONS];
        size_t n_open = 0;
        for (int i = 0; i < AGENT_RELAY_SESSIONS; i++)
            if (agent_relay_module_at(&g_relay, i, open[n_open].name, sizeof(open[n_open].name), open[n_open].version,
                                      sizeof(open[n_open].version)))
                n_open++;
        sync_engine_relay_sync(&g_engine, open, n_open);
        agent_relay_set_link(&g_relay, ID, sync_engine_online(&g_engine));
        for (int n = 0; n < 4 && sync_engine_online(&g_engine); n++) {
            char topic[BL_APP_MODULE_MAX + BL_RELAY_SUB_MAX];
            static char payload[BL_RELAY_PAYLOAD_MAX];
            size_t len = 0;
            bool keep = false;
            if (!agent_relay_take_publish(&g_relay, topic, sizeof(topic), payload, sizeof(payload), &len, &keep)) break;
            sync_engine_publish_relay(&g_engine, topic, payload, len, keep);
        }
        sync_engine_step(&g_engine, 20);
    } while (bl_now_ms() < until);
}

// ---- a homebrew's side of bl:link ----

static int g_slot = -1;

static bool shelf_hello(void)
{
    BlRelayHello h;
    memset(&h, 0, sizeof(h));
    h.protocol = BL_RELAY_PROTOCOL;
    snprintf(h.module, sizeof(h.module), "playshelf");
    snprintf(h.version, sizeof(h.version), "1.2.0");
    BlRelayHelloReply rep;
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.in = &h;
    c.in_len = sizeof(h);
    c.out = &rep;
    c.out_cap = sizeof(rep);
    return agent_relay_dispatch(&g_relay, &g_slot, BlRelayCmd_Hello, &c) == 0 && rep.accepted;
}

static bool shelf_publish(const char *sub, const char *doc)
{
    uint8_t buf[256];
    const size_t n = bl_relay_encode_publish(sub, doc, strlen(doc), true, buf, sizeof(buf));
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.buf = buf;
    c.buf_len = n;
    return n && agent_relay_dispatch(&g_relay, &g_slot, BlRelayCmd_Publish, &c) == 0;
}

// The order waiting for the session, "sub=payload"; "" when none.
static void shelf_pop(char *out, size_t cap)
{
    uint8_t buf[BL_RELAY_SUB_MAX + BL_RELAY_ORDER_MAX + 8];
    uint32_t len = 0;
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.buf_out = buf;
    c.buf_out_cap = sizeof(buf);
    c.out = &len;
    c.out_cap = sizeof(len);
    out[0] = '\0';
    if (agent_relay_dispatch(&g_relay, &g_slot, BlRelayCmd_PopOrder, &c) != 0 || len == 0) return;
    const char *sub = NULL;
    const void *payload = NULL;
    size_t plen = 0;
    bool keep = false;
    if (!bl_relay_decode_order(buf, len, &sub, &payload, &plen, &keep)) return;
    snprintf(out, cap, "%s=%.*s", sub, (int)plen, (const char *)payload);
}

static void test_link(void)
{
    agent_relay_init(&g_relay);
    engine_start();
    pump(1500);
    if (!CHECK(sync_engine_online(&g_engine))) {
        const SyncStatus *st = sync_engine_status(&g_engine);
        fprintf(stderr, "agent_e2e: not online (state %d, error \"%s\", connects %u)\n", (int)st->state, st->error, (unsigned)st->connects);
    }

    // The console's own documents and PlayGuard's, as the broker keeps them.
    retained(BASE "/#");
    CHECK(seen(BASE "/availability|online"));
    CHECK(seen(BASE "/system|{\"schema\":1"));
    CHECK(seen(BASE "/capabilities|") && seen("\"playguard\":{\"schema\":1"));
    CHECK(seen(BASE "/playguard/state|{\"schema\":1"));

    // A homebrew relays: its session lists it, its document goes out under its level.
    CHECK(shelf_hello());
    CHECK(shelf_publish("state", "{\"schema\":1,\"downloads\":{\"pending\":2}}"));
    pump(800);
    retained(BASE "/playshelf/#");
    CHECK(seen(BASE "/playshelf/state|{\"schema\":1,\"downloads\":{\"pending\":2}}"));
    retained(BASE "/capabilities");
    CHECK(seen("\"playshelf\":{\"version\":\"1.2.0\"}"));

    // Its order comes back to it, and only to it.
    put(BASE "/playshelf/pause_downloads/set", "PRESS", false);
    pump(800);
    char order[96];
    shelf_pop(order, sizeof(order));
    CHECK(!strcmp(order, "pause_downloads/set=PRESS"));
    shelf_pop(order, sizeof(order));
    CHECK(order[0] == '\0' && g_orders == 0);

    // PlayGuard's order: taken, answered, and the retained order cleared.
    put(BASE "/playguard/vr_restricted/set", "ON", true);
    pump(1000);
    CHECK(g_orders == 1 && !strcmp(g_last_order, "vr_restricted=ON"));
    retained(BASE "/playguard/vr_restricted/set");
    CHECK(g_seen[0] == '\0');

    // Its module goes off: what it published leaves the broker, `capabilities` drops it, its orders wait.
    sync_engine_set_module(&g_engine, false);
    pump(1000);
    retained(BASE "/playguard/#");
    CHECK(!seen(BASE "/playguard/state|") && !seen(BASE "/playguard/activity|"));
    retained(BASE "/capabilities");
    CHECK(seen("\"playshelf\"") && !seen("\"playguard\""));
    retained(BASE "/system");
    CHECK(seen("{\"schema\":1"));   // the console's own document stays
    put(BASE "/playguard/vr_restricted/set", "OFF", true);
    pump(1000);
    CHECK(g_orders == 1);
    retained(BASE "/playguard/vr_restricted/set");
    CHECK(seen("|OFF"));            // kept by the broker for when it is back

    // Back on: published again, and the order it kept is taken.
    sync_engine_set_module(&g_engine, true);
    pump(1500);
    CHECK(g_orders == 2 && !strcmp(g_last_order, "vr_restricted=OFF"));
    retained(BASE "/playguard/state");
    CHECK(seen("{\"schema\":1"));
    retained(BASE "/capabilities");
    CHECK(seen("\"playguard\":{\"schema\":1") && seen("\"playshelf\""));

    // The homebrew leaves: out of `capabilities`, its document stays retained.
    agent_relay_closed(&g_relay, g_slot);
    pump(800);
    retained(BASE "/capabilities");
    CHECK(!seen("\"playshelf\""));
    retained(BASE "/playshelf/state");
    CHECK(seen("\"pending\":2"));

    sync_engine_stop(&g_engine);
}

int main(void)
{
    if (!have("mosquitto") || !have("mosquitto_sub") || !have("mosquitto_pub")) {
        if (getenv("REQUIRE_MOSQUITTO")) {
            fprintf(stderr, "agent_e2e: mosquitto and mosquitto-clients are required (REQUIRE_MOSQUITTO is set)\n");
            return 1;
        }
        printf("agent_e2e: skipped, mosquitto is not installed\n");
        return 0;
    }
    signal(SIGPIPE, SIG_IGN);
    if (!start_broker()) {
        fprintf(stderr, "agent_e2e: the broker did not start\n");
        stop_broker();
        return 1;
    }
    test_link();
    stop_broker();
    return CHECK_DONE("agent_e2e: the agent's link against a real broker passed");
}

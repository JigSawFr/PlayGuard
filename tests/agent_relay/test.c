// Host tests for sysmodule/agent/source/agent_relay.c: the bl:link sessions
// (Hello and what refuses it, one session per module, room for two), what a
// module may publish and the newest document replacing the one waiting, the
// orders handed to the right session only, and a session closing.
#include "check.h"
#include <stdio.h>
#include <string.h>

#include "agent_relay.h"

static AgentRelay R;

static uint32_t hello(int *slot, const char *module, uint32_t protocol, BlRelayHelloReply *reply)
{
    BlRelayHello h;
    memset(&h, 0, sizeof(h));
    h.protocol = protocol;
    snprintf(h.module, sizeof(h.module), "%s", module);
    snprintf(h.version, sizeof(h.version), "1.0.0");
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.in = &h;
    c.in_len = sizeof(h);
    c.out = reply;
    c.out_cap = sizeof(*reply);
    return agent_relay_dispatch(&R, slot, BlRelayCmd_Hello, &c);
}

static uint32_t publish(int *slot, bool retained, const char *sub, const char *payload, size_t payload_len)
{
    uint8_t buf[BL_RELAY_PAYLOAD_MAX + BL_RELAY_SUB_MAX + 8];
    size_t n = 0;
    buf[n++] = retained ? 1 : 0;
    memcpy(buf + n, sub, strlen(sub) + 1);
    n += strlen(sub) + 1;
    memcpy(buf + n, payload, payload_len);
    n += payload_len;
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.buf = buf;
    c.buf_len = n;
    return agent_relay_dispatch(&R, slot, BlRelayCmd_Publish, &c);
}

static uint32_t pop(int *slot, uint8_t *buf, size_t cap, size_t *len)
{
    uint32_t n = 99;
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.buf_out = buf;
    c.buf_out_cap = cap;
    c.out = &n;
    c.out_cap = sizeof(n);
    const uint32_t rc = agent_relay_dispatch(&R, slot, BlRelayCmd_PopOrder, &c);
    *len = rc == 0 && c.out_len == sizeof(n) ? n : (size_t)-1;   // the length comes in the reply
    return rc;
}

static uint32_t simple(int *slot, uint32_t cmd, void *out, size_t cap)
{
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.out = out;
    c.out_cap = cap;
    return agent_relay_dispatch(&R, slot, cmd, &c);
}

static void test_hello(void)
{
    agent_relay_init(&R);
    agent_relay_set_link(&R, "a1b2c3d4", true);
    int a = -1, b = -1, c3 = -1;
    BlRelayHelloReply rep;

    CHECK(hello(&a, "playshelf", BL_RELAY_PROTOCOL, &rep) == 0 && rep.accepted && a == 0 && rep.online);
    CHECK(rep.protocol == BL_RELAY_PROTOCOL && !strcmp(rep.console_id, "a1b2c3d4"));
    CHECK(agent_relay_is_open(&R, "playshelf") && !agent_relay_is_open(&R, "playguard"));

    // Refused with success (the reply must be delivered): protocol, name, taken module.
    CHECK(hello(&b, "other", BL_RELAY_PROTOCOL + 1, &rep) == 0 && !rep.accepted && b == -1);
    CHECK(hello(&b, "system", BL_RELAY_PROTOCOL, &rep) == 0 && !rep.accepted && b == -1);
    CHECK(hello(&b, "playguard", BL_RELAY_PROTOCOL, &rep) == 0 && !rep.accepted && b == -1);   // the app's own level
    CHECK(hello(&b, "Bad Name", BL_RELAY_PROTOCOL, &rep) == 0 && !rep.accepted);
    CHECK(hello(&b, "playshelf", BL_RELAY_PROTOCOL, &rep) == 0 && !rep.accepted && b == -1);   // one session per module
    CHECK(hello(&a, "again", BL_RELAY_PROTOCOL, &rep) == 0 && !rep.accepted && a == 0);       // a session says Hello once

    CHECK(hello(&b, "second", BL_RELAY_PROTOCOL, &rep) == 0 && rep.accepted && b == 1);
    char name[BL_APP_MODULE_MAX];
    CHECK(agent_relay_module_at(&R, 0, name, sizeof(name)) && !strcmp(name, "playshelf"));
    CHECK(agent_relay_module_at(&R, 1, name, sizeof(name)) && !strcmp(name, "second"));
    CHECK(!agent_relay_module_at(&R, 2, name, sizeof(name)) && !agent_relay_module_at(&R, -1, name, sizeof(name)));
    CHECK(!agent_relay_module_at(&R, 0, name, 4));   // too small
    CHECK(hello(&c3, "third", BL_RELAY_PROTOCOL, &rep) == 0 && !rep.accepted && c3 == -1);     // no room

    // A reply buffer too small is a bad input; nothing opened.
    BlRelayHelloReply small;
    AgentCall c;
    memset(&c, 0, sizeof(c));
    BlRelayHello h = { BL_RELAY_PROTOCOL, "late", "1", {0} };
    c.in = &h;
    c.in_len = sizeof(h);
    c.out = &small;
    c.out_cap = sizeof(small) - 1;
    int s = -1;
    CHECK(agent_relay_dispatch(&R, &s, BlRelayCmd_Hello, &c) == AGENT_RC_BAD_INPUT && s == -1);
}

static void test_no_session(void)
{
    agent_relay_init(&R);
    int none = -1;
    uint8_t buf[600];
    size_t len;
    CHECK(publish(&none, false, "state", "{}", 2) == AGENT_RC_NO_SESSION);
    CHECK(pop(&none, buf, sizeof(buf), &len) == AGENT_RC_NO_SESSION);
    CHECK(simple(&none, BlRelayCmd_Status, buf, sizeof(buf)) == AGENT_RC_NO_SESSION);
    int stale = 1;   // a slot nobody opened
    CHECK(simple(&stale, BlRelayCmd_Close, NULL, 0) == AGENT_RC_NO_SESSION);
    CHECK(simple(&stale, 99, NULL, 0) == AGENT_RC_NO_SESSION);
    int bogus = 77;
    CHECK(simple(&bogus, BlRelayCmd_Status, buf, sizeof(buf)) == AGENT_RC_NO_SESSION);
}

static void test_publish(void)
{
    agent_relay_init(&R);
    agent_relay_set_link(&R, "a1b2c3d4", true);
    int a = -1;
    BlRelayHelloReply rep;
    hello(&a, "playshelf", BL_RELAY_PROTOCOL, &rep);

    CHECK(publish(&a, true, "state", "{\"n\":1}", 7) == 0);
    CHECK(publish(&a, false, "downloads/12", "x", 1) == 0);
    CHECK(publish(&a, true, "state", "{\"n\":2}", 7) == 0);   // replaces the first
    BlRelayStatus st;
    CHECK(simple(&a, BlRelayCmd_Status, &st, sizeof(st)) == 0 && st.outbox == 2 && st.online == 1);

    char topic[96], payload[BL_RELAY_PAYLOAD_MAX];
    size_t n;
    bool retained;
    CHECK(agent_relay_take_publish(&R, topic, sizeof(topic), payload, sizeof(payload), &n, &retained));
    CHECK(!strcmp(topic, "playshelf/state") && n == 7 && !memcmp(payload, "{\"n\":2}", 7) && retained);
    CHECK(agent_relay_take_publish(&R, topic, sizeof(topic), payload, sizeof(payload), &n, &retained));
    CHECK(!strcmp(topic, "playshelf/downloads/12") && n == 1 && !retained);
    CHECK(!agent_relay_take_publish(&R, topic, sizeof(topic), payload, sizeof(payload), &n, &retained));

    // An empty retained payload clears a topic: allowed.
    CHECK(publish(&a, true, "state", "", 0) == 0);
    CHECK(agent_relay_take_publish(&R, topic, sizeof(topic), payload, sizeof(payload), &n, &retained) && n == 0 && retained);

    // Topics a module may not name, and what is malformed.
    CHECK(publish(&a, false, "../playguard/state", "x", 1) == AGENT_RC_BAD_INPUT);
    CHECK(publish(&a, false, "a/+", "x", 1) == AGENT_RC_BAD_INPUT);
    CHECK(publish(&a, false, "", "x", 1) == AGENT_RC_BAD_INPUT);
    uint8_t unterminated[] = { 0, 's', 't', 'a', 't', 'e' };
    AgentCall c;
    memset(&c, 0, sizeof(c));
    c.buf = unterminated;
    c.buf_len = sizeof(unterminated);
    CHECK(agent_relay_dispatch(&R, &a, BlRelayCmd_Publish, &c) == AGENT_RC_BAD_INPUT);
    c.buf_len = 2;
    CHECK(agent_relay_dispatch(&R, &a, BlRelayCmd_Publish, &c) == AGENT_RC_BAD_INPUT);
    c.buf = NULL;
    CHECK(agent_relay_dispatch(&R, &a, BlRelayCmd_Publish, &c) == AGENT_RC_BAD_INPUT);

    // Size limits and room.
    static char big[BL_RELAY_PAYLOAD_MAX + 1];
    memset(big, 'x', sizeof(big));
    CHECK(publish(&a, false, "big", big, BL_RELAY_PAYLOAD_MAX) == 0);
    CHECK(publish(&a, false, "big", big, BL_RELAY_PAYLOAD_MAX + 1) == AGENT_RC_TOO_LARGE);
    CHECK(publish(&a, false, "t1", "1", 1) == 0 && publish(&a, false, "t2", "1", 1) == 0 &&
          publish(&a, false, "t3", "1", 1) == 0);
    CHECK(publish(&a, false, "t4", "1", 1) == AGENT_RC_FULL);
    CHECK(publish(&a, false, "t3", "2", 1) == 0);   // replacing needs no room

    // A buffer too small keeps the publication.
    char tiny[4];
    CHECK(!agent_relay_take_publish(&R, topic, sizeof(topic), tiny, 2, &n, &retained) || n <= 2);
}

static void test_orders(void)
{
    agent_relay_init(&R);
    int a = -1, b = -1;
    BlRelayHelloReply rep;
    hello(&a, "playshelf", BL_RELAY_PROTOCOL, &rep);
    hello(&b, "other", BL_RELAY_PROTOCOL, &rep);

    CHECK(agent_relay_deliver(&R, "playshelf/download/set", "go", 2, true));
    CHECK(agent_relay_deliver(&R, "other/x/set", "y", 1, false));
    CHECK(!agent_relay_deliver(&R, "nobody/x/set", "y", 1, false));       // no session
    CHECK(!agent_relay_deliver(&R, "playguard/limit/set", "y", 1, false)); // not relayed
    CHECK(!agent_relay_deliver(&R, "playshelf/../x", "y", 1, false) && !agent_relay_deliver(&R, "playshelf", "y", 1, false));

    uint8_t buf[1 + BL_RELAY_SUB_MAX + BL_RELAY_ORDER_MAX];
    size_t len;
    CHECK(pop(&a, buf, sizeof(buf), &len) == 0 && len == 1 + strlen("download/set") + 1 + 2);
    CHECK(buf[0] == 1 && !strcmp((char *)buf + 1, "download/set") && !memcmp(buf + 1 + 13, "go", 2));
    CHECK(pop(&a, buf, sizeof(buf), &len) == 0 && len == 0);        // none left for playshelf
    CHECK(pop(&b, buf, sizeof(buf), &len) == 0 && len > 0 && buf[0] == 0 && !strcmp((char *)buf + 1, "x/set"));
    CHECK(pop(&a, buf, 10, &len) == AGENT_RC_BAD_INPUT);            // buffer too small for any order
    AgentCall nolen;                                                 // no room for the length in the reply
    memset(&nolen, 0, sizeof(nolen));
    nolen.buf_out = buf;
    nolen.buf_out_cap = sizeof(buf);
    CHECK(agent_relay_dispatch(&R, &a, BlRelayCmd_PopOrder, &nolen) == AGENT_RC_BAD_INPUT);

    // A full queue refuses and counts.
    for (int i = 0; i < BL_RELAY_QUEUE; i++) CHECK(agent_relay_deliver(&R, "playshelf/a/set", "1", 1, false));
    CHECK(!agent_relay_deliver(&R, "playshelf/a/set", "1", 1, false));
    BlRelayStatus st;
    CHECK(simple(&a, BlRelayCmd_Status, &st, sizeof(st)) == 0 && st.orders == BL_RELAY_QUEUE && st.refused == 1);
}

static void test_close(void)
{
    agent_relay_init(&R);
    int a = -1, b = -1;
    BlRelayHelloReply rep;
    hello(&a, "playshelf", BL_RELAY_PROTOCOL, &rep);
    publish(&a, false, "state", "x", 1);
    agent_relay_deliver(&R, "playshelf/a/set", "1", 1, false);

    CHECK(simple(&a, BlRelayCmd_Close, NULL, 0) == 0 && a == -1);
    CHECK(!agent_relay_is_open(&R, "playshelf"));
    char topic[96], payload[16];
    size_t n;
    bool retained;
    CHECK(!agent_relay_take_publish(&R, topic, sizeof(topic), payload, sizeof(payload), &n, &retained));   // gone with it
    CHECK(!agent_relay_deliver(&R, "playshelf/a/set", "1", 1, false));

    // The module may come back, to a clean session; a crash closes by slot.
    CHECK(hello(&b, "playshelf", BL_RELAY_PROTOCOL, &rep) == 0 && rep.accepted && b == 0);
    BlRelayStatus st;
    CHECK(simple(&b, BlRelayCmd_Status, &st, sizeof(st)) == 0 && st.orders == 0 && st.outbox == 0);
    agent_relay_closed(&R, b);
    agent_relay_closed(&R, b);      // twice, and an unknown slot: nothing
    agent_relay_closed(&R, -1);
    agent_relay_closed(&R, 99);
    CHECK(!agent_relay_is_open(&R, "playshelf"));
}

int main(void)
{
    test_hello();
    test_no_session();
    test_publish();
    test_orders();
    test_close();
    return CHECK_DONE("agent_relay: sessions, publications, orders and closing assertions passed");
}

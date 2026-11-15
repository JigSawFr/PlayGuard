// agent_relay — the bl:link service's side that does not touch the console:
// the sessions of the homebrews that relay through the agent, what they ask
// it to publish, and the orders it hands back. The IPC thread (agent_ipc.c)
// parses a request and calls agent_relay_dispatch() with the shared lock
// held; the main loop takes the publications and delivers the orders the
// broker brings, under the same lock. No libnx: host-tested
// (tests/agent_relay). The rules of what a module may publish are
// boreale-link's bl_relay.
//
// One session per module, a few at a time. A document asked for twice
// before the main loop took it replaces the first (the newest state wins).
// The relay moves bytes and adds no right to the agent.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include "agent_core.h"
#include "bl_app.h"
#include "bl_relay.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AGENT_RELAY_SESSIONS 2
#define AGENT_RELAY_OUTBOX   4    // publications waiting, per session

// Wire format of the relay's structures (the same compiler on both sides),
// checked at build time: a change raises BL_RELAY_PROTOCOL in boreale-link.
#define AGENT_RELAY_SIZE_HELLO       64
#define AGENT_RELAY_SIZE_HELLO_REPLY 16
#define AGENT_RELAY_SIZE_STATUS      8

typedef struct {
    uint32_t protocol;
    char     module[BL_APP_MODULE_MAX];
    char     version[AGENT_VERSION_MAX];     // the homebrew's
    uint8_t  pad[3];
} RelayHello;

// Always answered with success; `accepted` says whether the session opened
// (another protocol, a module that may not relay, one already open, or no
// room: 0, and the homebrew keeps its own connection).
typedef struct {
    uint32_t protocol;
    char     console_id[SYNC_ID_LEN + 1];
    uint8_t  online;
    uint8_t  accepted;
    uint8_t  pad[1];
} RelayHelloReply;

typedef struct {
    uint32_t refused;      // orders refused, the queue being full
    uint8_t  online;
    uint8_t  orders;       // waiting for the homebrew
    uint8_t  outbox;       // publications waiting for the main loop
    uint8_t  pad;
} RelayStatus;

#ifdef __cplusplus
#define AGENT_RELAY_SIZE_CHECK(type, size) static_assert(sizeof(type) == (size), #type ": bump BL_RELAY_PROTOCOL")
#else
#define AGENT_RELAY_SIZE_CHECK(type, size) _Static_assert(sizeof(type) == (size), #type ": bump BL_RELAY_PROTOCOL")
#endif
AGENT_RELAY_SIZE_CHECK(RelayHello, AGENT_RELAY_SIZE_HELLO);
AGENT_RELAY_SIZE_CHECK(RelayHelloReply, AGENT_RELAY_SIZE_HELLO_REPLY);
AGENT_RELAY_SIZE_CHECK(RelayStatus, AGENT_RELAY_SIZE_STATUS);
#undef AGENT_RELAY_SIZE_CHECK

typedef struct {
    bool     used, retained;
    uint16_t sub_len;
    size_t   len;
    char     sub[BL_RELAY_SUB_MAX];
    char     payload[BL_RELAY_PAYLOAD_MAX];
} AgentRelayOut;

typedef struct {
    bool          open;
    char          module[BL_APP_MODULE_MAX];
    char          version[AGENT_VERSION_MAX];
    BlRelayQueue  orders;
    AgentRelayOut out[AGENT_RELAY_OUTBOX];
} AgentRelaySession;

typedef struct {
    AgentRelaySession sessions[AGENT_RELAY_SESSIONS];
    char              console_id[SYNC_ID_LEN + 1];
    bool              online;     // written by the main loop
} AgentRelay;

void agent_relay_init(AgentRelay *r);
// The main loop: the console's id and whether the broker connection is up.
void agent_relay_set_link(AgentRelay *r, const char *console_id, bool online);

// A bl:link request on session `*slot` (-1: none yet; Hello sets it). The
// publication buffer is: one byte of flags (bit 0: retained), the topic
// below the module's level, a NUL, then the payload; an order comes back in
// the same shape. Returns 0 or an AGENT_RC_*.
uint32_t agent_relay_dispatch(AgentRelay *r, int *slot, uint32_t cmd, AgentCall *c);
// The session ended (Close, exit or crash): its orders and publications go.
void agent_relay_closed(AgentRelay *r, int slot);

// The main loop's side. Moves the oldest publication of any session into
// `topic` (`<module>/<sub>`, to append to the base) and `payload`; false when
// none is waiting or a buffer is too small (the publication is kept).
bool agent_relay_take_publish(AgentRelay *r, char *topic, size_t topic_cap, void *payload, size_t payload_cap,
                              size_t *payload_len, bool *retained);
// An order the broker brought on `<module>/<sub>` (below the base): handed to
// the session of that module. False: no such session, or refused (full).
bool agent_relay_deliver(AgentRelay *r, const char *rel_topic, const void *payload, size_t len, bool retained);
// Whether a session of `module` is open (to say so in `capabilities`).
bool agent_relay_is_open(const AgentRelay *r, const char *module);

#ifdef __cplusplus
}
#endif

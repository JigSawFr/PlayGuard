// sync_icons_logic — which games' icons the remote link publishes
// (docs/sync-protocol.md, `icon/<app id>`) and what is left to send or clear,
// without borealis, the console or the network (action/sync_icons does
// those). Every game installed or played, each sent once: what was sent, and
// to which broker and console, is kept in nand/<system>/sync/icons.txt so a
// new run sends only what changed.
// Plain C++ so the host tests can run it (tests/sync_icons_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace sync_icons_logic
{

// Icons on the broker at most (as many as PlayGuard's icon cache on the SD
// card keeps): up to 128 KiB each (the control data's JPEG).
constexpr size_t ICONS_MAX = 512;

// A game of the play log.
struct Played
{
    uint64_t app_id      = 0;
    uint64_t last_played = 0;   // POSIX, 0 unknown
    uint32_t week_s      = 0;   // played over the last 7 days
};

// The games whose icons are published: every game played (those of the last
// 7 days first, then the most recently played) and every one `installed`
// (in its order), `max` at most, the first ones kept.
std::vector<uint64_t> wanted(std::vector<Played> games, const std::vector<uint64_t>& installed,
                             size_t max = ICONS_MAX);

// What the broker holds, as this console last published it.
struct Sent
{
    std::string target;                   // target_of() of the broker and topics they went to
    std::map<uint64_t, uint32_t> icons;   // app id -> bytes published (0: it has none to publish)
};

// Which broker and which topics: the icons published to another one (another
// host, port, topic prefix or console id) are sent again. A hash, so the
// file does not name the broker.
std::string target_of(const std::string& host, uint16_t port, const std::string& base);

struct Plan
{
    std::vector<uint64_t> send;    // read and published (wanted's order)
    std::vector<uint64_t> clear;   // no longer wanted: an empty retained message
    bool empty() const { return send.empty() && clear.empty(); }
};

// What to do so the broker holds `want`'s icons and no others. Sent to
// another target: every wanted one again, none cleared (those are on the
// other broker, or under the other topics).
Plan plan(const std::vector<uint64_t>& want, const Sent& sent, const std::string& target);

// One published (`bytes`, 0 when the game has none) or cleared (`cleared`).
void record(Sent& sent, const std::string& target, uint64_t app_id, uint32_t bytes, bool cleared);

// What the agent published itself (its agent_icons.txt), into `sent` when it
// went to `target`: those are on the broker too. True when one was added.
bool merge(Sent& sent, const Sent& agent, const std::string& target);

// icons.txt: "target=<hash>", then one "0100000000010000=<bytes>" line per icon sent.
std::string text(const Sent& sent);
// What the file says; lines it does not understand are ignored.
Sent parse(const std::string& text);

// The topic under <prefix>/<id>: "icon/0100000000010000".
std::string topic(uint64_t app_id);

}   // namespace sync_icons_logic

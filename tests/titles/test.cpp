// Host tests for source/util/titles.cpp: the record of the titles that are
// not games, and their names, kept after a forwarder is deleted.
#include "check.h"

#include "util/titles.hpp"

using namespace titles;

int main()
{
    const uint64_t fwd = 0x050537E2542E2000ULL, game = 0x0100000000010000ULL, nsp = 0x0100ABCDEF012000ULL;
    Record r;
    // From the ID: a forwarder is a homebrew, a game a game, the HOME menu the console's own.
    CHECK(kind(r, fwd) == SyncTitle_Homebrew && kind(r, game) == SyncTitle_Game);
    CHECK(kind(r, 0x0100000000001000ULL) == SyncTitle_System);
    // A forwarder seen with its name: kept; a game is not.
    CHECK(note(r, fwd, SyncTitle_Homebrew, "sphaira"));
    CHECK(!note(r, game, SyncTitle_Game, "Super Mario Odyssey"));
    CHECK(!note(r, 0x0100000000001000ULL, SyncTitle_System, ""));
    CHECK(!note(r, 0, SyncTitle_Homebrew, "x"));
    CHECK(r.size() == 1 && name(r, fwd) == "sphaira" && name(r, game).empty());
    // Seen again without a name (deleted since): the name stays; a newer one replaces it.
    CHECK(!note(r, fwd, SyncTitle_Homebrew, ""));
    CHECK(note(r, fwd, SyncTitle_Homebrew, "sphaira 2"));
    CHECK(!note(r, fwd, SyncTitle_Homebrew, "sphaira 2"));
    // A homebrew under a game's ID (a forwarder made here): only the record knows.
    CHECK(note(r, nsp, SyncTitle_Homebrew, "Line\nbreak"));
    CHECK(kind(r, nsp) == SyncTitle_Homebrew && name(r, nsp) == "Line break");

    // PlayGuard's own forwarder: told apart, in both files.
    const uint64_t self = 0x05446530ACA7E000ULL;
    CHECK(note(r, self, SyncTitle_PlayGuard, "PlayGuard"));
    CHECK(kind(r, self) == SyncTitle_PlayGuard);
    CHECK(text(r).find("05446530ACA7E000=playguard:PlayGuard\n") != std::string::npos);
    CHECK(kind(parse(text(r)), self) == SyncTitle_PlayGuard);
    CHECK(agent_text(r).find("05446530ACA7E000=playguard\n") != std::string::npos);
    r.erase(self);

    // The files: the record with its names, the agent's without.
    const std::string t = text(r);
    CHECK(t.find("050537E2542E2000=homebrew:sphaira 2\n") != std::string::npos);
    const Record back = parse(t);
    CHECK(back.size() == 2 && name(back, fwd) == "sphaira 2" && kind(back, nsp) == SyncTitle_Homebrew);
    const std::string a = agent_text(r);
    CHECK(a.find("0100ABCDEF012000=homebrew\n") != std::string::npos && a.find("sphaira") == std::string::npos);
    SyncTitles kinds;
    sync_titles_parse(&kinds, a.data(), a.size());
    CHECK(kinds.n == 2 && sync_titles_kind(&kinds, nsp) == SyncTitle_Homebrew);
    // A name with a colon; lines it does not understand; a kind a newer
    // version wrote, kept as not a game (a homebrew), with its name.
    const Record odd = parse("052EB87E6426D000=homebrew:Tool: the sequel\r\nnonsense\n0100000000010000=what:x\n");
    CHECK(odd.size() == 2 && name(odd, 0x052EB87E6426D000ULL) == "Tool: the sequel");
    CHECK(kind(odd, 0x0100000000010000ULL) == SyncTitle_Homebrew && name(odd, 0x0100000000010000ULL) == "x");
    // Bounded.
    Record many;
    for (uint64_t i = 1; i <= MAX + 10; i++) note(many, 0x0500000000000000ULL + (i << 12), SyncTitle_Homebrew, "x");
    CHECK(many.size() == MAX);
    return CHECK_DONE("titles: all tests passed");
}

// Host tests for source/util/backup.cpp: the JSON format, the validation that
// keeps a damaged file away from the console, save / list / load, and what a
// restore would write.
#include "check.h"
#include "temp_dir.hpp"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

#include "util/backup.hpp"
#include "util/app_paths.hpp"
#include "util/paths.hpp"

static backup::Snapshot full()
{
    backup::Snapshot s;
    s.created  = "2026-10-07 18:30";
    s.firmware = "23.0.1";
    s.level          = 1;
    s.custom         = backup::Custom{ 12, true, false };
    s.vr_restricted  = true;
    s.days           = std::array<uint16_t, 7>{ 180, 120, 120, 0xFFFF, 120, 0, 1440 };
    s.alarm_disabled = true;
    s.rating_org     = 6;
    s.raw_block      = std::string(4, '0') + "0101" + std::string(128, 'A');
    return s;
}

static bool same(const backup::Snapshot& a, const backup::Snapshot& b)
{
    return a.created == b.created && a.firmware == b.firmware && a.level == b.level && a.custom == b.custom &&
           a.vr_restricted == b.vr_restricted && a.days == b.days && a.alarm_disabled == b.alarm_disabled &&
           a.rating_org == b.rating_org && a.raw_block == b.raw_block;
}

static bool parses(const std::string& text)
{
    backup::Snapshot s;
    return backup::from_json(text, s);
}

// A valid file with one fragment replaced, to check each value is validated.
static std::string with(const std::string& from, const std::string& to)
{
    std::string text = backup::to_json(full());
    const size_t at = text.find(from);
    CHECK(at != std::string::npos);
    return text.replace(at, from.size(), to);
}

static void test_round_trip()
{
    backup::Snapshot in = full(), out;
    const std::string text = backup::to_json(in);
    CHECK(text.find("\"days_from_sunday\"") != std::string::npos);
    CHECK(text.find("null") != std::string::npos);   // no limit on Wednesday
    CHECK(text.find("pin") == std::string::npos);
    CHECK(backup::from_json(text, out) && same(in, out));

    // Values that could not be read are left out, and stay unset when read back.
    in = backup::Snapshot();
    in.vr_restricted = false;
    const std::string partial = backup::to_json(in);
    CHECK(partial.find("\"level\"") == std::string::npos && partial.find("play_timer") == std::string::npos);
    out = full();
    CHECK(backup::from_json(partial, out));
    CHECK(out.vr_restricted == false && !out.level && !out.custom && !out.days);

    // Nothing at all to restore: refused.
    CHECK(!parses(backup::to_json(backup::Snapshot())));
}

static void test_validation()
{
    CHECK(!parses(""));
    CHECK(!parses("not json"));
    CHECK(!parses("[]"));
    CHECK(!parses("{\"restrictions\":{\"level\":3}}"));                 // no format
    CHECK(!parses("{\"format\":2,\"restrictions\":{\"level\":3}}"));    // unknown format
    CHECK(parses("{\"format\":1,\"restrictions\":{\"level\":3}}"));

    CHECK(!parses(with("\"level\": 1", "\"level\": 5")));
    CHECK(!parses(with("\"level\": 1", "\"level\": -1")));
    CHECK(!parses(with("\"level\": 1", "\"level\": 1.5")));
    CHECK(!parses(with("\"level\": 1", "\"level\": \"1\"")));
    CHECK(!parses(with("\"rating_age\": 12", "\"rating_age\": 22")));
    CHECK(!parses(with("\"rating_age\": 12,", "")));                    // custom settings incomplete
    CHECK(!parses(with("\"sns_post_restricted\": true", "\"sns_post_restricted\": 1")));
    CHECK(!parses(with("\"vr_restricted\": true", "\"vr_restricted\": \"yes\"")));
    CHECK(!parses(with("180", "1441")));
    CHECK(!parses(with("180", "-5")));
    CHECK(!parses(with("180,", "")));                                   // six days
    CHECK(!parses(with("\"days_from_sunday\"", "\"days\"")));
    CHECK(parses(with("180", "0")));

    // The fields kept for the record are validated too.
    CHECK(!parses(with("\"alarm_disabled\": true", "\"alarm_disabled\": 1")));
    CHECK(!parses(with("\"rating_organization\": 6", "\"rating_organization\": 13")));
    CHECK(!parses(with("\"raw_0x44\": \"0000", "\"raw_0x44\": \"zz00")));
    CHECK(!parses(with("\"raw_0x44\": \"0000", "\"raw_0x44\": \"00")));       // 134 digits
    CHECK(parses(with("\"raw_0x44\": \"0000", "\"raw_0x44\": \"abcd")));      // either case

    // A backup from before these fields still reads.
    backup::Snapshot old_one = full();
    old_one.alarm_disabled.reset();
    old_one.rating_org.reset();
    old_one.raw_block.clear();
    backup::Snapshot back;
    CHECK(backup::from_json(backup::to_json(old_one), back) && !back.alarm_disabled.has_value() && !back.rating_org &&
          back.raw_block.empty());
}

static void test_files()
{
    TempDir tmp("backup", TempDir::Enter);   // paths::data_dir() is ./playguard_data on the host

    CHECK(backup::list().empty());
    std::string err;
    const std::string first = backup::save(full(), &err);
    CHECK(!first.empty() && err.empty());
    backup::Snapshot second_snap = full();
    second_snap.level = 3;
    const std::string second = backup::save(second_snap, &err);   // same second: a suffixed name
    CHECK(!second.empty() && second != first);

    const auto names = backup::list();
    CHECK(names.size() == 2);
    CHECK(paths::backups_dir() + "/" + names[0] == second);       // newest first
    backup::Snapshot s;
    CHECK(backup::load(names[0], s) && s.level == 3);
    CHECK(backup::load(names[1], s) && s.level == 1);
    CHECK(!backup::load("missing.json", s));

    // A damaged file is listed but cannot be loaded.
    CHECK(paths::atomic_write(paths::backups_dir() + "/00000000_000000.json", "{\"format\":1"));
    CHECK(backup::list().size() == 3 && backup::list().back() == "00000000_000000.json");
    CHECK(!backup::load("00000000_000000.json", s));

    // Keeping the 2 newest deletes the oldest (the damaged one); 0 keeps all.
    CHECK(backup::prune(0) == 0 && backup::list().size() == 3);
    CHECK(backup::prune(2) == 1);
    const auto kept = backup::list();
    CHECK(kept.size() == 2 && kept == names);
    CHECK(backup::prune(5) == 0 && backup::prune(1) == 1 && backup::list().front() == names[0]);

    // A backup made with the clock in the past sorts oldest: the one just
    // written is still kept, and counts as one of the kept.
    CHECK(paths::atomic_write(paths::backups_dir() + "/29990101_000000.json", backup::to_json(full())));
    const std::string past = paths::backups_dir() + "/20000101_000000.json";
    CHECK(paths::atomic_write(past, backup::to_json(full())));
    CHECK(backup::list().size() == 3 && paths::backups_dir() + "/" + backup::list().back() == past);
    CHECK(backup::prune(2, past) == 1);
    const auto after = backup::list();
    CHECK(after.size() == 2 && after[0] == "29990101_000000.json" && after[1] == "20000101_000000.json");
    CHECK(backup::prune(1, past) == 1 && backup::list() == std::vector<std::string>{ "20000101_000000.json" });

    CHECK(tmp.remove());
}

// A restore that would change nothing writes nothing: no unlock, no
// write, no history event.
static void test_tally()
{
    backup::RestoreTally t;
    CHECK(t.nothing_to_write());   // a backup with no value to compare
    t.add(true, true);
    t.add(true, true);
    CHECK(t.same == 2 && t.differ == 0 && t.nothing_to_write());
    // The alarm differs but is not restored (advanced actions off): listed,
    // still nothing to write.
    t.add(true, false, false);
    CHECK(t.differ == 1 && t.written == 0 && t.nothing_to_write());
    // A value the console does not report counts as different, and is written.
    t.add(false, true);
    CHECK(t.same == 2 && t.differ == 2 && t.written == 1 && !t.nothing_to_write());

    backup::RestoreTally u;
    u.add(true, false);
    CHECK(u.same == 0 && u.differ == 1 && !u.nothing_to_write());
}

static void test_forwarders()
{
    // Without the part (an older backup): none, nothing else changed.
    backup::Snapshot out;
    CHECK(backup::from_json(backup::to_json(full()), out) && !out.forwarders);
    // With it: what PlayGuard made, protected or not, and the lock.
    backup::Snapshot in = full();
    forwarders::Made a;
    a.id = 0x050537E2542E2000ull;
    a.nro = a.argv = "sdmc:/switch/sphaira/sphaira.nro";
    a.name = "sphaira";
    a.pin = true;
    a.gate = "sdmc:/config/playguard/gate.nro";
    a.verified = true;
    a.closed_days = 0x7F;
    a.closed_from = 1260;
    a.closed_to = 420;
    forwarders::Made b;
    b.id = 0x0512345678ABC000ull;
    b.nro = b.argv = "sdmc:/switch/StarAtlas.nro";
    b.name = "Star Atlas";
    b.icon = "sdmc:/Nintendo/Album/2026/10/20/2026102018300000-0.jpg";
    in.forwarders = backup::Forwarders{ { a, b }, true };
    CHECK(backup::from_json(backup::to_json(in), out) && out.forwarders && out.forwarders->hbmenu_locked && same(in, out));
    CHECK(out.forwarders && out.forwarders->made.size() == 2);
    if (out.forwarders && out.forwarders->made.size() == 2) {
        const auto& made = out.forwarders->made;
        CHECK(made[0].pin && made[0].verified && made[0].closed_days == 0x7F && made[0].closed_to == 420);
        CHECK(!made[1].pin && made[1].icon == b.icon);
    }
    // A damaged part: left out, the rest still read.
    CHECK(backup::from_json(with("\"restrictions\"", "\"forwarders\": {\"made\": 7}, \"restrictions\""), out) &&
          !out.forwarders);
}

int main()
{
    test_forwarders();
    test_round_trip();
    test_validation();
    test_files();
    test_tally();
    return CHECK_DONE("backup format, validation, file and restore tally assertions passed");
}

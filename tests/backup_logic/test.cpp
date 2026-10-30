// Host tests for source/action/backup_logic.cpp: what a backup keeps of the
// console's answers, the restore list's labels and the tally of a restore.
#include "check.h"
#include <cstring>
#include <string>

#include "action/backup_logic.hpp"

// Every value read: "Teen", custom settings kept from an earlier "Custom",
// VR restricted, PEGI, 2 h on weekdays and no limit at the weekend.
static void read_all(PctlStatus* st, PtState* pt)
{
    std::memset(st, 0, sizeof(*st));
    st->safety_level_ok = true;
    st->safety_level = PctlSafetyLevel_Teen;
    st->settings_ok = true;
    st->settings.rating_age = 12;
    st->settings.sns_post_restriction = true;
    st->settings.free_communication_restriction = false;
    st->stereo_vision_ok = true;
    st->stereo_vision_restricted = true;
    st->rating_org_ok = true;
    st->rating_org = 3;

    std::memset(pt, 0, sizeof(*pt));
    pt->fw_supported = pt->valid = true;
    for (int d = 0; d < 7; d++) pt->day_min[d] = (d == 0 || d == 6) ? PT_DAY_NOLIMIT : 120;
    for (int i = 0; i < PT_U16_COUNT; i++) pt->block[i] = (uint16_t)i;
    pt->alarm_disabled_valid = true;
    pt->alarm_disabled = true;
}

static void test_snapshot()
{
    PctlStatus st;
    PtState pt;
    read_all(&st, &pt);
    backup::Snapshot s = backup_logic::snapshot_of(st, pt);
    CHECK(s.level == PctlSafetyLevel_Teen);
    CHECK(s.custom && s.custom->rating_age == 12 && s.custom->sns_restricted && !s.custom->comm_restricted);
    CHECK(s.vr_restricted == true);
    CHECK(s.rating_org == 3u);
    CHECK(s.days && (*s.days)[0] == PT_DAY_NOLIMIT && (*s.days)[1] == 120 && (*s.days)[6] == PT_DAY_NOLIMIT);
    CHECK(s.raw_block.size() == 4 * PT_U16_COUNT && s.raw_block.compare(0, 8, "00000001") == 0);
    CHECK(s.alarm_disabled == true);
    CHECK(s.created.empty() && s.firmware.empty());   // the caller's
    CHECK(!s.empty());

    // A level the service does not know, an age past 21: not kept (a restore
    // would refuse them, backup::from_json too).
    st.safety_level = PctlSafetyLevel_Teen + 1;
    st.settings.rating_age = 22;
    s = backup_logic::snapshot_of(st, pt);
    CHECK(!s.level && !s.custom && s.vr_restricted.has_value());
    read_all(&st, &pt);
    st.settings.rating_age = 21;
    CHECK(backup_logic::snapshot_of(st, pt).custom.has_value());

    // A limit past 24 h: the limits, the raw block and the alarm go together.
    pt.day_min[4] = 1441;
    s = backup_logic::snapshot_of(st, pt);
    CHECK(!s.days && s.raw_block.empty() && !s.alarm_disabled.has_value());
    pt.day_min[4] = 1440;
    CHECK(backup_logic::snapshot_of(st, pt).days.has_value());

    // The play timer unread, or not on this firmware.
    pt.valid = false;
    s = backup_logic::snapshot_of(st, pt);
    CHECK(!s.days && s.raw_block.empty() && !s.alarm_disabled.has_value() && s.level);
    read_all(&st, &pt);
    pt.fw_supported = false;
    CHECK(!backup_logic::snapshot_of(st, pt).days);
    read_all(&st, &pt);
    pt.alarm_disabled_valid = false;
    s = backup_logic::snapshot_of(st, pt);
    CHECK(s.days && !s.alarm_disabled.has_value());

    // Nothing read at all: nothing a restore would write.
    std::memset(&st, 0, sizeof(st));
    std::memset(&pt, 0, sizeof(pt));
    s = backup_logic::snapshot_of(st, pt);
    CHECK(s.empty() && !s.alarm_disabled.has_value());
    // The JSON round trip keeps what was read.
    read_all(&st, &pt);
    s = backup_logic::snapshot_of(st, pt);
    s.created = "2026-10-07 18:30";
    backup::Snapshot back;
    CHECK(backup::from_json(backup::to_json(s), back));
    CHECK(back.level == s.level && back.days == s.days && back.raw_block == s.raw_block && back.alarm_disabled);
}

static void test_label()
{
    backup::Snapshot s;
    CHECK(backup_logic::list_label("20261007_183000.json", s) == "20261007_183000.json");
    s.firmware = "23.0.1";                          // a firmware without a date: still the file name
    CHECK(backup_logic::list_label("a.json", s) == "a.json");
    s.created = "2026-10-07 18:30";
    CHECK(backup_logic::list_label("a.json", s) == "2026-10-07 18:30 · 23.0.1");
    s.firmware.clear();
    CHECK(backup_logic::list_label("a.json", s) == "2026-10-07 18:30");
}

static void test_tally()
{
    backup_logic::Tally t;
    CHECK(!t.any_written() && t.failed_list().empty() && t.first == 0);
    t.add(0, "Level");
    CHECK(t.any_written() && t.failed.empty());

    // Every write tried, the first error kept, each failure named in order.
    t.add(0x1234, "VR");
    t.add(0, "Rating body");
    t.add(0x5678, "Limits");
    CHECK(t.tried == 4 && t.first == 0x1234);
    CHECK(t.failed_list() == "VR, Limits");
    CHECK(t.any_written());

    backup_logic::Tally refused;                     // everything refused: nothing to record
    refused.add(0x10, "Level");
    refused.add(0x20, "Limits");
    CHECK(!refused.any_written() && refused.first == 0x10 && refused.failed_list() == "Level, Limits");
}

static void test_listing()
{
    backup_logic::Listing l;
    int listed = 0;
    std::vector<std::string> on_card = { "b.json", "a.json" };
    auto list = [&]() {
        listed++;
        return on_card;
    };

    // Listed on first use, then kept: Today asking every 5 s reads nothing.
    CHECK(l.names(list) == on_card && listed == 1);
    on_card.insert(on_card.begin(), "c.json");     // a file another tool put there: not seen yet
    CHECK(l.names(list).size() == 2 && listed == 1);

    // A save (and its prune) forgets it: listed again once, then kept.
    l.forget();
    CHECK(l.names(list) == on_card && listed == 2);
    CHECK(l.names(list).front() == "c.json" && listed == 2);

    // A listing made anyway (the restore's) replaces it without another one.
    l.set({});
    CHECK(l.names(list).empty() && listed == 2);
    l.forget();
    on_card.clear();                                // none left: an empty listing is kept too
    CHECK(l.names(list).empty() && listed == 3);
    CHECK(l.names(list).empty() && listed == 3);
}

int main()
{
    test_snapshot();
    test_label();
    test_tally();
    test_listing();
    return CHECK_DONE("backup_logic snapshot, label, tally and listing tests passed");
}

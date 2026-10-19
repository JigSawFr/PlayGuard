// Host tests for action/schedule_logic.hpp: what a check does with a
// profile scheduled for a period, and the dates its pickers offer.
#include "check.h"
#include <string>

#include "action/schedule_logic.hpp"

using schedule_logic::Step;

int main()
{
    schedule_logic::Schedule s;
    CHECK(schedule_logic::step(s, "2026-10-19") == Step::None);   // nothing scheduled
    s.file = "holidays";
    s.name = "Holidays";
    s.from = "2026-10-19";
    s.to   = "2026-11-02";
    CHECK(schedule_logic::step(s, "2026-10-18") == Step::None);   // before
    CHECK(schedule_logic::step(s, "2026-10-19") == Step::Apply);  // its first day
    CHECK(schedule_logic::step(s, "2026-11-02") == Step::Apply);  // its last day, still not in force
    CHECK(schedule_logic::step(s, "2026-11-03") == Step::Drop);   // missed: forgotten
    s.applied = true;
    CHECK(schedule_logic::step(s, "2026-10-25") == Step::None);   // in force
    CHECK(schedule_logic::step(s, "2026-11-02") == Step::None);
    CHECK(schedule_logic::step(s, "2026-11-03") == Step::Revert); // over
    CHECK(schedule_logic::step(s, "2027-01-10") == Step::Revert);
    CHECK(schedule_logic::step(s, "today") == Step::None);

    // A period of one day; one ending before it starts, or not a date: none.
    s.applied = false;
    s.to = s.from;
    CHECK(schedule_logic::step(s, "2026-10-19") == Step::Apply && schedule_logic::step(s, "2026-10-20") == Step::Drop);
    s.to = "2026-10-18";
    CHECK(schedule_logic::step(s, "2026-10-19") == Step::None);
    CHECK(schedule_logic::valid_period("2026-10-19", "2026-10-19") && !schedule_logic::valid_period("2026-10-19", "2026-10-18"));
    CHECK(!schedule_logic::valid_period("2026-02-30", "2026-03-02") && !schedule_logic::valid_period("", "2026-03-02"));

    // The pickers' dates: across a month and a year.
    const auto d = schedule_logic::dates_from("2026-12-30", 4);
    CHECK(d.size() == 4 && d[0] == "2026-12-30" && d[2] == "2027-01-01" && d[3] == "2027-01-02");
    CHECK(schedule_logic::dates_from("soon", 4).empty());
    return CHECK_DONE("schedule_logic step and date assertions passed");
}

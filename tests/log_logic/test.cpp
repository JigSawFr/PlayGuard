// Host tests for source/action/log_logic.cpp: a log line split into its time
// and text, its kind, the loop's stops flagged, and the filter.
#include "check.h"

#include "action/log_logic.hpp"

using namespace log_logic;

static void test_parse()
{
    const Line app = parse("23:38:10 PlayGuard opened (1.4.0)");
    CHECK(app.time == "23:38:10");
    CHECK(app.text == "PlayGuard opened (1.4.0)");
    CHECK(app.kind == Kind::App);
    CHECK(!app.alert);

    CHECK(parse("23:37:40 remote link: online (broker, MQTT 5.0)").kind == Kind::Link);
    CHECK(parse("23:37:34 remote link: the console slept, connecting again").kind == Kind::Link);

    const Line again = parse("23:37:39 the loop turns again");
    CHECK(again.kind == Kind::Loop);
    CHECK(!again.alert);
    const Line stuck = parse("23:36:38 the loop has not turned for 61 s, stopped in the console's readings (psm, ts)");
    CHECK(stuck.kind == Kind::Loop);
    CHECK(stuck.alert);

    // The agent's other lines; leaving the broker is not a stop of the loop.
    CHECK(parse("22:27:20 agent 1.4.0 started, firmware 21.0.0").kind == Kind::Loop);
    CHECK(parse("22:27:21 link started (broker)").kind == Kind::Link);
    CHECK(parse("22:30:00 order timer: applied").kind == Kind::Link);
    const Line left = parse("22:31:00 stopped: asked to leave the broker");
    CHECK(left.kind == Kind::Link);
    CHECK(!left.alert);
    // PlayGuard's own lines carry two spaces after the time.
    const Line two = parse("22:28:09  PlayGuard opened (1.4.0)");
    CHECK(two.text == "PlayGuard opened (1.4.0)");
    CHECK(two.kind == Kind::App);
}

static void test_odd_lines()
{
    // No time: the whole line is the text.
    const Line bare = parse("something new");
    CHECK(bare.time.empty());
    CHECK(bare.text == "something new");
    CHECK(bare.kind == Kind::Other);
    // A time that is not one is kept in the text.
    CHECK(parse("ab:cd:ef text").time.empty());
    CHECK(parse("").text.empty());
    CHECK(parse("23:38:10").time.empty());
}

static void test_filter()
{
    const Line app = parse("23:38:10 PlayGuard closed");
    const Line link = parse("23:37:40 remote link: online");
    const Line other = parse("23:00:00 unknown");
    CHECK(matches(app, Filter::All) && matches(link, Filter::All) && matches(other, Filter::All));
    CHECK(matches(app, Filter::App) && !matches(link, Filter::App));
    CHECK(matches(link, Filter::Link) && !matches(app, Filter::Link));
    CHECK(!matches(other, Filter::Loop) && !matches(other, Filter::App) && !matches(other, Filter::Link));
}

int main()
{
    test_parse();
    test_odd_lines();
    test_filter();
    return CHECK_DONE("log_logic parse, kind, alert and filter assertions passed");
}

// Host tests for source/util/system_titles.cpp: the console's own programs
// named instead of an unknown game.
#include <cstring>

#include "check.h"

#include "util/system_titles.hpp"

int main()
{
    CHECK(!std::strcmp(system_titles::key(0x0100000000001000ULL), "home"));
    CHECK(!std::strcmp(system_titles::key(0x010000000000100DULL), "album"));
    CHECK(!std::strcmp(system_titles::key(0x010000000000100BULL), "eshop"));
    CHECK(!std::strcmp(system_titles::key(0x0100000000001042ULL), "web"));
    // A game, a system module, an applet nobody would name, nothing.
    CHECK(system_titles::key(0x0100000000010000ULL) == nullptr);
    CHECK(system_titles::key(0x010000000000001EULL) == nullptr);
    CHECK(system_titles::key(0x0100000000001008ULL) == nullptr);
    CHECK(system_titles::key(0) == nullptr);
    return CHECK_DONE("system_titles: all tests passed");
}

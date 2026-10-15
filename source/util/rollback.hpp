// rollback — going back to the build PlayGuard ran before the last one
// installed in place (Install another build, Update in place). Installing
// keeps the replaced .nro as "<nro>.previous" and its name in
// previous_build.json; starting a build just installed is checked
// (boot_check.json): a start that never reached its first screen (it
// crashed) offers the previous build at the next start, before anything
// else, and so does holding L + R while PlayGuard starts. The screen that
// asks is action/rollback_flow.hpp's; this is the plain C++ part, for the
// host tests (tests/rollback).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace rollback
{

// "<nro>.previous": the build that ran before.
std::string previous_file(const std::string& nro);

// What previous_build.json names ("1.2.0", "1.2.0 (abc1234)"); false when
// there is no such file, or it is damaged.
bool read_label(const std::string& json_file, std::string* label);
bool write_label(const std::string& json_file, const std::string& label);

// The start of a build just installed, under way (boot_check.json):
// `starts` is how many starts began without reaching the first screen.
struct Check
{
    std::string label;   // the build installed
    int         starts = 0;
};
bool read_check(const std::string& json_file, Check* out);
bool write_check(const std::string& json_file, const Check& check);

// Why the previous build is offered at start-up.
enum class Offer
{
    None,
    Crashed,   // the last start of the build installed did not reach its first screen
    Asked,     // L + R held while PlayGuard started
};
// `checked`: boot_check.json was there, `starts_before` read from it (before
// this start counts); `held`: L + R; `have_previous`: <nro>.previous is
// there. Nothing is offered without a previous build.
Offer offer_at_start(bool checked, int starts_before, bool held, bool have_previous);

// Swaps <nro> and <nro>.previous (the build running becomes the previous
// one), through "<nro>.swap". When a step fails, what was moved is put back.
bool swap(const std::string& nro, std::string* error);

}   // namespace rollback

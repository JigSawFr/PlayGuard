// week_qr — the last 7 days of the Activity tab's data as a few lines of
// text in the language shown (the total and its average, the trend, each
// day, the three games most played, the sessions and late play), shown as a
// QR code: a phone's camera reads it as text, nothing goes online.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

#include "util/pctl_ops_c.hpp"

namespace week_qr
{

// The text; `who`: whose figures (an account's nickname, or every account).
std::string text(const PlayStats& stats, const std::string& who);

// The QR code of text(), with a line saying what it is; refused with a
// toast while the last 7 days were not read.
void show(const PlayStats& stats, const std::string& who);

}   // namespace week_qr

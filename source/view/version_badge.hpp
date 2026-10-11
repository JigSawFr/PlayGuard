// VersionBadge — the running version, at the right of every screen's header.
// A release: "v1.2.0", quiet, in the notes' grey. Any other build (main, a
// pull request, a local one): an amber "DEV" tag then the version and the
// commit ("1.2.0 · 1a2b3c4"), so a development build is never taken for the
// published one, on the console or in a screenshot.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class VersionBadge : public brls::Box
{
  public:
    VersionBadge();

    // Puts a badge in the header of every screen, when it first takes the
    // focus. Called once, at start-up, before the first screen opens.
    static void install();
};

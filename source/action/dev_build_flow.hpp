// dev_build_flow — Developer tools › Install another build: lists the latest
// release, the last commits of main and the open pull requests
// (util/dev_builds.hpp), downloads the one picked, checks it, puts it in
// place of the running .nro and restarts PlayGuard on it through hbloader.
// The list is kept (memory and cache/dev_builds.json) and shown at once for
// ten minutes; its last line fetches it again. Its screen:
// activity/dev_builds_activity.hpp.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

#include "util/dev_builds.hpp"

namespace dev_build_flow
{

// `force`: fetch the list even when the one kept is fresh.
void open(bool force = false);

// For the list's screen:
// Whether `b` is the running build (a commit of main or of a pull request).
bool installed(const dev_builds::Build& b);
// "2026-10-09T12:03:00Z" -> "2026-10-09 12:03"
std::string short_date(const std::string& iso);
// "List checked 3 min ago.", for `cache`.
std::string checked_text(const dev_builds::Cache& cache);
// Asks to replace the running PlayGuard with `b`, then the PIN when one is
// set, then installs it.
void choose(const dev_builds::Build& b);
// Asks for the PIN when one is set, then installs `b` (already confirmed:
// the update screen's Update in place).
void install_now(const dev_builds::Build& b);
// Signs in to GitHub, then lists the builds again.
void sign_in();

}   // namespace dev_build_flow

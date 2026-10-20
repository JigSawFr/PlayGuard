// live_prefs — Preferences › Theme and Language applied at once, without
// restarting: the theme's variant set, the language's strings loaded again
// (patches/borealis/0012), then the main screen built again on the tab that
// was shown (every view takes its colours and words when it is made). Two
// changes still need a restart, offered as before: to or from Chinese (the
// system font is chosen for it at start-up, ui::use_latin_font), and back to
// the console's language when PlayGuard started in another one (it is not
// known then).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace live_prefs
{
// At start-up, before the preferences' theme replaces it: the console's own
// theme and language (what "system" means).
void remember_system();

// The variant of the theme in the preferences (config::Config::theme).
void apply_theme();

// The language the strings are in now: the preferences', or the console's.
std::string locale();

// After the preferences' theme or language changed: applied, and the screens
// built again. False when it needs a restart instead (nothing done).
bool theme_changed();
bool language_changed();
}   // namespace live_prefs

// screen_hooks — fills in ui::hooks (Aurore's Cadran: the main screen, the
// tabs, PlayGuard's identity and preferences) and ui::app_hooks (PlayGuard's
// own ui/ and view/: a few screens and flows), which they do not include.
// Once, at start-up, before any screen opens.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

void install_screen_hooks();

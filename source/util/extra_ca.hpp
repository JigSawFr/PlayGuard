// extra_ca — root certificates PlayGuard adds to the console's own. The
// Switch's ssl service checks servers against a CA list built into the
// firmware, which does not follow the new roots: since October 2026 bpa.st's
// certificate comes from Let's Encrypt's YR2, under "Root YR" (2025), and
// "Send a report online" failed with "SSL peer certificate or SSH remote key
// was not OK". http.cpp hands these to each connection, on top of the
// built-in list (which still decides for every other server).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

namespace extra_ca
{

// PEM texts, one certificate each.
const std::vector<const char*>& roots();

}   // namespace extra_ca

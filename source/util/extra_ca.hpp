// extra_ca — Let's Encrypt's roots, which the Switch's built-in CA list does
// not all follow. http.cpp hands them to the console's ssl service for each
// connection, on top of its own list; and they are the only roots
// util/tls_client.hpp trusts, for the servers that service refuses anyway
// (bpa.st since October 2026: a "generation Y" certificate under Root YR,
// without Key Encipherment, which its verifier wants in an RSA certificate:
// "SSL peer certificate or SSH remote key was not OK").
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

namespace extra_ca
{

// PEM texts, one certificate each.
const std::vector<const char*>& roots();

}   // namespace extra_ca

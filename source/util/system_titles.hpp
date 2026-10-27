// system_titles — the console's own programs a title ID may name instead of
// a game: the HOME menu, the Album, the eShop and the other system applets
// (switchbrew's title list, 0100000000001000 and after). They have no control
// data, so no name of their own: without this table they would read as an
// unknown game. Their names are the catalogs' playguard/system_titles/<key>.
//
// The play log itself never counts them (core/playstats_logic: only
// applications whose events the system's own statistics keep; the HOME menu
// taking the focus ends a game's session), so neither play time nor "now
// playing" has them: the table names one wherever a title ID still shows.
// No UI and no console calls: host-tested (tests/system_titles).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>

namespace system_titles
{

// The catalog key of a system program ("home", "album" …), nullptr for any
// other title ID.
const char* key(uint64_t app_id);

}   // namespace system_titles

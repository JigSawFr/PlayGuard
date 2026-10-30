// shared_bell_logic — the bell shared by the Boréale homebrews
// (sd:/config/boreale/notices/, boreale-link's bl_notices). PlayGuard's
// side, playguard.json: each entry with the tab where it is dealt with as
// its route, so that a click in another homebrew's bell opens PlayGuard
// there (--open=<tab>); action/notices writes it. The others' side: their
// notices, for PlayGuard's bell (activity/notices_activity). Plain C++ so
// the host tests can run it (tests/shared_bell_logic).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace shared_bell_logic
{

struct Entry
{
    std::string id;
    bool        todo = false;   // to do (else for information)
    std::string title;
    std::string detail;
    std::string route;          // its tab (notice_logic::tab_of); "" the home
};

// The file under the SD card's root `sd_root` ("/" on the console).
std::string file(const std::string& sd_root);

// The file's text for `entries` in `lang`, PlayGuard's .nro being `nro`
// (from the SD card's root; "" keeps the one `previous` says), read over
// `previous` (the file as it is, "" none): an entry that did not change
// keeps its `updated`, the members PlayGuard does not know are kept. ""
// when nothing changed (the SD card is not written), or when `previous` was
// written by a newer version (never written over).
std::string text(const std::vector<Entry>& entries, const std::string& lang, const std::string& nro,
                 const std::string& previous, int64_t now);

// Another homebrew's notice, as PlayGuard's bell shows it.
struct Notice
{
    std::string id;
    bool        attention = false;   // to do or failed: amber
    std::string title;
    std::string detail;              // with its progress, when it has one ("40 %")
    std::string route;               // the screen it opens at
};

struct Other
{
    std::string         module;   // "playshelf"
    std::string         name;     // its registry's (config/boreale/apps/), else the module
    std::string         nro;      // from the SD card's root, with its '/': "/switch/PlayShelf/PlayShelf.nro"; "" unknown
    std::vector<Notice> notices;
    // Its newest release when newer than the one installed (its registry
    // file's version), as the last update check made for all found it
    // (config/boreale/updates.conf, boreale-link's bl_updates); "" none.
    std::string         update;
};

// The other homebrews' notices shown at `now` on a console in `lang`, read
// from under `sd_root`, and their updates: by name; a homebrew whose .nro is
// no longer there, or with nothing to show, left out, and PlayGuard itself.
std::vector<Other> others(const std::string& sd_root, const std::string& lang, int64_t now);

}   // namespace shared_bell_logic

// shared_bell_logic — PlayGuard's bell as the other Boréale homebrews see
// it: sd:/config/boreale/notices/playguard.json (boreale-link's bl_notices),
// each entry with the tab where it is dealt with as its route, so that a
// click in another homebrew's bell opens PlayGuard there (--open=<tab>).
// Only the text: action/notices writes it. Plain C++ so the host tests can
// run it (tests/shared_bell_logic).
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

}   // namespace shared_bell_logic

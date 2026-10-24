// forwarder_library — PlayGuard's own icons for NRO forwarders, one per kind
// of homebrew (emulators, save managers, file managers…), in its look
// (branding/forwarder-icons/, drawn as resources/img/forwarder/<id>.png by
// tools/forwarder_icons.py), and the badge drawn over the icon of one
// protected with the PIN. A forwarder's icon names one as "lib:<id>"
// (forwarders::Made::icon) where a screenshot of the album is a path. Plain
// C++ (no UI) for the host tests (tests/forwarders).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>
#include <vector>

namespace forwarder_library
{

// The badge of a protected forwarder: transparent but for its shield,
// drawn over the icon (forwarder_icon::with_badge).
constexpr const char* BADGE = "img/forwarder/badge.png";

// Every icon's id, in the order the icon sheet shows them.
const std::vector<std::string>& ids();

// Whether `icon` (forwarders::Made::icon) names one of the library ("lib:…").
bool is_ref(const std::string& icon);
// The id `icon` names when it is one of the library's ("lib:files" ->
// "files"), else "" (a screenshot, none, an id no longer there).
std::string id_of(const std::string& icon);
// How a forwarder's icon names `id` ("files" -> "lib:files").
std::string ref_of(const std::string& id);
// Its image among the app's resources ("img/forwarder/files.png").
std::string asset_of(const std::string& id);

// The icon for a homebrew named `name` (its forwarder's, its NACP's) at
// `nro` (its path, its file's name read too): the first kind whose words
// either holds ("JKSV" -> "saves", "/switch/retroarch_switch.nro" ->
// "emulation"), ASCII case aside; "" for none.
std::string suggest(const std::string& name, const std::string& nro);

}   // namespace forwarder_library

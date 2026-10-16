// sys_updates — newer releases of what the console runs on: Atmosphère, hekate
// and the Switch firmware; and of PlayGuard's Home Assistant integration. The
// latest of each is the tag GitHub's releases/latest page redirects to (no
// API call, so no rate limit): Atmosphere-NX/Atmosphere, CTCaer/hekate,
// THZoria/NX_Firmware for the firmware (it mirrors each Nintendo release),
// JigSawFr/playguard-ha. Parsing is plain C++ (host-tested); the fetches go
// to the network (sys_updates_net.cpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <string>

namespace sys_updates
{

// The newest of each, "" when unknown.
struct Latest
{
    std::string atmosphere;   // "1.12.0"
    std::string hekate;       // "6.5.4"
    std::string firmware;     // "23.0.1"
};

// A release tag as a version: "v6.5.4" -> "6.5.4", "1.12.0" -> "1.12.0";
// "" when it is not one (two or three numbers).
std::string version_of_tag(const std::string& tag);
// The tag at the end of a release page's URL
// (".../releases/tag/v6.5.4" -> "v6.5.4"); "" for any other URL.
std::string tag_of_url(const std::string& url);

// hekate's version in bootloader/update.bin: the "ICTC" magic of its
// ipl_ver, then one byte per number ('0' + major, minor, hotfix), within
// the first kilobyte. "" when it is not there.
std::string hekate_version(const std::string& bytes);

// config.json's sys_latest: "atmosphere=1.12.0;hekate=6.5.4;firmware=23.0.1".
std::string to_text(const Latest& latest);
// Unknown keys and malformed versions left out.
Latest      from_text(const std::string& text);

// The three latest releases, each "" when its page could not be read.
// Blocking (network): run it off the main thread. On the desktop build,
// PLAYGUARD_SIM_SYS_LATEST ("1.12.0:6.5.4:23.0.1") replaces the network, and
// without it nothing is fetched.
Latest fetch(std::string* error);

// PlayGuard's Home Assistant integration (JigSawFr/playguard-ha): its latest
// release ("" when its page could not be read), and the CHANGELOG.md of a
// release (raw.githubusercontent.com, at its tag; release-please's format,
// util/changelog.hpp). Blocking, as fetch(). On the desktop build,
// PLAYGUARD_SIM_HA_LATEST ("0.2.0") and PLAYGUARD_SIM_HA_CHANGELOG (a file)
// replace the network, and without them nothing is fetched.
std::string fetch_integration(std::string* error);
bool        fetch_integration_changelog(const std::string& version, std::string* md, std::string* error);

}   // namespace sys_updates

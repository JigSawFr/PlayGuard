// syspatch_update — installs the latest sys-patch from the developer tools
// (Console › Game patches, in developer mode): sys-patch is what lets a
// forwarder start, and a firmware it has no patterns for leaves one "could
// not be started" (util/patches.hpp says which patterns are missing).
//
// The release is the latest of impeeza/sys-patch on GitHub (the maintained
// fork; the original repository is gone): its sys-patch-v<version>.zip,
// checked against the size and SHA-256 GitHub records. Only the files the
// release puts on the SD card are taken out of it, each to its own place:
// the sysmodule (atmosphere/contents/420000000000000B/) and its overlay
// (switch/.overlays/). The files they replace are kept in a backup folder
// first. sys-patch patches at boot: the console must restart for it to run.
//
// Everything but fetch() and download() is plain C++ for the host tests
// (tests/syspatch_update); those two are in syspatch_update_net.cpp.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

namespace syspatch_update
{

// Its repository and the page a user can check by hand.
constexpr const char* REPO = "impeeza/sys-patch";
constexpr const char* PAGE = "https://github.com/impeeza/sys-patch/releases";

struct Release
{
    std::string version;   // "1.6.3.0"
    std::string url;       // the zip's browser_download_url
    uint64_t    size = 0;
    std::string sha256;    // 64 hex digits; empty when GitHub gave none (not installed, then)
    std::string page;      // the release's page on GitHub
};

// The latest release (GET /repos/impeeza/sys-patch/releases/latest) with a
// sys-patch*.zip asset. False for anything else (a draft, a pre-release, no
// such asset, a tag that is not a version, a size out of bounds).
constexpr uint64_t MAX_ZIP = 8 * 1024 * 1024;
bool parse_release(const std::string& json, Release* out);

// Versions as dotted numbers ("1.6.3.0", "v1.6.2.3", "1.6.2.3-b962f75": a
// leading v and anything after the numbers left out): <0, 0 or >0. A text
// with no number sorts first.
int compare(const std::string& a, const std::string& b);

// Whether `r` is newer than the version sys-patch logged (Report::log_version,
// "" when unknown: then any release is offered).
bool newer(const Release& r, const std::string& installed);

// Takes the release's files out of `zip_path` and puts them on the SD card
// at `sd_root`; each file there before is copied first to `backup_dir`, at
// the same path. The sysmodule's three files are required, the overlay is
// taken when the zip has it. Nothing is replaced unless every required file
// came out whole (zip_read checks each CRC-32). False, with a short English
// reason in *error, when it could not.
bool install(const std::string& zip_path, const std::string& sd_root, const std::string& backup_dir,
             std::string* error);

// The latest release (network, blocking: run it off the main thread).
bool fetch(Release* out, std::string* error);
// Downloads `r` to `path` and checks it (size, SHA-256: a release without a
// digest is refused). `path` is removed on failure.
bool download(const Release& r, const std::string& path, std::string* error);

}   // namespace syspatch_update

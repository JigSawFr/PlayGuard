// app — process-wide state: version, the pctl availability probe, and the
// read-only / developer modes.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

namespace app
{

// Probes the pctl service once (open + immediately release: the privileged
// `pctl:a` slot must stay free for the system). Returns true on success.
bool init();
void shutdown();

uint32_t    pctl_init_result();   // 0 if the probe succeeded

std::string version();            // "1.0.0"
// The commit this build comes from ("1a2b3c4"), empty when unknown (built
// outside a git checkout). Tells which development build is installed.
std::string commit();
// True for a published release (built from its vX.Y.Z tag), false for any
// other build (main, a pull request, a local one): a development build.
bool        is_release();
// Where the running .nro is, from the SD card root ("/switch/playguard/playguard.nro"),
// as hbloader gave it in argv[0]; empty when unknown (nxlink, desktop).
std::string self_path();
void        set_self_path(const char* argv0);
const char* repo_url();
// The website (https://playguard.link/, tools/build_site.py) and a page of its
// documentation in the console's language ("troubleshooting", "locked-out"):
// a release's as published (docs/), main's for a development build (docs/dev/).
const char* site_url();
std::string doc_url(const std::string& page, const std::string& anchor = "");
// Funding links (.github/FUNDING.yml), shown in About.
constexpr const char* SPONSORS_URL = "https://github.com/sponsors/JigSawFr";
constexpr const char* KOFI_URL     = "https://ko-fi.com/jigsawfr";

// Read-only mode: the service layer refuses every change (core/write_guard.h)
// and the tabs hide their write controls. On at launch when the firmware is
// newer than the checked one, until the firmware screen says otherwise.
// After a change, call ui::on_mode_changed() to update what is on screen.
bool        read_only();
void        set_read_only(bool on);

// Developer mode: the developer tools at the end of About › Help & diagnostics, and the
// full diagnostic of Play timer › Advanced.
// Saved in the preferences, or on for this session only (firmware screen,
// "read-only + developer mode").
bool        dev_mode();
// Saved off the UI thread, with ui::save_config's toasts.
void        set_dev_mode(bool on, bool persist, const std::string& saved_text = "", const std::string& failed_text = "");

// False while the app is in the background (HOME menu, system PIN prompt …):
// periodic refreshes skip their pctl queries then, keeping the single
// privileged session free for the system.
bool        in_focus();

}   // namespace app

// app — process-wide state: version, the pctl availability probe, and the
// read-only / developer modes.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <cstdint>
#include <string>

#include "kit_app.hpp"   // version(), commit(), repo_url(), read_only(), dev_mode(): the kit asks for them

namespace app
{

// Probes the pctl service once (open + immediately release: the privileged
// `pctl:a` slot must stay free for the system). Returns true on success.
bool init();
void shutdown();

uint32_t    pctl_init_result();   // 0 if the probe succeeded

// is_release(), pr() and self_path() (kit_app.hpp) are defined here.
void        set_self_path(const char* argv0);
// The screen another Boréale homebrew opened PlayGuard at (a click in its
// bell: --open=<route>, boreale-link's bl_open_route): one of
// config::START_TABS, "" none or one PlayGuard does not know.
std::string open_route();
void        set_open_route(int argc, char** argv);
// The website (https://guard.boreale.dev/, tools/build_site.py) and a page of its
// documentation in the console's language ("troubleshooting", "locked-out"):
// a release's as published (docs/), main's for a development build (docs/dev/).
// Tagged for the site's audience (utm_source=playguard, utm_medium=qr).
const char* site_url();
std::string doc_url(const std::string& page, const std::string& anchor = "");
// The Home Assistant integration (JigSawFr/boreale-ha), through My Home
// Assistant's redirects (it asks once for the user's own instance, then opens
// the page there): the repository in the Marketplace (Home Assistant 2026.11
// and later: marketplace_repository), and adding the integration.
constexpr const char* HA_MARKETPLACE_URL =
    "https://my.home-assistant.io/redirect/marketplace_repository/?owner=JigSawFr&repository=boreale-ha&category=integration";
constexpr const char* HA_ADD_URL = "https://my.home-assistant.io/redirect/config_flow_start/?domain=playguard";
// Its releases, with what changed in each.
constexpr const char* HA_RELEASES_URL = "https://github.com/JigSawFr/playguard-ha/releases";

// Read-only mode: the service layer refuses every change (core/write_guard.h)
// and the tabs hide their write controls. On at launch when the firmware is
// newer than the checked one, until the firmware screen says otherwise.
// After a change, call ui::on_mode_changed() to update what is on screen.
void        set_read_only(bool on);

// Developer mode: the developer tools at the end of About › Help & diagnostics, and the
// full diagnostic of Play timer › Advanced.
// Saved in the preferences, or on for this session only (firmware screen,
// "read-only + developer mode").
// Saved off the UI thread, with ui::save_config's toasts.
void        set_dev_mode(bool on, bool persist, const std::string& saved_text = "", const std::string& failed_text = "");

// False while the app is in the background (HOME menu, system PIN prompt …):
// periodic refreshes skip their pctl queries then, keeping the single
// privileged session free for the system.
bool        in_focus();

}   // namespace app

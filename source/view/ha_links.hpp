// ha_links — PlayGuard's Home Assistant integration in the parent's own Home
// Assistant, from a phone, through My Home Assistant's redirects (it asks
// once for the instance's address): playguard-ha in HACS (app::HA_HACS_URL),
// and adding a console (app::HA_ADD_URL). Remote access › Home Assistant and
// the integration's update screen show them as QR codes.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>

namespace ha_links
{

// A QR code of `url` with its title and a caption under it, centred.
brls::Box* qr_card(const std::string& title, const std::string& url, const std::string& caption);

// The dialog with both codes: install the integration, then add this console
// (`device`, its id; "" not known yet) when Home Assistant does not offer it.
void show(const std::string& device);

}   // namespace ha_links

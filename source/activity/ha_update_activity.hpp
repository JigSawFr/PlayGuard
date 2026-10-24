// HaUpdateActivity — a newer release of PlayGuard's Home Assistant integration
// (action/ha_update_flow): the version Home Assistant runs, the QR code that
// opens the integration in HACS on a phone (Update, then restart Home
// Assistant), and what changed since. B closes it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

class HaUpdateActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/ha_update.xml");

    void onContentAvailable() override;

  private:
    BRLS_BIND(brls::Header, header,  "hu_header");
    BRLS_BIND(brls::Label,  body,    "hu_body");
    BRLS_BIND(brls::Box,    qr,      "hu_qr");
    BRLS_BIND(brls::Header, changes, "hu_changes");
    BRLS_BIND(brls::Box,    notes,   "hu_notes");
    BRLS_BIND(brls::Label,  note,    "hu_note");
};

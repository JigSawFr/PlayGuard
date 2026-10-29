// HaUpdateActivity — a newer release of PlayGuard's Home Assistant integration
// (action/ha_update_flow): the version Home Assistant runs, the QR code that
// opens the integration in HACS on a phone (Update, then restart Home
// Assistant), and what changed since. B closes it.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>

class HaUpdateActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/ha_update.xml");

    void onContentAvailable() override;

  private:
    AURORE_BIND(aurore::Header, header,  "hu_header");
    AURORE_BIND(aurore::Label,  body,    "hu_body");
    AURORE_BIND(aurore::Box,    qr,      "hu_qr");
    AURORE_BIND(aurore::Header, changes, "hu_changes");
    AURORE_BIND(aurore::Box,    notes,   "hu_notes");
    AURORE_BIND(aurore::Label,  note,    "hu_note");
};

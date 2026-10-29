// FirmwareGateActivity — shown over the main screen when the firmware is newer
// than the checked range (see action/fw_gate.hpp): says so, looks for a
// release that supports it, and offers to update, to continue read-only (with
// the developer tools or not) or to continue at the user's own risk.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <memory>

#include "action/fw_gate.hpp"

class FirmwareGateActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/firmware_gate.xml");

    void onContentAvailable() override;

  private:
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);   // guards the async update check
    bool chosen = false;

    void choose(fw_gate::Choice choice);

    AURORE_BIND(aurore::Label,       headline,   "fg_headline");
    AURORE_BIND(aurore::Label,       body,       "fg_body");
    AURORE_BIND(aurore::Label,       status,     "fg_status");
    AURORE_BIND(aurore::DetailCell,  update,     "fg_update");
    AURORE_BIND(aurore::DetailCell,  read_only,  "fg_read_only");
    AURORE_BIND(aurore::DetailCell,  probe,      "fg_probe");
    AURORE_BIND(aurore::DetailCell,  risk,       "fg_risk");
    AURORE_BIND(aurore::BooleanCell, remember,   "fg_remember");
    AURORE_BIND(aurore::Label,       note,       "fg_note");
    AURORE_BIND(aurore::DetailCell,  quit,       "fg_quit");
};

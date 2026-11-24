// CompanionActivity — Security › the companion phone app: linked or not, the
// last sync, the block comparison, unlink.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <aurore.hpp>
#include <aurore/cadran/ui/cadran_list.hpp>

#include <memory>

#include "ui/play_ui.hpp"

class CompanionActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/companion.xml");

    void onContentAvailable() override;
    void onResume() override;

  private:
    std::unique_ptr<cadran::List> list;
    ui::Alive alive = ui::make_alive();

    void refresh();

    AURORE_BIND(aurore::DetailCell, active,  "pr_active");
    AURORE_BIND(aurore::DetailCell, updated, "pr_updated");
    AURORE_BIND(aurore::DetailCell, decode,  "pr_decode");
    AURORE_BIND(aurore::DetailCell, unlink,  "pr_unlink");
    AURORE_BIND(aurore::Label,      note,    "pr_note");
};

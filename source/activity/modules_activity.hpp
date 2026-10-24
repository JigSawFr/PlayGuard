// ModulesActivity — Console › Optional modules: PlayGuard's two sysmodules,
// which it carries in its romfs and installs on the SD card
// (util/modules.hpp): the recovery module (acts once at boot when a RESCUE
// file asks it to) and the remote-link agent (keeps the link up while
// PlayGuard is closed). A sidebar, as on the main screen: an overview (what
// they are, each one's state; Ⓐ opens its tab), then a tab per module with
// its state, Install / Update, At boot, Start / Stop now (the agent), Remove.
// Every change asks for the PIN as a console change does: the recovery
// module can unlock the console.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "util/modules.hpp"

class ModulesActivity : public brls::Activity
{
  public:
    // The tab opened first: the overview, or a module's.
    enum class Tab { Overview, Rescue, Agent };

    explicit ModulesActivity(Tab first = Tab::Overview);

    CONTENT_FROM_XML_RES("activity/modules.xml");

    void onContentAvailable() override;
    void onResume() override;

    // "Installed", "Update available" … short enough to share one line with
    // the title of a cell that opens this screen (and the overview's), and
    // its colour when `color` is given.
    static std::string summary(modules::Id id, NVGcolor* color = nullptr);

  private:
    Tab start;

    BRLS_BIND(brls::TabFrame, tabs, "md_tabs");
};

// ModulesActivity — Console › Optional modules: PlayGuard's two sysmodules,
// which it carries in its romfs and installs on the SD card
// (util/modules.hpp): the recovery module (acts once at boot when a RESCUE
// file asks it to) and the Boréale agent (keeps the link up while
// PlayGuard is closed). A list as the other screens are (sync.xml): what
// they are, then a cell per module with its state; Ⓐ opens the module's own
// screen, a list as well: the recovery module's (ModuleActivity), or the
// agent's, which is boreale-kit's AgentActivity. Every change asks for the
// PIN as a console change does: the recovery module can unlock the console.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <memory>

#include <aurore.hpp>
#include <aurore/cadran/ui/cadran_list.hpp>
#include <aurore/cadran/ui/guarded.hpp>

#include "util/modules.hpp"

class ModulesActivity : public aurore::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/modules.xml");

    void onContentAvailable() override;
    void onResume() override;

    // "Installed", "Update available" … short enough to share one line with
    // the title of a cell that opens this screen (and the overview's), and
    // its colour when `color` is given.
    static std::string summary(modules::Id id, const char** color = nullptr);   // the recovery module's

  private:
    void refresh();

    std::unique_ptr<cadran::List> list;

    AURORE_BIND(aurore::Label,      intro,  "md_intro");
    AURORE_BIND(aurore::Label,      rescue_note, "md_rescue_note");
    AURORE_BIND(aurore::Label,      agent_note,  "md_agent_note");
    AURORE_BIND(aurore::DetailCell, rescue, "md_rescue");
    AURORE_BIND(aurore::DetailCell, agent,  "md_agent");
};

// One module's screen: what it does, its state, Install / Update, At boot,
// Remove. Opened from the overview, and from Security › Locked out?.
class ModuleActivity : public aurore::Activity
{
  public:
    explicit ModuleActivity(modules::Id which);

    CONTENT_FROM_XML_RES("activity/module.xml");

    void onContentAvailable() override;
    void onResume() override;

  private:
    modules::Id id;
    std::unique_ptr<cadran::List> list;
    // False once the screen is gone: a flow ending after it touches nothing
    // (ui::guarded).
    ui::Alive alive = ui::make_alive();

    void refresh();
    void install();
    void uninstall();

    AURORE_BIND(aurore::Label,       about,  "md_about");
    AURORE_BIND(aurore::DetailCell,  status, "md_status");
    AURORE_BIND(aurore::DetailCell,  action, "md_action");
    AURORE_BIND(aurore::BooleanCell, boot,   "md_boot");
    AURORE_BIND(aurore::DetailCell,  remove, "md_remove");
};

// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/screen_hooks.hpp"

#include <borealis.hpp>

#include "action/frame_stats_flow.hpp"
#include "action/notices.hpp"
#include "action/pt_log_flow.hpp"
#include "action/upload_flow.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/main_activity.hpp"
#include "activity/notices_activity.hpp"
#include "tab/help_tab.hpp"
#include "tab/tab_base.hpp"
#include "ui/hooks.hpp"

void install_screen_hooks()
{
    ui::hooks::Hooks& h = ui::hooks::get();

    h.is_main = [](brls::Activity* a) { return dynamic_cast<MainActivity*>(a) != nullptr; };
    h.open_tab = [](brls::Activity* main, int position, const std::string& row) {
        static_cast<MainActivity*>(main)->open_tab(position, row);
    };
    h.update_title = [](brls::Activity* main) { static_cast<MainActivity*>(main)->update_title(); };
    h.push_main = [](int position, const std::string& row) {
        auto* main = new MainActivity();
        brls::Application::pushActivity(main, brls::TransitionAnimation::NONE);
        main->open_tab(position, row);
    };

    h.refresh_shown = []() { TabBase::refresh_shown(); };
    h.shown_screen = []() { return TabBase::shown_screen(); };
    h.main_screen = []() { return TabBase::main_screen(); };
    h.is_tab = [](brls::View* v) { return dynamic_cast<TabBase*>(v) != nullptr; };

    h.open_notices = []() { NoticesActivity::open(); };
    h.open_firmware_screen = []() { brls::Application::pushActivity(new FirmwareGateActivity()); };
    h.make_help_tab = []() -> brls::View* { return new HelpTab(); };
    h.send_report = []() { upload_flow::choose(); };
    h.note_write = [](const char* what, bool ok) { notices::note_write(what, ok); };
    h.mode_changed = []() {
        pt_log_flow::apply();
        frame_stats_flow::apply();
    };
}

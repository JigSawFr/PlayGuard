// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "view/pt_state_header.hpp"

#include "action/notices.hpp"
#include "action/pt_logic.hpp"
#include "action/timer_health.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

PtStateHeader::PtStateHeader()
{
    this->inflateFromXMLRes("xml/view/pt_state_header.xml");
    alert->setSingleLine(false);
    health->setSingleLine(false);
    ui::set_visible(health, false);   // until a reading says so
}

void PtStateHeader::show(const PtState& pt)
{
    // Reached: the console restricts (suspending), or the time left went
    // below 0 (played past the limit: a warning only, or while unlocked).
    const int over = pt.fw_supported ? pt_logic::over_limit_min(pt, ui::today_weekday()) : -1;
    const bool reached = pt.fw_supported && ((pt.restricted_valid && pt.restricted) || over >= 0);

    // What the console does at the limit, once the block says (pure.h).
    alert->setText(!reached    ? ""
                   : !pt.valid ? "playguard/play_timer/state/reached_alert"_i18n
                   : pt.alarm_only ? "playguard/play_timer/state/reached_alert_alarm"_i18n
                                   : "playguard/play_timer/state/reached_alert_suspend"_i18n);
    ui::set_visible(alert, reached);
    const bool stalled = timer_health::observe(pt);
    notices::note_timer_stalled(stalled);
    ui::set_visible(health, stalled);
}

brls::View* PtStateHeader::create()
{
    return new PtStateHeader();
}

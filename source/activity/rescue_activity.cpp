// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/rescue_activity.hpp"

#include <aurore/cadran/ui/cadran_kit.hpp>

#include "action/console_actions.hpp"
#include "action/pin_lock.hpp"
#include "activity/lock_activity.hpp"
#include "activity/main_activity.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

RescueActivity::RescueActivity(RescueReport found, bool verified, bool old_sysmodule)
    : report(found), confirmed(verified), old_module(old_sysmodule && !verified)
{
}

void RescueActivity::onContentAvailable()
{
    outcome->setSingleLine(false);
    note->setSingleLine(false);

    // A confirmed recovery is the trusted context (see the header): no PIN is
    // asked before the changes made here. Otherwise the actions are hidden and
    // the usual check stays.
    if (confirmed) core_set_change_check(nullptr);

    reset_pin->registerClickAction([this](aurore::View*) {
        if (confirmed) console_actions::set_pin("rescue", [this]() { this->refresh(); });
        return true;
    });
    del->registerClickAction([this](aurore::View*) {
        if (confirmed) console_actions::delete_all("rescue", [this]() { this->refresh(); });
        return true;
    });
    cont->registerClickAction([this](aurore::View*) {
        this->proceed();
        return true;
    });
    // B continues too (rather than quitting): the app should open after a rescue.
    this->getContentView()->registerAction("playguard/rescue/continue"_i18n, aurore::BUTTON_B, [this](aurore::View*) {
        this->proceed();
        return true;
    });

    this->refresh();
}

void RescueActivity::willAppear(bool resetState)
{
    aurore::Activity::willAppear(resetState);
    this->refresh();
}

void RescueActivity::refresh()
{
    // A report the console does not confirm says nothing true about it: say
    // so instead of what the file claims (a failure or a refusal changed
    // nothing, so its own words stand).
    const bool claimed   = report.result == RescueResult_Ok || report.result == RescueResult_NoPin;
    const bool doubted   = claimed && !confirmed;
    const bool ok        = report.result == RescueResult_Ok && confirmed;
    const bool deleted   = ok && report.mode == RescueMode_Delete;

    // What the sysmodule did, in the status colour that matches it. A module
    // from before the proof: doing RESCUE again gives the same, so it says
    // to update the module rather than to try again.
    const char* key = old_module                            ? "playguard/rescue/outcome/old_module"
                    : doubted                               ? "playguard/rescue/outcome/unconfirmed"
                    : deleted                               ? "playguard/rescue/outcome/deleted"
                    : ok                                    ? "playguard/rescue/outcome/unlocked"
                    : report.result == RescueResult_NoPin   ? "playguard/rescue/outcome/no_pin"
                    : report.result == RescueResult_Refused ? "playguard/rescue/outcome/refused"
                                                            : "playguard/rescue/outcome/failed";
    outcome->setText(aurore::getStr(key));
    outcome->setThemeColor("textColor", ok                                                                ? "cadran/teal"
                                        : old_module || (report.result == RescueResult_NoPin && !doubted) ? "cadran/amber"
                                                                                                          : "cadran/coral");

    std::string n = old_module                            ? aurore::getStr("playguard/rescue/note_old_module", app::version())
                  : doubted                               ? "playguard/rescue/note_unconfirmed"_i18n
                  : deleted                               ? "playguard/rescue/note_deleted"_i18n
                  : ok                                    ? "playguard/rescue/note"_i18n
                  : report.result == RescueResult_NoPin   ? "playguard/rescue/note_no_pin"_i18n
                  : report.result == RescueResult_Refused ? "playguard/rescue/note_refused"_i18n
                                                          : "playguard/rescue/note_failed"_i18n;
    if (report.result == RescueResult_Failed)
        n += "\n" + aurore::getStr("playguard/rescue/failed_code", ui::rc_text(report.rc, false));   // the note says what next
    // The request file is still on the card: an unlock request unlocks again
    // at every start until it is removed from a computer.
    if (report.request == RescueRequest_Kept && report.result != RescueResult_Refused)
        n += "\n\n" + aurore::getStr("playguard/rescue/request_kept", RESCUE_REQUEST_NAME);
    if (app::read_only()) n += "\n" + "playguard/common/read_only_note"_i18n;
    note->setText(n);

    // After a delete there is nothing left to show, reset or delete: only
    // "Open PlayGuard" remains, so the actions section is hidden entirely.
    PctlStatus s;
    pctl_status_fetch(&s);
    const bool has_pin  = s.pin_length_ok && s.pin_length > 0;
    const bool writable = !app::read_only();

    // Short hints on the right, so a parent knows what each one does at a glance.
    reset_pin->setDetailText("playguard/rescue/hint/reset_pin"_i18n);
    del->setDetailText("playguard/rescue/hint/delete"_i18n);
    for (aurore::DetailCell* c : { (aurore::DetailCell*)reset_pin.getView(), (aurore::DetailCell*)del.getView() })
        c->detail->setThemeColor("textColor", "cadran/text3");

    // After the sysmodule deleted everything, only "Open PlayGuard" is left.
    // Otherwise: delete needs a PIN to exist; "Set a new PIN" is always
    // offered (it sets one when there is none).
    // Nothing at all unless the console confirmed the recovery.
    const bool actions = confirmed && !deleted;
    ui::set_visible_all({ { actions_header.getView(), actions },
                          { reset_pin.getView(), actions },
                          { del.getView(), actions && has_pin } });
    ui::show_writable(reset_pin, writable);
    ui::show_writable(del, writable, "cadran/coral", "cadran/text2");
}

void RescueActivity::proceed()
{
    // Restore the normal PIN-before-a-change gate, then open the app. If the
    // parent reset the PIN, "ask to open" would apply next launch; not now.
    // An unconfirmed report skips nothing: the lock screen comes first, as on
    // any start.
    pin_lock::install();
    if (!confirmed && pin_lock::at_start())
        ui::replace_screen(new LockActivity());
    else
        ui::replace_screen(new MainActivity());
}

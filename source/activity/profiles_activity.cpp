// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/profiles_activity.hpp"

#include <algorithm>

#include <borealis/cadran/ui/cadran_kit.hpp>
#include <borealis/cadran/ui/cadran_list.hpp>

#include "action/profile_flow.hpp"
#include "action/schedule_flow.hpp"
#include "activity/play_timer_perday_activity.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

void ProfilesActivity::onContentAvailable()
{
    cadran::kit::empty_around(empty, "calendar");
    hint->setSingleLine(false);
    save_cell->registerClickAction([this](brls::View*) {
        this->save_current();
        return true;
    });
    new_cell->registerClickAction([this](brls::View*) {
        this->create();
        return true;
    });
    // A profile for a period (holidays): offered when it begins, the limits
    // from before once it is over. PlayGuard acts only while it is open, so
    // a new one is Home Assistant's (Console › Remote access: its profile
    // select, from an automation); one set by an earlier version can still
    // be changed or ended.
    schedule_note->setSingleLine(false);
    schedule->registerClickAction([this](brls::View*) {
        if (schedule_flow::summary().empty()) {
            ui::info("playguard/schedule/soon_note"_i18n);
            return true;
        }
        schedule_flow::choose([this]() {
            // The profile put in force, or the limits back: the list's mark too.
            pctl_play_timer_query(&this->live);
            this->rebuild();
            this->show_schedule();
            brls::Application::giveFocus(schedule);
        });
        return true;
    });
    pctl_play_timer_query(&this->live);
    cadran::List(this->getContentView(), true).apply({ { "pf_save", "save" }, { "pf_new", "plus" }, { "pf_schedule", "calendar" } });
    this->show_schedule();   // after the list's look: its colour stays
    this->rebuild();
}

void ProfilesActivity::show_schedule()
{
    const std::string s = schedule_flow::summary();
    if (s.empty()) {
        // A new one: through Home Assistant (Console › Remote access).
        schedule->setDetailText("playguard/schedule/soon"_i18n);
        schedule->setDetailTextColor(cadran::kit::color("cadran/text2"));
        schedule_note->setText("playguard/schedule/soon_note"_i18n);
        return;
    }
    schedule->setDetailText(ui::opens(s));
    schedule->setDetailTextColor(cadran::kit::color("cadran/text2"));
    schedule_note->setText("playguard/schedule/note"_i18n);
}

void ProfilesActivity::rebuild(const std::string& focus_file)
{
    // Move the focus to a cell that survives before deleting the old list,
    // so borealis never holds a pointer to a deleted cell.
    brls::Application::giveFocus(save_cell);
    brls::Box* rows = cadran::List::rows_in(list);   // a card in the Cadran look
    rows->clearViews();

    const auto all = profiles::list();
    const std::string current = this->live.valid ? profiles::match(this->live.day_min) : "";
    brls::View* focus = nullptr;
    for (const auto& p : all) {
        auto* cell = new brls::DetailCell();
        cell->setText(p.name);
        const std::string summary = ui::days_summary(p.days.data());
        if (p.name == current) {
            cell->setDetailText(brls::getStr("playguard/play_timer/profile_current", summary));
            cell->setDetailTextColor(cadran::kit::color("cadran/teal"));
        } else {
            cell->setDetailText(summary);
        }
        cell->registerClickAction([this, p](brls::View*) {
            this->actions(p);
            return true;
        });
        rows->addView(cell);
        cadran::List::row(cell, "list");
        if (!focus || p.file == focus_file) focus = cell;
    }
    ui::set_visible(empty.getView(), all.empty());
    ui::set_visible(hint.getView(), !all.empty());
    brls::Application::giveFocus(focus ? focus : (brls::View*)save_cell.getView());
}

void ProfilesActivity::actions(const profiles::Profile& p)
{
    const std::vector<std::string> labels = {
        "playguard/play_timer/profile_action_apply"_i18n, "playguard/play_timer/profile_action_edit"_i18n,
        "playguard/play_timer/profile_action_rename"_i18n, "playguard/common/delete"_i18n,
    };
    ui::pick(p.name, labels, 0, [this, p](int index) {
        switch (index) {
            case 0: this->apply(p); break;
            case 1: this->edit_limits(p); break;
            case 2: this->rename(p); break;
            default: this->remove(p); break;
        }
    }, false);
}

void ProfilesActivity::apply(const profiles::Profile& p)
{
    const std::string file = p.file;
    profile_flow::apply(p, [this, file]() {
        pctl_play_timer_query(&this->live);
        this->rebuild(file);
    });
}

bool ProfilesActivity::store(const profiles::Profile& p, const std::string& ok_text)
{
    std::string err;
    if (!profiles::save(p, &err)) {
        ui::error("playguard/play_timer/profile_save_err"_i18n, err);
        return false;
    }
    ui::notify(ok_text);
    // The file kept its old case when only the case changed: find it again.
    const std::string file = profiles::file_stem(p.name);
    std::string focus = file;
    for (const auto& q : profiles::list())
        if (profiles::file_stem(q.name) == file) focus = q.file;
    if (!ui::on_top(this)) {   // the limits editor is still open
        this->relist_pending = true;
        this->relist_focus   = focus;
        return true;
    }
    brls::sync([this, focus]() { this->rebuild(focus); });
    return true;
}

void ProfilesActivity::onResume()
{
    if (!this->relist_pending) return;
    this->relist_pending = false;
    // Next frame: borealis first gives the focus back to the cell that opened
    // the editor, which rebuild() then moves away before deleting it.
    const std::string focus = this->relist_focus;
    brls::sync([this, focus]() { this->rebuild(focus); });
}

void ProfilesActivity::edit_limits(const profiles::Profile& p)
{
    brls::Application::pushActivity(new PlayTimerPerDayActivity(
        brls::getStr("playguard/play_timer/profile_edit_title", p.name), p.days,
        [this, p](const PlayTimerPerDayActivity::Days& days) {
            profiles::Profile edited = p;
            edited.days = days;
            return this->store(edited, brls::getStr("playguard/play_timer/profile_saved", p.name));
        }));
}

void ProfilesActivity::rename(const profiles::Profile& p)
{
    this->ask_name(p.name, p.file, [this, p](std::string name) {
        if (name == p.name) return;
        profiles::Profile renamed = p;
        renamed.name = name;
        this->store(renamed, brls::getStr("playguard/play_timer/profile_renamed", name));
    });
}

void ProfilesActivity::remove(const profiles::Profile& p)
{
    const std::string name = p.name, file = p.file;
    ui::confirm_danger(brls::getStr("playguard/play_timer/profile_delete_body", name), "playguard/common/delete"_i18n,
                       [this, name, file]() {
                           // Before the error's dialog opens (both on the next
                           // frame): rebuilt under it, the list would delete
                           // the cell the dialog gives the focus back to.
                           brls::sync([this]() { this->rebuild(); });
                           if (profiles::remove(file))
                               ui::notify(brls::getStr("playguard/play_timer/profile_deleted", name));
                           else
                               ui::error(brls::getStr("playguard/play_timer/profile_delete_err", name));
                       });
}

void ProfilesActivity::ask_name(const std::string& initial, const std::string& except_file,
                                std::function<void(std::string)> done)
{
    ui::prompt_text("playguard/play_timer/profile_name"_i18n, initial, (int)profiles::MAX_NAME,
                    [except_file, done](std::string typed) {
        const std::string name = profiles::sanitize_name(typed);
        if (name.empty()) return;   // nothing typed: as if cancelled
        profiles::Profile existing;
        if (profiles::find_same_file(name, except_file, &existing)) {
            // FAT ignores case (and the file name drops accents): this would
            // overwrite another profile.
            ui::confirm(brls::getStr("playguard/play_timer/profile_replace", existing.name),
                        "playguard/play_timer/profile_replace_confirm"_i18n, [done, name]() { done(name); }, nullptr, true);
            return;
        }
        done(name);
    });
}

void ProfilesActivity::save_current()
{
    pctl_play_timer_query(&this->live);
    if (!this->live.valid) {
        ui::refuse(NXM_RC_STATE_UNKNOWN);
        return;
    }
    std::array<uint16_t, 7> days;
    for (int i = 0; i < 7; i++) days[i] = this->live.day_min[i];
    this->ask_name("", "", [this, days](std::string name) {
        profiles::Profile p;
        p.name = name;
        p.days = days;
        this->store(p, brls::getStr("playguard/play_timer/profile_saved", name));
    });
}

void ProfilesActivity::create()
{
    // From the current limits when they can be read, else from no limit.
    pctl_play_timer_query(&this->live);
    std::array<uint16_t, 7> days;
    for (int i = 0; i < 7; i++) days[i] = this->live.valid ? this->live.day_min[i] : PT_DAY_NOLIMIT;
    this->ask_name("", "", [this, days](std::string name) {
        brls::Application::pushActivity(new PlayTimerPerDayActivity(
            brls::getStr("playguard/play_timer/profile_edit_title", name), days,
            [this, name](const PlayTimerPerDayActivity::Days& edited) {
                profiles::Profile p;
                p.name = name;
                p.days = edited;
                return this->store(p, brls::getStr("playguard/play_timer/profile_saved", name));
            }));
    });
}

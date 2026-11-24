// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "activity/companion_activity.hpp"

#include "action/console_actions.hpp"
#include "action/pairing_state.hpp"
#include "action/pt_block_flow.hpp"
#include "app.hpp"
#include "util/pctl_ops_c.hpp"

using namespace aurore::literals;

void CompanionActivity::onContentAvailable()
{
    note->setSingleLine(false);
    // While linked, anyone can save the block before and after a change made
    // in the phone app: the data ROADMAP.md waits for (Developer tools has it too).
    ui::set_detail(decode, ui::fmt_played(2));   // how long it takes
    decode->registerClickAction([](aurore::View*) {
        pt_block_flow::open();
        return true;
    });
    unlink->registerClickAction([this](aurore::View*) {
        console_actions::unlink("", ui::guarded(this->alive, [this]() { this->refresh(); }));
        return true;
    });
    this->list = std::make_unique<cadran::List>(this->getContentView(), true);
    this->list->apply({ { "pr_active", "phone" }, { "pr_updated", "refresh" }, { "pr_decode", "code" }, { "pr_unlink", "stop" } });
    this->refresh();
}

void CompanionActivity::onResume()
{
    this->refresh();
}

void CompanionActivity::refresh()
{
    PctlStatus s;
    pctl_status_fetch(&s);
    // While linked, its next sync overwrites what is set here, unless
    // Atmosphère's DNS blocks its server (then the note says so).
    const bool paired = s.pairing_active_ok && s.pairing_active;
    const bool syncs = pairing_state::syncs(s);
    std::string text = ui::bool_text(s.pairing_active_ok, s.pairing_active, "cadran/common/yes"_i18n, "cadran/common/no"_i18n);
    if (paired && !syncs) text += " · " + "playguard/pairing/dns_blocked"_i18n;
    ui::set_detail(active, text);
    active->detail->setThemeColor("textColor", syncs ? "cadran/amber" : "cadran/text2");
    ui::set_detail(updated, s.last_updated_ok ? ui::time_text(s.last_updated) : "cadran/common/unavailable"_i18n);
    ui::set_text(note, paired && !syncs ? "playguard/pairing/dns_note"_i18n : "playguard/pairing/note"_i18n);
    note->setThemeColor("textColor", syncs ? "cadran/amber" : "cadran/text3");
    // Nothing to unlink when the read says not linked; the block comparison
    // needs the app to sync.
    ui::set_visible_all({ { unlink.getView(), !s.pairing_active_ok || paired }, { decode.getView(), syncs } });
    ui::show_writable(unlink, !app::read_only());
    if (this->list) this->list->sync();
}

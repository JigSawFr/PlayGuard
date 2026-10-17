// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/support_flow.hpp"

#include "action/notices.hpp"
#include "activity/whats_new_activity.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/support.hpp"
#include "view/funding.hpp"
#include "view/release_notes.hpp"

using namespace brls::literals;

namespace support_flow
{
namespace
{
// Long enough for the start-up screens and questions to be on top already.
constexpr long START_DELAY_MS = 1500;

void show_reminder()
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(28, 32, 12, 32);
    auto* title = new brls::Label();
    title->setFontSize(26);
    title->setText("playguard/about/reminder_title"_i18n);
    box->addView(title);
    auto* body = new brls::Label();
    body->setSingleLine(false);
    body->setFontSize(20);
    body->setMarginTop(10);
    body->setText("playguard/about/support_body"_i18n);
    box->addView(body);
    brls::Box* codes = funding::cards(160, false);
    codes->setMarginTop(12);
    box->addView(codes);

    auto* dialog = new brls::Dialog(box);
    dialog->addButton("playguard/common/later"_i18n, []() {});
    dialog->addButton("playguard/about/reminder_never"_i18n, []() {
        config::get().support_reminder = false;
        ui::save_config("playguard/about/reminder_off"_i18n);
    });
    dialog->open();
}
}   // namespace

void at_start(brls::Activity* main)
{
    // Another version than the last one started: the bell says it was
    // installed (an update in place says it with the previous build).
    const std::string seen = config::get().seen_version;
    if (!seen.empty() && seen != app::version())
        notices::set("installed", notices::Level::Info, brls::getStr("playguard/notices/installed", app::version()),
                     brls::getStr("playguard/notices/updated_from", seen), "playguard/notices/whats_new"_i18n,
                     []() { brls::Application::pushActivity(new WhatsNewActivity()); });

    brls::delay(START_DELAY_MS, [main]() {
        if (!ui::on_top(main)) return;   // something else first: next start

        auto& cfg = config::get();
        const support::State state{ cfg.support_reminder, cfg.support_reminded, cfg.seen_version };
        const bool has_notes = !release_notes::current().empty() || !release_notes::highlights().empty();
        const support::Decision d = support::decide(state, app::version(), ui::today_date(), has_notes);
        // Another screen of its own came first this start (First steps, the
        // update): kept, unsaved, for the next.
        if (d.show != support::Show::Nothing && !ui::claim_start_screen("what's new or the reminder")) return;
        if (d.next.reminded != state.reminded || d.next.seen_version != state.seen_version) {
            cfg.support_reminded = d.next.reminded;
            cfg.seen_version     = d.next.seen_version;
            ui::save_config();
        }
        if (d.show == support::Show::WhatsNew) brls::Application::pushActivity(new WhatsNewActivity());
        else if (d.show == support::Show::Reminder) show_reminder();
    });
}

}   // namespace support_flow

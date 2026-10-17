// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/notices_activity.hpp"

#include "action/notices.hpp"
#include "action/notice_logic.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

NoticesActivity::~NoticesActivity()
{
    *this->alive = false;
    notices::unlisten(this->listener);
}

void NoticesActivity::open()
{
    brls::Logger::info("notices: {} to do, {} for information, {} about the console to do", notices::action_count(),
                       notices::info_count(), notices::console_count());
    if (notices::list().empty()) {
        ui::notify("playguard/notices/none"_i18n);
        return;
    }
    brls::Application::pushActivity(new NoticesActivity());
}

void NoticesActivity::onContentAvailable()
{
    this->rebuild();
    // A change while the list is open (a source cleared what it fixed):
    // rebuilt on the next frame, outside the source's own call.
    // `alive`: the screen can be gone by then.
    // Under a dialog (an upload's link, an error): rebuilt once back on top,
    // as the dialog gives the focus back to a cell a rebuild deletes, and
    // closing would close the dialog instead.
    this->listener = notices::listen([this]() {
        brls::sync([this, alive = this->alive]() {
            if (!*alive || this->closing) return;
            if (ui::has_focus(this)) this->rebuild();
            else this->stale = true;
        });
    });
}

void NoticesActivity::onResume()
{
    if (!this->stale) return;
    this->stale = false;
    // Next frame: borealis first gives the focus back to the cell that
    // opened the dialog, which rebuild() then moves away before deleting it.
    brls::sync([this, alive = this->alive]() {
        if (*alive && !this->closing) this->rebuild();
    });
}

void NoticesActivity::rebuild()
{
    const auto all = notices::list();
    if (all.empty()) {
        // Nothing left: back to the main screen, once.
        this->closing = true;
        brls::Application::popActivity();
        ui::notify("playguard/notices/none"_i18n);
        return;
    }

    // New cells in a new box, the focus moved to it, then the old box
    // removed: borealis would keep a pointer to a deleted focused cell.
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    brls::View* first = nullptr;
    brls::View* focus = nullptr;
    bool parental = false;
    for (const notices::Entry& e : all) parental |= e.item.group == notice_logic::Group::Parental;
    // The console's entries: on one line until opened, unless they are all
    // there is.
    const bool console_shown = this->console_open || !parental;
    enum Section { None, Todo, Info, Console } section = None;
    const auto header = [&](const std::string& title) {
        auto* h = new brls::Header();
        h->setTitle(title);
        h->setMarginTop(first ? 24 : 0);
        h->setMarginBottom(8);
        box->addView(h);
    };
    for (const notices::Entry& e : all) {
        const bool todo = e.item.level == notices::Level::Action;
        const bool console = e.item.group == notice_logic::Group::Console;
        const Section here = console ? Console : todo ? Todo : Info;
        if (here != section) {
            section = here;
            header(here == Todo ? "playguard/notices/section_todo"_i18n
                   : here == Info ? "playguard/notices/section_info"_i18n
                                  : "playguard/notices/section_console"_i18n);
            if (console && !console_shown) {
                int count = 0, todos = 0;
                for (const notices::Entry& c : all)
                    if (c.item.group == notice_logic::Group::Console) count++, todos += c.item.level == notices::Level::Action;
                auto* more = new brls::DetailCell();
                more->setText(brls::getStr("playguard/notices/console_count", count));
                if (todos) more->title->setTextColor(ui::color_warn());
                more->setDetailText(ui::opens("playguard/notices/console_show"_i18n));
                more->registerClickAction([this](brls::View*) {
                    // Next frame: the rebuild deletes this cell.
                    this->console_open = true;
                    brls::sync([this, alive = this->alive]() {
                        if (*alive && !this->closing) this->rebuild();
                    });
                    return true;
                });
                box->addView(more);
                if (!first) first = more;
                break;
            }
        }

        auto* cell = new brls::DetailCell();
        cell->setText(e.item.title);
        if (todo) cell->title->setTextColor(ui::color_warn());
        cell->setDetailTextColor(ui::color_neutral());
        const bool has_action = !e.action_label.empty() && e.action;
        // To do with nothing PlayGuard can do (fixed on the SD card, from a
        // computer): no button text, as it goes only once fixed.
        cell->setDetailText(has_action ? e.action_label + "  ›" : todo ? "" : "playguard/notices/seen"_i18n);
        const std::string id = e.item.id;
        const auto action = e.action;
        cell->registerClickAction([this, id, todo, has_action, action](brls::View*) {
            if (!has_action) {
                if (!todo) notices::dismiss(id);
                return true;
            }
            // Information acted on is read.
            if (!todo) notices::dismiss(id);
            this->closing = true;
            brls::Application::popActivity(brls::TransitionAnimation::FADE, [action]() { action(); });
            return true;
        });
        box->addView(cell);
        if (!first) first = cell;
        // Just opened: the focus on the console's first entry.
        if (console && this->console_open && !focus) focus = cell;

        if (!e.item.detail.empty()) {
            auto* text = new brls::Label();
            text->setSingleLine(false);
            text->setFontSize(18);
            text->setTextColor(ui::color_note());
            text->setText(e.item.detail);
            text->setMarginLeft(16);
            text->setMarginRight(16);
            text->setMarginTop(8);
            text->setMarginBottom(12);
            box->addView(text);
        }
    }

    root->addView(box);
    brls::Application::giveFocus(focus ? focus : first);
    if (this->content) root->removeView(this->content);
    this->content = box;
}

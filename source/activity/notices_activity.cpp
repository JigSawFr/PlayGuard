// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/notices_activity.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <aurore/cadran/ui/cadran_frame.hpp>
#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/ui/cadran_list.hpp>

#include "action/notices.hpp"
#include "action/notice_logic.hpp"
#include "action/pin_lock.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace aurore::literals;

namespace
{
// What an entry says below its title.
aurore::Label* detail_label(const std::string& detail)
{
    auto* text = new aurore::Label();
    text->setSingleLine(false);
    text->setFontSize(18);
    text->setThemeColor("textColor", "cadran/text3");
    text->setText(detail);
    text->setMarginLeft(16);
    text->setMarginRight(16);
    text->setMarginTop(8);
    text->setMarginBottom(12);
    return text;
}
}   // namespace

NoticesActivity::~NoticesActivity()
{
    *this->alive = false;
    notices::unlisten(this->listener);
}

void NoticesActivity::open()
{
    aurore::Logger::info("notices: {} to do, {} for information, {} about the console to do", notices::action_count(),
                       notices::info_count(), notices::console_count());
    if (notices::list().empty() && notices::ignored().empty()) {
        ui::notify("playguard/notices/none"_i18n);
        return;
    }
    aurore::Application::pushActivity(new NoticesActivity());
}

void NoticesActivity::onContentAvailable()
{
    // Every entry for information put away at once.
    this->getContentView()->registerAction("playguard/notices/read_all"_i18n, aurore::BUTTON_Y, [](aurore::View*) {
        notices::dismiss_all();
        return true;
    });
    // A screen's margins, as the other screens have.
    root->setPadding(10, 72, 32, 72);
    this->rebuild();
    // A change while the list is open (a source cleared what it fixed):
    // rebuilt on the next frame, outside the source's own call.
    // `alive`: the screen can be gone by then.
    // Under a dialog (an upload's link, an error): rebuilt once back on top,
    // as the dialog gives the focus back to a cell a rebuild deletes, and
    // closing would close the dialog instead.
    this->listener = notices::listen([this]() {
        aurore::sync(ui::guarded(this->alive, [this]() {
            if (this->closing) return;
            if (ui::has_focus(this)) this->rebuild();
            else this->stale = true;
        }));
    });
}

void NoticesActivity::onResume()
{
    if (!this->stale) return;
    this->stale = false;
    // Next frame: Aurore first gives the focus back to the cell that
    // opened the dialog, which rebuild() then moves away before deleting it.
    aurore::sync(ui::guarded(this->alive, [this]() { if (!this->closing) this->rebuild(); }));
}

// The console's entries folded into one line, with their count: A opens them.
aurore::DetailCell* NoticesActivity::console_line(const std::vector<notices::Entry>& all)
{
    int count = 0, todos = 0;
    for (const notices::Entry& c : all)
        if (c.item.group == notice_logic::Group::Console) count++, todos += c.item.level == notices::Level::Action;
    auto* more = new aurore::DetailCell();
    more->setText(aurore::getStr("playguard/notices/console_count", count));
    if (todos) more->title->setThemeColor("textColor", "cadran/amber");
    more->setDetailText(ui::opens("playguard/notices/console_show"_i18n));
    more->registerClickAction([this](aurore::View*) {
        // Next frame: the rebuild deletes this cell.
        this->console_open = true;
        aurore::sync(ui::guarded(this->alive, [this]() { if (!this->closing) this->rebuild(); }));
        return true;
    });
    return more;
}

// An entry: its title (amber when it is to do), and A for what fixes it, or
// to mark it read. X marks one for information read, or ignores one to do
// the parent may ignore (after a confirmation and the PIN).
aurore::DetailCell* NoticesActivity::entry_cell(const notices::Entry& e, bool todo)
{
    auto* cell = new aurore::DetailCell();
    cell->setText(e.item.title);
    if (todo) cell->title->setThemeColor("textColor", "cadran/amber");
    cell->detail->setThemeColor("textColor", "cadran/text2");
    const bool has_action = !e.action_label.empty() && e.action;
    // To do with nothing PlayGuard can do (fixed on the SD card, from a
    // computer): no button text, as it goes only once fixed.
    cell->setDetailText(has_action ? e.action_label + "  ›" : todo ? "" : "playguard/notices/seen"_i18n);
    const std::string id = e.item.id;
    const auto action = e.action;
    cell->registerClickAction([this, id, todo, has_action, action](aurore::View*) {
        if (!has_action) {
            if (!todo) notices::dismiss(id);
            return true;
        }
        // Information acted on is read.
        if (!todo) notices::dismiss(id);
        this->closing = true;
        aurore::Application::popActivity(aurore::TransitionAnimation::FADE, [action]() { action(); });
        return true;
    });
    if (!todo) {
        cell->registerAction("playguard/notices/seen"_i18n, aurore::BUTTON_X, [id](aurore::View*) {
            notices::dismiss(id);
            return true;
        });
    } else if (notice_logic::ackable(id)) {
        const std::string title = e.item.title;
        cell->registerAction("playguard/notices/ignore"_i18n, aurore::BUTTON_X, [id, title](aurore::View*) {
            const std::string body = aurore::getStr(config::get().notices_ack_remind ? "playguard/notices/ignore_confirm"
                                                                                   : "playguard/notices/ignore_confirm_kept",
                                                  title);
            ui::confirm(body, "playguard/notices/ignore"_i18n, [id]() {
                // A choice about the parental controls: the parent's.
                if (!pin_lock::confirm()) {
                    ui::refuse(pin_lock::refusal_text());
                    return;
                }
                notices::acknowledge(id);
            });
            return true;
        });
    }
    return cell;
}

// An entry ignored: its title (not amber: no longer counted), and A to
// count it again.
aurore::DetailCell* NoticesActivity::ignored_cell(const notices::Entry& e)
{
    auto* cell = new aurore::DetailCell();
    cell->setText(e.item.title);
    cell->detail->setThemeColor("textColor", "cadran/text2");
    cell->setDetailText("playguard/notices/reactivate"_i18n);
    const std::string id = e.item.id;
    cell->registerClickAction([id](aurore::View*) {
        notices::reactivate(id);
        return true;
    });
    return cell;
}

void NoticesActivity::rebuild()
{
    const auto all = notices::list();
    const auto ignored = notices::ignored();
    if (all.empty() && ignored.empty()) {
        // Nothing left: back to the main screen, once.
        this->closing = true;
        aurore::Application::popActivity();
        ui::notify("playguard/notices/none"_i18n);
        return;
    }

    // New cells in a new box, the focus moved to it, then the old box
    // removed: Aurore would keep a pointer to a deleted focused cell.
    auto* box = new aurore::Box(aurore::Axis::COLUMN);
    box->setAlignItems(aurore::AlignItems::STRETCH);
    aurore::View* first = nullptr;
    aurore::View* focus = nullptr;
    bool parental = false;
    for (const notices::Entry& e : all) parental |= e.item.group == notice_logic::Group::Parental;
    // The console's entries: on one line until opened, unless they are all
    // there is.
    const bool console_shown = this->console_open || !parental;
    enum Section { None, Todo, Info, Console } section = None;
    // Each entry's icon, as Today's pills mark them: a warning for what is to
    // do (amber), an "i" for what is only said, so the colour is not alone.
    std::vector<std::string> ids;
    std::vector<aurore::DetailCell*> todos;
    const auto mark = [&ids](aurore::DetailCell* cell) {
        ids.push_back("notice_line_" + std::to_string(ids.size()));
        cell->setId(ids.back());
    };
    const auto header = [&](const std::string& title) {
        auto* h = new aurore::Header();
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
                aurore::DetailCell* more = this->console_line(all);
                box->addView(more);
                if (!first) first = more;
                break;
            }
        }

        aurore::DetailCell* cell = this->entry_cell(e, todo);
        mark(cell);
        if (todo) todos.push_back(cell);
        box->addView(cell);
        if (!first) first = cell;
        // Just opened: the focus on the console's first entry.
        if (console && this->console_open && !focus) focus = cell;

        if (!e.item.detail.empty()) box->addView(detail_label(e.item.detail));
    }

    // Last, what the parent chose to ignore.
    if (!ignored.empty()) {
        header("playguard/notices/section_ignored"_i18n);
        for (const notices::Entry& e : ignored) {
            aurore::DetailCell* cell = this->ignored_cell(e);
            box->addView(cell);
            if (!first) first = cell;
            const std::string until = notices::ignored_until(e.item.id);
            box->addView(detail_label(until.empty() ? "playguard/notices/ignored_kept"_i18n
                                                    : aurore::getStr("playguard/notices/ignored_until", until)));
        }
    }

    // The entries in cards, before they are shown.
    std::vector<std::pair<const char*, const char*>> icons;
    for (size_t i = 0; i < ids.size(); i++) {
        auto* cell = static_cast<aurore::DetailCell*>(box->getView(ids[i]));
        const bool todo = std::ranges::contains(todos, cell);
        icons.push_back({ ids[i].c_str(), todo ? "alert" : "info" });
    }
    cadran::List(box, box).apply(icons);   // the icons found in the box, not on screen yet
    for (aurore::DetailCell* cell : todos) cadran::List::recolor_icon(cell, nullptr, "cadran/amber", "cadran/amber_surf");
    root->addView(box);
    aurore::Application::giveFocus(focus ? focus : first);
    if (this->content) root->removeView(this->content);
    this->content = box;
}

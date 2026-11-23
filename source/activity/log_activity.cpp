// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/log_activity.hpp"

#include <utility>

#include <aurore/cadran/ui/cadran_list.hpp>

#include "action/export_flow.hpp"
#include "action/sync_flow.hpp"
#include "ui/play_ui.hpp"
#include "util/table_export.hpp"

using namespace aurore::literals;

namespace
{
using log_logic::Filter;
using log_logic::Kind;

std::string filter_label(Filter f)
{
    return aurore::getStr(std::format("playguard/log/filters/{}", std::to_underlying(f)));
}

std::string kind_label(Kind k)
{
    switch (k) {
        case Kind::App: return filter_label(Filter::App);
        case Kind::Link: return filter_label(Filter::Link);
        case Kind::Loop: return filter_label(Filter::Loop);
        case Kind::Other: break;
    }
    return {};
}

aurore::DetailCell* new_slot()
{
    auto* cell = new aurore::DetailCell();
    cell->title->setSingleLine(false);   // a long line wraps instead of being cut
    cadran::List::row(cell, nullptr, true);
    return cell;
}
}   // namespace

void LogActivity::onContentAvailable()
{
    note->setSingleLine(false);
    empty->setSingleLine(false);
    filter_cell->registerClickAction([this](aurore::View*) {
        this->choose_filter();
        return true;
    });
    export_cell->registerClickAction([this](aurore::View*) {
        this->export_shown();
        return true;
    });
    this->rebuild();
}

void LogActivity::choose_filter()
{
    std::vector<std::string> labels;
    for (int f = 0; f < std::to_underlying(Filter::Count); f++) labels.push_back(filter_label((Filter)f));
    ui::pick("playguard/history/show"_i18n, labels, std::to_underlying(this->filter), [this](int index) {
        this->filter = (Filter)index;
        this->rebuild();
    });
}

void LogActivity::export_shown()
{
    if (this->shown.empty()) {
        ui::notify("playguard/log/none_shown"_i18n);
        return;
    }
    table_export::Table t;
    t.title    = "PlayGuard — " + "playguard/sync/log"_i18n;
    t.subtitle = ui::now_stamp();
    t.sheet    = "playguard/sync/log"_i18n;
    t.columns  = {
        { "when", "playguard/history/when"_i18n, false },
        { "kind", "playguard/log/kind"_i18n, false },
        { "text", "playguard/history/columns/details"_i18n, false },
    };
    for (const log_logic::Line& l : this->shown) t.rows.push_back({ l.time, kind_label(l.kind), l.text });
    export_flow::choose("export journal", "journal", [t]() { return t; });
}

void LogActivity::rebuild()
{
    // The log is read once per rebuild (a filter or the screen opening), not
    // per line; the agent's answer can take a moment, so not every second.
    this->all.clear();
    const std::vector<std::string> raw = sync_flow::log_lines();
    for (size_t i = raw.size(); i > 0; i--) this->all.push_back(log_logic::parse(raw[i - 1]));

    this->shown.clear();
    for (const log_logic::Line& l : this->all)
        if (log_logic::matches(l, this->filter)) this->shown.push_back(l);
    filter_cell->setDetailText(aurore::getStr("playguard/log/shown_count", filter_label(this->filter), (int)this->shown.size()));

    // The slots are reused in place (one can hold the focus); those past
    // what is shown are hidden, and the focus goes to the filter if it was on
    // one of them.
    aurore::View* focus = aurore::Application::getCurrentFocus();
    bool focus_hidden = false;
    ui::batch_layout(list, [&]() {
        for (size_t i = 0; i < this->shown.size(); i++) {
            if (i >= slots.size()) {
                slots.push_back(new_slot());
                list->addView(slots.back());
            }
            const log_logic::Line& l = this->shown[i];
            aurore::DetailCell* cell = slots[i];
            cell->setText(l.text);
            cell->setDetailText(l.time);
            cell->detail->setThemeColor("textColor", l.alert ? "cadran/amber" : "cadran/text2");
            ui::set_visible(cell, true);
        }
        for (size_t i = this->shown.size(); i < slots.size(); i++) {
            if (focus == slots[i]) focus_hidden = true;
            ui::set_visible(slots[i], false);
        }
    });
    ui::set_visible(empty.getView(), this->shown.empty());
    empty->setText(this->all.empty() ? "playguard/sync/log_empty"_i18n : "playguard/log/none_shown"_i18n);
    if (focus_hidden) aurore::Application::giveFocus(filter_cell.getView());
    aurore::Logger::info("journal: {} of {} lines", this->shown.size(), this->all.size());
}

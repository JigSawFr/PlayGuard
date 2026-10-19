// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/history_activity.hpp"

#include "action/export_flow.hpp"
#include "action/history_flow.hpp"
#include "tab/cadran_list.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "util/paths.hpp"
#include "util/perf.hpp"
#include "util/table_export.hpp"

using namespace brls::literals;

namespace
{
using history_logic::Filter;

// One line of the list: a day's title (shown when the line starts a day)
// over the change.
brls::Header* day_of_slot(brls::View* slot) { return (brls::Header*)((brls::Box*)slot)->getChildren()[0]; }
brls::DetailCell* cell_of_slot(brls::View* slot) { return (brls::DetailCell*)((brls::Box*)slot)->getChildren()[1]; }

brls::Box* new_slot()
{
    auto* slot = new brls::Box(brls::Axis::COLUMN);
    slot->setAlignItems(brls::AlignItems::STRETCH);
    auto* day = new brls::Header();
    day->setMarginTop(12);
    day->setMarginBottom(4);
    slot->addView(day);
    auto* cell = new brls::DetailCell();
    cell->setDetailTextColor(ui::color_neutral());
    slot->addView(cell);
    // Each change a card of its own, under its day.
    cadran::List::section(day);
    cadran::List::row(cell, nullptr, true);
    return slot;
}
}   // namespace

void HistoryActivity::onContentAvailable()
{
    note->setSingleLine(false);
    empty->setSingleLine(false);
    note->setText(brls::getStr("playguard/history/note", (int)history::MAX_ENTRIES, paths::history_file()));
    filter_cell->registerClickAction([this](brls::View*) {
        this->choose_filter();
        return true;
    });
    export_cell->registerClickAction([this](brls::View*) {
        this->export_shown();
        return true;
    });
    this->empty_card = cadran::kit::empty("history");
    this->empty_card.box->setMarginTop(16);
    if (auto* root = dynamic_cast<brls::Box*>(empty->getParent())) root->addView(this->empty_card.box);
    this->rebuild();
}

void HistoryActivity::choose_filter()
{
    std::vector<std::string> labels;
    for (int f = 0; f < (int)Filter::Count; f++) labels.push_back(history_flow::filter_label((Filter)f));
    ui::pick("playguard/history/show"_i18n, labels, (int)this->filter, [this](int index) {
        this->filter = (Filter)index;
        this->rebuild();
    });
}

void HistoryActivity::export_shown()
{
    if (this->shown.empty()) {
        ui::notify("playguard/history/none_shown"_i18n);
        return;
    }
    const table_export::Table t = history_flow::table(this->shown);
    export_flow::choose("export history", "history", [t]() { return t; });
}

void HistoryActivity::rebuild()
{
    perf::Timed timed("Change history list");   // the diagnostic report's timings
    // The file again only when an entry was recorded since (an undo, a change
    // made from a dialog), not for another filter.
    if (!this->read || this->read_at != history_flow::changes()) {
        this->read_at = history_flow::changes();
        this->all = history_flow::load();
        this->read = true;
    }
    this->shown.clear();
    for (const history::Entry& e : this->all)
        if (history_logic::matches(e, this->filter)) this->shown.push_back(e);
    filter_cell->setDetailText(brls::getStr("playguard/history/shown_count", history_flow::filter_label(this->filter),
                                            (int)this->shown.size()));

    // The slots are reused in place: one of them has the focus when this runs
    // after a change, and borealis would keep a pointer to a deleted cell.
    // There are only ever as many as the most shown at once (up to
    // history::MAX_ENTRIES); those past what is shown are hidden. Filled out
    // of the screen's layout, laid out once (ui::batch_layout).
    auto& slots = list->getChildren();
    // A hidden line must not keep the focus: it goes to the filter.
    brls::View* focus = brls::Application::getCurrentFocus();
    bool focus_hidden = false;
    ui::batch_layout(list, [&]() {
        std::string day;
        for (size_t i = 0; i < this->shown.size(); i++) {
            if (i >= slots.size()) list->addView(new_slot());
            brls::View* slot = slots[i];
            const history::Entry e = this->shown[i];
            // A day's title over its first change.
            const std::string this_day = history_logic::day_key(e);
            brls::Header* header = day_of_slot(slot);
            const bool starts_day = i == 0 || this_day != day;
            if (starts_day) {
                const std::string title = history_flow::day_title(this_day);
                header->setTitle(cadran::upper(title));
            }
            ui::set_visible(header, starts_day);
            day = this_day;

            brls::DetailCell* cell = cell_of_slot(slot);
            cell->setText(history_flow::title(e));
            cell->setDetailText(history_flow::time_of(e));
            // A change the console refused stands out.
            cell->setDetailTextColor(e.result.empty() ? ui::color_neutral() : ui::color_warn());
            cell->registerClickAction([this, e](brls::View*) {
                history_flow::open(e, [this]() { this->rebuild(); });
                return true;
            });
        }
        for (size_t i = this->shown.size(); i < slots.size(); i++) {
            if (focus == cell_of_slot(slots[i])) focus_hidden = true;
            ui::set_visible(slots[i], false);
        }
        for (size_t i = 0; i < this->shown.size(); i++) ui::set_visible(slots[i], true);
    });
    if (this->empty_card.box) {
        const bool all = this->filter == Filter::All;
        ui::set_visible(empty.getView(), false);
        ui::set_visible(this->empty_card.box, this->shown.empty());
        this->empty_card.title->setText(all ? "playguard/history/empty_title"_i18n : "playguard/history/none_shown"_i18n);
        this->empty_card.text->setText(all ? "playguard/history/empty_text"_i18n : "");
        ui::set_visible(this->empty_card.text, all);
    } else {
        ui::set_visible(empty.getView(), this->shown.empty());
        empty->setText(this->filter == Filter::All ? "playguard/history/none"_i18n : "playguard/history/none_shown"_i18n);
    }
    if (focus_hidden) brls::Application::giveFocus(filter_cell.getView());
    brls::Logger::info("history list: {} of {} changes", this->shown.size(), this->all.size());
}

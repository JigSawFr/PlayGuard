// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/tab_base.hpp"

#include "action/notices.hpp"
#include "action/frame_stats_flow.hpp"
#include "action/play_data.hpp"
#include "app.hpp"
#include "core/sim_env.h"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "view/tab_badge.hpp"

using namespace brls::literals;

// The tab last shown (tabs are deleted when another one is opened).
static TabBase* s_shown = nullptr;
static std::string s_main_screen;   // main_screen()

TabBase::TabBase(const std::string& xml_res, bool refreshes)
{
    this->inflateFromXMLRes(xml_res);
    // "xml/tab/overview.xml" -> "tab/overview"
    this->screen = xml_res.substr(xml_res.rfind("xml/") == 0 ? 4 : 0);
    if (this->screen.size() > 4 && this->screen.compare(this->screen.size() - 4, 4, ".xml") == 0)
        this->screen.resize(this->screen.size() - 4);
    if (refreshes)
        this->registerAction("playguard/hints/refresh"_i18n, brls::BUTTON_X, [this](brls::View*) {
            this->refresh();
            return true;
        });
    if (auto* note = dynamic_cast<brls::Label*>(this->getView("tab_ro_note"))) note->setSingleLine(false);
}

TabBase::~TabBase()
{
    if (s_shown == this) s_shown = nullptr;
    if (this->notice_listener) notices::unlisten(this->notice_listener);
}

void TabBase::mark(brls::Box* row, std::vector<std::string> ids)
{
    TabBadge* badge = TabBadge::on_row(row);
    if (!badge) return;
    this->marks.push_back({ row, badge, std::move(ids) });
    if (!this->notice_listener) this->notice_listener = notices::listen([this]() { this->update_marks(); });
    this->update_marks();
}

void TabBase::update_marks()
{
    for (const auto& m : this->marks) {
        const int count = notices::count_of(m.ids);
        m.badge->set_count(count);
        // A row whose value is only its "›": the badge says the way in.
        if (auto* cell = dynamic_cast<brls::DetailCell*>(m.row))
            if (cadran::kit::without_chevron(cell->detail->getFullText()).empty())
                cell->detail->setVisibility(count > 0 ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    }
}

brls::View* TabBase::getDefaultFocus()
{
    // Only the first time: back from the sidebar later, the row left.
    if (!this->focused_once)
        for (const auto& m : this->marks)
            if (m.row->getVisibility() == brls::Visibility::VISIBLE && notices::count_of(m.ids) > 0) return m.row;
    return brls::Box::getDefaultFocus();
}

void TabBase::onChildFocusGained(brls::View* directChild, brls::View* focusedView)
{
    brls::Box::onChildFocusGained(directChild, focusedView);
    this->focused_once = true;
}

void TabBase::update_ro_note()
{
    if (auto* note = dynamic_cast<brls::Label*>(this->getView("tab_ro_note"))) ui::set_visible(note, app::read_only());
}

void TabBase::willAppear(bool resetState)
{
    brls::Box::willAppear(resetState);
    s_shown = this;
    for (brls::View* v = this; v; v = v->hasParent() ? v->getParent() : nullptr)
        if (v->getId() == "main_tabs") s_main_screen = this->screen;
    frame_stats_flow::set_screen(this->screen);
    this->update_ro_note();
    this->refresh();
    // The smoke test's "alarm" (PLAYGUARD_SIM_TAB_RACE): a read of the play
    // data ending as a tab replaces another, the one replaced not freed yet.
    // Only the new tab may answer (the header's pill, #251).
    if (sim_env_flag("PLAYGUARD_SIM_TAB_RACE")) play_data::notify();
}

bool TabBase::shown() const
{
    return s_shown == this;
}

std::string TabBase::shown_screen()
{
    return s_shown ? s_shown->screen : "";
}

std::string TabBase::main_screen()
{
    return s_main_screen;
}

void TabBase::refresh_shown()
{
    if (!s_shown) return;
    s_shown->update_ro_note();
    s_shown->refresh();
}

void TabBase::enable_auto_refresh(int period_ms)
{
    this->auto_timer.setPeriod(period_ms);
    this->auto_timer.setCallback([this]() {
        if (!app::in_focus()) return;
        if (!this->shown()) return;   // replaced, waiting to be freed
        if (!ui::on_top(this->getParentActivity())) return;
        this->refresh();
    });
    this->auto_timer.start();
}

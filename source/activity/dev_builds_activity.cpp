// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/dev_builds_activity.hpp"

#include <fmt/format.h>

#include "action/dev_build_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"

using namespace brls::literals;

namespace
{
DevBuildsActivity* s_open = nullptr;   // the screen, while it is open
// The tab last chosen in this run; none yet: the installed build's.
bool              s_tab_chosen = false;
dev_builds::Kind  s_tab = dev_builds::Kind::Release;

constexpr dev_builds::Kind KINDS[] = { dev_builds::Kind::Release, dev_builds::Kind::Main,
                                       dev_builds::Kind::PullRequest };

dev_builds::Order order()
{
    return dev_builds::order_named(config::get().dev_builds_order);
}
}   // namespace

void DevBuildsActivity::show(const dev_builds::Cache& cache)
{
    if (s_open) {
        s_open->update(cache);
        return;
    }
    brls::Application::pushActivity(new DevBuildsActivity(cache));
}

DevBuildsActivity::DevBuildsActivity(const dev_builds::Cache& cache) : cache(cache)
{
    s_open = this;
    if (s_tab_chosen) {
        this->tab = s_tab;
        return;
    }
    for (const dev_builds::Build& b : cache.builds)
        if (dev_build_flow::installed(b)) this->tab = b.kind;
}

DevBuildsActivity::~DevBuildsActivity()
{
    if (s_open == this) s_open = nullptr;
}

void DevBuildsActivity::onContentAvailable()
{
    note->setSingleLine(false);
    // The version only: the running commit is marked in the list, and named
    // in the confirmation (and a commit here would change every screenshot).
    note->setText(brls::getStr("playguard/dev_build/note", app::version()));
    const std::pair<brls::Button*, dev_builds::Kind> buttons[] = { { tab_release.getView(), KINDS[0] },
                                                                   { tab_main.getView(), KINDS[1] },
                                                                   { tab_pulls.getView(), KINDS[2] } };
    for (const auto& b : buttons) {
        const dev_builds::Kind kind = b.second;
        b.first->registerClickAction([this, kind](brls::View*) {
            this->show_tab(kind, false);
            return true;
        });
    }
    sign_in->registerClickAction([](brls::View*) {
        dev_build_flow::sign_in();
        return true;
    });
    this->getContentView()->registerAction("playguard/dev_build/next_tab"_i18n, brls::BUTTON_RB, [this](brls::View*) {
        this->next_tab(1);
        return true;
    });
    this->getContentView()->registerAction("", brls::BUTTON_LB, [this](brls::View*) {
        this->next_tab(-1);
        return true;
    }, true);
    this->getContentView()->registerAction("playguard/dev_build/sort"_i18n, brls::BUTTON_Y, [this](brls::View*) {
        this->sort();
        return true;
    });
    this->getContentView()->registerAction("playguard/hints/refresh"_i18n, brls::BUTTON_X, [](brls::View*) {
        dev_build_flow::open(true);
        return true;
    });
    this->fill();
    // The screen opens on the first build, not on the tabs above it.
    if (!list->getChildren().empty() && list->getChildren()[0]->getVisibility() == brls::Visibility::VISIBLE)
        brls::Application::giveFocus(list->getChildren()[0]);
}

void DevBuildsActivity::update(const dev_builds::Cache& fresh)
{
    // Signed in from this screen: its line goes, the focus to the first build.
    const bool focus_moves = !fresh.needs_login && brls::Application::getCurrentFocus() == sign_in.getView();
    this->cache = fresh;
    this->fill();
    if (focus_moves && !list->getChildren().empty() &&
        list->getChildren()[0]->getVisibility() == brls::Visibility::VISIBLE)
        brls::Application::giveFocus(list->getChildren()[0]);
}

std::vector<dev_builds::Build> DevBuildsActivity::builds(dev_builds::Kind kind) const
{
    return dev_builds::of_kind(cache.builds, kind, order());
}

void DevBuildsActivity::fill()
{
    // A tab without builds is left out (signed out: the release only, and
    // then no tabs at all); the one shown always has some, when any has.
    if (this->builds(this->tab).empty())
        for (dev_builds::Kind k : KINDS)
            if (!this->builds(k).empty()) {
                this->tab = k;
                break;
            }
    const size_t mains = this->builds(dev_builds::Kind::Main).size();
    const size_t pulls = this->builds(dev_builds::Kind::PullRequest).size();
    tab_release->setText("playguard/dev_build/section_release"_i18n);
    tab_main->setText(brls::getStr("playguard/dev_build/section_main", (int)mains));
    tab_pulls->setText(brls::getStr("playguard/dev_build/section_pulls", (int)pulls,
                                    order() == dev_builds::Order::Number ? "playguard/dev_build/order_number"_i18n
                                                                         : "playguard/dev_build/order_newest"_i18n));
    tab_release->setStyle(this->tab == dev_builds::Kind::Release ? &brls::BUTTONSTYLE_PRIMARY : &brls::BUTTONSTYLE_DEFAULT);
    tab_main->setStyle(this->tab == dev_builds::Kind::Main ? &brls::BUTTONSTYLE_PRIMARY : &brls::BUTTONSTYLE_DEFAULT);
    tab_pulls->setStyle(this->tab == dev_builds::Kind::PullRequest ? &brls::BUTTONSTYLE_PRIMARY : &brls::BUTTONSTYLE_DEFAULT);
    ui::set_visible(tab_release.getView(), !this->builds(dev_builds::Kind::Release).empty());
    ui::set_visible(tab_main.getView(), mains > 0);
    ui::set_visible(tab_pulls.getView(), pulls > 0);
    brls::View* focus = brls::Application::getCurrentFocus();
    const bool tabs_go = mains == 0 && pulls == 0;
    ui::set_visible(tabs.getView(), !tabs_go);

    const std::vector<dev_builds::Build> shown = this->builds(this->tab);
    auto& cells = list->getChildren();
    for (size_t i = 0; i < shown.size(); i++) {
        brls::DetailCell* cell;
        if (i < cells.size()) {
            cell = (brls::DetailCell*)cells[i];
        } else {
            cell = new brls::DetailCell();
            list->addView(cell);
        }
        const dev_builds::Build b = shown[i];
        switch (b.kind) {
            case dev_builds::Kind::Release:
                cell->setText(brls::getStr("playguard/dev_build/release", b.version));
                cell->setDetailText(dev_build_flow::short_date(b.date));
                break;
            case dev_builds::Kind::Main:
                cell->setText(b.commit);
                cell->setDetailText(dev_build_flow::short_date(b.date));
                break;
            case dev_builds::Kind::PullRequest:
                cell->setText(fmt::format("#{} · {}", b.pr, b.title));
                cell->setDetailText(b.commit);
                break;
        }
        // The running build: "installed", in the brand teal.
        const bool here = dev_build_flow::installed(b);
        if (here) cell->setDetailText("playguard/dev_build/installed"_i18n);
        cell->setDetailTextColor(here ? ui::color_ok() : ui::color_neutral());
        cell->registerClickAction([b](brls::View*) {
            dev_build_flow::choose(b);
            return true;
        });
        ui::set_visible(cell, true);
    }
    for (size_t i = shown.size(); i < cells.size(); i++) ui::set_visible(cells[i], false);
    // Never the focus left on a tab that went with its row.
    if (tabs_go && focus && focus->getParent() == tabs.getView() && !shown.empty())
        brls::Application::giveFocus(cells[0]);
    // Without a token GitHub hands out the release only: a line signs in.
    ui::set_visible(sign_in.getView(), cache.needs_login);
    checked->setText(dev_build_flow::checked_text(cache));
}

void DevBuildsActivity::show_tab(dev_builds::Kind kind, bool focus_list)
{
    if (this->builds(kind).empty()) return;
    this->tab = kind;
    s_tab = kind;
    s_tab_chosen = true;
    this->fill();
    if (focus_list) brls::Application::giveFocus(list->getChildren()[0]);
}

void DevBuildsActivity::next_tab(int step)
{
    int at = (int)this->tab;
    for (int n = 0; n < 3; n++) {
        at = (at + step + 3) % 3;
        if (this->builds(KINDS[at]).empty()) continue;
        // From a tab, the focus goes to the tab chosen; else to its first build.
        brls::View* focus = brls::Application::getCurrentFocus();
        const bool on_tabs = focus && focus->getParent() == tabs.getView();
        this->show_tab(KINDS[at], !on_tabs);
        if (on_tabs) {
            brls::Button* b = at == 0 ? tab_release.getView() : at == 1 ? tab_main.getView() : tab_pulls.getView();
            brls::Application::giveFocus(b);
        }
        return;
    }
}

void DevBuildsActivity::sort()
{
    // The pull requests' tab, the focused one keeping the focus wherever it moves.
    int focused_pr = 0;
    auto& cells = list->getChildren();
    if (this->tab == dev_builds::Kind::PullRequest) {
        const std::vector<dev_builds::Build> before = this->builds(dev_builds::Kind::PullRequest);
        for (size_t i = 0; i < cells.size() && i < before.size(); i++)
            if (cells[i] == brls::Application::getCurrentFocus()) focused_pr = before[i].pr;
    }

    const bool by_number = order() == dev_builds::Order::Number;
    config::get().dev_builds_order = dev_builds::order_name(by_number ? dev_builds::Order::Newest : dev_builds::Order::Number);
    ui::save_config();
    this->fill();
    ui::notify(by_number ? "playguard/dev_build/order_newest"_i18n : "playguard/dev_build/order_number"_i18n);

    if (!focused_pr) return;
    const std::vector<dev_builds::Build> after = this->builds(dev_builds::Kind::PullRequest);
    for (size_t i = 0; i < after.size() && i < cells.size(); i++)
        if (after[i].pr == focused_pr) brls::Application::giveFocus(cells[i]);
}

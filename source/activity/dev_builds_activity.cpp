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
    release_header->setTitle("playguard/dev_build/section_release"_i18n);
    sign_in->registerClickAction([](brls::View*) {
        dev_build_flow::sign_in();
        return true;
    });
    refresh->registerClickAction([](brls::View*) {
        dev_build_flow::open(true);
        return true;
    });
    this->getContentView()->registerAction("playguard/dev_build/sort"_i18n, brls::BUTTON_Y, [this](brls::View*) {
        this->sort();
        return true;
    });
    this->fill();
}

void DevBuildsActivity::update(const dev_builds::Cache& fresh)
{
    // Signed in from this screen: its line goes, the focus to the first build.
    const bool focus_moves = !fresh.needs_login && brls::Application::getCurrentFocus() == sign_in.getView();
    this->cache = fresh;
    this->fill();
    if (focus_moves) {
        brls::Box* first = !main_list->getChildren().empty() ? main_list.getView() : release_list.getView();
        if (!first->getChildren().empty()) brls::Application::giveFocus(first->getChildren()[0]);
    }
}

void DevBuildsActivity::fill()
{
    const std::vector<dev_builds::Build> release = dev_builds::of_kind(cache.builds, dev_builds::Kind::Release, order());
    const std::vector<dev_builds::Build> main = dev_builds::of_kind(cache.builds, dev_builds::Kind::Main, order());
    const std::vector<dev_builds::Build> pulls = dev_builds::of_kind(cache.builds, dev_builds::Kind::PullRequest, order());
    this->fill(release_list.getView(), release);
    this->fill(main_list.getView(), main);
    this->fill(pulls_list.getView(), pulls);
    main_header->setTitle(brls::getStr("playguard/dev_build/section_main", (int)main.size()));
    pulls_header->setTitle(brls::getStr("playguard/dev_build/section_pulls", (int)pulls.size(),
                                        order() == dev_builds::Order::Number ? "playguard/dev_build/order_number"_i18n
                                                                             : "playguard/dev_build/order_newest"_i18n));
    ui::set_visible(release_header.getView(), !release.empty());
    ui::set_visible(main_header.getView(), !main.empty());
    ui::set_visible(pulls_header.getView(), !pulls.empty());
    // Without a token GitHub hands out the release only: a line signs in.
    ui::set_visible(sign_in.getView(), cache.needs_login);
    refresh->setText(dev_build_flow::refresh_text(cache));
}

void DevBuildsActivity::fill(brls::Box* box, const std::vector<dev_builds::Build>& builds)
{
    auto& cells = box->getChildren();
    for (size_t i = 0; i < builds.size(); i++) {
        brls::DetailCell* cell;
        if (i < cells.size()) {
            cell = (brls::DetailCell*)cells[i];
        } else {
            cell = new brls::DetailCell();
            box->addView(cell);
        }
        const dev_builds::Build b = builds[i];
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
    for (size_t i = builds.size(); i < cells.size(); i++) ui::set_visible(cells[i], false);
}

void DevBuildsActivity::sort()
{
    // The focused pull request keeps the focus, wherever it moves.
    int focused_pr = 0;
    auto& cells = pulls_list->getChildren();
    const std::vector<dev_builds::Build> before = dev_builds::of_kind(cache.builds, dev_builds::Kind::PullRequest, order());
    for (size_t i = 0; i < cells.size() && i < before.size(); i++)
        if (cells[i] == brls::Application::getCurrentFocus()) focused_pr = before[i].pr;

    const bool by_number = order() == dev_builds::Order::Number;
    config::get().dev_builds_order = dev_builds::order_name(by_number ? dev_builds::Order::Newest : dev_builds::Order::Number);
    ui::save_config();
    this->fill();
    ui::notify(by_number ? "playguard/dev_build/order_newest"_i18n : "playguard/dev_build/order_number"_i18n);

    if (!focused_pr) return;
    const std::vector<dev_builds::Build> after = dev_builds::of_kind(cache.builds, dev_builds::Kind::PullRequest, order());
    for (size_t i = 0; i < after.size() && i < cells.size(); i++)
        if (after[i].pr == focused_pr) brls::Application::giveFocus(cells[i]);
}

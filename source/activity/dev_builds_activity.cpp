// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/dev_builds_activity.hpp"

#include <fmt/format.h>

#include "action/dev_build_flow.hpp"
#include "action/rollback_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "view/build_cell.hpp"

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

// "20", or "20 · 3 new" when some came after the running build.
std::string count_text(size_t count, size_t newer)
{
    std::string text = std::to_string(count);
    if (newer == 1) text += " · " + "playguard/dev_build/count_new_one"_i18n;
    else if (newer) text += " · " + brls::getStr("playguard/dev_build/count_new", (int)newer);
    return text;
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

void DevBuildsActivity::loading(const std::string& text, float done, bool steady)
{
    if (!s_open) return;
    DevBuildsActivity* a = s_open;
    if (a->loading_box->getVisibility() != brls::Visibility::VISIBLE) {
        a->loading_bar->reset();
        ui::set_visible(a->loading_box.getView(), true);
    }
    a->loading_text->setText(text);
    ui::set_visible(a->loading_bar.getView(), true);
    // Steady: most of the way over ten seconds, the end left for the answer.
    if (steady) a->loading_bar->fillOver(0.9f, 10000000);
    else a->loading_bar->setProgress(done);
}

void DevBuildsActivity::loaded()
{
    if (s_open) ui::set_visible(s_open->loading_box.getView(), false);
}

void DevBuildsActivity::failed(const std::string& text)
{
    if (!s_open) return;
    ui::set_visible(s_open->loading_box.getView(), true);
    ui::set_visible(s_open->loading_bar.getView(), false);
    s_open->loading_text->setText(text);
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
    loading_text->setSingleLine(false);
    // The version only: the running commit is marked in the list, and named
    // in the confirmation (and a commit here would change every screenshot).
    note->setText(brls::getStr("playguard/dev_build/note", app::version()));
    const std::pair<brls::Button*, dev_builds::Kind> buttons[] = { { tab_release.getView(), KINDS[0] },
                                                                   { tab_main.getView(), KINDS[1] },
                                                                   { tab_pulls.getView(), KINDS[2] } };
    for (const auto& b : buttons) {
        const dev_builds::Kind kind = b.second;
        b.first->registerClickAction([this, kind](brls::View*) {
            // A on the pull requests' tab when it is the one shown: the other
            // order (the tab says which); else, that tab.
            if (kind == dev_builds::Kind::PullRequest && this->tab == kind) this->sort();
            else this->show_tab(kind, false);
            return true;
        });
    }
    sign_in->registerClickAction([](brls::View*) {
        dev_build_flow::sign_in();
        return true;
    });
    back->registerClickAction([](brls::View*) {
        dev_build_flow::go_back();
        return true;
    });
    // The L and R icons around the tabs say what moves between them (the same
    // glyphs as the footer's hints); no hint of their own in the footer.
    tab_l->setText(brls::Hint::getKeyIcon(brls::BUTTON_LB, true));
    tab_r->setText(brls::Hint::getKeyIcon(brls::BUTTON_RB, true));
    this->getContentView()->registerAction("", brls::BUTTON_RB, [this](brls::View*) {
        this->next_tab(1);
        return true;
    }, true);
    this->getContentView()->registerAction("", brls::BUTTON_LB, [this](brls::View*) {
        this->next_tab(-1);
        return true;
    }, true);
    // Ⓨ's place among the hints (before Ⓧ); fill() sets what it does.
    this->getContentView()->registerAction("", brls::BUTTON_Y, [](brls::View*) { return true; }, true);
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
    // So does a first list, the screen having opened empty under its bar.
    brls::View* focus = brls::Application::getCurrentFocus();
    const bool was_empty = this->cache.builds.empty();
    const bool focus_moves = (!fresh.needs_login && focus == sign_in.getView()) || was_empty;
    this->cache = fresh;
    if (was_empty && !s_tab_chosen)
        for (const dev_builds::Build& b : fresh.builds)
            if (dev_build_flow::installed(b)) this->tab = b.kind;
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
    // A tab without builds is left out (signed out: the releases only, and
    // then no tabs at all); the one shown always has some, when any has.
    if (this->builds(this->tab).empty())
        for (dev_builds::Kind k : KINDS)
            if (!this->builds(k).empty()) {
                this->tab = k;
                break;
            }
    const std::vector<dev_builds::Build> releases = this->builds(dev_builds::Kind::Release);
    const std::vector<dev_builds::Build> mains = this->builds(dev_builds::Kind::Main);
    const size_t pulls = this->builds(dev_builds::Kind::PullRequest).size();
    // Newer than the running build: a release with a higher version (on a
    // release build or not), a commit of main built after the running one.
    const dev_builds::Build* running = dev_build_flow::running_build(this->cache.builds);
    auto newer_count = [running](const std::vector<dev_builds::Build>& list) {
        size_t n = 0;
        for (const dev_builds::Build& b : list) n += dev_builds::newer(b, app::version(), running) ? 1 : 0;
        return n;
    };
    tab_release->setText(brls::getStr("playguard/dev_build/section_release", count_text(releases.size(), newer_count(releases))));
    tab_main->setText(brls::getStr("playguard/dev_build/section_main", count_text(mains.size(), newer_count(mains))));
    tab_pulls->setText(brls::getStr("playguard/dev_build/section_pulls", (int)pulls,
                                    order() == dev_builds::Order::Number ? "playguard/dev_build/order_number"_i18n
                                                                         : "playguard/dev_build/order_newest"_i18n));
    tab_release->setStyle(this->tab == dev_builds::Kind::Release ? &brls::BUTTONSTYLE_PRIMARY : &brls::BUTTONSTYLE_DEFAULT);
    tab_main->setStyle(this->tab == dev_builds::Kind::Main ? &brls::BUTTONSTYLE_PRIMARY : &brls::BUTTONSTYLE_DEFAULT);
    tab_pulls->setStyle(this->tab == dev_builds::Kind::PullRequest ? &brls::BUTTONSTYLE_PRIMARY : &brls::BUTTONSTYLE_DEFAULT);
    ui::set_visible(tab_release.getView(), !releases.empty());
    ui::set_visible(tab_main.getView(), !mains.empty());
    ui::set_visible(tab_pulls.getView(), pulls > 0);
    brls::View* focus = brls::Application::getCurrentFocus();
    const bool tabs_go = mains.empty() && pulls == 0;
    ui::set_visible(tabs.getView(), !tabs_go);
    // Ⓨ looks for the build of any pull request or commit: with a GitHub
    // token only (signed out, no hint: the Sign in line is there instead).
    const bool can_find = !cache.needs_login;
    this->getContentView()->registerAction(can_find ? "playguard/dev_build/find_hint"_i18n : "", brls::BUTTON_Y,
                                           [can_find](brls::View*) {
                                               if (can_find) dev_build_flow::find();
                                               return true;
                                           }, !can_find);
    brls::Application::getGlobalHintsUpdateEvent()->fire();

    const std::vector<dev_builds::Build> shown = this->builds(this->tab);
    auto& cells = list->getChildren();
    for (size_t i = 0; i < shown.size(); i++) {
        BuildCell* cell;
        if (i < cells.size()) {
            cell = (BuildCell*)cells[i];
        } else {
            cell = new BuildCell();
            list->addView(cell);
        }
        const dev_builds::Build b = shown[i];
        // Two lines: its title, then its tags (kind of change, checks,
        // installed or new); how old it is and its commit at the right.
        const bool is_newer = dev_builds::newer(b, app::version(), running);
        cell->set(b, is_newer);
        cell->registerClickAction([b, is_newer](brls::View*) {
            dev_build_flow::choose(b, is_newer);
            return true;
        });
        ui::set_visible(cell, true);
    }
    for (size_t i = shown.size(); i < cells.size(); i++) ui::set_visible(cells[i], false);
    // Never the focus left on a tab that went with its row.
    if (tabs_go && focus && focus->getParent() == tabs.getView() && !shown.empty())
        brls::Application::giveFocus(cells[0]);
    // Without a token GitHub hands out the releases only: a line signs in.
    ui::set_visible(sign_in.getView(), cache.needs_login);
    // The build installed before this one, kept: a line goes back to it.
    std::string previous;
    const bool have_previous = rollback_flow::previous(&previous);
    if (have_previous) back->setText(brls::getStr("playguard/dev_build/back", previous));
    ui::set_visible(back.getView(), have_previous);
    checked->setText(cache.builds.empty() ? " " : dev_build_flow::checked_text(cache));
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

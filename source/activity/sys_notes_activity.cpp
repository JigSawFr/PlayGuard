// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/sys_notes_activity.hpp"

#include "action/sys_notes_flow.hpp"
#include "ui/ui.hpp"
#include "util/markdown.hpp"
#include "util/text.hpp"
#include "view/build_cell.hpp"
#include "view/markdown_view.hpp"
#include "view/release_notes.hpp"

using namespace brls::literals;
using sys_notes::Which;

namespace
{
SysNotesActivity* s_open = nullptr;   // the screen, while it is open
bool              s_failed[sys_notes::COUNT] = {};   // the last fetch of each failed

std::string title_of(Which which)
{
    return which == Which::Firmware ? "playguard/tools/firmware"_i18n : std::string(sys_notes::product(which));
}

}   // namespace

void SysNotesActivity::show(Which which)
{
    s_failed[(int)which] = false;
    brls::Application::pushActivity(new SysNotesActivity(which));
}

void SysNotesActivity::changed(Which which)
{
    if (s_open && s_open->which == which) s_open->set_loading(true);
}

void SysNotesActivity::fetched(Which which, bool ok)
{
    s_failed[(int)which] = !ok;
    if (!s_open || s_open->which != which) return;
    s_open->set_loading(false);
    s_open->fill();
}

void SysNotesActivity::loaded()
{
    if (!s_open) return;
    s_open->set_loading(sys_notes_flow::fetching(s_open->which));
    s_open->fill();
}

SysNotesActivity::SysNotesActivity(Which which) : which(which)
{
    s_open = this;
}

SysNotesActivity::~SysNotesActivity()
{
    if (s_open == this) s_open = nullptr;
}

void SysNotesActivity::onContentAvailable()
{
    if (auto* applet = dynamic_cast<brls::AppletFrame*>(this->getContentView()))
        applet->setTitle(brls::getStr("playguard/sys_notes/title", title_of(this->which)));
    loading_text->setSingleLine(false);
    advice->setSingleLine(false);
    language->setSingleLine(false);
    empty->setSingleLine(false);
    more->setSingleLine(false);
    checked->setSingleLine(false);
    // A button per tab, made once and reused (one of them may have the focus).
    for (size_t i = 0; i < 1 + sys_notes::MAX_NEWER; i++) {
        auto* b = new brls::Button();
        b->setStyle(&brls::BUTTONSTYLE_DEFAULT);
        b->setMarginRight(12);
        b->setText(" ");
        b->registerClickAction([this, i](brls::View*) {
            this->show_tab(i);
            return true;
        });
        tab_list->addView(b);
    }
    // The notes take the focus (with no highlight) so the stick and the
    // D-pad scroll them.
    notes->setFocusable(true);
    notes->setHideHighlight(true);
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
    const Which w = this->which;
    this->getContentView()->registerAction("playguard/hints/refresh"_i18n, brls::BUTTON_X, [w](brls::View*) {
        sys_notes_flow::refresh(w, true);
        return true;
    });
    online->registerClickAction([this](brls::View*) {
        this->show_page();
        return true;
    });
    this->set_loading(sys_notes_flow::fetching(w));
    this->fill();
    if (brls::View* target = this->focus_target()) brls::Application::giveFocus(target);
}

brls::View* SysNotesActivity::focus_target()
{
    // The chosen tab, else the notes, else See online.
    if (tabs->getVisibility() == brls::Visibility::VISIBLE) return tab_list->getChildren()[this->tab];
    if (notes->getVisibility() == brls::Visibility::VISIBLE) return notes.getView();
    if (online->getVisibility() == brls::Visibility::VISIBLE) return online.getView();
    return nullptr;
}

void SysNotesActivity::set_loading(bool on)
{
    if (on) {
        if (loading_box->getVisibility() != brls::Visibility::VISIBLE) loading_bar->reset();
        loading_text->setText("playguard/sys_notes/fetching"_i18n);
        ui::set_visible(loading_bar.getView(), true);
        // Steady: most of the way over ten seconds, the end left for the answer.
        loading_bar->fillOver(0.9f, 10000000);
    }
    ui::set_visible(loading_box.getView(), on);
}

void SysNotesActivity::fill()
{
    const sys_notes::List& list = sys_notes_flow::cache().of(this->which);
    const std::string mine = sys_notes_flow::installed(this->which);
    size_t hidden = 0;
    int installed_at = -1;
    size_t newer_from = 0;
    this->shown = sys_notes::tabs(list, mine, &hidden, &installed_at, &newer_from);
    // Opens on the closest newer version, else the newest one shown.
    if (!this->tab_chosen)
        this->tab = newer_from < this->shown.size() ? newer_from : this->shown.empty() ? 0 : this->shown.size() - 1;
    if (this->tab >= this->shown.size()) this->tab = this->shown.empty() ? 0 : this->shown.size() - 1;

    // The tabs: the installed one says so; one alone is no choice.
    auto& buttons = tab_list->getChildren();
    for (size_t i = 0; i < buttons.size(); i++) {
        auto* b = (brls::Button*)buttons[i];
        const bool here = i < this->shown.size();
        ui::set_visible(b, here);
        if (!here) continue;
        const sys_notes::Release& r = list.releases[this->shown[i]];
        b->setText((int)this->shown[i] == installed_at ? brls::getStr("playguard/sys_notes/tab_installed", r.version)
                                                        : r.version);
        b->setStyle(i == this->tab ? &brls::BUTTONSTYLE_PRIMARY : &brls::BUTTONSTYLE_DEFAULT);
    }
    brls::View* focus = brls::Application::getCurrentFocus();
    const bool tabs_go = this->shown.size() < 2;
    // The tabs appearing (the first fetch, nothing kept before): the focus
    // goes to them from See online, or from the notes (PlayGuard's own, from
    // the app), where it waited.
    const bool tabs_came = !tabs_go && tabs->getVisibility() != brls::Visibility::VISIBLE;
    ui::set_visible(tabs.getView(), !tabs_go);

    // The firmware: what updating to the newest one needs first.
    const std::string need = this->which == Which::Firmware ? sys_notes_flow::firmware_advice() : "";
    advice->setText(need);
    ui::set_visible(advice.getView(), !need.empty());

    tags->clearViews();
    notes->clearViews();
    const bool fetching = sys_notes_flow::fetching(this->which);
    // PlayGuard's own: laid out as in About and on "What's new" (its
    // highlights as cards, then the lists); the installed version's are in
    // the app too, for when GitHub's are not there (offline, not fetched yet).
    const bool own = this->which == Which::PlayGuard;
    const NVGcolor accent = brls::Application::getTheme()["brls/sidebar/active_item"];
    if (this->shown.empty()) {
        // Nothing kept: the list of releases online.
        this->page = this->which == Which::Firmware ? "https://switchbrew.org/wiki/System_Versions"
                                                    : std::string("https://github.com/") + sys_notes::repo(this->which) + "/releases";
        std::string date;
        const auto lines = own ? release_notes::bundled() : std::vector<changelog::Line>{};
        if (!lines.empty()) {
            release_notes::current(&date);
            tags->addView(tag_view({ "playguard/sys_notes/tag_installed"_i18n, accent }, 18));
            header->setTitle(title_of(this->which) + " " + mine);
            header->setSubtitle(date);
            release_notes::fill_release(notes, lines);
        }
        const bool english = text::starts_with(brls::Application::getLocale(), "en");
        ui::set_visible_all({ { header.getView(), !lines.empty() },
                              { language.getView(), !lines.empty() && !english },
                              { tags.getView(), !lines.empty() } });
        empty->setText(fetching ? "playguard/sys_notes/fetching"_i18n : "playguard/sys_notes/none"_i18n);
        ui::set_visible(empty.getView(), !fetching && lines.empty());
    } else {
        const sys_notes::Release& r = list.releases[this->shown[this->tab]];
        this->page = r.page;
        // Tags: installed or new; what it supports; for the firmware, security
        // fixes and a change to the parental-control service.
        if ((int)this->shown[this->tab] == installed_at)
            tags->addView(tag_view({ "playguard/sys_notes/tag_installed"_i18n, accent }, 18));
        else if (!mine.empty() && this->tab >= newer_from)
            tags->addView(tag_view({ "playguard/sys_notes/tag_new"_i18n, ui::color_warn() }, 18));
        if (!r.supports.empty())
            tags->addView(tag_view({ brls::getStr("playguard/sys_notes/tag_supports", r.supports), ui::color_neutral() }, 18));
        if (r.security) tags->addView(tag_view({ "playguard/sys_notes/tag_security"_i18n, ui::color_warn() }, 18));
        if (r.pctl) tags->addView(tag_view({ "playguard/sys_notes/tag_pctl"_i18n, ui::color_warn() }, 18));
        ui::set_visible(tags.getView(), !tags->getChildren().empty());

        header->setTitle(title_of(this->which) + " " + r.version);
        header->setSubtitle(r.date);
        ui::set_visible(header.getView(), true);
        // Notes as published: in English, said when the console is not.
        const bool english = text::starts_with(brls::Application::getLocale(), "en");
        bool any = !r.notes.empty();
        if (own) {
            auto lines = changelog::parse(r.notes, 0);
            if (lines.empty() && (int)this->shown[this->tab] == installed_at) lines = release_notes::bundled();
            release_notes::fill_release(notes, lines);
            any = !lines.empty();
        } else if (any) {
            markdown_view::fill(notes, markdown::parse(r.notes));
        }
        ui::set_visible(language.getView(), any && !english);
        empty->setText(fetching ? "playguard/sys_notes/fetching"_i18n : "playguard/sys_notes/no_notes"_i18n);
        ui::set_visible(empty.getView(), !any);
    }
    ui::set_visible(notes.getView(), !notes->getChildren().empty());

    // What the tabs leave out: newer versions in between, the installed one
    // when it is older than the list.
    std::string left_out;
    if (hidden) left_out = brls::getStr("playguard/sys_notes/hidden", (int)hidden);
    if (!mine.empty() && installed_at < 0 && !list.releases.empty())
        left_out += (left_out.empty() ? "" : "\n") + brls::getStr("playguard/sys_notes/installed_missing", mine);
    more->setText(left_out);
    ui::set_visible(more.getView(), !left_out.empty());
    ui::set_visible(online.getView(), !this->page.empty());

    // When they were fetched; offline, the notes kept.
    std::string when;
    if (!list.fetched.empty())
        when = brls::getStr(s_failed[(int)this->which] ? "playguard/sys_notes/offline" : "playguard/sys_notes/fetched",
                            list.fetched);
    else if (s_failed[(int)this->which] && !this->shown.empty())
        when = "playguard/sys_notes/failed"_i18n;
    checked->setText(when.empty() ? " " : when);

    // The focus follows the chosen tab, and is never left on a view that went.
    brls::View* target = this->focus_target();
    const bool focus_gone = !focus || focus->getVisibility() != brls::Visibility::VISIBLE ||
                            (focus->getParent() == tab_list.getView() && tabs_go);
    if (target && (focus_gone || focus->getParent() == tab_list.getView() || (tabs_came && (focus == online.getView() || focus == notes.getView()))))
        brls::Application::giveFocus(target);
    brls::Application::getGlobalHintsUpdateEvent()->fire();
}

void SysNotesActivity::show_tab(size_t t)
{
    if (t >= this->shown.size()) return;
    this->tab = t;
    this->tab_chosen = true;
    this->fill();
}

void SysNotesActivity::next_tab(int step)
{
    const int n = (int)this->shown.size();
    if (n < 2) return;
    const int at = ((int)this->tab + step + n) % n;
    brls::View* focus = brls::Application::getCurrentFocus();
    const bool on_tabs = focus && focus->getParent() == tab_list.getView();
    this->show_tab((size_t)at);
    // From the notes, the focus stays there (at their top).
    if (!on_tabs && notes->getVisibility() == brls::Visibility::VISIBLE) brls::Application::giveFocus(notes.getView());
}

void SysNotesActivity::show_page()
{
    const std::string url = this->page;
    if (!url.empty()) ui::qr_dialog("playguard/sys_notes/online_body"_i18n, url);
}

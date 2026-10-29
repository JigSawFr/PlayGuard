// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/sys_notes_activity.hpp"

#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/ui/cadran_modal.hpp>
#include <aurore/cadran/ui/live_prefs.hpp>
#include <aurore/cadran/util/markdown.hpp>
#include <aurore/cadran/util/text.hpp>
#include <aurore/cadran/view/cadran_icon.hpp>
#include <aurore/cadran/view/markdown_view.hpp>

#include "action/sys_notes_flow.hpp"
#include "ui/ui.hpp"
#include "view/build_cell.hpp"
#include "view/release_notes.hpp"

using namespace aurore::literals;
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
    aurore::Application::pushActivity(new SysNotesActivity(which));
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

SysNotesActivity::SysNotesActivity(Which kind) : which(kind)
{
    s_open = this;
}

SysNotesActivity::~SysNotesActivity()
{
    if (s_open == this) s_open = nullptr;
}

void SysNotesActivity::onContentAvailable()
{
    auto* applet = dynamic_cast<aurore::AppletFrame*>(this->getContentView());
    if (applet) applet->setTitle(aurore::getStr("playguard/sys_notes/title", title_of(this->which)));
    // The bar in the header, at its right (as on Install another build), not
    // over the tabs.
    if (aurore::Box* top = applet ? applet->getHeader() : nullptr) {
        if (aurore::Box* parent = loading_box->getParent()) parent->removeView(loading_box.getView(), false);
        loading_box->setAlignSelf(aurore::AlignSelf::CENTER);
        loading_box->setMarginRight(16);
        top->addView(loading_box.getView(), 1);
    }
    // Installed (a check) or new (a sparkle) marks the version, right after it.
    this->mark = new aurore::Box(aurore::Axis::ROW);
    this->mark->setAlignSelf(aurore::AlignSelf::CENTER);
    this->mark->setMarginLeft(12);
    header->addView(this->mark, 2);
    loading_text->setSingleLine(false);
    advice->setSingleLine(false);
    language->setSingleLine(false);
    cadran::kit::empty_around(empty, "file");
    more->setSingleLine(false);
    checked->setSingleLine(false);
    // A button per tab, made once and reused (one of them may have the focus).
    for (size_t i = 0; i < 1 + sys_notes::MAX_NEWER; i++) {
        auto* b = new aurore::Button();
        b->setStyle(&aurore::BUTTONSTYLE_DEFAULT);
        b->setMarginRight(12);
        b->setText(" ");
        // After the version, a check on the installed one's tab (fill()).
        auto* check = new CadranIcon();
        check->setIcon("check");
        check->setWidth(18);
        check->setHeight(18);
        check->setMarginLeft(8);
        b->addView(check);
        b->registerClickAction([this, i](aurore::View*) {
            this->show_tab(i);
            return true;
        });
        tab_list->addView(b);
    }
    // The notes take the focus (with no highlight) so the stick and the
    // D-pad scroll them.
    notes->setFocusable(true);
    notes->setHideHighlight(true);
    tab_l->setText(aurore::Hint::getKeyIcon(aurore::BUTTON_LB, true));
    tab_r->setText(aurore::Hint::getKeyIcon(aurore::BUTTON_RB, true));
    this->getContentView()->registerAction("", aurore::BUTTON_RB, [this](aurore::View*) {
        this->next_tab(1);
        return true;
    }, true);
    this->getContentView()->registerAction("", aurore::BUTTON_LB, [this](aurore::View*) {
        this->next_tab(-1);
        return true;
    }, true);
    const Which w = this->which;
    this->getContentView()->registerAction("playguard/hints/refresh"_i18n, aurore::BUTTON_X, [w](aurore::View*) {
        sys_notes_flow::refresh(w, true);
        return true;
    });
    cadran::style_button(online.getView());   // a pill, as every button of the boards
    online->registerClickAction([this](aurore::View*) {
        this->show_page();
        return true;
    });
    this->set_loading(sys_notes_flow::fetching(w));
    this->fill();
    if (aurore::View* target = this->focus_target()) aurore::Application::giveFocus(target);
}

aurore::View* SysNotesActivity::focus_target()
{
    // The chosen tab, else the notes, else See online.
    if (tabs->getVisibility() == aurore::Visibility::VISIBLE) return tab_list->getChildren()[this->tab];
    if (notes->getVisibility() == aurore::Visibility::VISIBLE) return notes.getView();
    if (online->getVisibility() == aurore::Visibility::VISIBLE) return online.getView();
    return nullptr;
}

void SysNotesActivity::set_loading(bool on)
{
    if (on) {
        if (loading_box->getVisibility() != aurore::Visibility::VISIBLE) loading_bar->reset();
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

    // The tabs: the installed one has a check; one alone is no choice.
    auto& buttons = tab_list->getChildren();
    for (size_t i = 0; i < buttons.size(); i++) {
        auto* b = (aurore::Button*)buttons[i];
        const bool here = i < this->shown.size();
        ui::set_visible(b, here);
        if (!here) continue;
        const sys_notes::Release& r = list.releases[this->shown[i]];
        const aurore::ButtonStyle* style = i == this->tab ? &aurore::BUTTONSTYLE_PRIMARY : &aurore::BUTTONSTYLE_DEFAULT;
        b->setText(r.version);
        b->setStyle(style);
        // The check in the tab's ink: dark on the chosen tab's teal.
        auto* check = (CadranIcon*)b->getChildren().back();
        check->setThemeColor("color", style->enabledLabelColor);
        ui::set_visible(check, (int)this->shown[i] == installed_at);
    }
    aurore::View* focus = aurore::Application::getCurrentFocus();
    const bool tabs_go = this->shown.size() < 2;
    // The tabs appearing (the first fetch, nothing kept before): the focus
    // goes to them from See online, or from the notes (PlayGuard's own, from
    // the app), where it waited.
    const bool tabs_came = !tabs_go && tabs->getVisibility() != aurore::Visibility::VISIBLE;
    ui::set_visible(tabs.getView(), !tabs_go);

    // The firmware: what updating to the newest one needs first.
    const std::string need = this->which == Which::Firmware ? sys_notes_flow::firmware_advice() : "";
    advice->setText(need);
    ui::set_visible(advice.getView(), !need.empty());

    mark->clearViews();
    tags->clearViews();
    notes->clearViews();
    const bool fetching = sys_notes_flow::fetching(this->which);
    // PlayGuard's own: laid out as in About and on "What's new" (its
    // highlights as cards, then the lists); the installed version's are in
    // the app too, for when GitHub's are not there (offline, not fetched yet).
    const bool own = this->which == Which::PlayGuard;
    const char* accent = "aurore/sidebar/active_item";
    if (this->shown.empty()) {
        // Nothing kept: the list of releases online.
        this->page = this->which == Which::Firmware ? "https://switchbrew.org/wiki/System_Versions"
                                                    : std::string("https://github.com/") + sys_notes::repo(this->which) + "/releases";
        std::string date;
        const auto lines = own ? release_notes::bundled() : std::vector<changelog::Line>{};
        if (!lines.empty()) {
            release_notes::current(&date);
            mark->addView(tag_view({ "", accent, "check" }, 18));
            header->setTitle(title_of(this->which) + " " + mine);
            header->setSubtitle(date);
            release_notes::fill_release(notes, lines);
        }
        const bool english = text::starts_with(live_prefs::locale(), "en");
        ui::set_visible_all({ { header.getView(), !lines.empty() },
                              { language.getView(), !lines.empty() && !english },
                              { tags.getView(), false } });
        empty->setText(fetching ? "playguard/sys_notes/fetching"_i18n : "playguard/sys_notes/none"_i18n);
        ui::set_visible(empty.getView(), !fetching && lines.empty());
    } else {
        const sys_notes::Release& r = list.releases[this->shown[this->tab]];
        this->page = r.page;
        // Installed (a check) or newer (a sparkle), beside the version; tags:
        // what it supports; for the firmware, security fixes and a change to
        // the parental-control service.
        if ((int)this->shown[this->tab] == installed_at)
            mark->addView(tag_view({ "", accent, "check" }, 18));
        else if (!mine.empty() && this->tab >= newer_from)
            mark->addView(tag_view({ "", "cadran/amber", "sparkle" }, 18));
        if (!r.supports.empty())
            tags->addView(tag_view({ aurore::getStr("playguard/sys_notes/tag_supports", r.supports), "cadran/text2" }, 18));
        if (r.security) tags->addView(tag_view({ "playguard/sys_notes/tag_security"_i18n, "cadran/amber" }, 18));
        if (r.pctl) tags->addView(tag_view({ "playguard/sys_notes/tag_pctl"_i18n, "cadran/amber" }, 18));
        ui::set_visible(tags.getView(), !tags->getChildren().empty());

        header->setTitle(title_of(this->which) + " " + r.version);
        header->setSubtitle(r.date);
        ui::set_visible(header.getView(), true);
        // Notes as published: in English, said when the console is not.
        const bool english = text::starts_with(live_prefs::locale(), "en");
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
    if (hidden) left_out = aurore::getStr("playguard/sys_notes/hidden", (int)hidden);
    if (!mine.empty() && installed_at < 0 && !list.releases.empty())
        left_out += (left_out.empty() ? "" : "\n") + aurore::getStr("playguard/sys_notes/installed_missing", mine);
    more->setText(left_out);
    ui::set_visible(more.getView(), !left_out.empty());
    ui::set_visible(online.getView(), !this->page.empty());

    // When they were fetched; offline, the notes kept.
    std::string when;
    if (!list.fetched.empty())
        when = aurore::getStr(s_failed[(int)this->which] ? "playguard/sys_notes/offline" : "playguard/sys_notes/fetched",
                            list.fetched);
    else if (s_failed[(int)this->which] && !this->shown.empty())
        when = "playguard/sys_notes/failed"_i18n;
    checked->setText(when.empty() ? " " : when);

    // The focus follows the chosen tab, and is never left on a view that went.
    aurore::View* target = this->focus_target();
    const bool focus_gone = !focus || focus->getVisibility() != aurore::Visibility::VISIBLE ||
                            (focus->getParent() == tab_list.getView() && tabs_go);
    if (target && (focus_gone || focus->getParent() == tab_list.getView() || (tabs_came && (focus == online.getView() || focus == notes.getView()))))
        aurore::Application::giveFocus(target);
    aurore::Application::getGlobalHintsUpdateEvent()->fire();
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
    aurore::View* focus = aurore::Application::getCurrentFocus();
    const bool on_tabs = focus && focus->getParent() == tab_list.getView();
    this->show_tab((size_t)at);
    // From the notes, the focus stays there (at their top).
    if (!on_tabs && notes->getVisibility() == aurore::Visibility::VISIBLE) aurore::Application::giveFocus(notes.getView());
}

void SysNotesActivity::show_page()
{
    const std::string url = this->page;
    if (!url.empty()) ui::qr_dialog("playguard/sys_notes/online_body"_i18n, url);
}

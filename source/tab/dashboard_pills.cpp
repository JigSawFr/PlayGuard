// DashboardTab — Today's pills: the console's state, each in the colour of
// what it says.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/dashboard_tab.hpp"

#include <utility>

#include "action/nintendo_dns.hpp"
#include "action/pairing_state.hpp"
#include "action/session_pause.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_text.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

// The console's state as pills, in the colours of what each says; A opens
// where each is set.
DashboardTab::Pill DashboardTab::make_pill(const std::string& text, const char* dot)
{
    namespace kit = cadran::kit;
    Pill p;
    p.box = kit::pill(text, dot);
    p.box->setFocusable(true);
    p.box->setHideHighlightBackground(true);
    p.dot = dynamic_cast<kit::Mark*>(p.box->getChildren()[0]);
    p.text = dynamic_cast<brls::Label*>(p.box->getChildren()[1]);
    pills->addView(p.box);
    return p;
}

void DashboardTab::build_pills()
{
    const std::pair<Pill*, int> to_tab[] = { { &this->compat_pill, ui::tab::console }, { &this->pairing_pill, ui::tab::security },
                                             { &this->serial_pill, ui::tab::console } };
    for (const auto& [pill, tab] : to_tab) {
        *pill = this->make_pill("", "cadran/teal_fill");
        const int position = tab;
        pill->box->registerClickAction([position](brls::View* view) {
            ui::go_to_tab(view, position);
            return true;
        });
    }
    // Ⓐ: what the DNS blocks, and what that changes here.
    this->dns_pill = this->make_pill("", "cadran/teal_fill");
    this->dns_pill.box->registerClickAction([](brls::View*) {
        // The dialogs' layout (cadran::dialog_body): a title, the two
        // servers as a table, what that changes as notes.
        const bool sync = nintendo_dns::pctl_blocked(), time = nintendo_dns::time_blocked();
        const auto state = [](bool blocked) {
            return blocked ? "playguard/cadran/today/dns_blocked"_i18n : "playguard/cadran/today/dns_reachable"_i18n;
        };
        std::string body = "playguard/cadran/today/dns_title"_i18n + "\n\n" +
                           (sync || time ? "playguard/cadran/today/dns_intro"_i18n : "playguard/cadran/today/dns_intro_open"_i18n) +
                           "\n\n" + cadran_text::table({ { "playguard/cadran/today/dns_row_sync"_i18n, state(sync) },
                                                            { "playguard/cadran/today/dns_row_time"_i18n, state(time) } });
        if (sync) body += "\n\n" + "playguard/cadran/today/dns_sync_note"_i18n;
        if (time)
            body += "\n\n" + brls::getStr("playguard/cadran/today/dns_time_note",
                                         "playguard/tabs/console"_i18n + " › " + "playguard/tabs/clock"_i18n);
        ui::info(body);
        return true;
    });
    this->patches_pill = this->make_pill("", "cadran/teal_fill");
    this->patches_pill.box->registerClickAction([](brls::View* view) {
        ui::go_to_tab(view, ui::tab::console);
        return true;
    });
    // An application, PlayGuard's own time counts: opened over a game (as
    // that game) or from a forwarder's HOME menu icon. Said at length on Ⓐ.
    SysInfo si;
    sysinfo_get(&si);
    this->counted_pill = this->make_pill(si.forwarder ? "playguard/cadran/today/counted_icon_pill"_i18n
                                                      : "playguard/cadran/today/counted_pill"_i18n,
                                         "cadran/amber_fill");
    this->counted_pill.box->registerClickAction([this, icon = si.forwarder](brls::View*) {
        std::string body = "playguard/cadran/today/counted_title"_i18n + "\n\n"
                           + (icon ? "playguard/dashboard/counted_icon"_i18n : "playguard/dashboard/counted"_i18n);
        if (this->home_counts()) body += "\n\n" + "playguard/dashboard/home_counts"_i18n;
        ui::info(body);
        return true;
    });
    // A game left open on HOME keeps counting (the opened-over-a-game pill
    // says it too, when shown).
    this->home_pill = this->make_pill("playguard/cadran/today/home_pill"_i18n, "cadran/amber_fill");
    this->home_pill.box->registerClickAction([](brls::View*) {
        ui::info("playguard/cadran/today/home_title"_i18n + "\n\n" + "playguard/dashboard/home_counts"_i18n);
        return true;
    });
    // Ⓐ: the pills it stands for, in its place; the focus on the first one.
    this->fine_pill = this->make_pill("", "cadran/teal_fill");
    this->fine_pill.rank = 1;
    this->fine_pill.box->registerClickAction([this](brls::View*) {
        this->fine_open = true;
        this->order_pills();
        const Pill* folded[] = { &this->compat_pill, &this->pairing_pill, &this->serial_pill, &this->dns_pill,
                                 &this->patches_pill };
        for (brls::View* v : pills->getChildren())
            for (const Pill* p : folded)
                if (p->box == v && p->on && p->rank > 0) {
                    brls::Application::giveFocus(v);
                    return true;
                }
        return true;
    });
}

void DashboardTab::show_pills()
{
    namespace kit = cadran::kit;
    const SysInfo& si = this->sys;
    // Something to look at: its title and what it says, in its colour; else
    // the all-fine words, or nothing.
    const auto show = [](Pill& p, bool issue, const std::string& title, const std::string& value, NVGcolor color,
                         const std::string& fine) {
        const std::string text = issue ? title + " · " + value : fine;
        ui::set_text(p.text, text);
        const std::string dot = issue ? kit::dot_of(color) : "cadran/teal_fill";
        p.dot->set(dot.c_str());
        p.rank = dot == "cadran/teal_fill" ? 1 : dot == "cadran/text3" ? 2 : 0;
        p.on = !text.empty();
    };
    // The firmware's compatibility: the firmware with it when all is fine.
    NVGcolor c = cadran::kit::color("cadran/text2");
    const std::string compat = ui::compat_text(si, &c);
    show(this->compat_pill, sysinfo_compat(&si) != SysCompat_Ok, "playguard/dashboard/compat"_i18n, compat, c,
         ui::fw_text(si) + " · " + compat);
    // The phone app only while it can overwrite what is set here.
    const PctlStatus& s = this->status;
    show(this->pairing_pill, pairing_state::syncs(s), "playguard/dashboard/pairing"_i18n,
         ui::bool_text(s.pairing_active_ok, s.pairing_active, "playguard/dashboard/pairing_on"_i18n,
                       "playguard/dashboard/pairing_off"_i18n),
         cadran::kit::color("cadran/amber"), "");
    // The serial number visible on emuMMC.
    show(this->serial_pill, ui::serial_warning(si), "playguard/dashboard/serial"_i18n, "playguard/dashboard/serial_visible"_i18n,
         cadran::kit::color("cadran/amber"), "playguard/cadran/today/serial_hidden"_i18n);
    // Sigpatch files only, or sys-patch incomplete.
    const patches::Report& report = ui::patch_report();
    c = cadran::kit::color("cadran/text2");
    const std::string patches_text = ui::patches_text(report, &c);
    show(this->patches_pill, ui::patches_warning(report), "playguard/dashboard/patches"_i18n, patches_text, c, "");
    const bool sync = nintendo_dns::pctl_blocked(), time = nintendo_dns::time_blocked();
    // Blocking nothing, said too: the parent who set up the redirection
    // sees whether it holds.
    ui::set_text(this->dns_pill.text, sync && time ? "playguard/cadran/today/dns_nintendo"_i18n
                                      : sync       ? "playguard/cadran/today/dns_sync"_i18n
                                      : time       ? "playguard/cadran/today/dns_time"_i18n
                                                   : "playguard/cadran/today/dns_open"_i18n);
    this->dns_pill.dot->set(sync || time ? "cadran/teal_fill" : "cadran/text3");
    this->dns_pill.rank = sync || time ? 1 : 2;
    this->dns_pill.on = true;
    this->counted_pill.rank = 0;
    this->home_pill.rank = 0;
    // Not while paused for the parent here (session_pause): the time spent
    // here then is not the child's.
    this->counted_pill.on = !si.applet_mode && !session_pause::active();
    this->home_pill.on = this->home_counts() && si.applet_mode;
    this->order_pills();
}

void DashboardTab::order_pills()
{
    // What needs the parent first (amber, coral), then what is fine (teal),
    // then what is only said (grey); in their usual order within each.
    std::vector<Pill*> all = { &this->compat_pill, &this->pairing_pill, &this->serial_pill, &this->dns_pill,
                               &this->patches_pill, &this->counted_pill, &this->home_pill };
    std::stable_sort(all.begin(), all.end(), [](const Pill* a, const Pill* b) { return a->rank < b->rank; });
    // The fine ones folded into one pill, unless unfolded or alone (a
    // summary of one would say no less).
    int fine = 0;
    for (const Pill* p : all)
        if (p->on && p->rank > 0) fine++;
    const bool fold = !this->fine_open && fine > 1;
    ui::set_text(this->fine_pill.text, brls::getStr("playguard/cadran/today/fine_pill", fine));
    std::vector<std::pair<brls::View*, bool>> shown;
    for (const Pill* p : all) shown.push_back({ p->box, p->on && (p->rank == 0 || !fold) });
    shown.push_back({ this->fine_pill.box, fold });
    for (const auto& [view, visible] : shown) ui::set_visible(view, visible);
    all.push_back(&this->fine_pill);
    std::vector<brls::View*> wanted;
    for (const Pill* p : all)
        if (p->box) wanted.push_back(p->box);
    if (wanted == pills->getChildren()) return;
    // Not under the focus: moved, a focused pill would lose it.
    for (brls::View* v : wanted)
        if (v->isFocused()) return;
    for (brls::View* v : wanted) pills->removeView(v, false);
    for (brls::View* v : wanted) pills->addView(v);
}

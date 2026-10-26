// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "tab/clock_tab.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <vector>

#include <borealis/cadran/ui/cadran_frame.hpp>
#include <borealis/cadran/ui/cadran_kit.hpp>
#include <borealis/cadran/ui/cadran_modal.hpp>

#include "action/nintendo_dns.hpp"
#include "action/offline_clock_logic.hpp"
#include "app.hpp"
#include "core/sim_env.h"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/ntp_servers.hpp"
#include "util/paths.hpp"
#include "util/perf.hpp"

using namespace brls::literals;

namespace
{
constexpr auto SNAPSHOT_EVERY = std::chrono::seconds(10);

std::string region_label(const std::string& id)
{
    return brls::getStr("playguard/clock/region_names/" + id);
}

// A server's icon in the pickers: its flag or globe from the Twemoji set of
// img/emoji/ (ntp::host_emoji()); a big provider's, or one whose picture the
// set lacks, a server.
std::string host_icon(const std::string& host)
{
    const std::string hex = ntp::host_emoji(host);
    if (hex.empty() || !paths::file_exists(std::string(BRLS_ASSET("img/emoji/")) + hex + ".png")) return "server";
    return "img/emoji/" + hex + ".png";
}

// A region's: its first server's (Europe's flag, the globe turned to the
// Americas…); the saved servers, the save icon.
std::string region_icon(const std::string& id)
{
    for (const auto& r : ntp::regions())
        if (id == r.id) return host_icon(r.hosts.front());
    return "save";
}

std::string find_region(const std::string& host)
{
    for (const auto& r : ntp::regions())
        for (const char* h : r.hosts)
            if (host == h) return r.id;
    return "custom";
}

// "2026-10-08 16:00:00" -> { "2026-10-08", "16:00:00" }; else { "", text }.
std::pair<std::string, std::string> split_time(const std::string& text)
{
    const size_t space = text.find(' ');
    if (space == std::string::npos || space != 10) return { "", text };
    return { text.substr(0, space), text.substr(space + 1) };
}

bool plausible_host(const std::string& h)
{
    if (h.empty() || h.size() > 253) return false;
    for (char c : h)
        if (!(std::isalnum((unsigned char)c) || c == '.' || c == '-' || c == ':' || c == '_')) return false;
    return h.find("://") == std::string::npos;
}
}   // namespace

ClockTab::ClockTab()
    : TabBase("xml/tab/clock.xml"), list(this, true)
{
    namespace kit = cadran::kit;
    why->setSingleLine(false);
    // Nintendo's time server blocked here: the clock will not set itself.
    if (nintendo_dns::time_blocked()) why->setText("playguard/clock/why"_i18n + "\n\n" + "playguard/clock/dns_note"_i18n);
    result->setSingleLine(false);
    result_title->setSingleLine(false);
    warning->setSingleLine(false);
    // Only a notification at start-up (it never sets the clock): PlayGuard's own setting.
    check_at_start->init("playguard/tools/clock_check"_i18n, config::get().clock_check_at_start, [](bool on) {
        config::get().clock_check_at_start = on;
        ui::save_config();
    });

    this->server = clock_flow::current_server();
    this->region_id = find_region(this->server);

    // A state of System Settings, which an application cannot open: say where it is.
    autosync->registerClickAction([](brls::View*) {
        ui::info("playguard/clock/auto_how"_i18n);
        return true;
    });
    region->registerClickAction([this](brls::View*) { this->choose_region(); return true; });
    server_cell->registerClickAction([this](brls::View*) { this->choose_server(); return true; });
    custom->registerClickAction([this](brls::View*) { this->enter_custom(); return true; });
    measure_cell->registerClickAction([this](brls::View*) { this->measure(); return true; });
    apply_cell->registerClickAction([this](brls::View*) {
        if (!ui::refuse_read_only()) this->apply();
        return true;
    });
    // A long title and no value: the title gets the whole width (an empty
    // detail label still reserves its space).
    copy_cell->detail->setVisibility(brls::Visibility::GONE);
    copy_cell->registerClickAction([this](brls::View*) {
        if (!ui::refuse_read_only()) this->copy();
        return true;
    });
    ui::set_visible(warning.getView(), false);
    // Clocks tick and a measurement expires: keep the screen current.
    this->enable_auto_refresh(1000);
    // Ⓧ reads the clocks again at once (the timer moves them forward between reads).
    this->registerAction("playguard/hints/refresh"_i18n, brls::BUTTON_X, [this](brls::View*) {
        this->snap_ok = false;
        this->refresh();
        return true;
    });
    // Both clocks large, side by side.
    if (brls::Box* column = this->list.content()) {
        auto* row = new brls::Box(brls::Axis::ROW);
        row->setAlignItems(brls::AlignItems::STRETCH);
        row->setMarginBottom(12);

        brls::Box* console = kit::top_card(true);
        console->setMarginRight(12);
        console->addView(kit::kicker("gamepad", "playguard/cadran/clock/console"_i18n));
        this->user_time = new CadranDigits();
        this->user_time->setFontSize(46);
        this->user_time->setMarginTop(10);
        console->addView(this->user_time);
        this->user_date = kit::label("", 15.5f, "cadran/text2");
        this->user_date->setMarginTop(4);
        console->addView(this->user_date);
        row->addView(console);

        this->network_card = kit::top_card(true);
        this->network_card->addView(kit::kicker("globe", "playguard/cadran/clock/network"_i18n));
        this->network_time = new CadranDigits();
        this->network_time->setFontSize(46);
        this->network_time->setMarginTop(10);
        this->network_card->addView(this->network_time);
        auto* state = new brls::Box(brls::Axis::ROW);
        state->setAlignItems(brls::AlignItems::CENTER);
        state->setMarginTop(4);
        this->network_dot = new brls::Rectangle(brls::Application::getTheme()["cadran/teal_fill"]);
        this->network_dot->setWidth(9);
        this->network_dot->setHeight(9);
        this->network_dot->setCornerRadius(4.5f);
        this->network_dot->setMarginRight(8);
        state->addView(this->network_dot);
        this->network_state = kit::label("", 15.5f, "cadran/teal");
        state->addView(this->network_state);
        this->network_card->addView(state);
        row->addView(this->network_card);

        // Under the section's title.
        const auto& lines = column->getChildren();
        const auto at = std::find(lines.begin(), lines.end(), autosync.getView());
        column->addView(row, at == lines.end() ? 0 : (size_t)(at - lines.begin()));
    }
    this->list.apply({ { "ck_auto", "refresh" }, { "ck_zone", "pin" }, { "ck_region", "globe" }, { "ck_server", "server" },
                       { "ck_custom", "pencil" }, { "ck_measure", "timer" }, { "ck_apply", "check" }, { "ck_copy", "today" },
                       { "ck_check_at_start", "bolt" } });
}

void ClockTab::refresh()
{
    perf::Timed timed("Network clock refresh");   // the diagnostic report's timings
    const auto now = std::chrono::steady_clock::now();
    if (!this->snap_ok || now - this->snap_at >= SNAPSHOT_EVERY || now < this->snap_at) {
        time_clock_snapshot(&this->snap);
        this->snap_at = now;
        this->snap_ok = true;
    }
    const TimeSnapshot& s = this->snap;
    uint64_t ticked = (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(now - this->snap_at).count();
#ifndef __SWITCH__
    // Desktop simulation: a console time frozen by PLAYGUARD_SIM_NOW stays
    // frozen here too, or a screenshot would depend on when it was taken.
    if (sim_env_int("PLAYGUARD_SIM_NOW", 0, 0, INT64_MAX)) ticked = 0;
#endif
    const std::string na = "cadran/common/unavailable"_i18n;
    this->show_clocks(R_SUCCEEDED(s.user_rc) ? ui::time_text(s.user_time + ticked) : na,
                      R_SUCCEEDED(s.network_rc) ? ui::time_text(s.network_time + ticked) : na,
                      R_SUCCEEDED(s.accuracy_rc), s.accuracy);
    ui::set_detail(autosync, ui::bool_text(R_SUCCEEDED(s.automatic_rc), s.automatic,
                                           "playguard/common/on"_i18n, "playguard/common/off"_i18n));
    this->autosync_off = R_SUCCEEDED(s.automatic_rc) && !s.automatic;
    autosync->setDetailTextColor(this->autosync_off ? cadran::kit::color("cadran/amber") : cadran::kit::color("cadran/text2"));
    ui::set_detail(zone, R_SUCCEEDED(s.location_rc) && s.location[0] ? std::string(s.location) : na);

    ui::set_detail(region, region_label(this->region_id));
    ui::set_detail(server_cell, this->server);

    // "Set the network clock" only exists while a fresh measurement does.
    if (this->last.ok) {
        const int64_t left = clock_flow::seconds_left(this->last);
        if (left <= 0) {
            this->last = clock_flow::Measurement{};
            this->show_result("playguard/clock/expired"_i18n);
            ui::set_visible(warning.getView(), false);
        } else {
            // Counts down every second (the tab refreshes itself each second).
            // "1 min 45 s": "1:45" read like the hours:minutes typed elsewhere.
            const std::string span = left >= 60 ? brls::getStr("playguard/common/min_sec", (int)(left / 60), (int)(left % 60))
                                                : brls::getStr("playguard/common/sec", (int)left);
            ui::set_detail(apply_cell, brls::getStr("playguard/clock/valid_for", span));
        }
    }
    // Measuring only reads: also in read-only mode, where setting is greyed.
    ui::set_visible(apply_cell.getView(), this->last.ok);
    ui::show_writable(apply_cell, !app::read_only());
    // No server answered: the console clock instead (its time is at the top).
    ui::set_visible(copy_cell.getView(), this->offline);
    ui::show_writable(copy_cell, !app::read_only());
    this->list.sync();
}

void ClockTab::set_server(const std::string& host, const std::string& reg)
{
    this->server = host;
    this->region_id = reg;
    this->last = clock_flow::Measurement{};
    this->offline = false;
    this->show_result("");
    ui::set_visible(warning.getView(), false);
    auto& cfg = config::get();
    cfg.ntp_server = host;
    ui::save_config();
    this->refresh();
}

void ClockTab::show_clocks(const std::string& user_text, const std::string& network_text, bool known, bool accurate)
{
    namespace kit = cadran::kit;
    if (!this->user_time) return;
    brls::Theme theme = brls::Application::getTheme();
    const auto user = split_time(user_text);
    const auto network = split_time(network_text);
    this->user_time->setText(user.second);
    this->user_date->setText(user.first.empty() ? "" : kit::long_date(user.first));
    this->network_time->setText(network.second);

    const bool bad = known && !accurate;
    std::string state = !known ? "cadran/common/unavailable"_i18n
                        : bad  ? "playguard/cadran/clock/inaccurate"_i18n : "playguard/cadran/clock/accurate"_i18n;
    if (known && !network.first.empty() && network.first != user.first) state += " · " + kit::long_date(network.first);
    this->network_state->setText(state);
    this->network_state->setTextColor(!known ? theme["cadran/text3"] : bad ? theme["cadran/amber"] : theme["cadran/teal"]);
    this->network_dot->setColor(!known ? theme["cadran/text3"] : bad ? theme["cadran/amber_fill"] : theme["cadran/teal_fill"]);
    this->network_time->setTextColor(bad ? theme["cadran/amber"] : theme["cadran/text"]);
    this->network_card->setBackgroundColor(bad ? theme["cadran/amber_surf"] : theme["cadran/surf"]);
    this->network_card->setBorderColor(bad ? theme["cadran/amber_line"] : theme["cadran/line"]);
}

void ClockTab::choose_region()
{
    std::vector<std::string> ids, labels;
    for (const auto& r : ntp::regions()) { ids.push_back(r.id); labels.push_back(region_label(r.id)); }
    if (!config::get().custom_servers.empty()) { ids.push_back("custom"); labels.push_back(region_label("custom")); }
    std::vector<std::string> icons;
    for (const std::string& region_key : ids) icons.push_back(region_icon(region_key));
    int selected = (int)(std::find(ids.begin(), ids.end(), this->region_id) - ids.begin());
    if (selected >= (int)ids.size()) selected = 0;
    ui::pick("playguard/clock/region"_i18n, labels, selected, [this, ids](int index) {
        const std::string region_key = ids[index];
        if (region_key == "custom") {
            this->set_server(config::get().custom_servers.front(), region_key);
            return;
        }
        for (const auto& r : ntp::regions())
            if (region_key == r.id) this->set_server(r.hosts.front(), region_key);
    }, true, {}, icons);
}

void ClockTab::choose_server()
{
    std::vector<std::string> hosts;
    if (this->region_id == "custom") hosts = config::get().custom_servers;
    else
        for (const auto& r : ntp::regions())
            if (this->region_id == r.id) hosts.assign(r.hosts.begin(), r.hosts.end());
    if (hosts.empty()) return;
    int selected = (int)(std::find(hosts.begin(), hosts.end(), this->server) - hosts.begin());
    if (selected >= (int)hosts.size()) selected = 0;
    std::vector<std::string> icons;
    for (const std::string& h : hosts) icons.push_back(host_icon(h));
    ui::pick("playguard/clock/server"_i18n, hosts, selected, [this, hosts](int index) {
        this->set_server(hosts[index], this->region_id);
    }, true, {}, icons);
}

void ClockTab::enter_custom()
{
    ui::prompt_text("playguard/clock/custom_header"_i18n, this->server, 253, [this](std::string host) {
        if (!plausible_host(host)) {
            ui::refuse(NXM_RC_INVALID_ARGUMENT);
            return;
        }
        auto& servers = config::get().custom_servers;
        servers.erase(std::remove(servers.begin(), servers.end(), host), servers.end());
        servers.insert(servers.begin(), host);
        if (servers.size() > 10) servers.resize(10);
        this->set_server(host, find_region(host));
    });
}

void ClockTab::measure()
{
    if (this->busy) {
        ui::notify("playguard/clock/busy"_i18n);
        return;
    }
    this->busy = true;
    this->last = clock_flow::Measurement{};
    this->offline = false;
    ui::set_visible(apply_cell.getView(), false);
    ui::set_visible(copy_cell.getView(), false);
    ui::set_visible(warning.getView(), false);
    this->show_result("");
    // A spinner says it is working, the text with whom; Cancel (or B) drops
    // the late answer.
    auto cancelled = std::make_shared<bool>(false);
    auto* progress = new brls::Dialog(cadran::wait_body(brls::getStr("playguard/clock/measuring", this->server)));
    ui::add_cancel(progress, [cancelled, idle = ui::guarded(this->alive, [this]() { this->busy = false; })]() {
        *cancelled = true;
        idle();
    });
    progress->open();

    // Nothing once the tab is gone meanwhile.
    clock_flow::measure(this->server, ui::guarded(this->alive, [this, progress, cancelled](const clock_flow::Measurement& m) {
        if (*cancelled) return;   // the dialog was closed meanwhile
        *cancelled = true;
        this->busy = false;
        this->last = m;
        this->offline = offline_clock_logic::offer(m.ok, this->snap) != offline_clock_logic::Offer::None;
        // On the screen, the outcome in one line (the details are the dialog's).
        if (m.ok) {
            const std::string offset = m.offset_known ? clock_flow::offset_text(m.offset) : "cadran/common/unavailable"_i18n;
            this->show_result(brls::getStr("playguard/clock/last_result", offset, (int)this->answered(m), (int)m.lines.size()));
        } else {
            this->show_result(m.report, this->offline);
        }
        // The servers disagreeing, and a write that would be refused, in amber.
        std::string warn = m.warning;
        if (m.ok && this->autosync_off) warn += (warn.empty() ? "" : "\n") + "playguard/clock/autosync_off_note"_i18n;
        ui::set_text(warning, warn);
        ui::set_visible(warning.getView(), !warn.empty());
        this->refresh();   // shows "Set the network clock"
        progress->close([show = ui::guarded(this->alive, [this, m, warn]() { this->show_measurement(m, warn); })]() {
            brls::sync(show);
        });
    }));
}

size_t ClockTab::answered(const clock_flow::Measurement& m)
{
    return (size_t)std::count_if(m.lines.begin(), m.lines.end(), [](const clock_flow::ServerLine& l) { return l.ok; });
}

void ClockTab::show_measurement(const clock_flow::Measurement& m, const std::string& warn)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    // Within a second: nothing to set (the servers answer to the second).
    const bool on_time = m.ok && m.offset_known && m.offset >= -1 && m.offset <= 1;
    const char* tone = !m.ok ? "cadran/amber" : on_time ? "cadran/teal" : m.offset_known ? "cadran/amber" : "cadran/text2";

    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setAlignItems(brls::AlignItems::STRETCH);
    body->setPadding(30, 40, 22, 40);

    // The answer first, large: how far off the network clock is.
    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    brls::Box* badge = !m.ok    ? cadran::dialog_badge("alert", "cadran/amber", "cadran/amber_surf")
                       : on_time ? cadran::dialog_badge("check", "cadran/teal", "cadran/teal_soft")
                                 : cadran::dialog_badge("timer", tone, m.offset_known ? "cadran/amber_surf" : "cadran/surf2");
    badge->setMarginRight(18);
    head->addView(badge);
    auto* words = new brls::Box(brls::Axis::COLUMN);
    words->setGrow(1.0f);
    words->setWidth(0);
    brls::Label* kicker = kit::label("", kit::TEXT_KICKER, "cadran/text2");
    kit::set_kicker(kicker, "playguard/clock/dialog_kicker"_i18n);
    words->addView(kicker);
    std::string headline = !m.ok            ? "playguard/clock/dialog_none"_i18n
                           : m.offset_known ? clock_flow::offset_text(m.offset)
                                            : "cadran/common/unavailable"_i18n;
    brls::Label* title = kit::display(headline, m.ok ? 34 : kit::TEXT_TITLE, tone);
    title->setMarginTop(4);
    words->addView(title);
    head->addView(words);
    body->addView(head);

    // A rounded panel of rows: a key, a value (and, quieter, what follows it).
    auto panel = [&](const std::string& name) {
        if (!name.empty()) {
            brls::Label* label = kit::label("", kit::TEXT_KICKER, "cadran/text2");
            kit::set_kicker(label, name);
            label->setMarginTop(20);
            label->setMarginBottom(8);
            body->addView(label);
        }
        auto* table = new brls::Box(brls::Axis::COLUMN);
        table->setAlignItems(brls::AlignItems::STRETCH);
        table->setBackgroundColor(theme["cadran/surf2"]);
        table->setCornerRadius(kit::RADIUS_ROW);
        table->setPadding(6, 20, 6, 20);
        if (name.empty()) table->setMarginTop(20);
        body->addView(table);
        return table;
    };
    auto row = [&](brls::Box* table, const char* mark, const std::string& key, const std::string& value,
                   const char* value_key, const std::string& extra) {
        if (!table->getChildren().empty()) {
            brls::View* above = table->getChildren().back();
            above->setLineBottom(1);
            above->setLineColor(theme["cadran/line"]);
        }
        auto* line = new brls::Box(brls::Axis::ROW);
        line->setAlignItems(brls::AlignItems::CENTER);
        line->setPaddingTop(9);
        line->setPaddingBottom(9);
        if (mark) {
            auto* dot = new kit::Mark(mark);
            dot->setMarginRight(12);
            line->addView(dot);
        }
        brls::Label* k = kit::label(key, kit::TEXT_BODY, "cadran/text2");
        k->setGrow(1.0f);
        k->setWidth(0);
        k->setMarginRight(16);
        line->addView(k);
        brls::Label* v = kit::label(value, kit::TEXT_BODY, value_key);
        v->setHorizontalAlign(brls::HorizontalAlign::RIGHT);
        v->setShrink(0);
        line->addView(v);
        if (!extra.empty()) {
            brls::Label* e = kit::label(extra, kit::TEXT_META, "cadran/text3");
            e->setMinWidth(64);
            e->setHorizontalAlign(brls::HorizontalAlign::RIGHT);
            e->setMarginLeft(12);
            e->setShrink(0);
            line->addView(e);
        }
        table->addView(line);
    };
    // A time with its date only when it is not the console's day.
    const std::string today = split_time(ui::time_text(this->snap.user_time)).first;
    auto when = [&today](uint64_t t) {
        const auto parts = split_time(ui::time_text(t));
        return parts.first.empty() || parts.first == today ? parts.second : parts.first + " " + parts.second;
    };

    if (m.ok) {
        // The two times side by side: what the servers say, what the network clock says.
        brls::Box* compare = panel("");
        row(compare, nullptr, "playguard/clock/dialog_servers_time"_i18n, when(m.unix_seconds), "cadran/text", "");
        if (m.offset_known)
            row(compare, nullptr, "playguard/clock/dialog_network_time"_i18n, when(m.network_seconds),
                on_time ? "cadran/text" : "cadran/amber", "");
    }
    // Each server asked: its time, or why it did not answer.
    brls::Box* servers = panel(brls::getStr("playguard/clock/dialog_asked", (int)this->answered(m), (int)m.lines.size()));
    for (const auto& l : m.lines) {
        if (l.ok) row(servers, "cadran/teal", l.host, when(l.unix_seconds), "cadran/text", l.offset);
        else row(servers, "cadran/amber", l.host, l.error, "cadran/amber", "");
    }

    // What needs the parent, in an amber card: the servers disagreeing, a
    // write that would be refused, no server (and the console clock instead).
    std::string notes = warn;
    if (!m.ok) notes = this->offline ? "playguard/clock/offline_hint"_i18n : "";
    if (!notes.empty()) {
        auto* card = new brls::Box(brls::Axis::ROW);
        card->setAlignItems(brls::AlignItems::FLEX_START);
        card->setBackgroundColor(theme["cadran/amber_surf"]);
        card->setBorderColor(theme["cadran/amber_line"]);
        card->setBorderThickness(1);
        card->setCornerRadius(kit::RADIUS_ROW);
        card->setPadding(12, 18, 12, 16);
        card->setMarginTop(16);
        CadranIcon* icon = kit::icon("alert", 20, "cadran/amber");
        icon->setMarginTop(2);
        icon->setMarginRight(12);
        icon->setShrink(0);
        card->addView(icon);
        brls::Label* label = kit::label(notes, 17, "cadran/amber");
        label->setGrow(1.0f);
        label->setWidth(0);
        card->addView(label);
        body->addView(card);
    }
    // How long the result can be used to set the clock.
    const bool can_set = m.ok && !this->autosync_off && !app::read_only();
    if (can_set) {
        brls::Label* note = kit::label(brls::getStr("playguard/clock/dialog_valid", (int)(clock_flow::LIFETIME_S / 60)),
                                       kit::TEXT_META, "cadran/text3");
        note->setMarginTop(14);
        body->addView(note);
    }

    auto* dialog = ui::box_dialog(body);
    if (can_set) {
        // On time: closing is the answer, setting stays offered.
        dialog->addButton(on_time ? "hints/ok"_i18n : "hints/cancel"_i18n, []() {});
        dialog->addButton(on_time ? "playguard/clock/dialog_set_anyway"_i18n : "playguard/clock/apply_confirm"_i18n,
                          [apply = ui::guarded(this->alive, [this]() { this->apply(); })]() { brls::sync(apply); });
        dialog->setCancelable(true);
        cadran::style_dialog(dialog, on_time ? cadran::Emphasis::NONE : cadran::Emphasis::PRIMARY);
    } else if (!m.ok && this->offline && !app::read_only()) {
        dialog->addButton("hints/cancel"_i18n, []() {});
        dialog->addButton("playguard/clock/guided_copy"_i18n,
                          [copy = ui::guarded(this->alive, [this]() { this->copy(); })]() { brls::sync(copy); });
        dialog->setCancelable(true);
        cadran::style_dialog(dialog, cadran::Emphasis::PRIMARY);
    } else {
        dialog->addButton("hints/ok"_i18n, []() {});
        dialog->setCancelable(true);
        cadran::style_dialog(dialog, cadran::Emphasis::NONE);
    }
    dialog->open();
}

void ClockTab::show_result(const std::string& text, bool no_answer)
{
    brls::Theme theme = brls::Application::getTheme();
    std::string title, body = text;
    if (no_answer) {
        // The report's last line ("No server answered…") is what matters.
        const size_t last_line = text.find_last_of('\n');
        title = last_line == std::string::npos ? text : text.substr(last_line + 1);
        body = (last_line == std::string::npos ? "" : text.substr(0, last_line) + "\n\n") + "playguard/clock/offline_hint"_i18n;
    }
    ui::set_text(result_title, title);
    ui::set_visible(result_title.getView(), !title.empty());
    ui::set_text(result, body);
    result->setTextColor(theme[no_answer ? "cadran/text2" : "cadran/text"]);
    result_card->setBackgroundColor(theme[no_answer ? "cadran/amber_surf" : "cadran/none"]);
    result_card->setBorderColor(theme["cadran/amber_line"]);
    result_card->setBorderThickness(no_answer ? 1 : 0);
    const float pad = no_answer ? 18 : 0;
    result_card->setPadding(pad - 4, pad, pad - 4, pad);
}

void ClockTab::apply()
{
    clock_flow::apply(this->last, [this](const std::string& message) {
        this->last = clock_flow::Measurement{};
        this->show_result(message);
        ui::set_visible(warning.getView(), false);
        this->snap_ok = false;   // the network clock just changed: read it again
        this->refresh();
    });
}

void ClockTab::copy()
{
    clock_flow::apply_console_clock([this](const std::string& message) {
        this->offline = false;
        this->show_result(message);
        ui::set_visible(warning.getView(), false);
        this->snap_ok = false;   // the network clock just changed: read it again
        this->refresh();
    });
}

void ClockTab::open_screen()
{
    ui::open_screen(new ClockTab(), "playguard/tabs/clock"_i18n);
}

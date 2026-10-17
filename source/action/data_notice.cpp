// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/data_notice.hpp"

#include <borealis.hpp>
#include <string>

#include "action/history_flow.hpp"
#include "action/notices.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/history.hpp"

using namespace brls::literals;

namespace data_notice
{

namespace
{
bool s_rescue  = false;   // a report that could not be read
bool s_history = false;   // history.json put aside before at_start()
bool s_started = false;   // at_start() ran: later news is shown at once

// One entry of the notification bell per file: to do while it could not be
// put aside (nothing is saved in it), else for information.
void note(const char* id, bool stuck, const std::string& text)
{
    notices::set(id, stuck ? notices::Level::Action : notices::Level::Info, "playguard/notices/data"_i18n, text);
}
}   // namespace

void rescue_unreadable()
{
    s_rescue = true;
}

void history_put_aside()
{
    brls::Logger::warning("history: damaged file put aside as history.json.bad");
    if (!s_started) {
        s_history = true;
        return;
    }
    brls::sync([]() {
        note("data_history", false, "playguard/data_notice/history_reset"_i18n);
        ui::notify("playguard/notices/data_toast"_i18n);
    });
}

void at_start()
{
    if (s_started) return;
    s_started = true;

    bool found = false;
    auto add = [&found](const char* id, bool stuck, const std::string& text) {
        note(id, stuck, text);
        found = true;
    };

    const config::Loaded cfg = config::loaded();
    if (cfg == config::Loaded::PutAside) add("data_config", false, "playguard/data_notice/config_reset"_i18n);
    else if (cfg == config::Loaded::Stuck) add("data_config", true, "playguard/data_notice/config_stuck"_i18n);

    // Before the entry below, which would otherwise put it aside on its own.
    // Not while a change is being written to it (ui::write_later).
    ui::settle_writes("history");
    size_t entries = 0;
    const history::Damage hist = history::check(&entries);
    // The same reading counts them for the History tab.
    history_flow::counted(entries);
    if (hist == history::Damage::PutAside || s_history) add("data_history", false, "playguard/data_notice/history_reset"_i18n);
    else if (hist == history::Damage::Stuck) add("data_history", true, "playguard/data_notice/history_stuck"_i18n);
    if (s_rescue) add("data_rescue", false, "playguard/data_notice/rescue_unreadable"_i18n);

    if (cfg == config::Loaded::PutAside || cfg == config::Loaded::Stuck) {
        brls::Logger::warning("config: unreadable, defaults in use ({})",
                              cfg == config::Loaded::PutAside ? "kept as config.json.bad" : "could not be put aside");
        history_flow::record_event("config_reset");
    }
    if (hist != history::Damage::None)
        brls::Logger::warning("history: unreadable at start-up ({})",
                              hist == history::Damage::PutAside ? "kept as history.json.bad" : "could not be put aside");
    // Said in the notification bell; a toast points to it.
    if (found) brls::sync([]() { ui::notify("playguard/notices/data_toast"_i18n); });
}

}   // namespace data_notice

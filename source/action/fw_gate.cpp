// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/fw_gate.hpp"

#include <utility>

#include <aurore.hpp>

#include "action/update_flow.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/diagnostics.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/update.hpp"

namespace fw_gate
{

namespace
{
const char* const NAMES[] = { "read_only", "probe", "risk" };

std::string s_choice;            // applied this session ("" before any choice)
bool        s_remembered = false;

bool from_name(const std::string& name, Choice* out)
{
    for (int i = 0; i < 3; i++)
        if (name == NAMES[i]) {
            *out = (Choice)i;
            return true;
        }
    return false;
}

void apply_session(Choice c)
{
    app::set_read_only(c != Choice::Risk);
    if (c == Choice::Probe) app::set_dev_mode(true, false);
    s_choice = NAMES[std::to_underlying(c)];
}
}   // namespace

bool needed()
{
    SysInfo si;
    sysinfo_get(&si);
    return sysinfo_compat(&si) == SysCompat_UntestedNewer;
}

std::string firmware()
{
    return ui::hos_text(sysinfo_hos_version());
}

std::string tested_max()
{
    return ui::hos_text(PCTL_FW_TESTED_MAX);
}

void prepare()
{
    diagnostic::set_firmware_choice(summary);
    if (!needed()) return;
    app::set_read_only(true);
    const auto& cfg = config::get();
    Choice c;
    if (cfg.fw_gate_fw == firmware() && cfg.fw_gate_app == app::version() && from_name(cfg.fw_gate_choice, &c)) {
        apply_session(c);
        s_remembered = true;
    }
}

void on_main_screen()
{
    if (!needed()) return;
    if (!s_remembered) {
        ui::claim_start_screen("the firmware screen");   // first: nothing asked before it
        aurore::sync([]() { aurore::Application::pushActivity(new FirmwareGateActivity()); });
        return;
    }
    update_flow::check_once([](const update_flow::Offer& o) {
        if (o.result.verdict != update::Verdict::UpdateSupports) return;
        ui::notify(aurore::getStr("playguard/fw_gate/update_toast", o.result.latest.version, firmware()));
    });
}

void apply(Choice choice, bool remember)
{
    apply_session(choice);
    s_remembered = remember;
    auto& cfg = config::get();
    if (remember) {
        cfg.fw_gate_fw     = firmware();
        cfg.fw_gate_app    = app::version();
        cfg.fw_gate_choice = NAMES[std::to_underlying(choice)];
        ui::save_config();
    } else {
        forget();
    }
}

void forget()
{
    s_remembered = false;
    auto& cfg = config::get();
    if (cfg.fw_gate_fw.empty() && cfg.fw_gate_app.empty() && cfg.fw_gate_choice.empty()) return;
    cfg.fw_gate_fw.clear();
    cfg.fw_gate_app.clear();
    cfg.fw_gate_choice.clear();
    ui::save_config();
}

std::string summary()
{
    if (s_choice.empty()) return needed() ? "pending" : "none";
    return s_remembered ? s_choice + " (remembered)" : s_choice;
}

}   // namespace fw_gate

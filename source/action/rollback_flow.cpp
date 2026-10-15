// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/rollback_flow.hpp"

#include <borealis.hpp>
#include <cstdio>

#include "action/dev_build_flow.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "ui/ui.hpp"
#include "util/launcher.hpp"
#include "util/paths.hpp"
#include "util/rollback.hpp"

using namespace brls::literals;

namespace rollback_flow
{

namespace
{
std::string label_file() { return paths::data_dir() + "/previous_build.json"; }
std::string check_file() { return paths::data_dir() + "/boot_check.json"; }

bool exists(const std::string& file)
{
    FILE* f = std::fopen(file.c_str(), "rb");
    if (f) std::fclose(f);
    return f != nullptr;
}

std::string running_label()
{
    return app::commit().empty() ? app::version() : app::version() + " (" + app::commit() + ")";
}

// The swap, the running build's name kept for the one now previous. romfs:/
// is read from the running .nro and holds it open: released around it.
bool swap(const std::string& nro, std::string* error)
{
    platform_romfs_release();
    const bool ok = rollback::swap(nro, error);
    platform_romfs_mount();
    if (!ok) return false;
    rollback::write_label(label_file(), running_label());
    std::remove(check_file().c_str());
    return true;
}
}   // namespace

bool at_start()
{
    const std::string nro = dev_build_flow::nro_file();
    if (nro.empty()) return false;
    // This start counts: it is over once the first screen is up (started()).
    rollback::Check check;
    const bool checked = rollback::read_check(check_file(), &check);
    const int before = check.starts;
    if (checked) {
        check.starts++;
        rollback::write_check(check_file(), check);
    }
    const bool have_previous = exists(rollback::previous_file(nro));
    if (!have_previous) return false;
    const rollback::Offer offer = rollback::offer_at_start(checked, before, platform_held_lr(), have_previous);
    if (offer == rollback::Offer::None) return false;

    std::string previous;
    if (!rollback::read_label(label_file(), &previous)) previous = "the previous build";
    // Plain ASCII, in English: borealis (and the translations) are not up yet.
    std::string text = "\n  PlayGuard " + running_label() + "\n\n  ";
    text += offer == rollback::Offer::Crashed
                ? "Its last start did not finish (it may have crashed)."
                : "L + R were held at start-up.";
    text += "\n\n  (A)  Go back to " + previous + "\n  (B)  Start " + running_label() + "\n";
    brls::Logger::info("rollback: offered ({})", offer == rollback::Offer::Crashed ? "crashed" : "L + R");
    if (!platform_boot_choice(text.c_str())) return false;

    std::string err;
    if (!swap(nro, &err)) {
        platform_boot_choice(("\n  Could not go back: " + err + "\n\n  (A) / (B)  Start PlayGuard\n").c_str());
        return false;
    }
    brls::Logger::info("rollback: back to {}", previous);
    if (launcher::can_launch() && launcher::launch_nro(dev_build_flow::nro_sd_path())) return true;
    platform_boot_choice(("\n  " + previous + " is back in place.\n  Close PlayGuard and start it again.\n\n  (A) / (B)  Close\n").c_str());
    return true;
}

void started()
{
    std::remove(check_file().c_str());
}

void installed(const std::string& installed_label)
{
    rollback::write_label(label_file(), running_label());
    rollback::Check check;
    check.label = installed_label;
    rollback::write_check(check_file(), check);
}

bool previous(std::string* label)
{
    const std::string nro = dev_build_flow::nro_file();
    if (nro.empty() || !exists(rollback::previous_file(nro))) return false;
    if (!rollback::read_label(label_file(), label)) *label = "?";
    return true;
}

void go_back_now()
{
    std::string label, err;
    if (!previous(&label)) return;
    if (!swap(dev_build_flow::nro_file(), &err)) {
        ui::info(brls::getStr("playguard/dev_build/back_failed", err));
        return;
    }
    if (launcher::can_launch() && launcher::launch_nro(dev_build_flow::nro_sd_path())) {
        brls::Application::quit();   // hbloader starts it as soon as this one exits
        return;
    }
    ui::info(brls::getStr("playguard/dev_build/done_manual", label));
}

}   // namespace rollback_flow

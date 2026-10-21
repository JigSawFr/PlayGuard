// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/syspatch_flow.hpp"

#include <borealis.hpp>
#include <cstdio>
#include <ctime>

#include "action/pin_lock.hpp"
#include "ui/ui.hpp"
#include "util/dev_builds.hpp"
#include "util/paths.hpp"
#include "util/syspatch_update.hpp"

using namespace brls::literals;

namespace syspatch_flow
{

namespace
{
bool s_busy = false;

// The version sys-patch logged at this boot, without its commit ("1.6.2.3").
std::string installed_version()
{
    std::string v = ui::patch_report().log_version;
    const size_t dash = v.find('-');
    if (dash != std::string::npos) v.resize(dash);
    return v;
}

void install(const syspatch_update::Release& r)
{
    if (ui::refuse_read_only()) return;
    // A sysmodule runs at boot with the console's rights: as for a build of
    // PlayGuard, the PIN whenever one is set.
    if (!pin_lock::confirm()) {
        ui::refuse(pin_lock::refusal_text());
        return;
    }
    if (s_busy) return;
    s_busy = true;
    ui::notify(brls::getStr("playguard/syspatch/downloading", r.version));
    const std::string old = installed_version();
    ui::in_background("sys-patch update", [r, old]() {
        const std::string root = paths::sd_root();
        const std::string zip = paths::data_dir() + "/sys-patch.zip";
        // config/playguard/sys-patch-backups/<the version replaced>-<when>/
        char when[32];
        const std::time_t now = std::time(nullptr);
        std::strftime(when, sizeof(when), "%Y%m%d_%H%M%S", std::localtime(&now));
        const std::string backup = paths::data_dir() + "/sys-patch-backups/" +
                                   (old.empty() ? std::string("unknown") : old) + "-" + when;
        std::string err;
        bool ok = syspatch_update::download(r, zip, &err);
        if (ok) ok = syspatch_update::install(zip, root, backup, &err);
        std::remove(zip.c_str());
        brls::sync([r, ok, err, backup]() mutable {
            s_busy = false;
            if (err == dev_builds::NO_DIGEST) err = "playguard/dev_build/no_digest"_i18n;
            if (!ok) {
                brls::Logger::warning("sys-patch update: {}", err);
                ui::error(brls::getStr("playguard/syspatch/failed", err));
                return;
            }
            brls::Logger::info("sys-patch update: {} installed, the files replaced kept in {}", r.version, backup);
            ui::info(brls::getStr("playguard/syspatch/done", r.version));
        });
    });
}
}   // namespace

void open()
{
    if (s_busy) return;
    s_busy = true;
    ui::notify("playguard/syspatch/checking"_i18n);
    ui::in_background("sys-patch release", []() {
        syspatch_update::Release r;
        std::string err;
        const bool ok = syspatch_update::fetch(&r, &err);
        brls::sync([ok, r, err]() {
            s_busy = false;
            if (!ok) {
                ui::error(brls::getStr("playguard/syspatch/no_release", err));
                return;
            }
            const std::string old = installed_version();
            const std::string shown_old = old.empty() ? "?" : old;
            if (!syspatch_update::newer(r, old)) {
                ui::info(brls::getStr("playguard/syspatch/up_to_date", shown_old, r.version));
                return;
            }
            ui::confirm(brls::getStr("playguard/syspatch/confirm", r.version, shown_old),
                        "playguard/syspatch/install"_i18n, [r]() { install(r); });
        });
    });
}

}   // namespace syspatch_flow

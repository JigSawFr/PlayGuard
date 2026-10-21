// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/sigpatch_flow.hpp"

#include <borealis.hpp>
#include <ctime>

#include "action/pin_lock.hpp"
#include "ui/ui.hpp"
#include "util/patches.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace sigpatch_flow
{

bool offered()
{
    return patches::can_set_aside(ui::patch_report());
}

void set_aside(std::function<void(bool)> done)
{
    const auto finish = [done](bool moved) {
        if (done) done(moved);
    };
    if (!offered() || ui::refuse_read_only()) return finish(false);
    // config/playguard/sigpatches-aside/<when>/, the same paths under it.
    char when[32];
    const std::time_t now = std::time(nullptr);
    std::strftime(when, sizeof(when), "%Y%m%d_%H%M%S", std::localtime(&now));
    const std::string dest = paths::data_dir() + "/sigpatches-aside/" + when;
    const bool hekate = [] {
        for (const std::string& f : ui::patch_report().files)
            if (f == "hekate") return true;
        return false;
    }();
    std::string body = brls::getStr("playguard/sigpatch/confirm", "config/playguard/sigpatches-aside/" + std::string(when));
    if (hekate) body += "\n\n" + "playguard/sigpatch/hekate"_i18n;
    ui::confirm(body, "playguard/sigpatch/set_aside"_i18n,
                [dest, finish]() {
                    // Patch files decide what the console will run: the PIN,
                    // whenever one is set.
                    if (!pin_lock::confirm()) {
                        ui::refuse(pin_lock::refusal_text());
                        return finish(false);
                    }
                    std::vector<std::string> moved;
                    std::string err;
                    const bool ok = patches::set_aside(paths::sd_root(), dest, &moved, &err);
                    for (const std::string& m : moved) brls::Logger::info("sigpatches: {} moved to {}", m, dest);
                    ui::reread_patch_report();
                    if (!ok) {
                        brls::Logger::warning("sigpatches: {}", err);
                        ui::error(brls::getStr("playguard/sigpatch/failed", err));
                    } else {
                        ui::info("playguard/sigpatch/done"_i18n);
                    }
                    finish(!moved.empty());
                },
                [finish]() { finish(false); });
}

}   // namespace sigpatch_flow

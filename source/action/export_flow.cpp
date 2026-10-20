// export_flow — the format picked, then the table exported (see export_flow.hpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/export_flow.hpp"

#include <borealis.hpp>
#include <fmt/format.h>
#include <vector>

#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace export_flow
{

void choose(const char* what, const std::string& stem, std::function<table_export::Table()> make)
{
    std::vector<std::string> labels;
    for (int f = 0; f < 4; f++) labels.push_back(brls::getStr(fmt::format("playguard/activity/formats/{}", f)));
    ui::pick("playguard/activity/export_title"_i18n, labels, config::get().export_format, [what, stem, make](int index) {
        config::get().export_format = index;
        ui::save_config();
        const table_export::Table t = make();
        // Encoded (a PDF, a zip for XLSX) and written off the UI thread.
        const auto format = (table_export::Format)index;
        const std::string failed = "playguard/activity/export_err"_i18n;
        ui::write_later(what, [t, format, failed, stem]() {
            std::string err;
            const std::string path = table_export::save(t, format, paths::exports_dir(), stem, &err);
            if (path.empty()) ui::error(failed, err);
            else ui::notify(brls::getStr("playguard/activity/exported", path));
        });
    });
}

}   // namespace export_flow

// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/upload_flow.hpp"

#include <borealis.hpp>
#include <ctime>
#include <fmt/format.h>
#include <vector>

#include "action/github_login_flow.hpp"
#include "action/notices.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/diagnostics.hpp"
#include "util/github_auth.hpp"
#include "util/log_files.hpp"
#include "util/log_upload.hpp"
#include "util/paths.hpp"
#include "util/pctl_ops_c.hpp"
#include "view/qr_view.hpp"

using namespace brls::literals;

namespace upload_flow
{

namespace
{
bool s_sending = false;   // one upload at a time

brls::Label* centered_label(const std::string& text, int font_size, NVGcolor color)
{
    auto* label = new brls::Label();
    label->setFontSize(font_size);
    label->setTextColor(color);
    label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    label->setSingleLine(false);
    label->setAlignSelf(brls::AlignSelf::STRETCH);   // wraps at the box's width (ui::dialog)
    label->setText(text);
    return label;
}

brls::Box* qr_card(const std::string& title, const std::string& url, const std::string& caption)
{
    auto* card = new brls::Box(brls::Axis::COLUMN);
    card->setAlignItems(brls::AlignItems::CENTER);
    card->setWidth(300);
    card->addView(new QrView(url, 220));
    brls::Label* name = centered_label(title, 20, ui::color_text());
    name->setMarginTop(6);
    card->addView(name);
    card->addView(centered_label(caption, 17, ui::color_note()));
    return card;
}

// Keeps the link in a file of its own, logs/uploads/upload_<date>.url
// (util/log_files.hpp): an Internet shortcut, so one can be found again after
// the dialog is closed, and opened from a computer.
void remember(const log_upload::Result& r)
{
    log_files::save(paths::uploads_dir(), log_files::UPLOAD, log_files::shortcut(r.url, "", ui::now_stamp()),
                    std::time(nullptr));
}

void show_link(const log_upload::Result& r)
{
    SysInfo si;
    sysinfo_get(&si);
    char fw[16];
    sysinfo_version_string(si.hos_version, fw, sizeof(fw));
    log_upload::IssueFields fields;
    fields.version = app::version();
    fields.firmware = fw;
    if (si.ams_valid) fields.atmosphere = fmt::format("{}.{}.{}", si.ams_major, si.ams_minor, si.ams_micro);
    // The menu it was started from is not known: the reporter adds it.
    fields.launcher = si.applet_mode ? "from the Album (applet)" : "over a game (title override)";
    const SysStorage storage = sysinfo_storage(&si);
    if (storage != SysStorage_Unknown) fields.storage = storage == SysStorage_EmuMMC ? "emuMMC" : "sysMMC";
    const std::string issue = log_upload::issue_url(app::repo_url(), r.url, fields);

    std::string text = brls::getStr("playguard/upload/done", log_upload::short_url(r.url));

    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setPadding(24, 24, 12, 24);
    box->addView(centered_label(text, 20, ui::color_text()));
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setJustifyContent(brls::JustifyContent::SPACE_EVENLY);
    row->setMarginTop(14);
    row->addView(qr_card("playguard/upload/qr_report"_i18n, r.url, log_upload::short_url(r.url)));
    row->addView(qr_card("playguard/upload/qr_issue"_i18n, issue, "playguard/upload/qr_issue_note"_i18n));
    box->addView(row);
    auto* dialog = new brls::Dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

// To a secret gist in the GitHub account PlayGuard is signed in to.
void send_gist(const std::vector<log_upload::Part>& parts)
{
    // Too large (a long history): the start, with the report, is what matters.
    bool cut = false;
    const std::vector<log_upload::Part> files = log_upload::gist_files(parts, log_upload::GIST_MAX_BYTES, &cut);
    std::string list;
    for (const auto& p : parts) list += "\n• " + p.name;
    if (cut) list += "\n" + "playguard/upload/partial"_i18n;
    const int kb = (int)((log_upload::total_bytes(files) + 1023) / 1024);
    const std::string body = brls::getStr("playguard/upload/confirm", "playguard/upload/where_gist"_i18n, kb) + "\n" +
                             list + "\n\n" + "playguard/upload/confirm_gist"_i18n;
    const std::string token = github_auth::token();
    ui::confirm(body, "playguard/upload/send"_i18n, [files, token]() {
        ui::notify("playguard/upload/sending"_i18n);
        if (s_sending) return;
        s_sending = true;
        ui::in_background("report upload", [files, token]() {
            const log_upload::Result r = log_upload::upload_gist(files, token, app::version());
            brls::sync([r]() {
                s_sending = false;
                if (!r.ok) {
                    ui::info(r.relogin ? "playguard/upload/relogin"_i18n
                                       : brls::getStr("playguard/upload/failed", r.error));
                    return;
                }
                remember(r);
                show_link(r);
                // The link stays in the notification bell, for later.
                notices::set("upload", notices::Level::Info, "playguard/notices/upload"_i18n, log_upload::short_url(r.url),
                             "playguard/notices/upload_show"_i18n, [r]() { show_link(r); });
            });
        });
    });
}

// Signed out of GitHub: signing in comes first, then the report goes.
void send(const std::vector<log_upload::Part>& parts)
{
    if (!github_auth::token().empty()) {
        send_gist(parts);
        return;
    }
    ui::confirm("playguard/upload/needs_github"_i18n, "playguard/upload/sign_in"_i18n, [parts]() {
        github_login_flow::sign_in([parts]() { send_gist(parts); });
    });
}

// The report, the latest crashes (PlayGuard's, then Atmosphère's: before the
// debug files, so a cut takes the history first) and the debug files.
std::vector<log_upload::Part> with_debug_files(const std::string& report)
{
    std::vector<log_upload::Part> parts{ { "playguard/upload/part_report"_i18n, report } };
    ui::settle_writes();   // the files as they are once the writes asked for are done
    for (auto& f : log_upload::crash_files()) parts.push_back(std::move(f));
    for (auto& f : log_upload::atmosphere_crashes(std::time(nullptr))) parts.push_back(std::move(f));
    for (auto& f : log_upload::debug_files()) parts.push_back(std::move(f));
    return parts;
}
}   // namespace

void send_report(const std::string& report)
{
    send(with_debug_files(report));
}

void choose()
{
    const std::vector<std::string> saved = log_upload::saved_reports();
    if (saved.empty()) {
        send_report(diagnostic::current_report());
        return;
    }
    std::vector<std::string> labels{ "playguard/upload/current"_i18n };
    for (const auto& name : saved) labels.push_back(brls::getStr("playguard/upload/saved", name));
    ui::pick("playguard/upload/pick"_i18n, labels, 0, [saved](int i) {
        if (i == 0) {
            send_report(diagnostic::current_report());
            return;
        }
        const std::string name = saved[(size_t)i - 1];
        std::string content;
        if (!paths::read_file(paths::reports_dir() + "/" + name, content)) {
            ui::info(brls::getStr("playguard/upload/failed", "cannot read " + name));
            return;
        }
        send({ { "logs/reports/" + name, content } });
    });
}

}   // namespace upload_flow

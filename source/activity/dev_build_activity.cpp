// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/dev_build_activity.hpp"

#include <ctime>
#include <fmt/format.h>

#include "action/dev_build_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/changelog.hpp"
#include "view/release_notes.hpp"

using namespace brls::literals;

namespace
{
// The notes as lines: a release's under its own heading (release-please
// writes "## [1.2.0](…) (2026-10-10)" first), else every line of them.
std::vector<changelog::Line> notes_of(const dev_builds::Build& b)
{
    std::vector<changelog::Line> lines;
    if (b.kind == dev_builds::Kind::Release) lines = changelog::release_notes(b.notes, b.version);
    if (lines.empty()) lines = changelog::parse(b.notes, 0);
    return lines;
}

std::string now_iso()
{
    const std::time_t now = (std::time_t)dev_build_flow::now();
    std::tm t{};
    gmtime_r(&now, &t);
    char text[32];
    std::strftime(text, sizeof(text), "%Y-%m-%dT%H:%M:%SZ", &t);
    return text;
}
}   // namespace

void DevBuildActivity::show(const dev_builds::Build& build, bool newer)
{
    brls::Application::pushActivity(new DevBuildActivity(build, newer));
}

void DevBuildActivity::onContentAvailable()
{
    const dev_builds::Build& b = this->build;
    headline->setSingleLine(false);
    facts->setSingleLine(false);
    warning->setSingleLine(false);
    headline->setText(dev_build_flow::label(b));

    std::vector<std::string> lines;
    std::string built = brls::getStr("playguard/dev_build/built", dev_build_flow::short_date(b.date));
    const std::string age = dev_build_flow::age_text(b.date);
    if (!age.empty()) built += " (" + age + ")";
    lines.push_back(built + " · " + fmt::format("{:.1f} MB", (double)b.size / (1024.0 * 1024.0)));
    if (b.kind == dev_builds::Kind::Commit && !b.branch.empty())
        lines.push_back(brls::getStr("playguard/dev_build/branch", b.branch));
    switch (b.ci) {
        case dev_builds::Ci::Passed: lines.push_back("playguard/dev_build/ci_passed_long"_i18n); break;
        case dev_builds::Ci::Failed: lines.push_back("playguard/dev_build/ci_failed_long"_i18n); break;
        case dev_builds::Ci::Running: lines.push_back("playguard/dev_build/ci_running_long"_i18n); break;
        case dev_builds::Ci::Unknown: break;
    }
    if (!b.expires.empty()) {
        const int days = dev_builds::days_between(now_iso(), b.expires);
        lines.push_back(brls::getStr("playguard/dev_build/expires", days < 0 ? 0 : days));
    }
    if (dev_build_flow::installed(b)) {
        lines.push_back("playguard/dev_build/is_installed"_i18n);
    } else if (this->newer) {
        lines.push_back("playguard/dev_build/is_newer"_i18n);
    }
    std::string text;
    for (const std::string& l : lines) text += (text.empty() ? "" : "\n") + l;
    facts->setText(text);
    if (b.ci == dev_builds::Ci::Failed) facts->setTextColor(ui::color_bad());

    std::string running = app::version();
    if (!app::commit().empty()) running += " (" + app::commit() + ")";
    std::string note = brls::getStr("playguard/dev_build/replaces", running);
    if (b.kind != dev_builds::Kind::Release) note += " " + "playguard/dev_build/untested"_i18n;
    if (b.sha256.empty()) note += "\n" + "playguard/dev_build/no_digest"_i18n;
    warning->setText(note);

    install->registerClickAction([b](brls::View*) {
        dev_build_flow::install_now(b);
        return true;
    });

    const std::vector<changelog::Line> notes_lines = notes_of(b);
    notes_header->setTitle(b.kind == dev_builds::Kind::Release ? brls::getStr("playguard/dev_build/notes_release", b.version)
                                                                : "playguard/dev_build/notes_pr"_i18n);
    ui::set_visible(notes_header.getView(), !notes_lines.empty());
    release_notes::fill(notes, notes_lines);
    brls::Application::giveFocus(install.getView());
}

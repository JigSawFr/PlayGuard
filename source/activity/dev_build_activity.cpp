// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/dev_build_activity.hpp"

#include <ctime>
#include <fmt/format.h>

#include "action/dev_build_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/changelog.hpp"
#include "util/markdown.hpp"
#include "view/markdown_view.hpp"
#include "view/qr_view.hpp"
#include "view/release_notes.hpp"

using namespace brls::literals;

namespace
{
std::string now_iso()
{
    const std::time_t now = (std::time_t)dev_build_flow::now();
    std::tm t{};
    gmtime_r(&now, &t);
    char text[32];
    std::strftime(text, sizeof(text), "%Y-%m-%dT%H:%M:%SZ", &t);
    return text;
}

NVGcolor accent()
{
    return brls::Application::getTheme()["brls/sidebar/active_item"];
}

brls::Label* text_label(const std::string& text, float size, NVGcolor color)
{
    auto* l = new brls::Label();
    l->setSingleLine(false);
    l->setFontSize(size);
    l->setTextColor(color);
    l->setText(text);
    return l;
}
}   // namespace

void DevBuildActivity::show(const dev_builds::Build& build, bool newer)
{
    brls::Application::pushActivity(new DevBuildActivity(build, newer));
}

void DevBuildActivity::add_tag(const std::string& text, NVGcolor color)
{
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setBackgroundColor(nvgTransRGBA(color, 40));
    box->setCornerRadius(6);
    box->setPadding(3, 10, 3, 10);
    box->setMarginRight(8);
    auto* l = new brls::Label();
    l->setSingleLine(true);
    l->setFontSize(16);
    l->setTextColor(color);
    l->setText(text);
    box->addView(l);
    tags->addView(box);
}

void DevBuildActivity::add_fact(const std::string& name, const std::string& value, NVGcolor color)
{
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::FLEX_START);
    row->setPadding(6, 0, 6, 0);
    if (!facts->getChildren().empty()) {
        row->setLineTop(1);
        row->setLineColor(nvgTransRGBA(ui::color_note(), 60));
    }
    brls::Label* n = text_label(name, 18, ui::color_note());
    n->setWidth(230);
    n->setShrink(0.0f);
    row->addView(n);
    brls::Label* v = text_label(value, 18, color);
    v->setGrow(1.0f);
    v->setShrink(1.0f);
    row->addView(v);
    facts->addView(row);
}

std::string DevBuildActivity::page() const
{
    const dev_builds::Build& b = this->build;
    if (!b.page.empty()) return b.page;
    const std::string repo = app::repo_url();
    switch (b.kind) {
        case dev_builds::Kind::Release: return repo + "/releases/tag/v" + b.version;   // a version: safe_text::is_version
        case dev_builds::Kind::PullRequest: return repo + "/pull/" + std::to_string(b.pr);
        case dev_builds::Kind::Main:
        case dev_builds::Kind::Commit: return b.commit.empty() ? "" : repo + "/commit/" + b.commit;   // 7 hex digits
    }
    return "";
}

void DevBuildActivity::show_page()
{
    const std::string url = this->page();
    if (url.empty()) return;
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setPadding(20, 32, 10, 32);
    brls::Label* body = text_label("playguard/dev_build/open_github_body"_i18n, 20, ui::color_text());
    body->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    body->setAlignSelf(brls::AlignSelf::STRETCH);
    box->addView(body);
    auto* qr = new QrView(url, 240);
    qr->setMarginTop(12);
    qr->setMarginBottom(10);
    box->addView(qr);
    brls::Label* link = text_label(url.substr(url.find("://") + 3), 17, ui::color_note());
    link->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    link->setAlignSelf(brls::AlignSelf::STRETCH);
    box->addView(link);
    auto* dialog = new brls::Dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

void DevBuildActivity::onContentAvailable()
{
    const dev_builds::Build& b = this->build;
    headline->setSingleLine(false);
    warning->setSingleLine(false);
    facts->setBackgroundColor(nvgTransRGBA(ui::color_text(), 12));

    // Tags: its kind, how its checks ended, installed or newer.
    switch (b.kind) {
        case dev_builds::Kind::Release:
            add_tag(b.prerelease ? "playguard/dev_build/kind_prerelease"_i18n : "playguard/dev_build/kind_release"_i18n, accent());
            break;
        case dev_builds::Kind::Main: add_tag("main", accent()); break;
        case dev_builds::Kind::PullRequest: add_tag(brls::getStr("playguard/dev_build/kind_pr", b.pr), accent()); break;
        case dev_builds::Kind::Commit: add_tag("playguard/dev_build/kind_commit"_i18n, accent()); break;
    }
    switch (b.ci) {
        case dev_builds::Ci::Passed: add_tag("playguard/dev_build/ci_passed"_i18n, ui::color_ok()); break;
        case dev_builds::Ci::Failed: add_tag("playguard/dev_build/ci_failed"_i18n, ui::color_bad()); break;
        case dev_builds::Ci::Running: add_tag("playguard/dev_build/ci_running"_i18n, ui::color_warn()); break;
        case dev_builds::Ci::Unknown: break;
    }
    if (dev_build_flow::installed(b)) add_tag("playguard/dev_build/installed"_i18n, ui::color_ok());
    else if (this->newer) add_tag("playguard/dev_build/new"_i18n, accent());

    // The title: a pull request's own, else what the list calls it.
    switch (b.kind) {
        case dev_builds::Kind::Release: headline->setText(dev_build_flow::release_text(b)); break;
        case dev_builds::Kind::PullRequest: headline->setText(b.title); break;
        case dev_builds::Kind::Main: headline->setText("main · " + b.commit); break;
        case dev_builds::Kind::Commit: headline->setText(b.branch.empty() ? b.commit : b.branch + " · " + b.commit); break;
    }

    // The facts.
    std::string built = dev_build_flow::short_date(b.date);
    const std::string age = dev_build_flow::age_text(b.date);
    if (!age.empty()) built += " · " + age;
    add_fact("playguard/dev_build/fact_built"_i18n, built, ui::color_text());
    add_fact("playguard/dev_build/fact_size"_i18n,
             brls::getStr("playguard/dev_build/size_mb", fmt::format("{:.1f}", (double)b.size / (1024.0 * 1024.0))),
             ui::color_text());
    if (!b.commit.empty()) add_fact("playguard/dev_build/fact_commit"_i18n, b.commit, ui::color_text());
    if (b.kind == dev_builds::Kind::Commit && !b.branch.empty())
        add_fact("playguard/dev_build/fact_branch"_i18n, b.branch, ui::color_text());
    if (b.sha256.empty()) add_fact("playguard/dev_build/fact_checksum"_i18n, "playguard/dev_build/no_digest"_i18n, ui::color_warn());
    else add_fact("playguard/dev_build/fact_checksum"_i18n, b.sha256.substr(0, 16) + "…", ui::color_text());
    if (!b.expires.empty()) {
        const int days = std::max(0, dev_builds::days_between(now_iso(), b.expires));
        add_fact("playguard/dev_build/fact_expires"_i18n, brls::getStr("playguard/dev_build/expires_in", days),
                 days < 7 ? ui::color_warn() : ui::color_text());
    }

    std::string running = app::version();
    if (!app::commit().empty()) running += " (" + app::commit() + ")";
    std::string note = brls::getStr("playguard/dev_build/replaces", running);
    if (b.kind != dev_builds::Kind::Release) note += " " + "playguard/dev_build/untested"_i18n;
    warning->setText(note);

    install->registerClickAction([b](brls::View*) {
        dev_build_flow::install_now(b);
        return true;
    });
    github->registerClickAction([this](brls::View*) {
        this->show_page();
        return true;
    });
    ui::set_visible(github.getView(), !this->page().empty());

    // The notes: a release's in release-please's form (its sections, their
    // icons) when they are, else any notes and a pull request's description
    // as GitHub's Markdown.
    std::vector<changelog::Line> lines;
    if (b.kind == dev_builds::Kind::Release) lines = changelog::release_notes(b.notes, b.version);
    const std::vector<markdown::Block> blocks = lines.empty() ? markdown::parse(b.notes) : std::vector<markdown::Block>{};
    notes_header->setTitle(b.kind == dev_builds::Kind::Release ? brls::getStr("playguard/dev_build/notes_release", b.version)
                                                                : "playguard/dev_build/notes_pr"_i18n);
    ui::set_visible(notes_header.getView(), !lines.empty() || !blocks.empty());
    if (!lines.empty()) release_notes::fill(notes, lines);
    else markdown_view::fill(notes, blocks);
    brls::Application::giveFocus(install.getView());
}

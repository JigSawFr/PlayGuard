// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/build_cell.hpp"

#include "action/dev_build_flow.hpp"
#include "ui/ui.hpp"
#include "util/text.hpp"

using namespace brls::literals;

namespace
{
// The kind of change: a word and a colour (the common types named,
// others as they are written).
BuildTag type_tag(const std::string& type)
{
    struct Known { const char* type; const char* key; int color; };
    // color: 0 accent, 1 amber, 2 blue, 3 purple, 4 red, 5 grey
    static const Known KNOWN[] = {
        { "feat", "playguard/dev_build/type_feat", 0 },   { "fix", "playguard/dev_build/type_fix", 1 },
        { "docs", "playguard/dev_build/type_docs", 2 },   { "perf", "playguard/dev_build/type_perf", 3 },
        { "revert", "playguard/dev_build/type_revert", 4 }, { "refactor", "playguard/dev_build/type_refactor", 5 },
        { "test", "playguard/dev_build/type_test", 5 },   { "chore", "playguard/dev_build/type_chore", 5 },
    };
    int color = 5;
    std::string text = type;
    for (const Known& k : KNOWN)
        if (type == k.type) {
            text = brls::getStr(k.key);
            color = k.color;
        }
    const NVGcolor colors[] = { ui::color_accent(), ui::color_warn(), nvgRGB(0x2F, 0x81, 0xF7), nvgRGB(0x89, 0x57, 0xE5),
                                ui::color_bad(), ui::color_note() };
    return { text, colors[color] };
}
}   // namespace

std::vector<BuildTag> build_tags(const dev_builds::Build& b, bool newer)
{
    std::vector<BuildTag> tags;
    const dev_builds::Subject s = dev_builds::parse_subject(b.title);
    if (b.kind == dev_builds::Kind::PullRequest) tags.push_back({ "#" + std::to_string(b.pr), ui::color_accent() });
    if (b.kind != dev_builds::Kind::Release) {
        if (!s.type.empty()) tags.push_back(type_tag(s.type));
        if (!s.scope.empty()) tags.push_back({ s.scope, ui::color_note() });
        if (s.breaking) tags.push_back({ "playguard/dev_build/breaking"_i18n, ui::color_bad() });
        if (s.pr && b.kind != dev_builds::Kind::PullRequest) tags.push_back({ "#" + std::to_string(s.pr), ui::color_note() });
    }
    if (b.prerelease) tags.push_back({ "playguard/dev_build/kind_prerelease"_i18n, ui::color_warn() });
    switch (b.ci) {
        case dev_builds::Ci::Passed: tags.push_back({ "playguard/dev_build/tag_ci_passed"_i18n, ui::color_ok() }); break;
        case dev_builds::Ci::Failed: tags.push_back({ "playguard/dev_build/tag_ci_failed"_i18n, ui::color_bad() }); break;
        case dev_builds::Ci::Running: tags.push_back({ "playguard/dev_build/tag_ci_running"_i18n, ui::color_warn() }); break;
        case dev_builds::Ci::Unknown: break;
    }
    if (dev_build_flow::installed(b)) tags.push_back({ "playguard/dev_build/installed"_i18n, ui::color_ok() });
    else if (newer) tags.push_back({ "playguard/dev_build/new"_i18n, ui::color_accent() });
    return tags;
}

std::string build_title(const dev_builds::Build& b)
{
    switch (b.kind) {
        case dev_builds::Kind::Release: return brls::getStr("playguard/dev_build/release", b.version);
        case dev_builds::Kind::PullRequest:
        case dev_builds::Kind::Main:
        case dev_builds::Kind::Commit: {
            // A list kept by 1.1.0, which gave only pull requests a title.
            // Every list fetched from 1.2.0 on has them: this can go in 1.3.0.
            if (b.title.empty()) return b.commit;
            const std::string subject = text::capitalise_ascii(dev_builds::parse_subject(b.title).text);
            return subject.empty() ? b.title : subject;
        }
    }
    return b.commit;
}

brls::Box* tag_view(const BuildTag& tag, float font_size)
{
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setBackgroundColor(nvgTransRGBA(tag.color, 38));
    box->setCornerRadius(5);
    box->setPadding(2, 8, 2, 8);
    box->setMarginRight(6);
    box->setShrink(0.0f);
    auto* l = new brls::Label();
    l->setSingleLine(true);
    l->setFontSize(font_size);
    l->setTextColor(tag.color);
    l->setText(tag.text);
    box->addView(l);
    return box;
}

BuildCell::BuildCell()
{
    // A list cell's separator and focus (RecyclerCell, as DetailCell), two lines high.
    this->setAxis(brls::Axis::ROW);
    this->setHeight(brls::View::AUTO);
    this->setFocusable(true);
    this->setAlignItems(brls::AlignItems::CENTER);
    const float indent = brls::getStyle()["brls/listitem/descriptionIndent"];
    this->setPadding(10, indent, 10, indent);

    auto* left = new brls::Box(brls::Axis::COLUMN);
    left->setGrow(1.0f);
    left->setShrink(1.0f);
    left->setMarginRight(indent);
    this->title = new brls::Label();
    this->title->setSingleLine(true);
    this->title->setFontSize(21);
    left->addView(this->title);
    this->tags = new brls::Box(brls::Axis::ROW);
    this->tags->setMarginTop(6);
    left->addView(this->tags);
    this->addView(left);

    auto* right = new brls::Box(brls::Axis::COLUMN);
    right->setAlignItems(brls::AlignItems::FLEX_END);
    right->setShrink(0.0f);
    this->age = new brls::Label();
    this->age->setSingleLine(true);
    this->age->setFontSize(18);
    this->age->setTextColor(ui::color_neutral());
    this->age->setHorizontalAlign(brls::HorizontalAlign::RIGHT);
    right->addView(this->age);
    this->commit = new brls::Label();
    this->commit->setSingleLine(true);
    this->commit->setFontSize(18);
    this->commit->setTextColor(ui::color_note());
    this->commit->setHorizontalAlign(brls::HorizontalAlign::RIGHT);
    this->commit->setMarginTop(4);
    right->addView(this->commit);
    this->addView(right);
}

void BuildCell::set(const dev_builds::Build& b, bool newer)
{
    this->title->setText(build_title(b));
    this->tags->clearViews();
    for (const BuildTag& t : build_tags(b, newer)) this->tags->addView(tag_view(t));
    this->age->setText(dev_build_flow::age_text(b.date));
    // A release: its date; anything else: its commit.
    this->commit->setText(b.kind == dev_builds::Kind::Release ? b.date.substr(0, 10) : b.commit);
}

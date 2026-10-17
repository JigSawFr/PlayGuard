// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/release_notes.hpp"

#include <cctype>

#include "app.hpp"
#include "ui/ui.hpp"
#include "util/paths.hpp"

namespace release_notes
{
namespace
{
// "play time per game" (a commit's subject, a highlight's text after its
// title): "Play time per game".
std::string capitalised(std::string text)
{
    if (!text.empty()) text[0] = (char)std::toupper((unsigned char)text[0]);
    return text;
}

// The emoji `hex` ("1f680") as an image `size` pixels wide, from the Twemoji
// set in img/emoji/; nullptr when it is not one of them.
brls::Image* icon(const std::string& hex, float size)
{
    if (hex.empty() || !paths::file_exists(std::string(BRLS_ASSET("img/emoji/")) + hex + ".png")) return nullptr;
    auto* image = new brls::Image();
    image->setScalingType(brls::ImageScalingType::FIT);
    image->setImageFromRes("img/emoji/" + hex + ".png");
    image->setWidth(size);
    image->setHeight(size);
    return image;
}

// "🚀 Features": its icon and its title in the accent colour, over a rule.
brls::View* section(const changelog::Line& l, bool first)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setMarginTop(first ? 0 : 22);
    box->setMarginBottom(8);
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::CENTER);
    if (brls::Image* image = icon(l.icon, 28)) {
        image->setMarginRight(10);
        row->addView(image);
    }
    row->addView(ui::text_label(l.text, 23, ui::color_accent()));
    box->addView(row);
    auto* rule = new brls::Rectangle(nvgTransRGBA(ui::color_accent(), 90));
    rule->setHeight(2);
    rule->setMarginTop(6);
    box->addView(rule);
    return box;
}

// "activity" before an item of the full list: a small tinted tag.
brls::View* tag(const std::string& text)
{
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setBackgroundColor(nvgTransRGBA(ui::color_accent(), 34));
    box->setCornerRadius(4);
    box->setPadding(1, 7, 1, 7);
    box->setMarginRight(8);
    box->setMarginTop(3);
    box->setShrink(0.0f);
    auto* label = new brls::Label();
    label->setSingleLine(true);
    label->setFontSize(18);
    label->setTextColor(ui::color_accent());
    label->setText(text);
    box->addView(label);
    return box;
}

// One highlight: a tinted card, its icon on the left (an accent bar when it
// has none), its title in the accent colour over its text; in half the row
// (`left`: the first half), the gap between the two split over both.
brls::Box* card(const changelog::Line& l, bool left)
{
    auto* half = new brls::Box(brls::Axis::ROW);
    half->setWidthPercentage(50);
    half->setAlignItems(brls::AlignItems::STRETCH);
    if (left) half->setPaddingRight(6);
    else half->setPaddingLeft(6);
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setGrow(1.0f);
    box->setBackgroundColor(nvgTransRGBA(ui::color_accent(), 22));
    box->setCornerRadius(10);
    box->setAlignItems(brls::AlignItems::FLEX_START);
    box->setPadding(14, 16, 14, 14);
    if (brls::Image* image = icon(l.icon, 36)) {
        image->setMarginRight(14);
        box->addView(image);
    } else {
        box->setAlignItems(brls::AlignItems::STRETCH);
        auto* bar = new brls::Rectangle(ui::color_accent());
        bar->setWidth(4);
        bar->setCornerRadius(2);
        bar->setMarginRight(14);
        box->addView(bar);
    }
    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setGrow(1.0f);
    body->setShrink(1.0f);
    if (l.title.empty()) {
        body->addView(ui::text_label(l.text, 21, ui::color_accent()));   // no lead-in: the line is the title
    } else {
        brls::Label* title = ui::text_label(l.title, 21, ui::color_accent());
        title->setMarginBottom(4);
        body->addView(title);
        body->addView(ui::text_label(capitalised(l.text), 18, ui::color_text()));
    }
    box->addView(body);
    half->addView(box);
    return half;
}
}   // namespace

std::vector<changelog::Line> current(std::string* date)
{
    std::string md;
    paths::read_file(BRLS_ASSET("CHANGELOG.md"), md);
    return changelog::release_notes(md, app::version(), date);
}

std::vector<changelog::Line> highlights()
{
    std::string md;
    if (!paths::read_file(std::string(BRLS_ASSET("release-notes/")) + app::version() + ".md", md)) return {};
    return changelog::parse(md, 0);
}

void fill(brls::Box* box, const std::vector<changelog::Line>& lines)
{
    bool first = true;
    for (const changelog::Line& l : lines) {
        brls::View* view = nullptr;
        switch (l.kind) {
            case changelog::Line::Release:   // not under one version's heading
            case changelog::Line::Section:
                view = section(l, first);
                break;
            case changelog::Line::Item: {
                auto* row = new brls::Box(brls::Axis::ROW);
                row->setAlignItems(brls::AlignItems::FLEX_START);   // bullet and tag on the first line
                row->setMarginLeft(8 + 24 * l.depth);
                row->setMarginTop(6);
                brls::View* bullet = icon(l.icon, 20);
                if (!bullet) {
                    bullet = ui::text_label("•", 20, ui::color_accent());
                    bullet->setWidth(20);
                }
                bullet->setMarginRight(8);
                bullet->setShrink(0.0f);
                row->addView(bullet);
                if (!l.title.empty()) row->addView(tag(l.title));
                brls::Label* text = ui::text_label(capitalised(l.text), 20, ui::color_text());
                text->setGrow(1.0f);
                text->setShrink(1.0f);
                row->addView(text);
                view = row;
                break;
            }
            case changelog::Line::Text:
                view = ui::text_label(l.text, 20, ui::color_note());
                view->setMarginTop(6);
                break;
        }
        box->addView(view);
        first = false;
    }
}

void fill_highlights(brls::Box* box, const std::vector<changelog::Line>& lines)
{
    bool       after_items = false;
    brls::Box* row         = nullptr;   // the row with room for one more card
    for (const changelog::Line& l : lines) {
        if (l.kind == changelog::Line::Item && l.depth == 0) {
            if (!row) {
                row = new brls::Box(brls::Axis::ROW);
                row->setAlignItems(brls::AlignItems::STRETCH);
                row->setMarginTop(12);
                box->addView(row);
                row->addView(card(l, true));
            } else {
                row->addView(card(l, false));
                row = nullptr;
            }
            after_items = true;
            continue;
        }
        if (l.kind == changelog::Line::Item) continue;   // a nested item: not on a card
        brls::View* view = nullptr;
        if (l.kind == changelog::Line::Text) {
            view = after_items ? ui::text_label(l.text, 18, ui::color_note()) : ui::text_label(l.text, 21, ui::color_text());
            view->setMarginTop(after_items ? 16 : 2);
        } else {
            view = section(l, box->getChildren().empty());
        }
        box->addView(view);
        row = nullptr;
    }
}

}   // namespace release_notes

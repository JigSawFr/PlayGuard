// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/markdown_view.hpp"

#include <string>

#include "ui/ui.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace markdown_view
{

namespace
{
NVGcolor accent()
{
    return brls::Application::getTheme()["brls/sidebar/active_item"];
}

brls::Label* label(const std::string& text, float size, NVGcolor color)
{
    auto* l = new brls::Label();
    l->setSingleLine(false);
    l->setFontSize(size);
    l->setTextColor(color);
    l->setText(text);
    return l;
}

// A label that takes the rest of its row and wraps there.
brls::Label* growing(const std::string& text, float size, NVGcolor color)
{
    brls::Label* l = label(text, size, color);
    l->setGrow(1.0f);
    l->setShrink(1.0f);
    return l;
}

// The emoji `hex` from the Twemoji set in img/emoji/, `size` wide; null
// when it is not one of them. Only hex digits reach the path.
brls::Image* icon(const std::string& hex, float size)
{
    if (hex.empty() || hex.size() > 6 || hex.find_first_not_of("0123456789abcdef") != std::string::npos) return nullptr;
    if (!paths::file_exists(std::string(BRLS_ASSET("img/emoji/")) + hex + ".png")) return nullptr;
    auto* image = new brls::Image();
    image->setScalingType(brls::ImageScalingType::FIT);
    image->setImageFromRes("img/emoji/" + hex + ".png");
    image->setWidth(size);
    image->setHeight(size);
    image->setShrink(0.0f);
    return image;
}

brls::View* heading(const markdown::Block& b, bool first)
{
    static const float SIZES[] = { 26, 24, 22, 20, 20, 20 };
    const float size = SIZES[(b.level >= 1 && b.level <= 6 ? b.level : 6) - 1];
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setMarginTop(first ? 0 : (b.level <= 2 ? 24 : 16));
    box->setMarginBottom(6);
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::CENTER);
    if (brls::Image* image = icon(b.icon, size + 4)) {
        image->setMarginRight(10);
        row->addView(image);
    }
    row->addView(growing(b.text, size, b.level <= 3 ? accent() : ui::color_text()));
    box->addView(row);
    if (b.level <= 2) {
        auto* rule = new brls::Rectangle(nvgTransRGBA(accent(), 90));
        rule->setHeight(2);
        rule->setMarginTop(6);
        box->addView(rule);
    }
    return box;
}

// A task's box: ticked (filled, a white mark inside) or not (an outline).
brls::View* checkbox(bool done)
{
    auto* box = new brls::Box(brls::Axis::ROW);
    box->setWidth(20);
    box->setHeight(20);
    box->setShrink(0.0f);
    box->setCornerRadius(4);
    box->setBorderThickness(2);
    box->setBorderColor(done ? accent() : ui::color_note());
    box->setJustifyContent(brls::JustifyContent::CENTER);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setMarginTop(4);
    if (done) {
        box->setBackgroundColor(accent());
        auto* mark = new brls::Rectangle(nvgRGB(255, 255, 255));
        mark->setWidth(8);
        mark->setHeight(8);
        mark->setCornerRadius(2);
        box->addView(mark);
    }
    return box;
}

brls::View* item(const markdown::Block& b)
{
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::FLEX_START);
    row->setMarginLeft(8 + 26 * b.level);
    row->setMarginTop(6);
    brls::View* marker = nullptr;
    if (b.task >= 0) marker = checkbox(b.task == 1);
    else if (b.number > 0) marker = label(std::to_string(b.number) + ".", 19, accent());
    else if (!(marker = icon(b.icon, 20))) marker = label(b.level == 0 ? "•" : "–", 19, accent());
    marker->setMinWidth(22);
    marker->setMarginRight(10);
    marker->setShrink(0.0f);
    row->addView(marker);
    // A task done reads quieter, as GitHub greys it.
    row->addView(growing(b.text, 19, b.task == 1 ? ui::color_note() : ui::color_text()));
    return row;
}

brls::View* code(const markdown::Block& b)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setBackgroundColor(nvgTransRGBA(ui::color_text(), 16));
    box->setCornerRadius(8);
    box->setPadding(12, 16, 12, 16);
    box->setMarginTop(8);
    box->setMarginBottom(4);
    std::string text;
    for (const std::string& l : b.lines) text += (text.empty() ? "" : "\n") + l;
    box->addView(label(text, 16, ui::color_text()));
    return box;
}

brls::View* quote(const markdown::Block& b)
{
    // GitHub's alerts: their colour and their title.
    NVGcolor color = ui::color_note();
    std::string title;
    if (b.alert == "note") color = nvgRGB(0x2F, 0x81, 0xF7), title = "playguard/markdown/note"_i18n;
    else if (b.alert == "tip") color = ui::color_ok(), title = "playguard/markdown/tip"_i18n;
    else if (b.alert == "important") color = nvgRGB(0x89, 0x57, 0xE5), title = "playguard/markdown/important"_i18n;
    else if (b.alert == "warning") color = ui::color_warn(), title = "playguard/markdown/warning"_i18n;
    else if (b.alert == "caution") color = ui::color_bad(), title = "playguard/markdown/caution"_i18n;
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::STRETCH);
    row->setMarginTop(8);
    if (!title.empty()) {
        row->setBackgroundColor(nvgTransRGBA(color, 22));
        row->setCornerRadius(6);
    }
    auto* bar = new brls::Rectangle(color);
    bar->setWidth(4);
    bar->setCornerRadius(2);
    bar->setMarginRight(14);
    row->addView(bar);
    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setGrow(1.0f);
    body->setShrink(1.0f);
    body->setPadding(6, 8, 6, 0);
    if (!title.empty()) {
        brls::Label* t = label(title, 18, color);
        t->setMarginBottom(4);
        body->addView(t);
    }
    if (!b.text.empty()) body->addView(label(b.text, 19, title.empty() ? ui::color_note() : ui::color_text()));
    row->addView(body);
    return row;
}

brls::View* table(const markdown::Block& b)
{
    size_t columns = 0;
    for (const auto& r : b.rows) columns = std::max(columns, r.size());
    auto* grid = new brls::Box(brls::Axis::COLUMN);
    grid->setMarginTop(8);
    grid->setBorderThickness(1);
    grid->setBorderColor(nvgTransRGBA(ui::color_note(), 90));
    grid->setCornerRadius(6);
    for (size_t r = 0; r < b.rows.size(); r++) {
        auto* row = new brls::Box(brls::Axis::ROW);
        row->setAlignItems(brls::AlignItems::STRETCH);
        if (r == 0) row->setBackgroundColor(nvgTransRGBA(accent(), 26));
        else if (r % 2 == 0) row->setBackgroundColor(nvgTransRGBA(ui::color_text(), 8));
        for (size_t c = 0; c < columns; c++) {
            const std::string text = c < b.rows[r].size() ? b.rows[r][c] : "";
            auto* cell = new brls::Box(brls::Axis::ROW);
            cell->setWidthPercentage(100.0f / (float)columns);
            cell->setPadding(6, 10, 6, 10);
            cell->addView(growing(text, 17, r == 0 ? accent() : ui::color_text()));
            row->addView(cell);
        }
        grid->addView(row);
    }
    return grid;
}
}   // namespace

void fill(brls::Box* box, const std::vector<markdown::Block>& blocks)
{
    bool first = true;
    for (const markdown::Block& b : blocks) {
        brls::View* view = nullptr;
        switch (b.kind) {
            case markdown::Block::Heading: view = heading(b, first); break;
            case markdown::Block::Paragraph:
                view = label(b.text, 19, ui::color_text());
                view->setMarginTop(8);
                break;
            case markdown::Block::Item: view = item(b); break;
            case markdown::Block::Code: view = code(b); break;
            case markdown::Block::Quote: view = quote(b); break;
            case markdown::Block::Rule: {
                auto* rule = new brls::Rectangle(nvgTransRGBA(ui::color_note(), 90));
                rule->setHeight(1);
                rule->setMarginTop(14);
                rule->setMarginBottom(6);
                view = rule;
                break;
            }
            case markdown::Block::Table: view = table(b); break;
        }
        box->addView(view);
        first = false;
    }
}

}   // namespace markdown_view

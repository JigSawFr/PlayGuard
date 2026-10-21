// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "tab/cadran_list.hpp"

#include <algorithm>
#include <cstdint>
#include <yoga/Yoga.h>

#include "ui/cadran_kit.hpp"
#include "view/cadran_icon.hpp"

namespace kit = cadran::kit;

namespace
{
// A row's inner margins in a card: left with its icon, left without, right;
// the icon's square and the space after it.
constexpr float ROW_PAD_ICON = 20.0f;
constexpr float ROW_PAD      = 24.0f;
constexpr float ROW_PAD_END  = 24.0f;
constexpr float ICON_SQUARE  = 36.0f;
constexpr float ICON_GAP     = 14.0f;

void append_utf8(std::string& out, uint32_t cp)
{
    if (cp < 0x80) {
        out += (char)cp;
    } else if (cp < 0x800) {
        out += (char)(0xC0 | (cp >> 6));
        out += (char)(0x80 | (cp & 0x3F));
    } else {
        out += (char)(0xE0 | (cp >> 12));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
}

uint32_t upper_cp(uint32_t cp)
{
    if (cp >= 'a' && cp <= 'z') return cp - 0x20;
    if (cp >= 0xE0 && cp <= 0xFE && cp != 0xF7) return cp - 0x20;   // à…þ
    if (cp == 0xFF) return 0x178;                                    // ÿ
    if (cp == 0x153) return 0x152;                                   // œ
    if (cp >= 0x430 && cp <= 0x44F) return cp - 0x20;                // а…я
    if (cp >= 0x450 && cp <= 0x45F) return cp - 0x50;                // ѐ…џ
    return cp;
}

brls::ScrollingFrame* find_scroll(brls::View* view)
{
    if (auto* frame = dynamic_cast<brls::ScrollingFrame*>(view)) return frame;
    if (auto* box = dynamic_cast<brls::Box*>(view))
        for (brls::View* child : box->getChildren())
            if (brls::ScrollingFrame* frame = find_scroll(child)) return frame;
    return nullptr;
}

int index_in(brls::Box* parent, brls::View* child)
{
    const auto& children = parent->getChildren();
    const auto at = std::find(children.begin(), children.end(), child);
    return at == children.end() ? -1 : (int)(at - children.begin());
}

float margin_top(brls::View* view)
{
    const YGValue margin = YGNodeStyleGetMargin(view->getYGNode(), YGEdgeTop);
    return margin.unit == YGUnitPoint ? margin.value : 0.0f;
}

float margin_bottom(brls::View* view)
{
    const YGValue margin = YGNodeStyleGetMargin(view->getYGNode(), YGEdgeBottom);
    return margin.unit == YGUnitPoint ? margin.value : 0.0f;
}

// A row of a card that starts with its icon (row_icon(): the only box among a cell's lines).
bool has_icon(brls::View* row)
{
    auto* box = dynamic_cast<brls::Box*>(row);
    return box && !box->getChildren().empty() && dynamic_cast<brls::Box*>(box->getChildren()[0]);
}

// Moves a line to another box, keeping the view.
void move_to(brls::View* view, brls::Box* to)
{
    if (brls::Box* from = view->getParent()) from->removeView(view, false);
    to->addView(view);
}

CadranIcon* make_icon(const char* name, float size, NVGcolor color)
{
    auto* icon = new CadranIcon();
    icon->setIcon(name);
    icon->setWidth(size);
    icon->setHeight(size);
    icon->setColor(color);
    return icon;
}

// The icon before a row's title, in a soft rounded square.
brls::Box* row_icon(const char* name)
{
    brls::Theme theme = brls::Application::getTheme();
    auto* square = new brls::Box(brls::Axis::ROW);
    square->setWidth(ICON_SQUARE);
    square->setHeight(ICON_SQUARE);
    square->setCornerRadius(10);
    square->setShrink(0);
    square->setBackgroundColor(theme["cadran/surf2"]);
    square->setAlignItems(brls::AlignItems::CENTER);
    square->setJustifyContent(brls::JustifyContent::CENTER);
    square->setMarginRight(ICON_GAP);
    square->addView(make_icon(name, 19, theme["cadran/text2"]));
    return square;
}
}   // namespace

std::string cadran::upper(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        const unsigned char c = (unsigned char)text[i];
        if (c < 0x80) {
            append_utf8(out, upper_cp(c));
            i++;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < text.size()) {
            append_utf8(out, upper_cp(((uint32_t)(c & 0x1F) << 6) | ((unsigned char)text[i + 1] & 0x3F)));
            i += 2;
        } else {   // 3- and 4-byte sequences (CJK, symbols): unchanged
            const size_t n = (c & 0xF0) == 0xE0 ? 3 : (c & 0xF8) == 0xF0 ? 4 : 1;
            out.append(text, i, n);
            i += n;
        }
    }
    return out;
}

// ---------------------------------------------------------------------------

cadran::Card::Card()
    : brls::Box(brls::Axis::COLUMN)
{
    brls::Theme theme = brls::Application::getTheme();
    this->fill = theme["cadran/surf"];
    this->border = theme["cadran/line"];
    this->setAlignItems(brls::AlignItems::STRETCH);
}

void cadran::Card::setAccent(NVGcolor color) { this->border = color; }
void cadran::Card::setFill(NVGcolor color) { this->fill = color; }

void cadran::Card::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style, brls::FrameContext* ctx)
{
    if (height < 1) return;   // every row hidden
    // Its rows hidden, its foot (a note) still shown: the note alone, no card.
    bool rows = false;
    for (brls::View* row : this->getChildren())
        rows |= row->getVisibility() != brls::Visibility::GONE && !dynamic_cast<brls::Label*>(row);
    if (!rows) {
        brls::Box::draw(vg, x, y, width, height, style, ctx);
        return;
    }
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 0.5f, y + 0.5f, width - 1, height - 1, kit::RADIUS_CARD);
    nvgFillColor(vg, this->a(this->fill));
    nvgFill(vg);
    nvgStrokeColor(vg, this->a(this->border));
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
    // A line between the visible rows, from the title (after the icon) to the end.
    brls::View* previous = nullptr;
    for (brls::View* row : this->getChildren()) {
        if (row->getVisibility() == brls::Visibility::GONE) continue;
        // No line above a card's foot (a note): it belongs to the rows.
        if (previous && !dynamic_cast<brls::Label*>(row)) {
            const float inset = has_icon(previous) ? ROW_PAD_ICON + ICON_SQUARE + ICON_GAP : ROW_PAD;
            nvgBeginPath(vg);
            nvgRect(vg, x + inset, row->getY() - 0.5f, width - inset - ROW_PAD_END, 1);
            nvgFillColor(vg, this->a(this->border));
            nvgFill(vg);
        }
        previous = row;
    }
    brls::Box::draw(vg, x, y, width, height, style, ctx);
}

// ---------------------------------------------------------------------------

cadran::Switch::Switch(brls::BooleanCell* cell)
    : cell(cell)
{
    brls::Theme theme = brls::Application::getTheme();
    this->on = theme["cadran/teal_fill"];
    this->off = theme["cadran/track"];
    // A white knob either way: the track's colour says on or off.
    this->knob_on = theme["cadran/knob"];
    this->knob_off = theme["cadran/knob"];
    this->setFocusable(false);
    this->setWidth(50);
    this->setHeight(28);
    this->setShrink(0);
}

void cadran::Switch::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style, brls::FrameContext*)
{
    const bool state = this->cell->isOn();
    const float r = height / 2;
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, width, height, r);
    nvgFillColor(vg, this->a(state ? this->on : this->off));
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgCircle(vg, state ? x + width - r : x + r, y + r, r - 4);
    nvgFillColor(vg, this->a(state ? this->knob_on : this->knob_off));
    nvgFill(vg);
    // Off on a light track, a hairline keeps the knob's edge.
    if (!state) {
        nvgStrokeColor(vg, this->a(brls::Application::getTheme()["cadran/knob_line"]));
        nvgStrokeWidth(vg, 1.0f);
        nvgStroke(vg);
    }
}

// ---------------------------------------------------------------------------

cadran::List::List(brls::View* tab, bool sub_screen)
    : tab(tab)
{
    if (brls::ScrollingFrame* frame = find_scroll(tab))
        for (brls::View* child : frame->getChildren())
            if (auto* column = dynamic_cast<brls::Box*>(child)) this->box = column;
    if (!this->box) return;
    if (sub_screen) this->box->setPadding(20, 72, 36, 72);
    else this->box->setPadding(16, 48, 36, 32);
}

cadran::List::List(brls::View* screen, brls::Box* column)
    : tab(screen), box(column)
{
}

void cadran::List::state(const char* id, const char* color_key)
{
    for (auto& [cell, view] : this->marks) {
        if (cell != this->tab->getView(id)) continue;
        auto* mark = static_cast<kit::Mark*>(view);
        if (color_key) mark->set(color_key);
        mark->setVisibility(color_key ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
    }
}

void cadran::List::tag(const char* id, const std::string& text, bool warn, const std::string& note)
{
    brls::Theme theme = brls::Application::getTheme();
    for (auto& [cell, pill] : this->tags) {
        if (cell != this->tab->getView(id)) continue;
        auto* words = static_cast<brls::Label*>(pill->getChildren()[0]);
        words->setText(text);
        words->setTextColor(theme[warn ? "cadran/amber" : "cadran/pill_ok_ink"]);
        pill->setBackgroundColor(theme[warn ? "cadran/amber_surf" : "cadran/pill_ok"]);
        pill->setBorderColor(theme[warn ? "cadran/amber_line" : "cadran/none"]);
        pill->setVisibility(text.empty() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
        cell->setBorderColor(theme[warn && !text.empty() ? "cadran/amber_line" : "cadran/line"]);
        // The note: the view right after the pill (tiles()).
        const auto& children = cell->getChildren();
        const auto at = std::find(children.begin(), children.end(), pill);
        if (at != children.end() && at + 1 != children.end())
            if (auto* under = dynamic_cast<brls::Label*>(*(at + 1))) {
                under->setText(note);
                under->setVisibility(note.empty() || text.empty() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
            }
    }
}

brls::Box* cadran::List::head(const char* id) const
{
    for (brls::DetailCell* cell : this->readings)
        if (cell == this->tab->getView(id) && !cell->getChildren().empty())
            return dynamic_cast<brls::Box*>(cell->getChildren()[0]);
    return nullptr;
}

void cadran::List::banners(const std::vector<const char*>& ids)
{
    brls::Theme theme = brls::Application::getTheme();
    for (const char* id : ids) {
        auto* cell = dynamic_cast<brls::DetailCell*>(this->tab->getView(id));
        Card* card = cell ? dynamic_cast<Card*>(cell->getParent()) : nullptr;
        if (!card) continue;
        // On its own: out of the card it shares, into a card of its own colours.
        if (card->getChildren().size() > 1) {
            auto* own = new Card();
            const int at = index_in(this->box, card);
            const int row = index_in(card, cell);
            this->box->addView(own, row == 0 ? at : at + 1);
            move_to(cell, own);
            card = own;
        }
        card->setFill(theme["cadran/amber_surf"]);
        card->setAccent(theme["cadran/amber_line"]);
        // The line's own margin under the card, not in it: the card ends
        // where the line and its focus do.
        card->setMarginBottom(12);
        cell->setMarginBottom(0);
        cell->setBackgroundColor(theme["cadran/none"]);
        cell->title->setTextColor(theme["cadran/amber"]);
        cell->detail->setTextColor(theme["cadran/amber"]);
    }
}

brls::Box* cadran::List::tiles(const std::vector<std::pair<const char*, const char*>>& cells, bool status)
{
    if (!this->box || cells.empty()) return nullptr;
    brls::Theme theme = brls::Application::getTheme();
    brls::View* first = this->tab->getView(cells[0].first);
    const int at = first ? index_in(this->box, first) : -1;
    if (at < 0) return nullptr;
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::STRETCH);
    row->setMarginBottom(12);
    // Spaced by the row, not by the tiles' margins: a hidden tile (hekate
    // without its update.bin) leaves no gap behind.
    YGNodeStyleSetGap(row->getYGNode(), YGGutterColumn, 12);
    this->box->addView(row, at);
    for (size_t i = 0; i < cells.size(); i++) {
        auto* cell = dynamic_cast<brls::DetailCell*>(this->tab->getView(cells[i].first));
        if (!cell) continue;
        move_to(cell, row);
        cell->setAxis(brls::Axis::COLUMN);
        cell->setAlignItems(brls::AlignItems::STRETCH);
        cell->setJustifyContent(brls::JustifyContent::FLEX_START);
        cell->setGrow(1.0f);
        cell->setWidth(0);
        // At least this tall, taller when a title takes two lines (larger text).
        cell->setHeight(brls::View::AUTO);
        cell->setMinHeight(status ? 128 : 120);
        if (status) this->readings.push_back(cell);
        cell->setPadding(20, 22, 18, 22);
        cell->setMarginRight(0);
        cell->setCornerRadius(kit::RADIUS_CARD);
        cell->setHighlightCornerRadius(kit::RADIUS_CARD);
        cell->setBackgroundColor(theme["cadran/surf"]);
        cell->setBorderColor(theme["cadran/line"]);
        cell->setBorderThickness(1);
        cell->setLineBottom(0);
        auto* head = new brls::Box(brls::Axis::ROW);
        head->setAlignItems(brls::AlignItems::CENTER);
        head->setMarginBottom(status ? 10 : 14);
        head->addView(make_icon(cells[i].second, status ? 22 : 26, theme["cadran/teal"]));
        cell->addView(head, 0);
        cell->title->setGrow(0);
        cell->title->setMarginRight(0);
        cell->title->setSingleLine(false);
        cell->title->setFontSize(status ? kit::TEXT_META : kit::TEXT_BODY);
        if (status) cell->title->setTextColor(theme["cadran/text3"]);
        cell->detail->setHorizontalAlign(brls::HorizontalAlign::LEFT);
        cell->detail->setMaxWidth(10000);
        cell->detail->setFontSize(status ? 22.0f : kit::TEXT_META);
        cell->detail->setMarginTop(status ? 4 : 2);
        // A status tile's value is a figure, on one line; another's words
        // (a language's name, "Español (Latinoamérica)") take two rather than
        // end in "…".
        cell->detail->setSingleLine(status);
        if (!status) cell->detail->setTextColor(theme["cadran/text2"]);
        if (status) {
            // Its value after a mark of its state (state()), hidden until set.
            auto* value = new brls::Box(brls::Axis::ROW);
            value->setAlignItems(brls::AlignItems::CENTER);
            value->setMarginTop(4);
            cell->detail->setMarginTop(0);
            auto* mark = new kit::Mark("cadran/text3");
            mark->setMarginRight(10);
            mark->setShrink(0);
            mark->setVisibility(brls::Visibility::GONE);
            const int at = index_in(cell, cell->detail);
            cell->removeView(cell->detail, false);
            value->addView(mark);
            value->addView(cell->detail);
            cell->addView(value, at);
            this->marks.push_back({ cell, mark });
            // Its pill under the value (tag()), hidden until set: as wide as
            // its words, which wrap rather than end in "…".
            auto* pill = new brls::Box(brls::Axis::ROW);
            pill->setAlignSelf(brls::AlignSelf::FLEX_START);
            pill->setAlignItems(brls::AlignItems::CENTER);
            pill->setMarginTop(10);
            pill->setPadding(4, 12, 5, 12);
            pill->setCornerRadius(13);
            pill->setBorderThickness(1);
            pill->setVisibility(brls::Visibility::GONE);
            brls::Label* words = kit::label("", kit::TEXT_META, "cadran/text2");
            words->setSingleLine(false);
            pill->addView(words);
            cell->addView(pill, at + 1);
            brls::Label* note = kit::label("", kit::TEXT_META, "cadran/text3");
            note->setSingleLine(false);
            note->setMarginTop(6);
            note->setVisibility(brls::Visibility::GONE);
            cell->addView(note, at + 2);
            this->tags.push_back({ cell, pill });
        }
    }
    return row;
}

void cadran::List::apply(const std::vector<std::pair<const char*, const char*>>& icons)
{
    if (!this->box) return;
    // Marks the column as dressed (the screens' skin leaves it alone).
    auto* mark = new brls::Box();
    mark->setId("cadran_list");
    mark->setVisibility(brls::Visibility::GONE);
    this->box->addView(mark);
    const auto icon_of = [this, &icons](brls::View* cell) -> const char* {
        for (const auto& [id, icon] : icons)
            if (this->tab->getView(id) == cell) return icon;
        return nullptr;
    };

    Card* card = nullptr;   // the card the next cell joins, while cells follow each other
    const std::vector<brls::View*> lines = this->box->getChildren();
    for (brls::View* line : lines) {
        Card* above = card;   // the card just before this line, if any
        if (auto* header = dynamic_cast<brls::Header*>(line)) {
            card = nullptr;
            this->headers.push_back(header);
            section(header);
            continue;
        }
        auto* cell = dynamic_cast<brls::DetailCell*>(line);
        if (!cell) {
            card = nullptr;
            // A note (a headline, larger, keeps its size).
            if (auto* note = dynamic_cast<brls::Label*>(line); note && note->getFontSize() <= 20.5f) {
                note->setFontSize(kit::TEXT_NOTE);
                note->setMarginLeft(0);
                note->setMarginRight(0);
                if (margin_top(note) >= 10) note->setMarginTop(10);
                // Right under a card it explains: the card's foot, in it,
                // so it reads as that card's (and the next card stays apart).
                if (above) {
                    const float below = std::max(margin_bottom(note), 18.0f);
                    note->setMarginTop(2);
                    note->setMarginBottom(0);
                    note->setMargins(2, ROW_PAD_END, 16, ROW_PAD);
                    note->setTextColor(brls::Application::getTheme()["cadran/text3"]);
                    move_to(note, above);
                    above->setMarginBottom(std::max(margin_bottom(above), below));
                }
            }
            continue;
        }
        if (!card) {
            card = new Card();
            card->setMarginTop(margin_top(cell));
            cell->setMarginTop(0);
            this->box->addView(card, index_in(this->box, cell));
        }
        move_to(cell, card);
        row(cell, icon_of(cell));
    }
    this->sync();
}

void cadran::List::section(brls::Header* header)
{
    header->setLineBottom(0);
    header->setPaddingTop(0);
    header->setPaddingBottom(0);
    header->setMarginLeft(0);   // flush with the cards, as the boards' kickers
    header->setMarginBottom(10);
    if (margin_top(header) > 0) header->setMarginTop(26);
    if (!header->getChildren().empty()) header->getChildren()[0]->setVisibility(brls::Visibility::GONE);   // the bar
    // A kicker: read from across the room (its capitals: sync()).
    for (const char* part : { "brls/header/title", "brls/header/subtitle" })
        kit::set_kicker(dynamic_cast<brls::Label*>(header->getView(part)));
}

void cadran::List::row(brls::DetailCell* cell, const char* icon, bool alone)
{
    cell->setHeight(60);
    cell->setPaddingLeft(ROW_PAD_ICON);
    cell->setPaddingRight(ROW_PAD_END);
    cell->setLineBottom(0);
    cell->setLineTop(0);
    cell->setHighlightCornerRadius(kit::RADIUS_ROW);
    cell->title->setFontSize(18);
    cell->detail->setFontSize(kit::TEXT_NOTE);
    if (icon) cell->addView(row_icon(icon), 0);
    else cell->setPaddingLeft(ROW_PAD);
    if (auto* toggle = dynamic_cast<brls::BooleanCell*>(cell)) {
        toggle->detail->setVisibility(brls::Visibility::GONE);
        toggle->addView(new Switch(toggle));
    }
    if (alone) {
        brls::Theme theme = brls::Application::getTheme();
        cell->setCornerRadius(kit::RADIUS_ROW);
        cell->setBackgroundColor(theme["cadran/surf"]);
        cell->setBorderColor(theme["cadran/line"]);
        cell->setBorderThickness(1);
        cell->setMarginBottom(8);
    }
}

void cadran::List::row_image(brls::DetailCell* cell, const std::string& image)
{
    row(cell);
    cell->setPaddingLeft(ROW_PAD_ICON);
    // In a box as row_icon()'s square, so the rows' lines start after it too.
    auto* square = new brls::Box(brls::Axis::ROW);
    square->setWidth(ICON_SQUARE);
    square->setHeight(ICON_SQUARE);
    square->setShrink(0);
    square->setAlignItems(brls::AlignItems::CENTER);
    square->setJustifyContent(brls::JustifyContent::CENTER);
    square->setMarginRight(ICON_GAP);
    auto* picture = new brls::Image();
    picture->setScalingType(brls::ImageScalingType::FIT);
    picture->setImageFromRes(image);
    picture->setWidth(30);
    picture->setHeight(30);
    square->addView(picture);
    cell->addView(square, 0);
}

void cadran::List::recolor_icon(brls::DetailCell* cell, const char* icon, const char* color_key, const char* square_key)
{
    auto* square = cell && !cell->getChildren().empty() ? dynamic_cast<brls::Box*>(cell->getChildren()[0]) : nullptr;
    if (!square) return;   // a row without its icon
    brls::Theme theme = brls::Application::getTheme();
    square->setBackgroundColor(theme[square_key ? square_key : "cadran/surf2"]);
    for (brls::View* v : square->getChildren())
        if (auto* glyph = dynamic_cast<CadranIcon*>(v)) {
            if (icon) glyph->setIcon(icon);
            glyph->setColor(theme[color_key]);
        }
}

brls::Box* cadran::List::rows_in(brls::Box* list)
{
    if (!list) return list;
    for (brls::View* child : list->getChildren())
        if (auto* card = dynamic_cast<Card*>(child)) return card;
    auto* card = new Card();
    list->addView(card);
    return card;
}

cadran::Card* cadran::List::wrap(const char* id)
{
    brls::View* line = this->box ? this->tab->getView(id) : nullptr;
    const int at = line ? index_in(this->box, line) : -1;
    if (at < 0) return nullptr;
    auto* card = new Card();
    card->setPadding(18, 24, 18, 24);
    this->box->addView(card, at);
    move_to(line, card);
    return card;
}

cadran::Card* cadran::List::card_of(const char* id) const
{
    brls::View* view = this->tab->getView(id);
    return view ? dynamic_cast<Card*>(view->getParent()) : nullptr;
}

void cadran::List::sync()
{
    for (brls::Header* header : this->headers) {
        auto* text = dynamic_cast<brls::Label*>(header->getView("brls/header/title"));
        if (!text) continue;
        const std::string title = text->getFullText();
        const std::string capitals = upper(title);
        if (capitals != title) text->setText(capitals);
    }
    brls::Theme theme = brls::Application::getTheme();
    const NVGcolor neutral = theme["brls/list/listItem_value_color"];
    for (brls::DetailCell* cell : this->readings) {
        // A reading that also opens something: the tile is the way in, no "›".
        const std::string value = cell->detail->getFullText();
        const std::string bare = kit::without_chevron(value);
        if (bare != value) cell->detail->setText(bare);
        const NVGcolor c = cell->detail->getTextColor();
        if (c.r == neutral.r && c.g == neutral.g && c.b == neutral.b && c.a == neutral.a)
            cell->detail->setTextColor(theme["cadran/text"]);
    }
}

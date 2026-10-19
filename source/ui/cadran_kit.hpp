// cadran::kit — the small pieces every Cadran screen builds with: a label, an
// icon, a card, a pill, a chip, an empty state; a line's value without its
// "›", a dot for a value's colour, a date in words.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <string>

#include "view/cadran_icon.hpp"

namespace cadran::kit
{
// A label of `size` px in the theme colour `color_key` ("cadran/text2"…);
// several lines when it has to.
brls::Label* label(const std::string& text, float size, const char* color_key = "cadran/text");

// An icon of the boards, `size` px square.
CadranIcon* icon(const char* name, float size, const char* color_key = "cadran/text2");

// A card: the surface, a 1 px line, rounded; `focusable` takes the focus
// ring (no highlight fill); `grow` shares a row with its neighbours.
brls::Box* card(bool focusable, bool grow);

// A card at the top of a screen (not a list: no rows), larger, `grow`
// shares a row.
brls::Box* top_card(bool grow);
// A kicker: an icon and a title in capitals.
brls::Box* kicker(const char* icon_name, const std::string& title);

// A pill's mark: its colour, and a shape that says the same to whom the
// colours look alike (WCAG 1.4.1): a check for what is on (teal), a
// triangle for what needs the parent (amber, coral), a dot for what is only
// said (grey).
class Mark : public brls::View
{
  public:
    explicit Mark(const char* color_key);
    void set(const char* color_key);
    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

  private:
    enum class Shape
    {
        DOT,
        CHECK,
        WARN,
    } shape = Shape::DOT;
    NVGcolor color;
};

// A pill: a mark of `dot_key` (its first child) and its text, on a thin
// border.
brls::Box* pill(const std::string& text, const char* dot_key);

// A rounded chip with a text in the display face (an amount, a period…).
brls::Box* chip(const std::string& text, float height = 48);

// What an empty list shows instead: a card with `icon_name` in a soft circle,
// `title` (why it is empty) and `text` under it (what will come; may be
// empty), centred; `box` takes an action (a chip) at its end.
struct Empty
{
    brls::Box*   box   = nullptr;
    brls::Label* title = nullptr;
    brls::Label* text  = nullptr;
};
Empty empty(const char* icon_name);

// A row whose items go on to the next line when they do not fit (the XML
// has no attribute for it).
void wrap(brls::View* row);

// "12 changes  ›" -> "12 changes"; "›" -> "".
std::string without_chevron(const std::string& value);

// The value colour a line was given, as a dot: teal (ok), amber (warn),
// coral (bad), or the quiet one.
const char* dot_of(NVGcolor value_color);

// "jeudi 8 octobre", from the console's clock, in the app's language; the
// same for a "2026-10-08" (the text itself when it is not a date).
std::string long_date();
std::string long_date(const std::string& ymd);
}   // namespace cadran::kit

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

// The same in the boards' display face (Sora; the other scripts from the
// app's fonts): a dialog's title, a code to read out, a figure.
brls::Label* display(const std::string& text, float size, const char* color_key = "cadran/text");

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

// A controller key as the boards draw it: its letter ("A", "Y", "L") in a
// round bubble, `size` px across, no margins.
brls::Box* key(const char* letter, float size = 24);

// A QR code of `payload`, `side` px (dark on white whatever the theme, as
// phones read it best), its `title` under it and a `caption` smaller (either
// may be empty), centred; `width` (0: the code's own) keeps cards alike.
brls::Box* qr(const std::string& payload, float side, const std::string& title, const std::string& caption,
              float width = 0);

// A progress bar, 12 px high: the well, filled in teal up to `set`.
class Bar : public brls::View
{
  public:
    Bar();
    void set(float fraction);   // 0..1
    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override;

  private:
    float fraction = 0;
};

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

// Over the foot of a scrolling frame drawn at (x, y, width, height): the
// background (`color_key`) fading in over 32 px while there is more below,
// so a line cut by the frame's edge reads as "more", not as the end.
// `corner`: the radius of the card whose foot the frame's is (a picker's
// list), the fade then inside its rounded corners and its 1 px border.
void draw_more_below(NVGcontext* vg, brls::ScrollingFrame* frame, float x, float y, float width, float height,
                     const char* color_key, float corner = 0.0f);

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

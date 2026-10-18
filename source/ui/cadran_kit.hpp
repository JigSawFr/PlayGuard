// cadran::kit — the small pieces every Cadran screen builds with: a label, an
// icon, a card, a pill, a chip; and the glue to the screen it dresses: run a
// line's own A action (a card standing for a line that is out of sight), show
// a view while a line is shown, a line's value without its "›".
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

// A pill: a dot of `dot_key` and its text, on a thin border.
brls::Box* pill(const std::string& text, const char* dot_key);

// A rounded chip with a text in the display face (an amount, a period…).
brls::Box* chip(const std::string& text, float height = 48);

// A row whose items go on to the next line when they do not fit (the XML
// has no attribute for it).
void wrap(brls::View* row);

// Runs the A action `line` registered (its own code, its own checks). False
// when it has none.
bool press(brls::View* line);

// `view` shown while `line` is (after the screen's own refresh).
void follow(brls::View* view, brls::View* line);

// "12 changes  ›" -> "12 changes"; "›" -> "".
std::string without_chevron(const std::string& value);

// The value colour a line was given, as a dot: teal (ok), amber (warn),
// coral (bad), or the quiet one.
const char* dot_of(NVGcolor value_color);

// "jeudi 8 octobre", from the console's clock, in the app's language.
std::string long_date();
}   // namespace cadran::kit

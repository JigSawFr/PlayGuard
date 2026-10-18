// cadran — the Cadran look of borealis' dialogs and pickers: a rounded card
// on a navy backdrop, the buttons as pills (a confirmation in teal, or coral
// when it deletes or stops something), the picker floating above the hints
// with its current row in teal. The toasts and the backdrop are theme values
// (cadran::install_theme).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <cstdint>
#include <functional>
#include <string>

namespace cadran
{
bool enabled();   // tab/cadran_tab.cpp

// Under a dressed dialog's content: its button row (a 56 px pill and 24 px
// below), and what the card keeps from the screen's top and bottom.
constexpr float DIALOG_BUTTONS = 80;
constexpr float DIALOG_MARGIN  = 24;

enum class Emphasis
{
    NONE,      // OK in teal, any other button plain
    PRIMARY,   // the last button is the confirmation: teal
    DANGER,    // the same, in coral
};

// A dialog's text laid out as the boards do (ui::dialog under Cadran),
// left-aligned: its question as a title, a paragraph of "label: value" lines
// (playguard/common/line) as a table, the notes the app adds (the linked
// phone app and its help before unlinking, the temporary unlock, a game about
// to be suspended) at the bottom, small, each with its icon; any other paragraph as text. `extra`
// (the week as it will be) goes after the text, before the notes.
brls::Box* dialog_body(const std::string& text, brls::View* extra = nullptr);

// `dialog` as a card, its buttons as pills. Once: a dressed dialog is left as
// it is (every screen that takes the focus comes through here, see
// install_screens). Call it after the dialog's last addButton. DANGER also
// puts a coral warning before a dialog_body's title.
void style_dialog(brls::Dialog* dialog, Emphasis emphasis = Emphasis::NONE);

// A duration chosen with the stick, in place of the system number pad: up /
// down 15 min, L / R 5 min (or a tap on the arrows), from 0 to 24:00, OK
// or Cancel; `on_value` runs once the dialog has closed.
void prompt_minutes(const std::string& header, uint16_t initial, std::function<void(uint16_t)> on_value);

// A toast's icon: a check for something done, a warning for something not
// done; nullptr in the classic look.
brls::View* toast_icon(bool done);

// A picker's frame as a card above the hints.
void style_picker(brls::Dropdown* dropdown);

// A picker's row; `checked`, the current value's.
void style_pick_row(brls::RadioCell* cell, bool checked);
}   // namespace cadran

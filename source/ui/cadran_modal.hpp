// cadran — the Cadran look of borealis' dialogs and pickers: a rounded card
// on a navy backdrop, the buttons as pills (a confirmation in teal, or coral
// when it deletes or stops something), the picker floating above the hints
// with its current row in teal. The toasts and the backdrop are theme values
// (cadran::install_theme).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
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
// phone app, the temporary unlock, a game about to be suspended) at the
// bottom, small, each with its icon; any other paragraph as text. `extra`
// (the week as it will be) goes after the text, before the notes.
brls::Box* dialog_body(const std::string& text, brls::View* extra = nullptr);

// `dialog` as a card, its buttons as pills. Once: a dressed dialog is left as
// it is (every screen that takes the focus comes through here, see
// install_screens). Call it after the dialog's last addButton. DANGER also
// puts a coral warning before a dialog_body's title.
void style_dialog(brls::Dialog* dialog, Emphasis emphasis = Emphasis::NONE);

// A picker's frame as a card above the hints.
void style_picker(brls::Dropdown* dropdown);

// A picker's row; `checked`, the current value's.
void style_pick_row(brls::RadioCell* cell, bool checked);
}   // namespace cadran

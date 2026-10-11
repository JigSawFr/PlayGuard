// CadranWeekView — "Cadran" prototype: the week's limits as seven tall
// columns that are the editor. Each day takes the focus: up / down change
// its limit by 15 min, left / right change day, L copies it to Monday to
// Friday, R to the weekend, Y sets no limit, A opens the picker. The caller
// keeps the pending values and saves them (+). Strings not localised.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <cstdint>
#include <functional>

#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"

class CadranWeekView : public brls::Box
{
  public:
    CadranWeekView();

    // `days`: the limits as they will be; `live`: as the console has them (a
    // day that differs is drawn as unsaved). 0 = Sunday, laid out Monday first.
    void show(const uint16_t days[7], const uint16_t live[7]);
    void set_on_change(std::function<void(int day, uint16_t minutes)> on_change);   // stick edits
    void set_on_pick(std::function<void(int day)> on_pick);                         // A
    void set_on_copy(std::function<void(int day, bool weekend)> on_copy);           // L / R
    int  focused_day() const { return this->focused; }

    static brls::View* create();

  private:
    class Column;
    struct Col
    {
        Column*       box   = nullptr;
        CadranIcon*   up    = nullptr;
        CadranDigits* value = nullptr;
        brls::Box*    bar   = nullptr;
        brls::Label*  day   = nullptr;
        CadranIcon*   down  = nullptr;
    };
    Col      cols[7];
    uint16_t days[7] = {};
    uint16_t live[7] = {};
    bool     valid = false;
    int      focused = -1;
    std::function<void(int, uint16_t)> on_change;
    std::function<void(int)>           on_pick;
    std::function<void(int, bool)>     on_copy;

    void render();
    void nudge(int day, int delta);
};

// CadranWeekView — the week's limits as seven tall columns that are the
// editor (Cadran's Play timer). The stick moves between the days and out of
// the week; A edits the focused day: then up / down change its limit by
// 15 min, left / right go on to the next day (still editing), L copies it to
// Monday to Friday, R to the weekend, Y sets no limit, A or B ends the edit.
// X, editing or not, opens the day's picker (the quick values, a duration,
// no limit). By touch: a day tapped is edited; on the day being edited, a tap
// on its upper third adds 15 min, on its lower third takes them off (the
// arrows are there), in between ends the edit. The caller keeps the pending
// values and saves them.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <cstdint>
#include <functional>
#include <string>

#include "action/pt_logic.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"

class CadranWeekView : public brls::Box
{
  public:
    CadranWeekView();

    // `days`: the limits as they will be; `live`: as the console has them (a
    // day that differs is drawn as unsaved). 0 = Sunday, laid out Monday first.
    void show(const uint16_t days[7], const uint16_t live[7]);
    // The time played on each weekday's last occurrence (today's included),
    // in minutes, drawn inside its bar against its limit; `ok` false: none.
    void show_played(const uint32_t minutes[7], bool ok);
    // Each day's bedtime alarm, under its name ("21:00", "—"), when the days
    // differ; all empty: none shown, and the week is as tall as before.
    void show_bedtimes(const std::string texts[7]);
    void set_on_change(std::function<void(int day, uint16_t minutes)> on_change);   // stick edits, Y
    void set_on_pick(std::function<void(int day)> on_pick);                         // X
    void set_on_copy(std::function<void(int day, bool weekend)> on_copy);           // L / R
    // Asked before an edit starts (A, X): false refuses it (read-only mode…;
    // the callee says why).
    void set_can_edit(std::function<bool()> can_edit);
    // The edit starts or ends (the screen's key hints follow).
    void set_on_mode(std::function<void(bool editing)> on_mode);
    // The focused day changes (-1: none any more).
    void set_on_focus(std::function<void(int day)> on_focus);
    bool editing() const { return this->edit; }
    void focus_day(int day);   // 0 = Sunday
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
        brls::Box*    fill  = nullptr;   // the time played, from the bar's foot
        brls::Label*  day   = nullptr;
        CadranIcon*   down  = nullptr;
        brls::Box*    bed   = nullptr;   // the bedtime: a moon and the time
        brls::Label*  bed_time = nullptr;
    };
    Col      cols[7];
    uint16_t days[7] = {};
    uint16_t live[7] = {};
    uint32_t played[7] = {};
    bool     played_ok = false;
    bool     valid = false;
    int      focused = -1;
    bool     edit = false;
    bool     moving = false;   // left / right while editing: the edit goes on
    std::function<void(int, uint16_t)> on_change;
    std::function<void(int)>           on_pick;
    std::function<void(int, bool)>     on_copy;
    std::function<bool()>              can_edit;
    std::function<void(bool)>          on_mode;
    std::function<void(int)>           on_focus;

    void render();
    void nudge(int day, int delta);
    pt_logic::Held held;   // ↑/↓ held (nudge())
    void set_edit(bool on);
    bool step(int day, int direction);   // left / right while editing
    void tap(int day, float y);          // a touch on a day, at height y
};

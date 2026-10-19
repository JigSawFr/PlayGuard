// PlayTimerPerDayActivity — per-day limits editor, the one place the week's
// limits are changed day by day, and a saved profile's. The week is the
// editor, as on the Play timer (CadranWeekView: the stick changes a day, X
// opens its picker: a quick value, an exact duration or "No limit"); the
// edits are staged into `pending[]`, drawn in the warning colour until saved;
// presets fill several days at once; "Save" (also +) writes the seven values
// through the play-timer gate. B leaves (asking first when there are unsaved
// edits). For a saved profile, it starts from the profile's days and "Save"
// hands them back instead of writing anything.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#pragma once

#include <array>
#include <borealis.hpp>
#include <functional>
#include <string>

#include "util/pctl_ops_c.hpp"
#include "view/pt_state_header.hpp"

class CadranWeekView;

class PlayTimerPerDayActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/play_timer_perday.xml");

    using Days = std::array<u16, 7>;

    PlayTimerPerDayActivity() = default;
    // Opens on `start_day` (0 = Sunday), its limit picker already open.
    explicit PlayTimerPerDayActivity(int start_day);
    // Opens with the Monday–Friday picker, then the weekend one.
    static PlayTimerPerDayActivity* week_split();
    // Profile mode: `title` names the screen, `days` are the starting point,
    // `on_save` stores the result and returns true when it did (the screen
    // then closes). Nothing is written to the console.
    PlayTimerPerDayActivity(std::string title, const Days& days, std::function<bool(const Days&)> on_save);

    void onContentAvailable() override;

  private:
    bool profile_mode = false;
    std::string profile_title;
    std::function<bool(const Days&)> on_profile_save;
    int  start_day   = -1;      // the picker to open first, if any
    bool start_split = false;   // weekdays, then weekend

    PtState live = {};
    u16     pending[7] = { PT_DAY_NOLIMIT, PT_DAY_NOLIMIT, PT_DAY_NOLIMIT, PT_DAY_NOLIMIT,
                           PT_DAY_NOLIMIT, PT_DAY_NOLIMIT, PT_DAY_NOLIMIT };

    // The week as on the Play timer (view/cadran_week_card.xml).
    brls::View*     cadran_card = nullptr;
    CadranWeekView* cadran_week = nullptr;

    // onContentAvailable()'s actions: the cells', then the screen's buttons.
    void bind_cells();
    void bind_buttons();
    void build_week();
    void reload_from_service();
    void rerender();
    bool has_changes() const;
    void edit_day(int d);
    void fill_days(std::initializer_list<int> days, const std::string& title, std::function<void()> then = nullptr);
    void save();

    BRLS_BIND(PtStateHeader,    state_header, "pt_state_header");
    BRLS_BIND(brls::Label,      unavailable,  "pt_unavailable");
    BRLS_BIND(brls::DetailCell, pt_weekdays,  "pt_weekdays");
    BRLS_BIND(brls::DetailCell, pt_weekend,   "pt_weekend");
    BRLS_BIND(brls::DetailCell, pt_copy_today,"pt_copy_today");
    BRLS_BIND(brls::DetailCell, pt_revert,    "pt_revert");
    BRLS_BIND(brls::DetailCell, pt_save,      "pt_save");
};

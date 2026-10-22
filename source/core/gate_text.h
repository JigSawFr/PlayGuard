// gate_text — the PIN gate's few lines (core/gate_screen.h), in the
// console's language: by libnx's SetLanguage, whose values this takes as
// they are (0 Japanese, 1 American English … 17 Brazilian Portuguese);
// American English for any other. And which of the console's own fonts
// draws that language. Plain C (tests/gate_screen).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GateText_Asking = 0,   // "Enter the parental-control PIN to start it."
    GateText_AskingHint,
    GateText_Opening,
    GateText_Free,
    GateText_Refused,
    GateText_RefusedHint,
    GateText_Error,
    GateText_Tagline,      // "Parental controls, right on the console"
    GateText_Tried,        // "Refused today: %d" (printf: an int)
    GateText_FreeHours,    // "Without the PIN: %s" (printf: gate_hours_text's)
    GateText_Days,         // "Sun,Mon,…,Sat": the days' short names, Sunday first, by commas
    GateText_Paused,       // paused after refusals (GatePauseRule)
    GateText_PausedUntil,  // "It asks again at %s." (printf: "21:30")
    GateText_PausedHint,
    GateText_Closed,       // in its hours without any access (GATE_CLOSED_NAME)
    GateText_ClosedHours,  // "Closed: %s" (printf: gate_hours_text's)
    GateText_ClosedHint,
    GateText_Limit,        // the day's play time used up (GateRules.limit)
    GateText_LimitHint,
    GateText_Badge,        // "Protected by PlayGuard"
    GateText_RetryA,       // Ⓐ's label once refused: "Try again"
    GateText_RetryB,       // Ⓑ's: "HOME menu"
    GateText_TimeLeft,     // "Play time left today: %s" (printf: GateText_Minutes's)
    GateText_Minutes,      // "%d min" (printf: an int)
    GateText_HoursMinutes, // "%d h %02d min" (printf: two ints)
    GateText_FreeUntil,    // "Without the PIN until %s." (printf: "20:00")
    GateText_InMinutes,    // "in %d min" (printf: an int), after the time a pause ends
    GateText_ErrorCode,    // "Error code: %s" (printf: "2054-0012")
    GateText_ClockDiffers, // free hours not applied: the console clock is not the network clock's
    GateText_Count,
} GateText;

const char *gate_text(int language, GateText which);

// The console's shared fonts (pl): the standard one has Latin and Japanese.
typedef enum {
    GateFont_Standard = 0,
    GateFont_ChineseSimplified,
    GateFont_ChineseTraditional,
    GateFont_Korean,
} GateFont;
GateFont gate_text_font(int language);

#ifdef __cplusplus
}
#endif

// gate — a forwarder protected with the PIN (Console › Forwarders): its
// loader starts the gate first, "<gate> --pin-gate <the homebrew's argv>";
// the gate asks for the PIN (the system's PIN screen), then hands over to
// the homebrew or ends the forwarder. The gate is a tiny .nro of its own
// (gate/, written to config/playguard/gate.nro), or PlayGuard itself for a
// forwarder made before it (main.cpp); both run gate_nx.c.
//
// This part is plain C, no libnx (host-tested, tests/gate): the argv, the
// free hours a forwarder may have (no PIN asked then), the log of each try
// that PlayGuard reads into the change history.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GATE_FLAG "--pin-gate"
// In the data directory (sd:/config/playguard/):
#define GATE_NRO_NAME   "gate.nro"         // the gate, as PlayGuard writes it from its romfs
#define GATE_RULES_NAME "gate_rules.txt"   // the free hours, one forwarder a line
#define GATE_LOG_NAME   "gate_log.txt"     // each try, until PlayGuard reads them
#define GATE_LOG_MAX    0x8000             // the gate stops writing past it
#define GATE_CLOSED_NAME "gate_closed.txt" // the hours it never opens, as GATE_RULES_NAME's lines
#define GATE_PAUSE_NAME "gate_pause.txt"   // the pause after refusals: its rule, then each forwarder's
#define GATE_PAUSE_MAX  0x4000

// The homebrew to start, from the gate's argv as one string ("<gate path>
// --pin-gate <argv>"): its path ("sdmc:/…", unquoted, into `nro`) and its
// argv (*argv, pointing into `raw`). False when it is not a gate's argv, or
// the path does not fit.
bool gate_argv_parse(const char *raw, char *nro, size_t nro_size, const char **argv);

// Free hours: on the days of `days` (bit 0 Sunday … bit 6 Saturday), from
// `from` to `to` (minutes of the day, 0..1440); `to` before `from` runs
// past midnight (into the next day, from a day of `days`).
typedef struct {
    uint8_t  days;
    uint16_t from, to;
} GateHours;

// Whether `h` lets the homebrew start without the PIN at `wday` (0 Sunday)
// and `minute` (0..1439). Never with no day or an empty span.
bool gate_hours_free(const GateHours *h, int wday, int minute);

// A line of the rules file: "<days hex>\t<from>\t<to>\t<nro>\n". Length, or
// 0 when it does not fit or `h` is not valid.
size_t gate_rule_format(const GateHours *h, const char *nro, char *buf, size_t size);
// The hours for `nro` in the rules file's text; false when it has none.
bool gate_rule_find(const char *text, size_t len, const char *nro, GateHours *out);

typedef enum {
    GateResult_Pin = 0,     // the PIN entered: started
    GateResult_Free = 1,    // in its free hours, or no PIN on the console: started
    GateResult_Refused = 2, // left with B: not started
    GateResult_Error = 3,   // the PIN screen did not open: not started
    GateResult_Paused = 4,  // paused after refusals: the PIN not asked, not started
    GateResult_Closed = 5,  // in its hours without any access: not started
} GateResult;

// A line of the log: "<YYYY-MM-DD HH:MM>\t<pin|free|refused|error>\t<nro>\n".
size_t gate_log_format(const char *when, GateResult r, const char *nro, char *buf, size_t size);
// One line back (without its "\n"); false for anything else. `when` and
// `nro` point into `line`, with their lengths.
bool gate_log_parse(const char *line, size_t len, const char **when, size_t *when_len, GateResult *r,
                    const char **nro, size_t *nro_len);

const char *gate_result_name(GateResult r);   // "pin", "free", "refused", "error", "paused", "closed"

// The pause after refusals (GATE_PAUSE_NAME): `tries` refused in a row (the
// PIN screen left with B) and the forwarder does not ask again for
// `minutes`. Its file: "rule\t<tries>\t<minutes>\n" (PlayGuard's; tries 0:
// never), then "<streak>\t<until>\t<nro>\n" per forwarder (the gate's:
// the refusals in a row, the POSIX time the pause ends, 0 for none).
typedef struct {
    unsigned tries, minutes;
} GatePauseRule;
#define GATE_PAUSE_TRIES   5     // without a rule line
#define GATE_PAUSE_MINUTES 15
GatePauseRule gate_pause_rule(const char *text, size_t len);
// `nro`'s line: false when it has none (0 refusals, no pause).
bool gate_pause_find(const char *text, size_t len, const char *nro, unsigned *streak, uint64_t *until);
// The file after a try of `nro` at `now` (POSIX): refused, one more in a row,
// and the pause from `now` once they reach the rule's; opened, its line gone;
// anything else (paused, closed, an error), as it was.
// With `rule` NULL, the rule line as it was. Length written, 0 when it does
// not fit in `size`.
size_t gate_pause_update(const char *text, size_t len, const char *nro, GateResult r, uint64_t now, char *out,
                         size_t size);
// The file with `rule` as its rule line, the forwarders' lines kept; or
// without `nro`'s line (`rule` NULL, a pause lifted).
size_t gate_pause_edit(const char *text, size_t len, const GatePauseRule *rule, const char *nro, char *out, size_t size);

// How many lines of the log's `text` say `r` for `nro` on `day` ("2026-10-20",
// the first ten characters of their time): the gate tells the tries of the day.
int gate_log_count(const char *text, size_t len, const char *nro, const char *day, GateResult r);

// "Sat–Sun 09:00–20:00": the days of `h` by the names in `days` (Sunday
// first, 7 of them by commas: GateText_Days), then the span, as Console ›
// Forwarders writes it (forwarders_logic::hours_text). Length, 0 when it
// does not fit or `h` has no day.
size_t gate_hours_text(const GateHours *h, const char *days, char *buf, size_t size);

#ifdef __cplusplus
}
#endif

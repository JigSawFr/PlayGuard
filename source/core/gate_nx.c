// gate_nx — the gate, with libnx (see gate_nx.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gate_nx.h"

#include <stdio.h>
#include <string.h>
#include <switch.h>

#include "gate.h"
#include "nx_types.h"
#include "pctl_ops_internal.h"

#define GATE_DIR "sdmc:/config/playguard/"

// libnx's own exit: 1 ends the application (back to the HOME menu) instead
// of going back to the forwarder's loader.
extern u32 __nx_applet_exit_mode;

// The console's local time: its day of the week (0 Sunday), its minute of
// the day, and "YYYY-MM-DD HH:MM" for the log. False when it cannot be read.
static bool local_time(int *wday, int *minute, char *when, size_t size)
{
    u64 t = 0;
    TimeCalendarTime cal;
    TimeCalendarAdditionalInfo info;
    if (R_FAILED(timeGetCurrentTime(TimeType_LocalSystemClock, &t)) ||
        R_FAILED(timeToCalendarTimeWithMyRule(t, &cal, &info)))
        return false;
    *wday = (int)info.wday;
    *minute = cal.hour * 60 + cal.minute;
    snprintf(when, size, "%04u-%02u-%02u %02u:%02u", cal.year, cal.month, cal.day, cal.hour, cal.minute);
    return true;
}

// Whether `nro` is in its free hours now (GATE_RULES_NAME).
static bool free_now(const char *nro, int wday, int minute)
{
    static char text[4096];
    FILE *f = fopen(GATE_DIR GATE_RULES_NAME, "rb");
    if (!f) return false;
    const size_t n = fread(text, 1, sizeof(text) - 1, f);
    fclose(f);
    text[n] = '\0';
    GateHours h;
    return gate_rule_find(text, n, nro, &h) && gate_hours_free(&h, wday, minute);
}

// The system's PIN screen, unless the console has no PIN.
static GateResult ask_pin(void)
{
    if (R_FAILED(pctlInitialize())) return GateResult_Error;
    u32 len = 0;
    const Result rc = rd_u32(pctlGetServiceSession_Service(), 1206, &len);
    // The PIN screen opens its own privileged session: ours must be closed.
    pctlExit();
    if (R_FAILED(rc)) return GateResult_Error;
    if (len == 0) return GateResult_Free;
    const Result shown = pctlauthShowForConfiguration();
    if (R_SUCCEEDED(shown)) return GateResult_Pin;
    return shown == PCTL_RC_AUTH_CANCELLED ? GateResult_Refused : GateResult_Error;
}

static void log_try(const char *when, GateResult r, const char *nro)
{
    char line[FS_MAX_PATH + 64];
    const size_t n = gate_log_format(when, r, nro, line, sizeof(line));
    if (n == 0) return;
    FILE *f = fopen(GATE_DIR GATE_LOG_NAME, "ab");
    if (!f) return;
    // Not past its size: PlayGuard empties it at its next start.
    if (ftell(f) < GATE_LOG_MAX) fwrite(line, 1, n, f);
    fclose(f);
}

bool gate_nx_run(void)
{
    if (!envHasArgv()) return false;
    char nro[FS_MAX_PATH];
    const char *argv = NULL;
    if (!gate_argv_parse((const char *)envGetArgv(), nro, sizeof(nro), &argv)) return false;

    int wday = -1, minute = -1;
    char when[32] = "?";
    const bool timed = local_time(&wday, &minute, when, sizeof(when));
    const GateResult r = timed && free_now(nro, wday, minute) ? GateResult_Free : ask_pin();
    log_try(when, r, nro);
    if ((r == GateResult_Pin || r == GateResult_Free) && envHasNextLoad() && R_SUCCEEDED(envSetNextLoad(nro, argv)))
        return true;
    __nx_applet_exit_mode = 1;
    return true;
}

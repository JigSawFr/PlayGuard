// PlayGuard — the pctl part of the diagnostic report: pctl_dump() (see
// pctl_ops.h), every read-only query and its raw result.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  This program is free software under the GNU
// General Public License v3 or later; it comes with NO WARRANTY. See the
// LICENSE file or <https://www.gnu.org/licenses/gpl-3.0.html> for details.
#include "pctl_ops.h"
#include "pctl_ops_internal.h"
#include "pure.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static void rep(char **p, char *end, const char *fmt, ...)
{
    if (*p >= end) return;
    size_t available = (size_t)(end - *p);
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(*p, available, fmt, ap);
    va_end(ap);
    if (n > 0) {
        if ((size_t)n >= available) *p = end;
        else *p += n;
    }
}

static void rep_hex(char **p, char *e, const u8 *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        rep(p, e, "%02X ", b[i]);
        if ((i & 15) == 15) rep(p, e, "\n");
    }
    rep(p, e, "\n");
}

static void rep_bool(char **p, char *e, Service *srv, u32 cmd, const char *name)
{
    bool b = false;
    Result r = rd_bool(srv, cmd, &b);
    rep(p, e, "%6u %-38s rc=0x%08X  %s\n", (unsigned)cmd, name, (unsigned)r,
        R_SUCCEEDED(r) ? (b ? "true" : "false") : "-");
}

static void rep_u32(char **p, char *e, Service *srv, u32 cmd, const char *name)
{
    u32 v = 0;
    Result r = rd_u32(srv, cmd, &v);
    rep(p, e, "%6u %-38s rc=0x%08X  ", (unsigned)cmd, name, (unsigned)r);
    if (R_SUCCEEDED(r)) rep(p, e, "%u\n", (unsigned)v); else rep(p, e, "-\n");
}

static void rep_u64(char **p, char *e, Service *srv, u32 cmd, const char *name)
{
    u64 v = 0;
    Result r = rd_u64(srv, cmd, &v);
    rep(p, e, "%6u %-38s rc=0x%08X  ", (unsigned)cmd, name, (unsigned)r);
    if (R_SUCCEEDED(r)) rep(p, e, "0x%016llX (%llu)\n", (unsigned long long)v, (unsigned long long)v);
    else rep(p, e, "-\n");
}

// Opens a fresh session for the next group of commands. Returns NULL (and
// records the failure) when it cannot; the caller then stops the dump.
static Service *dump_session(char **p, char *e, const char *group)
{
    Result ir = pctl_ops_reinit();
    if (R_FAILED(ir)) {
        rep(p, e, "reconnect before %s failed: 0x%08X\n", group, (unsigned)ir);
        return NULL;
    }
    return pctlGetServiceSession_Service();
}

void pctl_dump(char *buf, size_t bufsz)
{
    if (!buf || bufsz == 0) { pctl_ops_exit(); return; }
    char *p = buf, *e = buf + bufsz;
    buf[0] = '\0';
    Service *srv;

    rep(&p, e, "=== pctl state (read-only) ===\n");

    if (!(srv = dump_session(&p, e, "status"))) goto done;
    rep_u32 (&p, e, srv, 1032, "GetSafetyLevel");
    rep_bool(&p, e, srv, 1031, "IsRestrictionEnabled");
    rep_bool(&p, e, srv, 1006, "IsRestrictionTemporaryUnlocked");
    // Whether a PIN is set, not its length: a report may be posted online,
    // and the length narrows the guessing.
    { u32 len = 0; Result r = rd_u32(srv, 1206, &len);
      rep(&p, e, "%6u %-38s rc=0x%08X  %s\n", 1206u, "GetPinCodeLength", (unsigned)r,
          R_SUCCEEDED(r) ? (len ? "set (length not recorded)" : "none") : "-"); }
    rep_u32 (&p, e, srv, 1037, "GetDefaultRatingOrganization");
    rep_u32 (&p, e, srv, 1039, "GetFreeCommunicationApplicationListCount");
    rep_bool(&p, e, srv, 1403, "IsPairingActive");
    rep_u64 (&p, e, srv, 1406, "GetSettingsLastUpdated");
    { u8 raw[3] = {0}; Result r = serviceDispatchOut(srv, 1035, raw);
      rep(&p, e, "%6u %-38s rc=0x%08X  ", 1035u, "GetCurrentSettings", (unsigned)r);
      if (R_SUCCEEDED(r)) rep(&p, e, "rating_age=%u sns=%u comm=%u\n", raw[0], raw[1], raw[2]);
      else rep(&p, e, "-\n"); }
    if (hosversionAtLeast(4, 0, 0)) rep_bool(&p, e, srv, 1062, "GetStereoVisionRestriction");
    for (u32 level = PctlSafetyLevel_YoungChild; level <= PctlSafetyLevel_Teen; level++) {
        u8 raw[3] = {0};
        Result r = serviceDispatchInOut(srv, 1034, level, raw);
        rep(&p, e, "%6u GetSafetyLevelSettings(%u)%-15s rc=0x%08X  ", 1034u, (unsigned)level, "", (unsigned)r);
        if (R_SUCCEEDED(r)) rep(&p, e, "rating_age=%u sns=%u comm=%u\n", raw[0], raw[1], raw[2]);
        else rep(&p, e, "-\n");
    }

    // Software allowed to communicate: the entry layout is not documented,
    // so the bytes as returned (an application ID per entry is expected).
    if (!(srv = dump_session(&p, e, "1044"))) goto done;
    { u8 list[0x200]; memset(list, 0, sizeof(list));
      u32 offset = 0, count = 0;
      Result r = serviceDispatchInOut(srv, 1044, offset, count,
          .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
          .buffers      = { { list, sizeof(list) } });
      rep(&p, e, "%6u %-38s rc=0x%08X  ", 1044u, "GetFreeCommunicationApplicationList(0)", (unsigned)r);
      if (R_SUCCEEDED(r)) {
          rep(&p, e, "count=%u\n", (unsigned)count);
          size_t n = (size_t)count * 0x20;
          if (n > sizeof(list)) n = sizeof(list);
          if (n) rep_hex(&p, e, list, n);
      } else {
          rep(&p, e, "-\n");
      } }

    // Software exempt from the restrictions (a debug list, expected empty).
    if (!(srv = dump_session(&p, e, "1903"))) goto done;
    { u32 exempt = 0;
      Result r = rd_u32(srv, 1903, &exempt);
      rep(&p, e, "%6u %-38s rc=0x%08X  ", 1903u, "GetExemptApplicationListCountForDebug", (unsigned)r);
      if (R_SUCCEEDED(r)) rep(&p, e, "%u\n", (unsigned)exempt); else rep(&p, e, "-\n");
      if (R_SUCCEEDED(r) && exempt > 0) {
          if (!(srv = dump_session(&p, e, "1904"))) goto done;
          u8 list[0x200]; memset(list, 0, sizeof(list));
          u32 offset = 0, count = 0;
          Result lr = serviceDispatchInOut(srv, 1904, offset, count,
              .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
              .buffers      = { { list, sizeof(list) } });
          rep(&p, e, "%6u %-38s rc=0x%08X  ", 1904u, "GetExemptApplicationListForDebug(0)", (unsigned)lr);
          if (R_SUCCEEDED(lr)) {
              rep(&p, e, "count=%u\n", (unsigned)count);
              size_t n = (size_t)count * 8;   // an application ID each, expected
              if (n > sizeof(list)) n = sizeof(list);
              if (n) rep_hex(&p, e, list, n);
          } else {
              rep(&p, e, "-\n");
          }
      } }

    // GetPinCode is probed for compatibility only: the output buffer and the
    // returned length are wiped and never recorded.
    if (!(srv = dump_session(&p, e, "1208"))) goto done;
    { char pin[PIN_BUF]; u32 len = 0;
      Result r = read_pin(srv, pin, &len);
      secure_zero(pin, sizeof(pin));
      secure_zero(&len, sizeof(len));
      rep(&p, e, "%6u %-38s rc=0x%08X  content=not recorded\n", 1208u, "GetPinCode", (unsigned)r); }

    rep(&p, e, "\n=== play timer ===\n");
    if (!pt_fw_supported()) {
        rep(&p, e, "firmware below 21.0.0: PlayTimerSettings layout unknown, skipped\n");
        goto done;
    }
    if (!(srv = dump_session(&p, e, "play timer"))) goto done;
    rep_bool(&p, e, srv, 1453, "IsPlayTimerEnabled");
    rep_bool(&p, e, srv, 1455, "IsRestrictedByPlayTimer");
    rep_bool(&p, e, srv, 1458, "IsPlayTimerAlarmDisabled");
    rep_u64 (&p, e, srv, 1454, "GetPlayTimerRemainingTime (ns)");
    rep_bool(&p, e, srv, 1954, "IsBedtimeAlarmEnabled");
    { u8 h = 0, m = 0; Result rh = rd_u8(srv, 1956, &h), rm = rd_u8(srv, 1957, &m);
      rep(&p, e, "  1956/1957 BedtimeAlarmTime hour/minute       rc=0x%08X/0x%08X  %02u:%02u\n",
          (unsigned)rh, (unsigned)rm, h, m); }
    if (hosversionAtLeast(20, 0, 0)) {
        u8 h = 0, m = 0; Result rh = rd_u8(srv, 1958, &h), rm = rd_u8(srv, 1959, &m);
        rep(&p, e, "  1958/1959 BedtimeAlarmResetTime hour/minute  rc=0x%08X/0x%08X  %02u:%02u\n",
            (unsigned)rh, (unsigned)rm, h, m);
    }

    if (!(srv = dump_session(&p, e, "145601"))) goto done;
    { u8 b[0x44]; memset(b, 0xCC, sizeof(b));
      Result r = serviceDispatchOut(srv, 145601, b);
      rep(&p, e, "\n145601 GetPlayTimerSettings rc=0x%08X (0x44 bytes)\n", (unsigned)r);
      if (R_SUCCEEDED(r)) {
          rep_hex(&p, e, b, sizeof(b));
          u16 c[PT_U16_COUNT], d[7];
          memcpy(c, b, sizeof(c));
          pt_decode(c, d);
          rep(&p, e, "decoded per-day minutes Sun..Sat:");
          for (int n = 0; n < 7; n++) {
              if (d[n] != PT_DAY_NOLIMIT) rep(&p, e, " %u", (unsigned)d[n]);
              else rep(&p, e, " -");
          }
          rep(&p, e, "   header=%04X %04X\n", c[0], c[1]);
          rep(&p, e, "modes: timerMode=%s restrictionMode=%s\n",
              pt_daily(c) ? "DAILY" : "EACH_DAY_OF_THE_WEEK", pt_suspends(c) ? "FORCED_TERMINATION" : "ALARM");
          PtBedtime bt[7];
          pt_bedtime_decode(c, bt);
          rep(&p, e, "decoded bedtime Sun..Sat (alarm/allowed again):");
          for (int n = 0; n < 7; n++)
              rep(&p, e, " %s%02u:%02u/%02u:%02u", bt[n].on ? "" : "off ", bt[n].hour, bt[n].minute,
                  bt[n].end_hour, bt[n].end_minute);
          rep(&p, e, "\n");
      } }

    if (hosversionAtLeast(20, 0, 0)) {
        if (!(srv = dump_session(&p, e, "1459"))) goto done;
        u8 b[0x20]; memset(b, 0, sizeof(b));
        Result r = serviceDispatchOut(srv, 1459, b);
        rep(&p, e, "\n1459 GetPlayTimerRemainingTimeDisplayInfo rc=0x%08X (0x20 bytes)\n", (unsigned)r);
        if (R_SUCCEEDED(r)) rep_hex(&p, e, b, sizeof(b));
    }
    if (hosversionAtLeast(23, 0, 0)) {
        if (!(srv = dump_session(&p, e, "1460"))) goto done;
        u8 in = 0;
        u8 b[0x18]; memset(b, 0, sizeof(b));
        Result r = serviceDispatchInOut(srv, 1460, in, b);
        rep(&p, e, "\n1460 GetWatcherStatusDisplayInfo(0) rc=0x%08X (0x18 bytes)\n", (unsigned)r);
        if (R_SUCCEEDED(r)) rep_hex(&p, e, b, sizeof(b));
    }

    // Test / debug getters: raw values, interpretation unverified on hardware.
    if (!(srv = dump_session(&p, e, "1952"))) goto done;
    rep(&p, e, "\n");
    rep_u64(&p, e, srv, 1952, "GetPlayTimerSpentTimeForTest (raw)");
    if (hosversionAtLeast(20, 0, 0)) {
        if (!(srv = dump_session(&p, e, "1960"))) goto done;
        rep_u64(&p, e, srv, 1960, "GetExtraPlayingTimeForDebug (raw)");
    }

done:
    pctl_ops_exit();
    rep(&p, e, "\nTool-owned pctl session released.\n");
}

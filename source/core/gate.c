// gate — argv, free hours and log (see gate.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gate.h"

#include <stdio.h>
#include <string.h>

bool gate_argv_parse(const char *raw, char *nro, size_t nro_size, const char **argv)
{
    if (!raw || !nro || nro_size == 0 || !argv) return false;
    nro[0] = '\0';
    *argv = NULL;
    // The gate's path, then the flag: the rest is the homebrew's argv.
    const char *p = raw;
    if (*p == '"') {
        p = strchr(p + 1, '"');
        if (!p) return false;
        p++;
    } else {
        p = strchr(p, ' ');
        if (!p) return false;
    }
    static const char flag[] = " " GATE_FLAG " ";
    const size_t flag_len = strlen(flag);
    if (strncmp(p, flag, flag_len) != 0) return false;
    const char *rest = p + flag_len;
    // Its path: its first word, or what its quotes hold.
    const char *start = rest, *end;
    if (*rest == '"') {
        start = rest + 1;
        end = strchr(start, '"');
        if (!end) return false;
    } else {
        end = strchr(rest, ' ');
        if (!end) end = rest + strlen(rest);
    }
    const size_t n = (size_t)(end - start);
    if (n < 7 || n >= nro_size || strncmp(start, "sdmc:/", 6) != 0) return false;
    memcpy(nro, start, n);
    nro[n] = '\0';
    *argv = rest;
    return true;
}

bool gate_hours_free(const GateHours *h, int wday, int minute)
{
    if (!h || (h->days & 0x7F) == 0 || h->from == h->to || h->from > 1440 || h->to > 1440) return false;
    if (wday < 0 || wday > 6 || minute < 0 || minute >= 1440) return false;
    if (h->from < h->to) return (h->days >> wday & 1) && minute >= h->from && minute < h->to;
    // Past midnight: the evening of a day of `days`, or the morning after one.
    const int before = (wday + 6) % 7;
    return ((h->days >> wday & 1) && minute >= h->from) || ((h->days >> before & 1) && minute < h->to);
}

static bool valid(const GateHours *h)
{
    return h && (h->days & 0x7F) && !(h->days & 0x80) && h->from <= 1440 && h->to <= 1440 && h->from != h->to;
}

size_t gate_rule_format(const GateHours *h, const char *nro, char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    buf[0] = '\0';
    if (!valid(h) || !nro || !*nro || strpbrk(nro, "\t\r\n")) return 0;
    const int n = snprintf(buf, size, "%02x\t%u\t%u\t%s\n", h->days, h->from, h->to, nro);
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)n;
}

// A number of `digits` base `base` at *p, up to `stop`; false otherwise.
static bool number(const char **p, const char *end, char stop, unsigned base, unsigned max, unsigned *out)
{
    unsigned v = 0;
    const char *s = *p;
    if (s >= end || *s == stop) return false;
    for (; s < end && *s != stop; s++) {
        unsigned d;
        if (*s >= '0' && *s <= '9') d = (unsigned)(*s - '0');
        else if (base == 16 && *s >= 'a' && *s <= 'f') d = (unsigned)(*s - 'a' + 10);
        else if (base == 16 && *s >= 'A' && *s <= 'F') d = (unsigned)(*s - 'A' + 10);
        else return false;
        if (d >= base) return false;
        v = v * base + d;
        if (v > max) return false;
    }
    if (s >= end) return false;
    *out = v;
    *p = s + 1;
    return true;
}

bool gate_rule_find(const char *text, size_t len, const char *nro, GateHours *out)
{
    if (!text || !nro || !out) return false;
    const size_t want = strlen(nro);
    for (size_t at = 0; at < len;) {
        const char *line = text + at;
        size_t n = 0;
        while (at + n < len && line[n] != '\n') n++;
        at += n + 1;
        size_t m = n;
        if (m > 0 && line[m - 1] == '\r') m--;
        const char *p = line, *end = line + m;
        unsigned days, from, to;
        if (!number(&p, end, '\t', 16, 0x7F, &days) || !number(&p, end, '\t', 10, 1440, &from) ||
            !number(&p, end, '\t', 10, 1440, &to))
            continue;
        if ((size_t)(end - p) != want || memcmp(p, nro, want) != 0) continue;
        const GateHours h = { (uint8_t)days, (uint16_t)from, (uint16_t)to };
        if (!valid(&h)) continue;
        *out = h;
        return true;
    }
    return false;
}

const char *gate_result_name(GateResult r)
{
    switch (r) {
        case GateResult_Pin: return "pin";
        case GateResult_Free: return "free";
        case GateResult_Refused: return "refused";
        default: return "error";
    }
}

size_t gate_log_format(const char *when, GateResult r, const char *nro, char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    buf[0] = '\0';
    if (!when || !*when || strpbrk(when, "\t\r\n") || !nro || !*nro || strpbrk(nro, "\t\r\n")) return 0;
    const int n = snprintf(buf, size, "%s\t%s\t%s\n", when, gate_result_name(r), nro);
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)n;
}

bool gate_log_parse(const char *line, size_t len, const char **when, size_t *when_len, GateResult *r,
                    const char **nro, size_t *nro_len)
{
    if (!line) return false;
    if (len > 0 && line[len - 1] == '\r') len--;
    const char *tab1 = memchr(line, '\t', len);
    if (!tab1 || tab1 == line) return false;
    const char *rest = tab1 + 1;
    const size_t rest_len = len - (size_t)(rest - line);
    const char *tab2 = memchr(rest, '\t', rest_len);
    if (!tab2) return false;
    const size_t name_len = (size_t)(tab2 - rest);
    GateResult res;
    if (name_len == 3 && memcmp(rest, "pin", 3) == 0) res = GateResult_Pin;
    else if (name_len == 4 && memcmp(rest, "free", 4) == 0) res = GateResult_Free;
    else if (name_len == 7 && memcmp(rest, "refused", 7) == 0) res = GateResult_Refused;
    else if (name_len == 5 && memcmp(rest, "error", 5) == 0) res = GateResult_Error;
    else return false;
    const char *path = tab2 + 1;
    const size_t path_len = len - (size_t)(path - line);
    if (path_len == 0 || memchr(path, '\t', path_len)) return false;
    *when = line;
    *when_len = (size_t)(tab1 - line);
    *r = res;
    *nro = path;
    *nro_len = path_len;
    return true;
}

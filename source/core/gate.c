// gate — argv, free hours and log (see gate.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gate.h"

#include <stdio.h>
#include <string.h>

// The span of a "--gate-free=" or "--gate-closed=" token: "<days hex>,<from>,<to>".
static bool number(const char **p, const char *end, char stop, unsigned base, unsigned max, unsigned *out);
static bool valid(const GateHours *h);
static bool span_of(const char *s, const char *end, GateHours *out)
{
    unsigned days, from, to;
    // `number` wants a stop after each: the last one ends at `end`, read with a sentinel.
    char last[8];
    const char *p = s;
    if (!number(&p, end, ',', 16, 0x7F, &days) || !number(&p, end, ',', 10, 1440, &from)) return false;
    const size_t n = (size_t)(end - p);
    if (n == 0 || n >= sizeof(last) - 1) return false;
    memcpy(last, p, n);
    last[n] = ';';
    const char *q = last;
    if (!number(&q, last + n + 1, ';', 10, 1440, &to)) return false;
    const GateHours h = { (uint8_t)days, (uint16_t)from, (uint16_t)to };
    if (!valid(&h)) return false;
    *out = h;
    return true;
}

// One rules token, [s, end): read into *r.
static void rule_token(const char *s, const char *end, GateRules *r)
{
    const size_t n = (size_t)(end - s);
#define IS(lit) (n >= sizeof(lit) - 1 && memcmp(s, lit, sizeof(lit) - 1) == 0)
    if (n == sizeof("--gate-rules") - 1 && IS("--gate-rules")) r->carried = true;
    else if (n == sizeof("--gate-limit") - 1 && IS("--gate-limit")) r->limit = true;
    else if (IS("--gate-free=")) {
        if (r->free_n < GATE_SPANS && span_of(s + sizeof("--gate-free=") - 1, end, &r->free[r->free_n])) r->free_n++;
    } else if (IS("--gate-closed=")) {
        if (r->closed_n < GATE_SPANS && span_of(s + sizeof("--gate-closed=") - 1, end, &r->closed[r->closed_n]))
            r->closed_n++;
    } else if (IS("--gate-pause=")) {
        const char *p = s + sizeof("--gate-pause=") - 1;
        unsigned tries, minutes;
        char last[12];
        if (!number(&p, end, ',', 10, 1000, &tries)) return;
        const size_t m = (size_t)(end - p);
        if (m == 0 || m >= sizeof(last) - 1) return;
        memcpy(last, p, m);
        last[m] = ';';
        const char *q = last;
        if (!number(&q, last + m + 1, ';', 10, 100000, &minutes)) return;
        r->has_pause = true;
        r->pause_tries = minutes ? tries : 0;
        r->pause_minutes = r->pause_tries ? minutes : 0;
    }
#undef IS
}

bool gate_argv_parse(const char *raw, char *nro, size_t nro_size, const char **argv)
{
    return gate_argv_rules(raw, nro, nro_size, argv, NULL);
}

bool gate_argv_rules(const char *raw, char *nro, size_t nro_size, const char **argv, GateRules *rules)
{
    if (rules) memset(rules, 0, sizeof(*rules));
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
    // The rules, each "--gate-…" word, before the homebrew's path.
    GateRules scratch;
    while (strncmp(rest, "--gate-", 7) == 0) {
        const char *word_end = strchr(rest, ' ');
        if (!word_end) return false;   // nothing after them: no homebrew
        rule_token(rest, word_end, rules ? rules : &scratch);
        rest = word_end + 1;
    }
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

bool gate_spans_hold(const GateHours *spans, unsigned n, int wday, int minute)
{
    for (unsigned i = 0; spans && i < n; i++)
        if (gate_hours_free(&spans[i], wday, minute)) return true;
    return false;
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

size_t gate_rules_format(const GateRules *r, char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    buf[0] = '\0';
    if (!r) return 0;
    size_t used = 0;
    bool fits = true;
#define ADD(...)                                                                       \
    do {                                                                               \
        const int k = snprintf(buf + used, size - used, __VA_ARGS__);                  \
        if (k < 0 || (size_t)k >= size - used) fits = false;                           \
        else used += (size_t)k;                                                        \
    } while (0)
    ADD(" --gate-rules");
    for (unsigned i = 0; fits && i < r->free_n && i < GATE_SPANS; i++)
        if (valid(&r->free[i])) ADD(" --gate-free=%02x,%u,%u", r->free[i].days, r->free[i].from, r->free[i].to);
    for (unsigned i = 0; fits && i < r->closed_n && i < GATE_SPANS; i++)
        if (valid(&r->closed[i]))
            ADD(" --gate-closed=%02x,%u,%u", r->closed[i].days, r->closed[i].from, r->closed[i].to);
    if (fits && r->has_pause) ADD(" --gate-pause=%u,%u", r->pause_tries, r->pause_tries ? r->pause_minutes : 0);
    if (fits && r->limit) ADD(" --gate-limit");
#undef ADD
    if (!fits) {
        buf[0] = '\0';
        return 0;
    }
    return used;
}

const char *gate_result_name(GateResult r)
{
    switch (r) {
        case GateResult_Pin: return "pin";
        case GateResult_Free: return "free";
        case GateResult_Refused: return "refused";
        case GateResult_Paused: return "paused";
        case GateResult_Closed: return "closed";
        case GateResult_Limit: return "limit";
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
    else if (name_len == 6 && memcmp(rest, "paused", 6) == 0) res = GateResult_Paused;
    else if (name_len == 6 && memcmp(rest, "closed", 6) == 0) res = GateResult_Closed;
    else if (name_len == 5 && memcmp(rest, "limit", 5) == 0) res = GateResult_Limit;
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

int gate_log_count(const char *text, size_t len, const char *nro, const char *day, GateResult r)
{
    if (!text || !nro || !day || strlen(day) != 10) return 0;
    const size_t want = strlen(nro);
    int n = 0;
    for (size_t at = 0; at < len;) {
        const char *line = text + at;
        size_t m = 0;
        while (at + m < len && line[m] != '\n') m++;
        at += m + 1;
        const char *when, *path;
        size_t wn, pn;
        GateResult got;
        if (!gate_log_parse(line, m, &when, &wn, &got, &path, &pn)) continue;
        if (got == r && wn >= 10 && memcmp(when, day, 10) == 0 && pn == want && memcmp(path, nro, want) == 0) n++;
    }
    return n;
}

size_t gate_hours_text(const GateHours *h, const char *days, char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    buf[0] = '\0';
    if (!valid(h) || !days) return 0;
    // The 7 names, Sunday first.
    char names[7][24];
    const char *p = days;
    for (int i = 0; i < 7; i++) {
        const char *end = strchr(p, ',');
        const size_t n = end ? (size_t)(end - p) : strlen(p);
        if (n == 0 || n >= sizeof(names[i]) || (!end && i < 6)) return 0;
        memcpy(names[i], p, n);
        names[i][n] = '\0';
        p = end ? end + 1 : p + n;
    }
    char d[200] = "";
    const unsigned days7 = h->days & 0x7F;
    // Monday to Sunday, Monday to Friday, Saturday and Sunday: as spans.
    if (days7 == 0x7F) snprintf(d, sizeof(d), "%s\xE2\x80\x93%s", names[1], names[0]);
    else if (days7 == 0x3E) snprintf(d, sizeof(d), "%s\xE2\x80\x93%s", names[1], names[5]);
    else if (days7 == 0x41) snprintf(d, sizeof(d), "%s\xE2\x80\x93%s", names[6], names[0]);
    else
        for (int i = 1; i <= 7; i++) {
            if (!(days7 >> (i % 7) & 1)) continue;
            const size_t used = strlen(d);
            snprintf(d + used, sizeof(d) - used, "%s%s", used ? ", " : "", names[i % 7]);
        }
    const int n = snprintf(buf, size, "%s %02u:%02u\xE2\x80\x93%02u:%02u", d, h->from / 60u, h->from % 60u, h->to / 60u,
                           h->to % 60u);
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)n;
}

size_t gate_spans_text(const GateHours *spans, unsigned n, const char *days, char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    buf[0] = '\0';
    size_t used = 0;
    for (unsigned i = 0; spans && i < n; i++) {
        char one[200];
        if (!gate_hours_text(&spans[i], days, one, sizeof(one))) continue;
        const int k = snprintf(buf + used, size - used, "%s%s", used ? ", " : "", one);
        if (k < 0 || (size_t)k >= size - used) {
            buf[0] = '\0';
            return 0;
        }
        used += (size_t)k;
    }
    return used;
}

int gate_spans_until(const GateHours *spans, unsigned n, int wday, int minute)
{
    for (unsigned i = 0; spans && i < n; i++)
        if (gate_hours_free(&spans[i], wday, minute)) return spans[i].to % 1440;
    return -1;
}

size_t gate_minutes_text(uint64_t minutes, const char *min_fmt, const char *hm_fmt, char *buf, size_t size)
{
    if (!buf || size == 0 || !min_fmt || !hm_fmt) return 0;
    const int k = minutes < 60 ? snprintf(buf, size, min_fmt, (int)minutes)
                               : snprintf(buf, size, hm_fmt, (int)(minutes / 60 > 99999 ? 99999 : minutes / 60), (int)(minutes % 60));
    if (k < 0 || (size_t)k >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)k;
}

size_t gate_error_code(uint32_t rc, char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    const int k = snprintf(buf, size, "%04u-%04u", 2000u + (rc & 0x1FFu), (rc >> 9) & 0x1FFFu);
    if (k < 0 || (size_t)k >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)k;
}

// The pause file, line by line: `fn` with each line (without "\n", "\r").
typedef bool (*LineFn)(const char *line, size_t n, void *ctx);
static void each_line(const char *text, size_t len, LineFn fn, void *ctx)
{
    for (size_t at = 0; text && at < len;) {
        const char *line = text + at;
        size_t n = 0;
        while (at + n < len && line[n] != '\n') n++;
        at += n + 1;
        size_t m = n;
        if (m > 0 && line[m - 1] == '\r') m--;
        if (!fn(line, m, ctx)) return;
    }
}

// A decimal number up to `stop` (or the line's end when stop is 0).
static bool decimal(const char **p, const char *end, char stop, uint64_t max, uint64_t *out)
{
    uint64_t v = 0;
    const char *s = *p;
    if (s >= end || *s == stop) return false;
    for (; s < end && *s != stop; s++) {
        if (*s < '0' || *s > '9') return false;
        v = v * 10 + (uint64_t)(*s - '0');
        if (v > max) return false;
    }
    if (stop && s >= end) return false;
    *out = v;
    *p = s < end ? s + 1 : s;
    return true;
}

struct RuleCtx {
    GatePauseRule rule;
};
static bool rule_line(const char *line, size_t n, void *ctx)
{
    const char *p = line, *end = line + n;
    if (n < 5 || memcmp(line, "rule\t", 5) != 0) return true;
    p += 5;
    uint64_t tries, minutes;
    if (decimal(&p, end, '\t', 1000, &tries) && decimal(&p, end, 0, 24 * 60, &minutes) && p == end) {
        ((struct RuleCtx *)ctx)->rule.tries = (unsigned)tries;
        ((struct RuleCtx *)ctx)->rule.minutes = (unsigned)minutes;
    }
    return false;
}

GatePauseRule gate_pause_rule(const char *text, size_t len)
{
    struct RuleCtx c = { { GATE_PAUSE_TRIES, GATE_PAUSE_MINUTES } };
    each_line(text, len, rule_line, &c);
    if (c.rule.minutes == 0) c.rule.tries = 0;   // a pause of nothing: none
    return c.rule;
}

// A forwarder's line: its streak, its end, its .nro.
static bool entry_of(const char *line, size_t n, unsigned *streak, uint64_t *until, const char **nro, size_t *nro_len)
{
    const char *p = line, *end = line + n;
    uint64_t s, u;
    if (n >= 5 && memcmp(line, "rule\t", 5) == 0) return false;
    if (!decimal(&p, end, '\t', 1000000, &s) || !decimal(&p, end, '\t', UINT64_MAX / 10, &u) || p >= end) return false;
    *streak = (unsigned)s;
    *until = u;
    *nro = p;
    *nro_len = (size_t)(end - p);
    return true;
}

struct FindCtx {
    const char *nro;
    size_t      nro_len;
    bool        found;
    unsigned    streak;
    uint64_t    until;
};
static bool find_line(const char *line, size_t n, void *ctx)
{
    struct FindCtx *c = (struct FindCtx *)ctx;
    unsigned s;
    uint64_t u;
    const char *p;
    size_t pn;
    if (entry_of(line, n, &s, &u, &p, &pn) && pn == c->nro_len && memcmp(p, c->nro, pn) == 0) {
        c->found = true;
        c->streak = s;
        c->until = u;
        return false;
    }
    return true;
}

bool gate_pause_find(const char *text, size_t len, const char *nro, unsigned *streak, uint64_t *until)
{
    if (!nro) return false;
    struct FindCtx c = { nro, strlen(nro), false, 0, 0 };
    each_line(text, len, find_line, &c);
    if (streak) *streak = c.found ? c.streak : 0;
    if (until) *until = c.found ? c.until : 0;
    return c.found;
}

// Writes into an output buffer, tracking overflow.
struct Out {
    char  *buf;
    size_t size, n;
    bool   over;
};
static void put(struct Out *o, const char *s, size_t n)
{
    if (o->over || o->n + n >= o->size) {
        o->over = true;
        return;
    }
    memcpy(o->buf + o->n, s, n);
    o->n += n;
    o->buf[o->n] = '\0';
}

struct CopyCtx {
    struct Out *out;
    const char *skip;         // the .nro whose line goes (NULL: none)
    size_t      skip_len;
    bool        rule;         // copy the rule line
};
static bool copy_line(const char *line, size_t n, void *ctx)
{
    struct CopyCtx *c = (struct CopyCtx *)ctx;
    unsigned s;
    uint64_t u;
    const char *p;
    size_t pn;
    if (n >= 5 && memcmp(line, "rule\t", 5) == 0) {
        // Only one that reads as a rule.
        struct RuleCtx r = { { 1001, 0 } };
        rule_line(line, n, &r);
        if (!c->rule || r.rule.tries > 1000) return true;
    } else if (!entry_of(line, n, &s, &u, &p, &pn)) {
        return true;   // nothing PlayGuard or the gate wrote
    } else if (c->skip && pn == c->skip_len && memcmp(p, c->skip, pn) == 0) {
        return true;
    }
    put(c->out, line, n);
    put(c->out, "\n", 1);
    return true;
}

static size_t finish(struct Out *o)
{
    if (o->over) {
        if (o->size) o->buf[0] = '\0';
        return 0;
    }
    return o->n;
}

size_t gate_pause_update(const char *text, size_t len, const char *nro, GateResult r, uint64_t now, char *out,
                         size_t size)
{
    return gate_pause_update_with(text, len, nro, r, now, NULL, out, size);
}

size_t gate_pause_update_with(const char *text, size_t len, const char *nro, GateResult r, uint64_t now,
                              const GatePauseRule *carried, char *out, size_t size)
{
    if (!out || size == 0 || !nro || !*nro || strpbrk(nro, "\t\r\n")) return 0;
    out[0] = '\0';
    struct Out o = { out, size, 0, false };
    // Opened: its line goes; refused: written again below; anything else
    // (paused, closed, the PIN screen not opened): as it was.
    const bool rewrite = r == GateResult_Pin || r == GateResult_Free || r == GateResult_Refused;
    struct CopyCtx c = { &o, rewrite ? nro : NULL, rewrite ? strlen(nro) : 0, true };
    each_line(text, len, copy_line, &c);
    if (r == GateResult_Refused) {
        const GatePauseRule rule = carried ? *carried : gate_pause_rule(text, len);
        unsigned streak = 0;
        uint64_t until = 0;
        gate_pause_find(text, len, nro, &streak, &until);
        // A pause over: counted again from nothing.
        if (until && until <= now) streak = 0, until = 0;
        streak++;
        if (rule.tries && streak >= rule.tries) {
            until = now + (uint64_t)rule.minutes * 60u;
            streak = 0;
        }
        char line[64];
        const int n = snprintf(line, sizeof(line), "%u\t%llu\t", streak, (unsigned long long)until);
        if (n > 0) put(&o, line, (size_t)n);
        put(&o, nro, strlen(nro));
        put(&o, "\n", 1);
    }
    return finish(&o);
}

size_t gate_pause_edit(const char *text, size_t len, const GatePauseRule *rule, const char *nro, char *out, size_t size)
{
    if (!out || size == 0) return 0;
    out[0] = '\0';
    struct Out o = { out, size, 0, false };
    if (rule) {
        char line[64];
        const int n = snprintf(line, sizeof(line), "rule\t%u\t%u\n", rule->tries, rule->minutes);
        if (n > 0) put(&o, line, (size_t)n);
    }
    struct CopyCtx c = { &o, nro, nro ? strlen(nro) : 0, rule == NULL };
    each_line(text, len, copy_line, &c);
    return finish(&o);
}

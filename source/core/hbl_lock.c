// hbl_lock — override_config.ini locked and unlocked (see hbl_lock.h).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "hbl_lock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The first words of each block's first line, and of its last.
#define LOCK_BEGIN     "; >>> PlayGuard: the homebrew menu is locked"
#define DEFAULTS_BEGIN "; >>> PlayGuard: Atmosphere's defaults"
#define BLOCK_BEGIN    "; >>> PlayGuard"
#define BLOCK_END      "; <<< PlayGuard"

// ASCII only, short lines: Atmosphère's ini reader takes them as they are.
static const char LOCK_BLOCK[] =
    LOCK_BEGIN " (PlayGuard: Console > Forwarders).\n"
    "; Atmosphere reads it last: neither the album nor a game started with R\n"
    "; opens the homebrew menu. To unlock without PlayGuard: remove this block\n"
    "; (to its end line) or the whole file, then restart the console.\n"
    "[hbl_config]\n"
    "program_id_0=0000000000000000\n"
    "program_id_1=0000000000000000\n"
    "program_id_2=0000000000000000\n"
    "program_id_3=0000000000000000\n"
    "program_id_4=0000000000000000\n"
    "program_id_5=0000000000000000\n"
    "program_id_6=0000000000000000\n"
    "program_id_7=0000000000000000\n"
    "override_any_app=false\n"
    BLOCK_END "\n";

static const char DEFAULTS_HEAD[] =
    DEFAULTS_BEGIN ", written back when the homebrew menu\n"
    "; was unlocked (it keeps a key it read until the console restarts).\n"
    "; Nothing changes without it once the console has restarted.\n"
    "[hbl_config]\n";
static const char DEFAULT_ALBUM[] = "program_id_0=010000000000100D\n";
static const char DEFAULT_ANY_APP[] = "override_any_app=true\n";

typedef struct {
    char  *data;
    size_t size, len;
    bool   full;
} Out;

static void put(Out *o, const char *s, size_t n)
{
    if (o->full || o->len + n + 1 > o->size) {
        o->full = true;
        return;
    }
    memcpy(o->data + o->len, s, n);
    o->len += n;
    o->data[o->len] = '\0';
}

static void puts_(Out *o, const char *s)
{
    put(o, s, strlen(s));
}

static bool starts(const char *line, size_t n, const char *prefix)
{
    const size_t p = strlen(prefix);
    return n >= p && memcmp(line, prefix, p) == 0;
}

static char lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static bool same_word(const char *a, size_t n, const char *b)
{
    if (strlen(b) != n) return false;
    for (size_t i = 0; i < n; i++)
        if (lower(a[i]) != lower(b[i])) return false;
    return true;
}

static bool blank(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

// One line of `ini` from `at`: its start, its length without "\n", the next one.
static size_t line_at(const char *ini, size_t len, size_t at, const char **line, size_t *n)
{
    size_t end = at;
    while (end < len && ini[end] != '\n') end++;
    *line = ini + at;
    *n = end - at;
    return end < len ? end + 1 : end;
}

// A line without its leading spaces.
static void trim_start(const char **line, size_t *n)
{
    while (*n > 0 && blank(**line)) {
        (*line)++;
        (*n)--;
    }
}

// Whether a block of `which` starts on this line (BLOCK_BEGIN: any of PlayGuard's).
static bool block_start(const char *line, size_t n, const char *which)
{
    trim_start(&line, &n);
    return starts(line, n, which);
}

static bool block_end(const char *line, size_t n)
{
    trim_start(&line, &n);
    return starts(line, n, BLOCK_END);
}

static bool empty_line(const char *line, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (!blank(line[i])) return false;
    return true;
}

static bool has_block(const char *ini, size_t len, const char *which)
{
    if (!ini) return false;
    for (size_t at = 0; at < len;) {
        const char *line;
        size_t n;
        at = line_at(ini, len, at, &line, &n);
        if (block_start(line, n, which)) return true;
    }
    return false;
}

bool hbl_lock_locked(const char *ini, size_t len)
{
    return has_block(ini, len, LOCK_BEGIN);
}

bool hbl_lock_has_defaults(const char *ini, size_t len)
{
    return has_block(ini, len, DEFAULTS_BEGIN);
}

// `ini` without PlayGuard's blocks (`which`: BLOCK_BEGIN for all, or one
// kind), and without the empty line put before each.
static void strip(const char *ini, size_t len, const char *which, Out *o)
{
    const char *held = NULL;   // an empty line, kept until the next line says whether it goes
    size_t held_n = 0;
    bool in_block = false;
    for (size_t at = 0; at < len;) {
        const char *line;
        size_t n;
        const size_t next = line_at(ini, len, at, &line, &n);
        const bool newline = next > at + n;
        at = next;
        if (in_block) {
            if (block_end(line, n)) in_block = false;
            continue;
        }
        if (block_start(line, n, which)) {
            held = NULL;
            in_block = !block_end(line, n);
            continue;
        }
        if (held) {
            put(o, held, held_n);
            held = NULL;
        }
        if (empty_line(line, n) && newline) {
            held = line;
            held_n = n + 1;
            continue;
        }
        put(o, line, n + (newline ? 1 : 0));
    }
    if (held) put(o, held, held_n);
}

// Which of the keys the lock sets the file sets itself, in [hbl_config].
static void keys_set(const char *ini, size_t len, bool *album, bool *any_app)
{
    *album = *any_app = false;
    bool hbl = false;
    for (size_t at = 0; at < len;) {
        const char *line;
        size_t n;
        at = line_at(ini, len, at, &line, &n);
        trim_start(&line, &n);
        if (n == 0 || line[0] == ';' || line[0] == '#') continue;
        if (line[0] == '[') {
            size_t e = 1;
            while (e < n && line[e] != ']') e++;
            const char *name = line + 1;
            size_t k = e - 1;
            while (k > 0 && blank(*name)) name++, k--;
            while (k > 0 && blank(name[k - 1])) k--;
            hbl = same_word(name, k, "hbl_config");
            continue;
        }
        if (!hbl) continue;
        size_t eq = 0;
        while (eq < n && line[eq] != '=') eq++;
        if (eq == n) continue;
        size_t k = eq;
        while (k > 0 && blank(line[k - 1])) k--;
        if (same_word(line, k, "program_id") || same_word(line, k, "program_id_0")) *album = true;
        if (same_word(line, k, "override_any_app")) *any_app = true;
    }
}

bool hbl_lock_apply(const char *ini, size_t len, HblLockOp op, char *out, size_t size, size_t *out_len)
{
    if (out_len) *out_len = 0;
    if (!out || size == 0) return false;
    out[0] = '\0';
    if (!ini) len = 0;
    Out o = { out, size, 0, false };
    strip(ini ? ini : "", len, op == HblLock_DropDefaults ? DEFAULTS_BEGIN : BLOCK_BEGIN, &o);
    if (o.full) {
        out[0] = '\0';
        return false;
    }

    bool album = true, any_app = true;
    if (op == HblLock_Unlock) keys_set(out, o.len, &album, &any_app);
    const bool block = op == HblLock_Lock || (op == HblLock_Unlock && (!album || !any_app));
    if (block) {
        // On a line of its own, after an empty one.
        if (o.len > 0 && out[o.len - 1] != '\n') puts_(&o, "\n");
        if (o.len > 0) puts_(&o, "\n");
        if (op == HblLock_Lock) {
            puts_(&o, LOCK_BLOCK);
        } else {
            puts_(&o, DEFAULTS_HEAD);
            if (!album) puts_(&o, DEFAULT_ALBUM);
            if (!any_app) puts_(&o, DEFAULT_ANY_APP);
            puts_(&o, BLOCK_END "\n");
        }
    }
    if (o.full) {
        out[0] = '\0';
        return false;
    }
    if (out_len) *out_len = o.len;
    return true;
}

void hbl_lock_effective(const char *ini, size_t len, bool *album, bool *any_app)
{
    static const unsigned long long ALBUM = 0x010000000000100DULL;
    unsigned long long slots[8] = { ALBUM, 0, 0, 0, 0, 0, 0, 0 };
    bool any = true, hbl = false;
    for (size_t at = 0; ini && at < len;) {
        const char *line;
        size_t n;
        at = line_at(ini, len, at, &line, &n);
        trim_start(&line, &n);
        while (n > 0 && blank(line[n - 1])) n--;
        if (n == 0 || line[0] == ';' || line[0] == '#') continue;
        if (line[0] == '[') {
            size_t e = 1;
            while (e < n && line[e] != ']') e++;
            const char *name = line + 1;
            size_t k = e - 1;
            while (k > 0 && blank(*name)) name++, k--;
            while (k > 0 && blank(name[k - 1])) k--;
            hbl = same_word(name, k, "hbl_config");
            continue;
        }
        if (!hbl) continue;
        size_t eq = 0;
        while (eq < n && line[eq] != '=') eq++;
        if (eq == n) continue;
        size_t k = eq;
        while (k > 0 && blank(line[k - 1])) k--;
        const char *v = line + eq + 1;
        size_t vn = n - eq - 1;
        while (vn > 0 && blank(*v)) v++, vn--;
        // An inline comment ends the value, as inih reads it.
        for (size_t i = 0; i < vn; i++)
            if (v[i] == ';' && (i == 0 || blank(v[i - 1]))) {
                vn = i;
                break;
            }
        while (vn > 0 && blank(v[vn - 1])) vn--;
        char value[32];
        if (vn >= sizeof(value)) continue;
        memcpy(value, v, vn);
        value[vn] = '\0';
        int slot = -1;
        if (same_word(line, k, "program_id") || same_word(line, k, "program_id_0")) slot = 0;
        for (int i = 1; i < 8; i++) {
            char key[16];
            snprintf(key, sizeof(key), "program_id_%d", i);
            if (same_word(line, k, key)) slot = i;
        }
        if (slot >= 0) slots[slot] = strtoull(value, NULL, 16);
        if (same_word(line, k, "override_any_app")) {
            if (same_word(value, vn, "true") || same_word(value, vn, "1")) any = true;
            else if (same_word(value, vn, "false") || same_word(value, vn, "0")) any = false;
        }
    }
    bool open = false;
    for (int i = 0; i < 8; i++) open = open || slots[i] == ALBUM;
    if (album) *album = open;
    if (any_app) *any_app = any;
}

// ---------------------------------------------------------------- state

static bool parse_version(const char *s, size_t n, uint8_t v[3])
{
    unsigned part = 0, value = 0;
    bool digit = false;
    for (size_t i = 0; i <= n; i++) {
        const char c = i < n ? s[i] : '.';
        if (c >= '0' && c <= '9') {
            value = value * 10 + (unsigned)(c - '0');
            if (value > 255) return false;
            digit = true;
        } else if (c == '.' && digit && part < 3) {
            v[part++] = (uint8_t)value;
            value = 0;
            digit = false;
        } else {
            return false;
        }
    }
    return part == 3;
}

// The value of `key=` among the lines of `text` (spaces and "\r" around it
// trimmed); false when it is not there.
static bool value_of(const char *text, size_t len, const char *key, const char **v, size_t *vn)
{
    const size_t k = strlen(key);
    for (size_t at = 0; at < len;) {
        const char *line;
        size_t n;
        at = line_at(text, len, at, &line, &n);
        trim_start(&line, &n);
        while (n > 0 && blank(line[n - 1])) n--;
        if (n > k && memcmp(line, key, k) == 0 && line[k] == '=') {
            *v = line + k + 1;
            *vn = n - k - 1;
            return true;
        }
    }
    return false;
}

size_t hbl_lock_state_format(uint8_t major, uint8_t minor, uint8_t micro, char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    const int n = snprintf(buf, size, "ams=%u.%u.%u\n", major, minor, micro);
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)n;
}

size_t hbl_lock_state_format_unknown(char *buf, size_t size)
{
    if (!buf || size == 0) return 0;
    const int n = snprintf(buf, size, "ams=unknown\n");
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)n;
}

bool hbl_lock_state_parse(const char *text, size_t len, uint8_t *major, uint8_t *minor, uint8_t *micro)
{
    const char *v;
    size_t vn;
    uint8_t ver[3];
    if (!text || !value_of(text, len, "ams", &v, &vn) || !parse_version(v, vn, ver)) return false;
    *major = ver[0];
    *minor = ver[1];
    *micro = ver[2];
    return true;
}

// ---------------------------------------------------------------- report

bool hbl_lock_boot_lift(bool rescue, bool state, const uint8_t locked_on[3], bool now, const uint8_t ams[3],
                        HblLockReport *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (rescue) {
        out->reason = HblLockReason_Rescue;
        return true;
    }
    if (!state || !now || !locked_on || !ams) return false;
    if (locked_on[0] == ams[0] && locked_on[1] == ams[1] && locked_on[2] == ams[2]) return false;
    out->reason = HblLockReason_Atmosphere;
    snprintf(out->from, sizeof(out->from), "%u.%u.%u", locked_on[0], locked_on[1], locked_on[2]);
    snprintf(out->to, sizeof(out->to), "%u.%u.%u", ams[0], ams[1], ams[2]);
    return true;
}

size_t hbl_lock_report_format(const HblLockReport *r, char *buf, size_t size)
{
    if (!r || !buf || size == 0) return 0;
    int n;
    if (r->reason == HblLockReason_Atmosphere)
        n = snprintf(buf, size, "reason=atmosphere\nresult=%s\nfrom=%s\nto=%s\n", r->ok ? "ok" : "failed", r->from,
                     r->to);
    else
        n = snprintf(buf, size, "reason=rescue\nresult=%s\n", r->ok ? "ok" : "failed");
    if (n < 0 || (size_t)n >= size) {
        buf[0] = '\0';
        return 0;
    }
    return (size_t)n;
}

static void copy_value(char *dst, size_t size, const char *v, size_t vn)
{
    if (vn >= size) vn = size - 1;
    memcpy(dst, v, vn);
    dst[vn] = '\0';
}

bool hbl_lock_report_parse(const char *text, size_t len, HblLockReport *out)
{
    if (!text || !out) return false;
    HblLockReport r;
    memset(&r, 0, sizeof(r));
    const char *v;
    size_t vn;
    if (!value_of(text, len, "reason", &v, &vn)) return false;
    if (same_word(v, vn, "atmosphere")) r.reason = HblLockReason_Atmosphere;
    else if (same_word(v, vn, "rescue")) r.reason = HblLockReason_Rescue;
    else return false;
    if (!value_of(text, len, "result", &v, &vn)) return false;
    if (same_word(v, vn, "ok")) r.ok = true;
    else if (!same_word(v, vn, "failed")) return false;
    if (value_of(text, len, "from", &v, &vn)) copy_value(r.from, sizeof(r.from), v, vn);
    if (value_of(text, len, "to", &v, &vn)) copy_value(r.to, sizeof(r.to), v, vn);
    *out = r;
    return true;
}

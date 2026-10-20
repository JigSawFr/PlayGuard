// Host tests for source/core/hbl_lock.c: override_config.ini locked and
// unlocked, read as Atmosphère's loader reads it (cfg_override: the file
// again at every program start, a key it read kept until the console
// restarts), so each step is checked by what the console would do: whether
// the album and R on a game open the homebrew menu. The rest of the file is
// kept byte for byte; the state and report files round-trip.
#include "check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "hbl_lock.h"

#define ALBUM 0x010000000000100DULL

// What Atmosphère's loader holds: from boot, then each file read on top.
typedef struct {
    unsigned long long program_id[8];
    int any_app;
} Hbl;

static Hbl boot(void)
{
    Hbl h;
    memset(&h, 0, sizeof(h));
    h.program_id[0] = ALBUM;
    h.any_app = 1;
    return h;
}

// cfg_override's OverrideConfigIniHandler, for the keys the lock touches:
// a key read sets its value, nothing else is reset.
static void read_ini(Hbl *h, const char *ini)
{
    char section[64] = "";
    const char *p = ini;
    while (*p) {
        const char *end = strchr(p, '\n');
        size_t n = end ? (size_t)(end - p) : strlen(p);
        char line[256];
        if (n >= sizeof(line)) n = sizeof(line) - 1;
        memcpy(line, p, n);
        line[n] = '\0';
        p += n + (end ? 1 : 0);
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        size_t l = strlen(s);
        while (l > 0 && (s[l - 1] == '\r' || s[l - 1] == ' ')) s[--l] = '\0';
        if (*s == ';' || *s == '#' || *s == '\0') continue;
        if (*s == '[') {
            char *close = strchr(s, ']');
            if (close) *close = '\0';
            snprintf(section, sizeof(section), "%s", s + 1);
            continue;
        }
        char *eq = strchr(s, '=');
        if (!eq || strcasecmp(section, "hbl_config") != 0) continue;
        // inih's own trimming: spaces around the name and the value.
        *eq = '\0';
        for (char *k = eq - 1; k >= s && (*k == ' ' || *k == '\t'); k--) *k = '\0';
        const char *key = s, *value = eq + 1;
        while (*value == ' ' || *value == '\t') value++;
        if (strcasecmp(key, "program_id") == 0 || strcasecmp(key, "program_id_0") == 0)
            h->program_id[0] = strtoull(value, NULL, 16);
        for (int i = 1; i < 8; i++) {
            char name[16];
            snprintf(name, sizeof(name), "program_id_%d", i);
            if (strcasecmp(key, name) == 0) h->program_id[i] = strtoull(value, NULL, 16);
        }
        if (strcasecmp(key, "override_any_app") == 0) {
            if (strcasecmp(value, "true") == 0) h->any_app = 1;
            else if (strcasecmp(value, "false") == 0) h->any_app = 0;
        }
    }
}

// Whether the album opens the homebrew menu.
static int album_opens(const Hbl *h)
{
    for (int i = 0; i < 8; i++)
        if (h->program_id[i] == ALBUM) return 1;
    return 0;
}

static char out[HBL_LOCK_MAX_FILE];

static const char *apply(const char *in, HblLockOp op)
{
    size_t n = 0;
    if (!hbl_lock_apply(in, in ? strlen(in) : 0, op, out, sizeof(out), &n)) return NULL;
    CHECK(n == strlen(out));
    return out;
}

static char *copy(const char *s)
{
    char *c = (char *)malloc(strlen(s) + 1);
    strcpy(c, s);
    return c;
}

static void test_no_file(void)
{
    // No file: Atmosphère's defaults, the menu open.
    Hbl h = boot();
    CHECK(album_opens(&h) && h.any_app);
    char *locked = copy(apply(NULL, HblLock_Lock));
    CHECK(hbl_lock_locked(locked, strlen(locked)));
    read_ini(&h, locked);
    CHECK(!album_opens(&h) && !h.any_app);
    // Locking again changes nothing.
    CHECK(strcmp(apply(locked, HblLock_Lock), locked) == 0);

    // Unlocked in the same boot: the defaults written back, the menu open at once.
    char *open = copy(apply(locked, HblLock_Unlock));
    CHECK(!hbl_lock_locked(open, strlen(open)) && hbl_lock_has_defaults(open, strlen(open)));
    read_ini(&h, open);
    CHECK(album_opens(&h) && h.any_app);
    // Without them it would not be: the keys read during the lock stay.
    Hbl stale = boot();
    read_ini(&stale, locked);
    read_ini(&stale, "");
    CHECK(!album_opens(&stale));

    // At boot the sysmodule takes the defaults away: nothing left.
    CHECK(strcmp(apply(open, HblLock_DropDefaults), "") == 0);
    // Locked again from there: the defaults go with it.
    const char *again = apply(open, HblLock_Lock);
    CHECK(strcmp(again, locked) == 0 && !hbl_lock_has_defaults(again, strlen(again)));
    free(locked);
    free(open);
}

static void test_user_file(void)
{
    // A file of the user's: a game as the homebrew title, R on others off,
    // its own sections and comments, a last line without its newline.
    static const char USER[] =
        "; mine\r\n"
        "[hbl_config]\r\n"
        "program_id_0=0100000000010000\r\n"
        "override_key_0=R\r\n"
        "override_any_app = false\r\n"
        "\r\n"
        "[default_config]\r\n"
        "override_key=!L";
    Hbl h = boot();
    read_ini(&h, USER);
    CHECK(h.program_id[0] == 0x0100000000010000ULL && !h.any_app);

    char *locked = copy(apply(USER, HblLock_Lock));
    CHECK(strncmp(locked, USER, strlen(USER)) == 0);   // kept as it was, the block after it
    CHECK(locked[strlen(USER)] == '\n');
    read_ini(&h, locked);
    for (int i = 0; i < 8; i++) CHECK(h.program_id[i] == 0);

    // Unlocked: its own values again, and no defaults block (it sets both keys).
    const char *open = apply(locked, HblLock_Unlock);
    CHECK(!hbl_lock_has_defaults(open, strlen(open)));
    CHECK(strncmp(open, USER, strlen(USER)) == 0 && strlen(open) == strlen(USER) + 1);
    read_ini(&h, open);
    CHECK(h.program_id[0] == 0x0100000000010000ULL && !h.any_app);
    free(locked);

    // A file that sets only the album: the any-app default written back alone.
    static const char ALBUM_ONLY[] = "[hbl_config]\nprogram_id=010000000000100D\n";
    char *l2 = copy(apply(ALBUM_ONLY, HblLock_Lock));
    h = boot();
    read_ini(&h, l2);
    CHECK(!album_opens(&h) && !h.any_app);
    const char *o2 = apply(l2, HblLock_Unlock);
    CHECK(strstr(o2, "override_any_app=true") && !strstr(o2, "program_id_0=010000000000100D"));
    read_ini(&h, o2);
    CHECK(album_opens(&h) && h.any_app);
    free(l2);

    // A key of another section does not count.
    static const char OTHER[] = "[default_config]\noverride_any_app=true\n";
    const char *o3 = apply(OTHER, HblLock_Unlock);
    CHECK(strstr(o3, "override_any_app=true\n; <<<") != NULL);
}

static void test_blocks(void)
{
    // A file with no block of PlayGuard's: unchanged by an unlock (beyond
    // the defaults it does not set) and by a drop.
    static const char PLAIN[] = "[hbl_config]\nprogram_id_0=010000000000100D\noverride_any_app=true\n";
    CHECK(strcmp(apply(PLAIN, HblLock_Unlock), PLAIN) == 0);
    CHECK(strcmp(apply(PLAIN, HblLock_DropDefaults), PLAIN) == 0);
    CHECK(!hbl_lock_locked(PLAIN, strlen(PLAIN)) && !hbl_lock_locked(NULL, 0));

    // Lines added by hand after the block stay; a block without its end
    // line runs to the end of the file.
    char *locked = copy(apply(PLAIN, HblLock_Lock));
    char edited[4096];
    snprintf(edited, sizeof(edited), "%s[default_config]\ncheat_enable_key=!L\n", locked);
    const char *open = apply(edited, HblLock_Unlock);
    CHECK(strcmp(open, "[hbl_config]\nprogram_id_0=010000000000100D\noverride_any_app=true\n"
                       "[default_config]\ncheat_enable_key=!L\n") == 0);
    char cut[4096];
    snprintf(cut, sizeof(cut), "%s", locked);
    *strstr(cut, "; <<< PlayGuard") = '\0';
    CHECK(strcmp(apply(cut, HblLock_Unlock), PLAIN) == 0);
    // Indented markers count too; the drop leaves a lock block alone.
    CHECK(hbl_lock_locked("  ; >>> PlayGuard: the homebrew menu is locked\n", 47));
    CHECK(strcmp(apply(locked, HblLock_DropDefaults), locked) == 0);
    free(locked);

    // Too small a buffer: refused, out empty.
    char small[16];
    size_t n = 7;
    CHECK(!hbl_lock_apply(PLAIN, strlen(PLAIN), HblLock_Lock, small, sizeof(small), &n) && n == 0 && small[0] == 0);
    CHECK(!hbl_lock_apply(PLAIN, strlen(PLAIN), HblLock_Lock, NULL, 0, &n));
}

static void test_state_report(void)
{
    char buf[64];
    CHECK(hbl_lock_state_format(1, 9, 4, buf, sizeof(buf)) && strcmp(buf, "ams=1.9.4\n") == 0);
    uint8_t a = 0, b = 0, c = 0;
    CHECK(hbl_lock_state_parse(buf, strlen(buf), &a, &b, &c) && a == 1 && b == 9 && c == 4);
    CHECK(hbl_lock_state_parse(" ams=10.0.255 \r\n", 16, &a, &b, &c) && a == 10 && b == 0 && c == 255);
    CHECK(!hbl_lock_state_parse("ams=1.9\n", 8, &a, &b, &c));
    CHECK(!hbl_lock_state_parse("ams=1.9.256\n", 12, &a, &b, &c));
    CHECK(!hbl_lock_state_parse("ams=1..4\n", 9, &a, &b, &c));
    CHECK(!hbl_lock_state_parse("", 0, &a, &b, &c) && a == 10);
    CHECK(!hbl_lock_state_format(1, 2, 3, buf, 4));

    HblLockReport r;
    memset(&r, 0, sizeof(r));
    r.reason = HblLockReason_Atmosphere;
    r.ok = true;
    strcpy(r.from, "1.9.4");
    strcpy(r.to, "1.10.0");
    char text[128];
    CHECK(hbl_lock_report_format(&r, text, sizeof(text)));
    CHECK(strcmp(text, "reason=atmosphere\nresult=ok\nfrom=1.9.4\nto=1.10.0\n") == 0);
    HblLockReport back;
    CHECK(hbl_lock_report_parse(text, strlen(text), &back));
    CHECK(back.reason == HblLockReason_Atmosphere && back.ok && strcmp(back.from, "1.9.4") == 0 &&
          strcmp(back.to, "1.10.0") == 0);
    CHECK(hbl_lock_report_parse("result=failed\r\nreason=rescue\r\nextra=1\r\n", 39, &back));
    CHECK(back.reason == HblLockReason_Rescue && !back.ok && back.from[0] == 0);
    r.reason = HblLockReason_Rescue;
    CHECK(hbl_lock_report_format(&r, text, sizeof(text)) && strcmp(text, "reason=rescue\nresult=ok\n") == 0);
    CHECK(!hbl_lock_report_parse("reason=other\nresult=ok\n", 23, &back));
    CHECK(!hbl_lock_report_parse("reason=rescue\nresult=maybe\n", 27, &back));
    CHECK(!hbl_lock_report_parse("result=ok\n", 10, &back));
    CHECK(!hbl_lock_report_format(&r, text, 8));
}

static void test_boot_lift(void)
{
    const uint8_t v194[3] = { 1, 9, 4 }, v195[3] = { 1, 9, 5 };
    HblLockReport r;
    // A RESCUE request: lifted, whatever the versions say.
    CHECK(hbl_lock_boot_lift(true, false, NULL, false, NULL, &r) && r.reason == HblLockReason_Rescue && !r.ok);
    // Atmosphère updated since the lock: lifted, both versions told.
    CHECK(hbl_lock_boot_lift(false, true, v194, true, v195, &r) && r.reason == HblLockReason_Atmosphere);
    CHECK(strcmp(r.from, "1.9.4") == 0 && strcmp(r.to, "1.9.5") == 0);
    // The same version, or one of them unknown: left locked.
    CHECK(!hbl_lock_boot_lift(false, true, v194, true, v194, &r));
    CHECK(!hbl_lock_boot_lift(false, false, v194, true, v195, &r));
    CHECK(!hbl_lock_boot_lift(false, true, v194, false, v195, &r));
    CHECK(!hbl_lock_boot_lift(false, true, v194, true, v195, NULL));
}

int main(void)
{
    test_no_file();
    test_user_file();
    test_blocks();
    test_state_report();
    test_boot_lift();
    return CHECK_DONE("hbl_lock lock, unlock, defaults, kept lines, state, report and boot assertions passed");
}

// playguard-rescue — a tiny boot sysmodule: the "last chance" for a parent who
// forgot the parental-control PIN while the play timer keeps every program
// (PlayGuard included) from starting.
//
// At boot it looks for sd:/config/playguard/RESCUE (see core/rescue.h). If it
// is there it removes it (used once; renamed RESCUE.done should that fail),
// then, with the PIN the console stores:
//   - unlocks parental controls temporarily (1208 reads the PIN, 1201 unlocks)
//     so the HOME menu and PlayGuard can start;
//   - or, when a line of the file reads "delete", deletes every parental
//     control (1043) — only once the request is gone, so it never repeats.
// It leaves the outcome in rescue_report.txt for PlayGuard, then exits — it
// holds no service and no memory for the rest of the session. An unlock's
// report carries a proof (core/rescue.h, RescueProof): the challenge PlayGuard
// left in rescue_nonce, and an HMAC-SHA256 of the report keyed with the stored
// PIN and a secret of the console that is not on the SD card (its Mii author
// ID), which PlayGuard checks before it opens the recovery actions.
//
// At every boot it also removes atmosphere/contents/<id>/restart_pending,
// which PlayGuard leaves when it updates the module: the bell says to restart
// the console until the new module has run.
//
// And the homebrew menu PlayGuard locked (core/hbl_lock.h), a way out of it
// for the parent: lifted at boot when a RESCUE request is there, or when
// Atmosphère was updated since it was locked (its signature patches may lag,
// and no forwarder starts without them), both things a child cannot do from
// the console. Deleting a forwarder, which a child can, lifts nothing. The
// outcome goes to hbmenu_lock_report.txt for PlayGuard. The block of
// defaults an unlock leaves is taken away at the next boot, when Atmosphère
// starts from its own defaults again. A file without PlayGuard's blocks is
// never written.
//
// This needs no applet and no screen, which is why it can run while everything
// else is blocked. It only reads a PIN the console already stores; it never
// writes one anywhere (only the MAC made with it and the console's secret,
// which does not give it back). The pctl command IDs and
// the buffer details (HIPC pointer buffers, the NUL-terminated PIN) are the
// app's, proven on hardware: the PIN read and the unlock come from
// source/core/pctl_ops_internal.h.
//
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <switch.h>

#include "hbl_lock.h"
#include "pctl_ops_internal.h"
#include "rescue.h"

#define DATA_DIR    "sdmc:/config/playguard"
#define PATH_REQ    DATA_DIR "/" RESCUE_REQUEST_NAME
#define PATH_REQTXT DATA_DIR "/" RESCUE_REQUEST_NAME_TXT
#define PATH_DONE   DATA_DIR "/" RESCUE_REQUEST_NAME_DONE
#define PATH_REPORT DATA_DIR "/" RESCUE_REPORT_NAME
#define PATH_NONCE  DATA_DIR "/" RESCUE_NONCE_NAME
#define PATH_HBL_CONFIG "sdmc:/" HBL_LOCK_CONFIG_PATH
#define PATH_HBL_STATE  DATA_DIR "/" HBL_LOCK_STATE_NAME
#define PATH_HBL_REPORT DATA_DIR "/" HBL_LOCK_REPORT_NAME

// A sysmodule, not an application: no applet, one fs session, a small heap.
u32 __nx_applet_type = AppletType_None;
u32 __nx_fs_num_sessions = 1;

#define INNER_HEAP_SIZE 0x40000
char g_heap[INNER_HEAP_SIZE];

void __libnx_initheap(void)
{
    extern char* fake_heap_start;
    extern char* fake_heap_end;
    fake_heap_start = g_heap;
    fake_heap_end   = g_heap + sizeof(g_heap);
}

// Whether sdmc: is mounted. Not an abort when it is not: without the SD card
// there is no request to read and no report to leave, so main() does nothing.
static bool s_sd_mounted;

void __appInit(void)
{
    Result rc = smInitialize();
    if (R_FAILED(rc)) diagAbortWithResult(rc);
    // The firmware version, as libnx's own start-up sets it for a program
    // that keeps the default __appInit: pctlInitialize picks the command
    // that opens its session by it (CreateService before 4.0.0). Not an
    // abort when it cannot be read: libnx then takes the version as 0.
    if (hosversionGet() == 0 && R_SUCCEEDED(setsysInitialize())) {
        SetSysFirmwareVersion fw;
        if (R_SUCCEEDED(setsysGetFirmwareVersion(&fw)))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }
    rc = fsInitialize();
    if (R_FAILED(rc)) diagAbortWithResult(rc);
    s_sd_mounted = R_SUCCEEDED(fsdevMountSdmc());
}

void __appExit(void)
{
    fsdevUnmountAll();
    fsExit();
    smExit();
}

// ---- the pctl commands this module needs (core/pctl_ops_internal.h) ----

// 1208 GetPinCode into `pin` (PIN_BUF bytes, always NUL-terminated), its
// length in *n (rescue_pin_length). The caller wipes `pin`.
static Result read_stored_pin(Service* srv, char* pin, size_t* n)
{
    *n = 0;
    u32 len = 0;
    Result rc = read_pin(srv, pin, &len);
    if (R_SUCCEEDED(rc)) *n = rescue_pin_length(pin, PIN_BUF, len);
    secure_zero(&len, sizeof(len));
    return rc;
}

// 1208 reads the PIN; 1201 then unlocks with it. The PIN stays in this one
// buffer, wiped before returning; it is never written to the SD card.
static Result pctl_unlock_with_stored_pin(Service* srv)
{
    char pin[PIN_BUF];
    size_t n = 0;
    Result rc = read_stored_pin(srv, pin, &n);
    if (R_SUCCEEDED(rc)) rc = unlock_with_pin(srv, pin, n + 1);   // NUL-terminated, like the app
    secure_zero(pin, sizeof(pin));
    return rc;
}

// The console's secret in the proof's key (core/rescue.h, RESCUE_SECRET_SIZE):
// set:sys GetMiiAuthorId, which the app reads too. False when it cannot be read.
static bool read_secret(u8* out)
{
    if (R_FAILED(setsysInitialize())) return false;
    Uuid id;
    const bool ok = R_SUCCEEDED(setsysGetMiiAuthorId(&id));
    setsysExit();
    if (ok) memcpy(out, id.uuid, RESCUE_SECRET_SIZE);
    secure_zero(&id, sizeof(id));
    return ok;
}

// The report's proof (core/rescue.h): rescue_proof_mac of rescue_mac_message,
// keyed with the stored PIN, read again (1208) here and wiped, and the
// console's secret. False, and *out left absent, when either cannot be read
// or there is no PIN.
static bool sign_report(Service* srv, const RescueReport* r, const u8* nonce, RescueProof* out)
{
    static char msg[256];
    const size_t m = rescue_mac_message(r, nonce, msg, sizeof(msg));
    if (m == 0) return false;
    u8 secret[RESCUE_SECRET_SIZE];
    if (!read_secret(secret)) return false;
    char pin[PIN_BUF];
    size_t n = 0;
    const Result rc = read_stored_pin(srv, pin, &n);
    const bool ok = R_SUCCEEDED(rc) && rescue_proof_mac(hmacSha256CalculateMac, secret, pin, n, msg, m, out->mac);
    if (ok) {
        memcpy(out->nonce, nonce, RESCUE_NONCE_SIZE);
        out->present = true;
    }
    secure_zero(pin, sizeof(pin));
    secure_zero(secret, sizeof(secret));
    return ok;
}

// ---- SD files ----

static bool read_small(const char* path, char* buf, size_t size, size_t* out_len)
{
    *out_len = 0;
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    size_t n = fread(buf, 1, size - 1, f);
    fclose(f);
    buf[n] = '\0';
    *out_len = n;
    return true;
}

// The request, under either name. Returns true and fills *mode when one is
// there.
static bool take_request(RescueMode* mode)
{
    static char buf[512];
    size_t n = 0;
    if (!read_small(PATH_REQ, buf, sizeof(buf), &n) && !read_small(PATH_REQTXT, buf, sizeof(buf), &n))
        return false;
    *mode = rescue_request_mode(buf, n);
    return true;
}

// The challenge PlayGuard left (RESCUE_NONCE_NAME), for it to replace.
static bool read_nonce(u8* out)
{
    static char buf[128];
    size_t n = 0;
    return read_small(PATH_NONCE, buf, sizeof(buf), &n) && rescue_nonce_parse(buf, n, out);
}

static bool exists(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

// Takes one request file away so it is acted on once: removed, or, should that
// fail, renamed RESCUE.done (replacing an earlier one). Kept when it is still
// there afterwards; Removed too when it was never there.
static RescueRequest consume(const char* path)
{
    if (remove(path) == 0 || !exists(path)) return RescueRequest_Removed;
    remove(PATH_DONE);
    if (rename(path, PATH_DONE) == 0 && !exists(path)) return RescueRequest_Renamed;
    return RescueRequest_Kept;
}

// Both names (either may be there, or both): the worst outcome of the two.
static RescueRequest consume_requests(void)
{
    RescueRequest a = consume(PATH_REQ);
    RescueRequest b = consume(PATH_REQTXT);
    return a > b ? a : b;
}

// `n` bytes to `path` through a "<path>.tmp" sibling, as the app's own
// files: a file cut short (power off, SD card full) is never read as a whole
// one. PlayGuard reads the ".tmp" when the file itself is missing
// (paths::read_file); Atmosphère, without the file, takes its defaults.
// Owner-writable only (fopen would ask for 0666).
static bool write_whole(const char* path, const char* data, size_t n)
{
    char tmp[FS_MAX_PATH];
    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp)) return false;
    const int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    FILE* f = fd >= 0 ? fdopen(fd, "wb") : NULL;
    if (!f) {
        if (fd >= 0) close(fd);
        return false;
    }
    const bool written = fwrite(data, 1, n, f) == n;
    if (fclose(f) != 0 || !written) {
        remove(tmp);
        return false;
    }
    remove(path);
    return rename(tmp, path) == 0;
}

static void write_report(const RescueReport* r, const RescueProof* proof)
{
    static char buf[256];
    size_t n = rescue_report_format(r, buf, sizeof(buf));
    if (n == 0) return;
    n += rescue_proof_format(proof, buf + n, sizeof(buf) - n);   // nothing when absent
    write_whole(PATH_REPORT, buf, n);
}

// ---- the homebrew menu's lock (core/hbl_lock.h) ----

// Atmosphère's version, from exosphère (spl config item 65000, as the app's
// sysinfo.c reads it); false when it cannot be read.
static bool ams_version(u8* major, u8* minor, u8* micro)
{
    if (R_FAILED(splInitialize())) return false;
    u64 v = 0;
    const bool ok = R_SUCCEEDED(splGetConfig((SplConfigItem)65000, &v));
    splExit();
    if (!ok) return false;
    *major = (u8)((v >> 56) & 0xFF);
    *minor = (u8)((v >> 48) & 0xFF);
    *micro = (u8)((v >> 40) & 0xFF);
    return true;
}

// Locked, and lifted at this boot: why (core/hbl_lock.h), or false to leave
// it. `rescue`: a RESCUE request was there.
static bool lift_reason(bool rescue, HblLockReport* report)
{
    static char buf[64];
    size_t n = 0;
    u8 locked_on[3] = { 0 }, now[3] = { 0 };
    const bool state = !rescue && read_small(PATH_HBL_STATE, buf, sizeof(buf), &n) &&
                       hbl_lock_state_parse(buf, n, &locked_on[0], &locked_on[1], &locked_on[2]);
    const bool known = state && ams_version(&now[0], &now[1], &now[2]);
    return hbl_lock_boot_lift(rescue, state, locked_on, known, now, report);
}

static void hbl_lock_at_boot(bool rescue)
{
    static char in[HBL_LOCK_MAX_FILE + 1];
    static char out[HBL_LOCK_MAX_FILE + 2048];
    size_t n = 0, k = 0;
    if (!read_small(PATH_HBL_CONFIG, in, sizeof(in), &n) || n >= sizeof(in) - 1) return;   // none, or too large
    if (hbl_lock_locked(in, n)) {
        HblLockReport report;
        if (!lift_reason(rescue, &report)) return;
        // With the defaults written back, as the app does: Atmosphère may
        // have read the locked file already at this boot (the HOME menu up
        // and a program started before this runs). The next boot takes them away.
        report.ok = hbl_lock_apply(in, n, HblLock_Unlock, out, sizeof(out), &k) && write_whole(PATH_HBL_CONFIG, out, k);
        if (report.ok) remove(PATH_HBL_STATE);
        static char text[128];
        const size_t t = hbl_lock_report_format(&report, text, sizeof(text));
        if (t) write_whole(PATH_HBL_REPORT, text, t);
        return;
    }
    if (hbl_lock_has_defaults(in, n) && hbl_lock_apply(in, n, HblLock_DropDefaults, out, sizeof(out), &k))
        write_whole(PATH_HBL_CONFIG, out, k);
}

// pctl can take a moment to come up at boot; the SD card too. Try for a while.
static bool wait_for_sd(void)
{
    for (int i = 0; i < 40 && !s_sd_mounted; i++) {   // ~10 s
        svcSleepThread(250000000ULL);
        s_sd_mounted = R_SUCCEEDED(fsdevMountSdmc());
    }
    return s_sd_mounted;
}

static bool wait_for_pctl(void)
{
    for (int i = 0; i < 40; i++) {   // ~10 s
        if (R_SUCCEEDED(pctlInitialize())) return true;
        svcSleepThread(250000000ULL);
    }
    return false;
}

static void do_rescue(Service* srv, RescueMode mode, RescueReport* out)
{
    out->mode    = mode;
    out->result  = RescueResult_Failed;
    out->rc      = 0;
    out->unlocks = 0;

    if (mode == RescueMode_Delete) {
        Result rc = serviceDispatch(srv, 1043);   // DeleteSettings — irreversible
        out->rc     = rc;
        out->result = R_SUCCEEDED(rc) ? RescueResult_Ok : RescueResult_Failed;
        return;
    }

    u32 len = 0;
    Result rc = rd_u32(srv, 1206, &len);
    if (R_FAILED(rc)) { out->rc = rc; return; }
    if (len == 0) { out->result = RescueResult_NoPin; return; }

    rc = pctl_unlock_with_stored_pin(srv);
    if (R_FAILED(rc)) { out->rc = rc; return; }
    out->unlocks = 1;

    // Confirm the unlock took; if the read fails, say so rather than claim it.
    bool unlocked = false;
    rc = rd_bool(srv, 1006, &unlocked);
    if (R_FAILED(rc)) { out->rc = rc; return; }
    out->result = unlocked ? RescueResult_Ok : RescueResult_Failed;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (!wait_for_sd()) return 0;
    // This module runs: PlayGuard no longer waits for a restart to use it.
    remove("sdmc:/" RESCUE_RESTART_PATH);

    RescueMode mode;
    const bool requested = take_request(&mode);
    if (requested) {
        // Take the request away first: it is acted on once, present or not next
        // boot. A delete still there would run again at every boot, so it is
        // not done; an unlock is (the parent needs PlayGuard to open), and the
        // report says the file is still there.
        const RescueRequest request = consume_requests();
        u8 nonce[RESCUE_NONCE_SIZE];
        const bool challenged = read_nonce(nonce);

        RescueReport report;
        memset(&report, 0, sizeof(report));
        report.request = request;   // before the proof, which covers it
        RescueProof proof;
        memset(&proof, 0, sizeof(proof));
        if (mode == RescueMode_Delete && request == RescueRequest_Kept) {
            report.mode = mode;
            report.result = RescueResult_Refused;
        } else if (wait_for_pctl()) {
            Service* srv = pctlGetServiceSession_Service();
            do_rescue(srv, mode, &report);
            // Without a challenge (PlayGuard never opened since it was
            // installed) the report goes without a proof: PlayGuard then
            // shows it without the recovery actions.
            if (challenged && rescue_report_needs_proof(&report)) sign_report(srv, &report, nonce, &proof);
            pctlExit();
        } else {
            report.mode = mode;
            report.result = RescueResult_Failed;
            report.rc = MAKERESULT(Module_Libnx, LibnxError_InitFail_SM);
        }
        write_report(&report, &proof);
    }
    hbl_lock_at_boot(requested);

    return 0;   // boot2 reclaims the process; nothing stays resident
}

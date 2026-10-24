// rescue — app side (see rescue.hpp).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/rescue.hpp"

#include <borealis.hpp>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include "action/data_notice.hpp"
#include "action/history_flow.hpp"
#include "action/notices.hpp"
#include "action/bundled_modules.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "ui/ui.hpp"
#include "util/modules.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/paths.hpp"

using namespace brls::literals;

namespace rescue
{

namespace
{
std::string report_path()
{
    return paths::data_dir() + "/" + RESCUE_REPORT_NAME;
}

std::string nonce_path()
{
    return paths::data_dir() + "/" + RESCUE_NONCE_NAME;
}

// The challenge on the card, when there is one that reads well.
bool read_nonce(uint8_t out[RESCUE_NONCE_SIZE])
{
    std::string text;
    return paths::read_file(nonce_path(), text) && rescue_nonce_parse(text.c_str(), text.size(), out);
}

// A new challenge for the sysmodule's next report. Should it not be
// written, a report answers the old one, or none (logged).
void new_nonce()
{
    uint8_t nonce[RESCUE_NONCE_SIZE];
    char text[2 * RESCUE_NONCE_SIZE + 2];
    std::string error = "no random bytes";
    if (platform_random(nonce, sizeof(nonce)) && rescue_nonce_format(nonce, text, sizeof(text)) &&
        paths::ensure_dir(paths::data_dir()) && paths::atomic_write(nonce_path(), text, &error))
        return;
    brls::Logger::warning("rescue: {} could not be written: {}", RESCUE_NONCE_NAME, error);
}

// Whether `proof` is the sysmodule's for `report`: its MAC made again with
// the stored PIN and the console's secret (rescue_proof_mac).
bool proven(const RescueReport& report, const RescueProof& proof)
{
    if (!proof.present) return false;
    char msg[256];
    const size_t n = rescue_mac_message(&report, proof.nonce, msg, sizeof(msg));
    uint8_t mac[RESCUE_MAC_SIZE];
    const Result rc = n ? pctl_pin_hmac(msg, n, mac) : NXM_RC_INVALID_ARGUMENT;
    const bool ok = R_SUCCEEDED(rc) && rescue_mac_equal(mac, proof.mac);
    if (R_FAILED(rc)) brls::Logger::warning("rescue: the proof could not be checked (rc=0x{:08X})", (unsigned)rc);
    return ok;
}

// atmosphere/contents/<id>/ on the SD card, with its trailing slash.
std::string module_dir()
{
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';   // "/" on the console
    path += RESCUE_SYSMODULE_PATH;
    return path.substr(0, path.rfind('/') + 1);
}

std::string restart_path()
{
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';
    return path + RESCUE_RESTART_PATH;
}

// The module was updated and the console has not restarted since (the old
// one still runs at boot until then): an entry to do until it has.
// `updated`: just now, whether or not the marker could be written.
void note_restart(bool updated = false)
{
    if (!updated && !paths::exists_or_pending(restart_path())) {
        notices::clear("rescue_restart");
        return;
    }
    notices::set("rescue_restart", notices::Level::Action, "playguard/notices/rescue_restart"_i18n,
                 "playguard/notices/rescue_restart_detail"_i18n);
}
}   // namespace

const char* revision()
{
#ifdef APP_RESCUE_REV
    return APP_RESCUE_REV;
#else
    return "";
#endif
}

namespace
{
// The installed module's playguard-rescue.txt, "1.2.0 84d0eaa86878": the
// version it came with (into *version), and its revision, returned. Empty
// when there is none (installed before it was written).
std::string installed_revision(std::string* version)
{
    std::string text, rev;
    if (version) version->clear();
    if (paths::read_file(module_dir() + "playguard-rescue.txt", text)) {
        const size_t space = text.find(' ');
        if (space != std::string::npos) {
            if (version) *version = text.substr(0, space);
            rev = text.substr(space + 1);
            rev = rev.substr(0, rev.find_first_of("\r\n "));
        }
    }
    return rev;
}
}   // namespace

bool outdated()
{
    return installed() && *revision() && installed_revision(nullptr) != revision();
}

void note_version()
{
    if (!installed() || !*revision()) {
        notices::clear("rescue_old");
        notices::clear("rescue_restart");
        return;
    }
    note_restart();
    std::string version;
    const std::string rev = installed_revision(&version);
    if (rev == revision()) {
        notices::clear("rescue_old");
        return;
    }
    if (!bundled()) {
        const std::string detail = version.empty()
            ? brls::getStr("playguard/notices/rescue_unknown", app::version())
            : brls::getStr("playguard/notices/rescue_old_detail", version, app::version());
        notices::set("rescue_old", notices::Level::Action, "playguard/notices/rescue_old"_i18n, detail);
        return;
    }
    const std::string detail = version.empty()
        ? brls::getStr("playguard/notices/rescue_unknown_update", app::version())
        : brls::getStr("playguard/notices/rescue_old_update", version, app::version());
    notices::set("rescue_old", notices::Level::Action, "playguard/notices/rescue_old"_i18n, detail,
                 "playguard/notices/rescue_update"_i18n, []() { update_now(); });
}

std::string installed_version()
{
    std::string version;
    if (installed()) installed_revision(&version);
    return version;
}

void update_now(std::function<void(bool)> done)
{
    std::string error;
    if (!update(&error)) {
        ui::error(brls::getStr("playguard/notices/rescue_update_failed", error));
        if (done) done(false);
        return;
    }
    notices::clear("rescue_old");
    note_restart(true);
    ui::info("playguard/notices/rescue_updated"_i18n);
    if (done) done(true);
}

bool bundled()
{
    return modules::bundled(bundled_modules::dir(), modules::get(modules::Id::Rescue)).present;
}

bool update(std::string* error)
{
    // The one Console › Optional modules installs (romfs:/sysmodules/rescue/).
    const modules::Bundle b = modules::bundled(bundled_modules::dir(), modules::get(modules::Id::Rescue));
    std::string nsp;
    if (!b.present || !paths::read_file(b.nsp, nsp) || nsp.empty()) {
        if (error) *error = "This build does not carry the module";
        return false;
    }
    const std::string dir = module_dir();
    // The module first, then the file naming it: should the module not be
    // written, the next start still reads the old revision and offers this again.
    if (!paths::atomic_write(dir + "exefs.nsp", nsp, error)) {
        brls::Logger::error("rescue: the module could not be updated: {}", error ? *error : "");
        return false;
    }
    if (!paths::atomic_write(dir + "playguard-rescue.txt", app::version() + " " + revision() + "\n", error)) {
        brls::Logger::error("rescue: playguard-rescue.txt could not be written: {}", error ? *error : "");
        return false;
    }
    // Until the module removes it at the next boot (sysmodule/rescue/source/main.c).
    std::string ignore;
    if (!paths::atomic_write(restart_path(), "", &ignore))
        brls::Logger::warning("rescue: {} could not be written: {}", RESCUE_RESTART_PATH, ignore);
    brls::Logger::info("rescue: module updated to revision {} ({} bytes)", revision(), nsp.size());
    return true;
}

bool installed()
{
    std::string path = paths::sd_root();
    if (path.empty() || path.back() != '/') path += '/';   // "/" on the console
    path += RESCUE_SYSMODULE_PATH;
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::optional<RescueReport> take(RescueProof* proof)
{
    if (proof) *proof = RescueProof{};
    // The challenge a report left since the last start answers, then a new
    // one at every start: a report is believed at the first start after the
    // boot that wrote it, or never (a copy kept for later answers nothing).
    uint8_t expected[RESCUE_NONCE_SIZE];
    const bool challenged = read_nonce(expected);
    new_nonce();
    const std::string path = report_path();
    std::string text;
    if (!paths::read_file(path, text)) return std::nullopt;
    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());

    RescueReport r;
    if (!rescue_report_parse(text.c_str(), text.size(), &r)) {
        // The sysmodule may or may not have acted: the parent is told (once,
        // on the main screen) rather than left guessing.
        brls::Logger::warning("rescue report present but unreadable");
        data_notice::rescue_unreadable();
        return std::nullopt;
    }
    RescueProof p;
    rescue_proof_parse(text.c_str(), text.size(), &p);
    const bool answers = p.present && challenged && std::memcmp(p.nonce, expected, sizeof(expected)) == 0;
    brls::Logger::info("rescue report: mode={} result={} rc=0x{:08X} proof={}", rescue_mode_name(r.mode),
                       rescue_result_name(r.result), (unsigned)r.rc,
                       answers ? "present" : p.present ? "for another challenge" : "none");
    if (!answers) p = RescueProof{};
    if (proof) *proof = p;
    // A plain event so a parent sees recovery was used; the specifics are on
    // the recovery screen and in the log. The delete / PIN actions the parent
    // then takes record their own entries.
    history_flow::record_event("rescue");
    return r;
}

bool confirmed(const RescueReport& report, const RescueProof& proof, bool* old_module)
{
    if (old_module) *old_module = false;
    u32 pin_length = 0;
    bool unlocked = false;
    const Result rc = pctl_lock_state(&pin_length, &unlocked);
    const bool state = R_SUCCEEDED(rc) && rescue_report_confirmed(&report, pin_length, unlocked);
    // The proof only once the state agrees: it reads the PIN.
    const bool needs_proof = rescue_report_needs_proof(&report);
    const bool made_by_module = needs_proof ? state && proven(report, proof) : installed();
    const bool ok = state && made_by_module;
    // The console agrees but no proof that checks came with the unlock, and
    // the module installed is not this PlayGuard's: one from before the
    // proof writes none, one from before the console's secret (core/rescue.h)
    // one that never checks, and doing RESCUE again gives the same.
    if (old_module) *old_module = !ok && state && needs_proof && !made_by_module && outdated();
    brls::Logger::info("rescue report {} by the console ({} {}, rc=0x{:08X}, PIN {}, unlocked={})",
                       ok ? "confirmed" : "not confirmed", needs_proof ? "proof" : "sysmodule",
                       needs_proof ? (made_by_module ? "valid" : proof.present ? "invalid" : "missing")
                                   : (made_by_module ? "installed" : "missing"),
                       (unsigned)rc, pin_length ? "set" : "none", unlocked);
    return ok;
}

}   // namespace rescue

// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "app.hpp"

#include <cctype>
#include <cstdlib>

#include "core/platform.h"
#include "core/sim_env.h"
#include "ui/live_prefs.hpp"
#include "ui/ui.hpp"
#include "util/config.hpp"
#include "util/pctl_ops_c.hpp"
#include "util/text.hpp"

#ifndef APP_VERSION
#define APP_VERSION "0.0.0-dev"
#endif
#ifndef APP_PR
#define APP_PR 0
#endif
#ifndef APP_RELEASE
#define APP_RELEASE 0
#endif

namespace app
{

static bool     s_ok          = false;
static uint32_t s_init_result = 0;
static bool     s_dev_session = false;

bool init()
{
    Result rc     = pctl_ops_init();
    s_init_result = (uint32_t)rc;
    s_ok          = R_SUCCEEDED(rc);
    pctl_ops_exit();   // every operation acquires its own session
    return s_ok;
}

void shutdown()
{
    pctl_ops_exit();
    s_ok = false;
}

uint32_t    pctl_init_result() { return s_init_result; }
#ifndef __SWITCH__
// Desktop simulation: the smoke test runs as one made-up build, whatever this
// build's version and commit (PLAYGUARD_SIM_VERSION="1.0.0:abcdef0", or "1.0.0:"
// for a release), so its screenshots and its update offer (a simulated 1.1.0)
// do not change with each release.
static const char* sim_version() { return sim_env("PLAYGUARD_SIM_VERSION"); }
static std::string sim_part(bool want_commit)
{
    const std::string s = sim_version();
    const size_t colon  = s.find(':');
    if (!want_commit) return s.substr(0, colon);
    return colon == std::string::npos ? "" : s.substr(colon + 1);
}
std::string version()    { return sim_version() ? sim_part(false) : APP_VERSION; }
std::string commit()     { return sim_version() ? sim_part(true) : APP_COMMIT; }
bool        is_release() { return sim_version() ? commit().empty() : APP_RELEASE != 0; }
int         pr()         { return sim_version() ? 0 : APP_PR; }
#else
std::string version()          { return APP_VERSION; }
std::string commit()           { return APP_COMMIT; }
bool        is_release()       { return APP_RELEASE != 0; }
int         pr()               { return APP_PR; }
#endif

namespace
{
std::string s_self_path;
}

std::string self_path() { return s_self_path; }

void set_self_path(const char* argv0)
{
    // hbloader passes "sdmc:/switch/…/x.nro"; anything else (nxlink's
    // "sdmc:/switch/.overlays…", a missing argv) is not a file to replace.
    s_self_path.clear();
    if (!argv0) return;
    const std::string path = argv0;
    const std::string prefix = "sdmc:/", suffix = ".nro";
    if (path.size() > prefix.size() + suffix.size() && text::starts_with(path, prefix) &&
        text::ends_with(path, suffix))
        s_self_path = path.substr(prefix.size() - 1);
}
const char* repo_url()         { return "https://github.com/JigSawFr/PlayGuard"; }
const char* site_url()         { return "https://playguard.link/"; }

std::string doc_url(const std::string& page, const std::string& anchor)
{
    // The site's folder of a language: its code in lower case, English at
    // the root (and for a language the site does not have).
    static const char* const FOLDERS[] = {"fr", "fr-ca", "de", "es", "es-419", "it", "nl", "pt",
                                          "pt-br", "ru", "ja", "ko", "zh-hans", "zh-hant"};
    std::string lang = live_prefs::locale();
    for (char& c : lang) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    std::string url = site_url();
    for (const char* folder : FOLDERS)
        if (lang == folder) url += lang + "/";
    url += (is_release() ? "docs/" : "docs/dev/") + page + "/";
    if (!anchor.empty()) url += "#" + anchor;
    return url;
}

bool in_focus()
{
    return platform_in_focus();
}

bool read_only()            { return core_read_only(); }
void set_read_only(bool on) { core_set_read_only(on); }

bool dev_mode() { return s_dev_session || config::get().dev_mode; }

void set_dev_mode(bool on, bool persist, const std::string& saved_text, const std::string& failed_text)
{
    // Logged: a report's header gives the mode at the time it was made, and
    // the toasts before it may tell otherwise.
    if (on != dev_mode())
        brls::Logger::info("developer mode: {}{}", on ? "on" : "off", persist || !on ? "" : " (this session)");
    s_dev_session = on;
    if (persist || !on) {
        config::get().dev_mode = on;
        ui::save_config(saved_text, failed_text);
    }
}

}   // namespace app

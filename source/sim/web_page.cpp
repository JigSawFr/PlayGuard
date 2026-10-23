// What the web simulator's page (web/simulator.js) calls while PlayGuard
// runs, on the page's thread, between two frames (web/README.md).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <emscripten.h>

#include <cstdio>
#include <cstring>

#include <borealis.hpp>

#include "core/pctl_ops.h"
#include "core/sim_env.h"
#include "core/time_ops.h"
#include "sim/sim_games.h"
#include "sim/sim_state.h"

// A PLAYGUARD_SIM_* variable changed without a restart: the ones read at each
// call (web/params.json's `live` entries) take it at their next read. An
// empty or null value removes it. Kept by core/sim_env.h, under its lock:
// the process environment, which other threads read, stays as it was.
extern "C" EMSCRIPTEN_KEEPALIVE int playguard_web_setenv(const char* name, const char* value)
{
    if (!name || std::strncmp(name, "PLAYGUARD_SIM_", 14) != 0)
        return 0;
    sim_env_set(name, value);
    // The HOME menu taking the screen, or giving it back: the console tells
    // PlayGuard (its focus event), which locks again, pauses, asks for the PIN.
    if (std::strcmp(name, "PLAYGUARD_SIM_BACKGROUND") == 0) {
        const bool focused = !sim_env_flag(name);
        brls::sync([focused]() { brls::Application::getWindowFocusChangedEvent()->fire(focused); });
    }
    return 1;
}

// The CJK font, fetched by the page after PlayGuard started (it is only
// waited for when the page is in Japanese, Korean or Chinese): added behind
// the Latin one, as borealis does at start-up when it is there already.
extern "C" EMSCRIPTEN_KEEPALIVE int playguard_web_cjk_font(const char* path)
{
    NVGcontext* vg = brls::Application::getNVGContext();
    if (!vg || !path || brls::Application::getFont(brls::FONT_CHINESE_SIMPLIFIED) != brls::FONT_INVALID)
        return 0;
    if (!brls::Application::loadFontFromFile(brls::FONT_CHINESE_SIMPLIFIED, path))
        return 0;
    nvgAddFallbackFontId(vg, brls::Application::getFont(brls::FONT_REGULAR),
                         brls::Application::getFont(brls::FONT_CHINESE_SIMPLIFIED));
    brls::Logger::info("Using {} font: {}", brls::FONT_CHINESE_SIMPLIFIED, path);
    return 1;
}

// The smoke test (tools/web_smoke.py) before a screenshot: 1 while borealis
// holds the input, a screen still opening or closing (the footer's hints
// greyed until it lets go). On a busy runner, SwiftShader's frames can make
// that last longer than the test's pause.
extern "C" EMSCRIPTEN_KEEPALIVE int playguard_web_busy()
{
    return brls::Application::isInputBlocks() ? 1 : 0;
}

// What the simulated console does now, for the page's line under the screen
// (SimLogic.consoleLine, web/sim_logic.js): the game running, the time spent
// today and left (below 0 past the limit, while the console only warns),
// whether it restricts, and whether it lives on (PLAYGUARD_SIM_KEEP). JSON,
// valid until the next call. With PLAYGUARD_SIM_KEEP, each call counts the
// time that passed (sim_games.c), however long PlayGuard's screens leave the
// timer unread.
extern "C" EMSCRIPTEN_KEEPALIVE const char* playguard_web_console()
{
    static char out[512];
    PtState pt;
    pctl_play_timer_query(&pt);
    LocalTime now{};
    const int wd = time_local_now(nullptr, &now) ? now.wday % 7 : 0;
    const bool limited = pt.valid && pt.enabled && pt.day_min[wd] != PT_DAY_NOLIMIT;
    const SimGame* game = sim_game_running();
    std::snprintf(out, sizeof(out),
                  "{\"game\":\"%s\",\"spent\":%llu,\"limit\":%d,\"left\":%lld,\"restricted\":%s,"
                  "\"alarm_only\":%s,\"unlocked\":%s,\"living\":%s}",
                  game ? game->name : "", static_cast<unsigned long long>(sim_played_today_s()),
                  limited ? static_cast<int>(pt.day_min[wd]) : -1,
                  static_cast<long long>(pt.remaining_ns / 1000000000LL),
                  // Suspended: 1455 reads true past the limit even while
                  // temporarily unlocked, when nothing is suspended.
                  pt.restricted && !pt.temporary_unlocked ? "true" : "false",
                  pt.alarm_only ? "true" : "false", pt.temporary_unlocked ? "true" : "false",
                  sim_keeping() ? "true" : "false");
    return out;
}

// The page's Restart the console (PLAYGUARD_SIM_KEEP only): the console comes
// back locked; the page then reloads, PlayGuard starting again on it.
extern "C" EMSCRIPTEN_KEEPALIVE void playguard_web_console_restart()
{
    sim_restart();
}

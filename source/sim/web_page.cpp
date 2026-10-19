// What the web simulator's page (web/simulator.js) calls while PlayGuard
// runs, on the page's thread, between two frames (web/README.md).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include <emscripten.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include <borealis.hpp>

// A PLAYGUARD_SIM_* variable changed without a restart: the ones read at each
// call (web/params.json's `live` entries) take it at their next read. An
// empty or null value removes it. The "NAME=value" strings are never freed:
// another thread may be reading the one it replaces. A name not set yet
// grows the environment's table, which a thread reading it at that very
// moment can miss for that read: a simulator's race, not a console's.
extern "C" EMSCRIPTEN_KEEPALIVE int playguard_web_setenv(const char* name, const char* value)
{
    if (!name || std::strncmp(name, "PLAYGUARD_SIM_", 14) != 0 || std::strchr(name, '='))
        return 0;
    if (!value || !*value)
        return unsetenv(name) == 0;
    const std::string entry = std::string(name) + "=" + value;
    return putenv(strdup(entry.c_str())) == 0;
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

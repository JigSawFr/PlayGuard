// ApiActivity — the first screen when another homebrew starts PlayGuard with
// a request (action/api_flow, docs/api.md): who asks, each request and how it
// went, then back to the caller (or out of PlayGuard) a moment later, or at
// once with A or B. Shown instead of the main screen; never covered by the
// lock screen (what needs the PIN asks for it here).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>
#include <vector>

class ApiActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/api.xml");

    // `routes`: one line per request ("POST /v1/clock/sync"); `start` runs
    // once the screen is up.
    ApiActivity(const std::string& caller, const std::vector<std::string>& routes,
                std::function<void(ApiActivity*)> start);

    void onContentAvailable() override;

    // Under request `i`: what is happening, then how it went.
    void set_line(size_t i, const std::string& text);
    void set_line(size_t i, const std::string& text, bool ok);
    // In place of the requests (the request itself was refused).
    void set_message(const std::string& text);
    // All answered: says where PlayGuard goes next, and goes a moment later.
    void done(bool ok, const std::string& return_to);

    // How long the result stays before PlayGuard goes on.
    static constexpr int LEAVE_MS = 2500;

  private:
    std::string caller;
    std::vector<std::string> routes;
    std::function<void(ApiActivity*)> start;
    std::vector<brls::Label*> results;
    bool finished = false;
    bool left = false;

    void leave();

    BRLS_BIND(brls::Label, from, "api_from");
    BRLS_BIND(brls::Box, rows, "api_rows");
    BRLS_BIND(brls::Label, next, "api_next");
};

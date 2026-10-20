// ApiActivity — the first screen when another homebrew starts PlayGuard with
// a request (action/api_flow, docs/api.md): who asks (by the name it gives
// itself), each request by its name with how it went (an icon, not only a
// colour), then back to the caller (or out of PlayGuard) after a countdown,
// or at once with Continue. A request refused because the parent has not
// allowed it offers Allow…, which opens Preferences › Other homebrews behind
// the PIN. Shown instead of the main screen; never covered by the lock
// screen (what needs the PIN asks for it here).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>
#include <vector>

class CadranIcon;

class ApiActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/api.xml");

    struct Row
    {
        std::string name;    // "Set the network clock"
        std::string route;   // "POST /v1/clock/sync"
    };

    // `start` runs once the screen is up.
    ApiActivity(const std::string& caller, const std::vector<Row>& rows, std::function<void(ApiActivity*)> start);

    void onContentAvailable() override;

    // Under request `i`: what is happening, then how it went.
    void set_line(size_t i, const std::string& text);
    void set_line(size_t i, const std::string& text, bool ok);
    // In place of the requests (the request itself was refused).
    void set_message(const std::string& text);
    // All answered: where PlayGuard goes next, after a countdown.
    // `offer_allow`: a request was refused for want of a permission.
    void done(const std::string& return_to, bool offer_allow);

    // Seconds the result stays before PlayGuard goes on.
    static constexpr int LEAVE_S = 5;

  private:
    std::string caller;
    std::vector<Row> rows;
    std::function<void(ApiActivity*)> start;
    std::vector<brls::Label*> results;
    std::vector<CadranIcon*> icons;
    std::string target;     // where PlayGuard goes: the caller's file name, or ""
    bool finished = false;
    bool left = false;
    bool counting = false;  // the countdown runs (stopped by Allow…)
    int seconds_left = LEAVE_S;

    void tick();
    void show_next();
    void leave();

    BRLS_BIND(brls::Label, from, "api_from");
    BRLS_BIND(brls::Box, list, "api_rows");
    BRLS_BIND(brls::Label, next, "api_next");
    BRLS_BIND(brls::Button, go_on, "api_continue");
    BRLS_BIND(brls::Button, allow, "api_allow");
};

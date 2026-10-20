// ApiConsentActivity — the parent's yes or no before another homebrew
// changes something (action/api_flow, docs/api.md): one card over the
// request's screen, dimmed, as a phone asks for a permission. The app's own
// icon and name from its .nro (the name the request gives only said apart,
// when it differs), the question, what it costs, "Allow for 30 days" (an app
// the parent trusts: api_logic::Trust), Allow with the PIN, Refuse (focused,
// and B), Details (the file, the request).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>
#include <vector>

#include "action/api_logic.hpp"

class ApiConsentActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/api_consent.xml");

    struct Consent
    {
        std::string declared;    // the request's `caller` ("" none)
        std::string app_name;    // its .nro's NACP name ("" unread)
        std::string app_meta;    // "v1.2.0 · by Untel" ("" unread)
        std::vector<uint8_t> icon;   // its .nro's JPEG (empty: none)
        std::string nro_shown;   // "sdmc:/switch/…" ("" none)
        std::string route;       // "POST /v1/clock/sync", for Details
        std::string question;    // "Allow MyApp to set the clock?"
        std::string text;        // what it changes and costs
        bool can_trust = false;  // its file was read: "Allow for 30 days" offered
    };

    // `answer(yes, trust)` once the card is gone: trust is the "Allow for 30
    // days" switch (only with yes).
    ApiConsentActivity(Consent consent, std::function<void(bool yes, bool trust)> answer);

    void onContentAvailable() override;
    // The request's screen stays drawn under the card, dimmed.
    bool isTranslucent() override { return true; }

  private:
    Consent consent;
    std::function<void(bool, bool)> answer;
    bool answered = false;
    bool trust    = false;

    void reply(bool yes);

    BRLS_BIND(brls::Image, icon, "ac_icon");
    BRLS_BIND(brls::Label, title, "ac_title");
    BRLS_BIND(brls::Label, text, "ac_text");
    BRLS_BIND(brls::Label, claim, "ac_claim");
    BRLS_BIND(brls::BooleanCell, trust_cell, "ac_trust");
    BRLS_BIND(brls::Button, allow, "ac_allow");
    BRLS_BIND(brls::Button, refuse, "ac_refuse");
    BRLS_BIND(brls::Button, details, "ac_details");
};

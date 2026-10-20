// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/api_consent_activity.hpp"

#include "ui/ui.hpp"

using namespace brls::literals;

ApiConsentActivity::ApiConsentActivity(Consent consent, std::function<void(bool, bool)> answer)
    : consent(std::move(consent)), answer(std::move(answer))
{
}

void ApiConsentActivity::onContentAvailable()
{
    for (brls::Label* l : { title.getView(), text.getView(), claim.getView() }) l->setSingleLine(false);
    if (!this->consent.icon.empty())
        icon->setImageFromMem(this->consent.icon.data(), (int)this->consent.icon.size());
    else
        ui::set_visible(icon.getView(), false);
    title->setText(this->consent.question);
    text->setText(this->consent.text);
    // The name the request gives, when its file says another or cannot be read.
    const std::string& declared = this->consent.declared;
    const bool differs = !declared.empty() && declared != this->consent.app_name;
    claim->setText(!differs ? "" : brls::getStr(this->consent.app_name.empty() ? "playguard/api/consent/says_unchecked"
                                                                               : "playguard/api/consent/says_it_is",
                                                declared));
    ui::set_visible(claim.getView(), differs);
    trust_cell->init("playguard/api/consent/trust"_i18n, false, [this](bool on) { this->trust = on; });
    ui::set_visible(trust_cell.getView(), this->consent.can_trust);

    allow->registerClickAction([this](brls::View*) {
        this->reply(true);
        return true;
    });
    refuse->registerClickAction([this](brls::View*) {
        this->reply(false);
        return true;
    });
    details->registerClickAction([this](brls::View*) {
        const Consent& c = this->consent;
        std::string body = c.app_name.empty() ? (c.declared.empty() ? "playguard/api/consent/unnamed"_i18n : c.declared)
                                              : c.app_name;
        if (!c.app_meta.empty()) body += "\n" + c.app_meta;
        if (!c.nro_shown.empty()) body += "\n" + c.nro_shown;
        body += "\n\n" + c.route;
        ui::info(body);
        return true;
    });
    this->getContentView()->registerAction("playguard/api/consent/refuse"_i18n, brls::BUTTON_B, [this](brls::View*) {
        this->reply(false);
        return true;
    });
    // No by default: a stray A never allows.
    brls::sync([this]() { brls::Application::giveFocus(this->refuse.getView()); });
}

void ApiConsentActivity::reply(bool yes)
{
    if (this->answered) return;
    this->answered = true;
    auto answer = this->answer;
    const bool trust = yes && this->trust;
    // Next frame: this runs from a button's action, and borealis still plays
    // that button's click once it returns (a closed screen's button is freed).
    brls::sync([answer, yes, trust]() {
        brls::Application::popActivity(brls::TransitionAnimation::NONE, [answer, yes, trust]() {
            brls::sync([answer, yes, trust]() { answer(yes, trust); });
        });
    });
}

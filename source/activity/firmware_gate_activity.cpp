// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/firmware_gate_activity.hpp"

#include "action/pt_flow.hpp"
#include "action/update_flow.hpp"
#include "app.hpp"
#include "ui/ui.hpp"
#include "util/update.hpp"

using namespace aurore::literals;

void FirmwareGateActivity::onContentAvailable()
{
    const std::string fw = fw_gate::firmware();
    for (aurore::Label* l : { headline.getView(), body.getView(), status.getView(), note.getView() })
        l->setSingleLine(false);
    headline->setText(aurore::getStr("playguard/fw_gate/headline", fw));
    body->setText(aurore::getStr("playguard/fw_gate/body", app::version(), fw_gate::tested_max()));

    // Shown once the check finds a newer release.
    update->setText(update_flow::update_label());
    update->registerClickAction([](aurore::View*) {
        update_flow::update_now();
        return true;
    });
    ui::set_visible(update.getView(), false);

    read_only->registerClickAction([this](aurore::View*) {
        this->choose(fw_gate::Choice::ReadOnly);
        return true;
    });
    probe->registerClickAction([this](aurore::View*) {
        this->choose(fw_gate::Choice::Probe);
        return true;
    });
    risk->registerClickAction([this](aurore::View*) {
        ui::confirm_danger(aurore::getStr("playguard/fw_gate/risk_body", fw_gate::firmware()),
                           "playguard/fw_gate/risk_confirm"_i18n, [this]() { this->choose(fw_gate::Choice::Risk); });
        return true;
    });
    remember->init("playguard/fw_gate/remember"_i18n, false, [](bool) {});
    quit->registerClickAction([](aurore::View*) {
        aurore::Application::quit();
        return true;
    });

    // B continues read-only, the safe choice (nothing is remembered unless ticked).
    this->getContentView()->registerAction("playguard/fw_gate/read_only_short"_i18n, aurore::BUTTON_B, [this](aurore::View*) {
        this->choose(fw_gate::Choice::ReadOnly);
        return true;
    });

    update_flow::check_once(ui::guarded(this->alive, [this, fw](const update_flow::Offer& o) {
        if (this->chosen) return;
        const update::Result& r = o.result;
        this->status->setText(update_flow::status_text(r, fw));
        const bool newer = r.verdict == update::Verdict::UpdateSupports || r.verdict == update::Verdict::UpdateNoSupport;
        this->update->setDetailText(newer ? r.latest.version : "");
        ui::set_visible(this->update.getView(), newer);
        // Not from under the "risk" confirmation: its buttons keep the focus.
        if (r.verdict == update::Verdict::UpdateSupports && ui::has_focus(this))
            aurore::Application::giveFocus(this->update.getView());
    }));
}

void FirmwareGateActivity::choose(fw_gate::Choice choice)
{
    if (this->chosen) return;   // a second press while the screen fades out
    this->chosen = true;
    fw_gate::apply(choice, this->remember->isOn());
    aurore::Application::popActivity(aurore::TransitionAnimation::FADE, [choice]() {
        ui::on_mode_changed();
        if (choice == fw_gate::Choice::Risk) {
            // Writable now: what start-up skipped in read-only mode.
            pt_flow::relock_if_interrupted();
            pt_flow::offer_extra_time_restore();
        }
    });
}

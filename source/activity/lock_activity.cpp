// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/lock_activity.hpp"

#include "action/pin_lock.hpp"
#include "activity/main_activity.hpp"
#include "activity/rescue_activity.hpp"
#include "ui/ui.hpp"

using namespace aurore::literals;

void LockActivity::onContentAvailable()
{
    enter->registerClickAction([this](aurore::View*) {
        this->try_unlock();
        return true;
    });
    this->getContentView()->registerAction("hints/exit"_i18n, aurore::BUTTON_B, [](aurore::View*) {
        aurore::Application::quit();
        return true;
    });
    // Straight to the PIN screen; this one stays behind it for a retry.
    aurore::sync([this]() { this->try_unlock(); });
}

void LockActivity::try_unlock()
{
    if (!pin_lock::ask()) {
        ui::refuse(pin_lock::refusal_text());
        return;
    }
    // Back to the screens it covered, or the main screen in place of this
    // one (nothing to come back to).
    if (over) aurore::Application::popActivity(aurore::TransitionAnimation::NONE);
    else ui::replace_screen(new MainActivity());
}

void LockActivity::lock_again()
{
    // The recovery screen is where a forgotten PIN is fixed: never covered.
    for (aurore::Activity* a : aurore::Application::getActivitiesStack())
        if (dynamic_cast<LockActivity*>(a) || dynamic_cast<RescueActivity*>(a)) return;
    aurore::Logger::info("pin_lock: back after a while away, locked again");
    aurore::Application::pushActivity(new LockActivity(true), aurore::TransitionAnimation::NONE);
}

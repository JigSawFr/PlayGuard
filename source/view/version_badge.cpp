// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/version_badge.hpp"

#include "app.hpp"
#include "ui/ui.hpp"

static constexpr const char* BADGE_ID = "version_badge";

VersionBadge::VersionBadge()
    : brls::Box(brls::Axis::ROW)
{
    this->setId(BADGE_ID);
    this->setFocusable(false);
    this->setAlignItems(brls::AlignItems::CENTER);
    this->setAlignSelf(brls::AlignSelf::CENTER);
    this->setMarginLeft(20);

    // On the desktop, one made-up build (PLAYGUARD_SIM_VERSION, see app.cpp).
    const std::string version = app::version(), commit = app::commit();
    const bool release = app::is_release();

    auto* text = new brls::Label();
    text->setFontSize(18);
    text->setSingleLine(true);
    if (release) {
        text->setText("v" + version);
        text->setTextColor(ui::color_note());
        this->addView(text);
        return;
    }
    // The tag: the amber of the warnings, filled, its text in the background's colour.
    auto* tag = new brls::Box(brls::Axis::ROW);
    tag->setBackgroundColor(ui::color_warn());
    tag->setCornerRadius(4);
    tag->setPadding(2, 8, 2, 8);
    tag->setMarginRight(10);
    auto* dev = new brls::Label();
    dev->setText("DEV");
    dev->setFontSize(15);
    dev->setSingleLine(true);
    dev->setTextColor(brls::Application::getTheme()["brls/background"]);
    tag->addView(dev);
    this->addView(tag);
    text->setText(commit.empty() ? version : version + " · " + commit);
    text->setTextColor(ui::color_warn());
    this->addView(text);
}

// Adds the badge to the header of the screen holding `focus` (its root view:
// borealis gives a new screen the focus before putting it on the activity
// stack), unless it has one. Screens without the applet frame's header (the
// loading screen, dialogs) are left alone.
static void badge_screen_of(brls::View* focus)
{
    if (!focus) return;
    while (focus->hasParent()) focus = focus->getParent();
    auto* frame  = dynamic_cast<brls::AppletFrame*>(focus);
    auto* header = frame ? dynamic_cast<brls::Box*>(frame->getView("brls/applet_frame/header")) : nullptr;
    if (!header || header->getView(BADGE_ID)) return;
    header->addView(new VersionBadge());
}

void VersionBadge::install()
{
    brls::Application::getGlobalFocusChangeEvent()->subscribe(badge_screen_of);
}

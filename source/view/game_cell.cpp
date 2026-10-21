// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/game_cell.hpp"

#include "action/game_icons.hpp"
#include "ui/cadran_kit.hpp"

GameCell::GameCell(bool with_icon)
{
    if (!with_icon) return;
    this->slot = new brls::Box();
    this->slot->setWidth(44);
    this->slot->setHeight(44);
    this->slot->setMarginRight(16);
    this->slot->setShrink(0);
    this->icon = new brls::Image();
    this->icon->setWidth(44);
    this->icon->setHeight(44);
    this->icon->setCornerRadius(8);
    this->icon->setScalingType(brls::ImageScalingType::FIT);
    // Until the icon arrives (or for a game without one): the placeholder.
    if (const int waiting = game_icons::placeholder()) {
        this->icon->innerSetImage(waiting);
        this->icon->setFreeTexture(false);
    }
    this->slot->addView(this->icon);
    this->addView(this->slot, 0);   // before the title
}

void GameCell::set_badge(const char* icon_name, const char* color_key)
{
    if (!this->slot) return;
    if (this->badge) {
        this->slot->removeView(this->badge);
        this->badge = nullptr;
    }
    if (!icon_name) return;
    brls::Theme theme = brls::Application::getTheme();
    this->badge = new brls::Box();
    this->badge->setPositionType(brls::PositionType::ABSOLUTE);
    this->badge->setPositionRight(-6);
    this->badge->setPositionBottom(-6);
    this->badge->setWidth(22);
    this->badge->setHeight(22);
    this->badge->setCornerRadius(11);
    this->badge->setBackgroundColor(theme["cadran/surf2"]);
    this->badge->setBorderColor(theme[color_key]);
    this->badge->setBorderThickness(1.5f);
    this->badge->setAlignItems(brls::AlignItems::CENTER);
    this->badge->setJustifyContent(brls::JustifyContent::CENTER);
    this->badge->addView(cadran::kit::icon(icon_name, 13, color_key));
    this->slot->addView(this->badge);
}

void GameCell::set_icon_texture(int texture, bool owned)
{
    if (!this->icon || texture == 0) return;
    // The one shown before is freed only if it was this cell's own: the flag
    // still says so while it is replaced.
    this->icon->innerSetImage(texture);
    this->icon->setFreeTexture(owned);
}

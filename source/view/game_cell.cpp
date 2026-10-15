// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/game_cell.hpp"

GameCell::GameCell(bool with_icon)
{
    if (!with_icon) return;
    this->icon = new brls::Image();
    this->icon->setWidth(44);
    this->icon->setHeight(44);
    this->icon->setCornerRadius(8);
    this->icon->setMarginRight(16);
    this->icon->setScalingType(brls::ImageScalingType::FIT);
    this->addView(this->icon, 0);   // before the title
}

void GameCell::set_icon_texture(int texture, bool owned)
{
    if (!this->icon || texture == 0) return;
    // The one shown before is freed only if it was this cell's own: the flag
    // still says so while it is replaced.
    this->icon->innerSetImage(texture);
    this->icon->setFreeTexture(owned);
}

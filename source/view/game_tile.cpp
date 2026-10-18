// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/game_tile.hpp"

#include "ui/ui.hpp"

namespace
{
constexpr float MARGIN = 8;
constexpr float PADDING = 8;
constexpr float COVER = GameTile::WIDTH - 2 * MARGIN - 2 * PADDING;   // 144
}   // namespace

GameTile::GameTile()
{
    this->setAxis(brls::Axis::COLUMN);
    this->setAlignItems(brls::AlignItems::STRETCH);
    this->setWidth(WIDTH - 2 * MARGIN);
    this->setMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    this->setPadding(PADDING, PADDING, PADDING, PADDING);
    this->setCornerRadius(10);
    this->setHighlightCornerRadius(12);
    this->setFocusable(true);

    this->icon = new brls::Image();
    this->icon->setWidth(COVER);
    this->icon->setHeight(COVER);
    this->icon->setCornerRadius(8);
    this->icon->setBackgroundColor(ui::color_track());   // until the icon arrives
    this->icon->setScalingType(brls::ImageScalingType::FIT);
    this->addView(this->icon);

    this->name = new brls::Label();
    this->name->setFontSize(17);
    this->name->setSingleLine(true);
    this->name->setMarginTop(8);
    this->addView(this->name);

    this->detail = new brls::Label();
    this->detail->setFontSize(15);
    this->detail->setSingleLine(true);
    this->detail->setTextColor(ui::color_note());
    this->detail->setMarginTop(2);
    this->addView(this->detail);
}

void GameTile::set_name(const std::string& text)
{
    this->name->setText(text);
}

void GameTile::set_detail(const std::string& text)
{
    this->detail->setText(text);
}

void GameTile::set_name_color(NVGcolor color)
{
    this->name->setTextColor(color);
}

void GameTile::set_detail_color(NVGcolor color)
{
    this->detail->setTextColor(color);
}

void GameTile::set_icon_texture(int texture, bool owned)
{
    if (texture == 0) return;
    // The one shown before is freed only if it was this tile's own: the flag
    // still says so while it is replaced.
    this->icon->innerSetImage(texture);
    this->icon->setFreeTexture(owned);
    this->iconed = true;
}

void GameTile::clear_icon()
{
    if (!this->iconed) return;
    this->icon->clear();
    this->iconed = false;
}

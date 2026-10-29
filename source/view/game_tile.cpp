// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/game_tile.hpp"

#include <algorithm>

#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/ui/effects.hpp>
#include <aurore/cadran/ui/motion.hpp>

#include "action/game_icons.hpp"
#include "ui/ui.hpp"

namespace
{
constexpr float MARGIN = 8;
constexpr float FADE_US = 180000.0f;   // a cover's arrival
constexpr float PADDING = 8;
constexpr float COVER = GameTile::WIDTH - 2 * MARGIN - 2 * PADDING;   // 144
}   // namespace

GameTile::GameTile()
{
    this->setAxis(aurore::Axis::COLUMN);
    this->setAlignItems(aurore::AlignItems::STRETCH);
    this->setWidth(WIDTH - 2 * MARGIN);
    this->setMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    this->setPadding(PADDING, PADDING, PADDING, PADDING);
    this->setCornerRadius(10);
    this->setHighlightCornerRadius(12);
    this->setFocusable(true);

    this->icon = new aurore::Image();
    this->icon->setWidth(COVER);
    this->icon->setHeight(COVER);
    this->icon->setCornerRadius(8);
    this->icon->setThemeColor("backgroundColor", "cadran/well");
    // Until the icon arrives (or for a game without one): the placeholder,
    // its own size in the middle of the cover.
    if (const int waiting = game_icons::placeholder()) {
        this->icon->setScalingType(aurore::ImageScalingType::CENTER);
        this->icon->innerSetImage(waiting);
        this->icon->setFreeTexture(false);
    }
    this->addView(this->icon);

    this->name = new aurore::Label();
    this->name->setFontSize(17);
    this->name->setSingleLine(true);
    this->name->setMarginTop(8);
    this->addView(this->name);

    this->detail = new aurore::Label();
    this->detail->setFontSize(15);
    this->detail->setSingleLine(true);
    this->detail->setThemeColor("textColor", "cadran/text3");
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

void GameTile::set_name_color(const char* key)
{
    this->name->setThemeColor("textColor", key);
}

void GameTile::set_detail_color(const char* key)
{
    this->detail->setThemeColor("textColor", key);
}

void GameTile::set_icon_texture(int texture, bool owned)
{
    if (texture == 0) return;
    // The one shown before is freed only if it was this tile's own: the flag
    // still says so while it is replaced.
    this->icon->setScalingType(aurore::ImageScalingType::FIT);
    this->icon->innerSetImage(texture);
    this->icon->setFreeTexture(owned);
    // Arriving in place of the placeholder: the cover fades in.
    if (!this->iconed && effects::motion()) {
        this->icon_since = motion::now();
        this->icon->setAlpha(0.02f);
    }
    this->iconed = true;
}

void GameTile::draw(NVGcontext* vg, float x, float y, float width, float height, aurore::Style style,
                    aurore::FrameContext* ctx)
{
    if (this->icon_since) {
        const float t = motion::progress(this->icon_since, FADE_US);
        this->icon->setAlpha(std::max(0.02f, motion::ease_out(t)));
        if (t >= 1.0f) this->icon_since = 0;
    }
    aurore::Box::draw(vg, x, y, width, height, style, ctx);
}

void GameTile::clear_icon()
{
    if (!this->iconed) return;
    this->icon->clear();
    this->icon->setAlpha(1.0f);
    this->icon_since = 0;
    this->iconed = false;
}

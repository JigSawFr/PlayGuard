// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/tab_badge.hpp"

#include <string>

#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"

static constexpr float SIZE = 26;
static constexpr const char* BADGE_ID = "tab_badge";

TabBadge::TabBadge()
{
    this->setId(BADGE_ID);
    this->setFocusable(false);
    this->setWidth(SIZE);
    this->setHeight(SIZE);
    this->setAlignSelf(brls::AlignSelf::CENTER);
    this->setMarginRight(16);
    this->setVisibility(brls::Visibility::GONE);
}

void TabBadge::set_count(int count)
{
    if (count == this->count) return;
    this->count = count;
    this->setVisibility(count > 0 ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

void TabBadge::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                    brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    if (this->count <= 0) return;
    const std::string n = this->count > 9 ? "9+" : std::to_string(this->count);
    const float r = height * 0.5f, cx = x + width * 0.5f, cy = y + height * 0.5f;
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, r);
    nvgFillColor(vg, cadran::kit::color("cadran/amber"));
    nvgFill(vg);
    nvgFontFaceId(vg, brls::Application::getDefaultFont());
    nvgFontSize(vg, r * 1.3f);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, brls::Application::getTheme()["brls/background"]);
    nvgText(vg, cx, cy + r * 0.06f, n.c_str(), nullptr);
}

TabBadge* TabBadge::on_row(brls::Box* row)
{
    if (!row) return nullptr;
    if (auto* badge = dynamic_cast<TabBadge*>(row->getView(BADGE_ID))) return badge;
    auto* badge = new TabBadge();
    // Last, so it lines up at the right whatever the value (an empty one
    // still takes room).
    badge->setMarginLeft(12);
    badge->setMarginRight(0);
    row->addView(badge);
    return badge;
}

TabBadge* TabBadge::of(brls::SidebarItem* item)
{
    if (!item) return nullptr;
    if (auto* badge = dynamic_cast<TabBadge*>(item->getView(BADGE_ID))) return badge;
    auto* badge = new TabBadge();
    item->addView(badge);
    return badge;
}

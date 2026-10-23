// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/tab_badge.hpp"

#include <string>

#include "ui/cadran_kit.hpp"
#include "ui/effects.hpp"
#include "ui/motion.hpp"
#include "ui/ui.hpp"

static constexpr float SIZE = 26;
static constexpr const char* BADGE_ID = "tab_badge";
static constexpr float POP_US = 380000.0f;

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

void TabBadge::set_count(int n)
{
    if (n == this->count) return;
    // A number that comes or changes pops in (one that goes takes the badge
    // with it).
    if (n > 0 && effects::motion()) this->pop_since = motion::now();
    this->count = n;
    this->setVisibility(n > 0 ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

void TabBadge::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                    brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    if (this->count <= 0) return;
    const std::string n = this->count > 9 ? "9+" : std::to_string(this->count);
    const float r = height * 0.5f, cx = x + width * 0.5f, cy = y + height * 0.5f;
    nvgSave(vg);
    if (this->pop_since) {
        const float t = motion::progress(this->pop_since, POP_US);
        if (t >= 1.0f) {
            this->pop_since = 0;
        } else {
            const float scale = motion::back_out(t);
            nvgTranslate(vg, cx, cy);
            nvgScale(vg, scale, scale);
            nvgTranslate(vg, -cx, -cy);
        }
    }
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, r);
    nvgFillColor(vg, cadran::kit::color("cadran/amber"));
    nvgFill(vg);
    nvgFontFaceId(vg, brls::Application::getDefaultFont());
    nvgFontSize(vg, r * 1.3f);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, brls::Application::getTheme()["brls/background"]);
    nvgText(vg, cx, cy + r * 0.06f, n.c_str(), nullptr);
    nvgRestore(vg);
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

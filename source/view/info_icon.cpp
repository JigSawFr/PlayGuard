// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/info_icon.hpp"

#include "ui/ui.hpp"

static constexpr float SIZE = 26;
static constexpr const char* ICON_ID = "info_icon";

InfoIcon::InfoIcon()
{
    this->setId(ICON_ID);
    this->setFocusable(false);
    this->setWidth(SIZE);
    this->setHeight(SIZE);
    this->setAlignSelf(brls::AlignSelf::CENTER);
    this->setMarginRight(12);
    this->setVisibility(brls::Visibility::GONE);
}

void InfoIcon::set_shown(bool shown)
{
    this->setVisibility(shown ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

void InfoIcon::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                    brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    // In the value's colour: something to read, not something wrong.
    const float r = height * 0.5f, cx = x + width * 0.5f, cy = y + height * 0.5f;
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, r);
    nvgFillColor(vg, ui::color_neutral());
    nvgFill(vg);
    nvgFontFaceId(vg, brls::Application::getDefaultFont());
    nvgFontSize(vg, r * 1.4f);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, brls::Application::getTheme()["brls/background"]);
    nvgText(vg, cx, cy + r * 0.06f, "i", nullptr);
}

InfoIcon* InfoIcon::on_row(brls::Box* row)
{
    if (!row) return nullptr;
    if (auto* icon = dynamic_cast<InfoIcon*>(row->getView(ICON_ID))) return icon;
    auto* icon = new InfoIcon();
    // After the title (which grows), so it sits right before the value.
    row->addView(icon, 1);
    return icon;
}

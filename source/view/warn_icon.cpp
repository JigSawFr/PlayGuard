// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/warn_icon.hpp"

#include "ui/ui.hpp"

static constexpr float SIZE = 26;
static constexpr const char* ICON_ID = "warn_icon";

WarnIcon::WarnIcon()
{
    this->setId(ICON_ID);
    this->setFocusable(false);
    this->setWidth(SIZE);
    this->setHeight(SIZE);
    this->setAlignSelf(brls::AlignSelf::CENTER);
    this->setMarginRight(12);
    this->setVisibility(brls::Visibility::GONE);
}

void WarnIcon::set_shown(bool shown)
{
    this->setVisibility(shown ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

void WarnIcon::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                    brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    // Rounded corners: a stroke of the same colour along the filled triangle.
    const float inset = 2.5f;
    nvgBeginPath(vg);
    nvgMoveTo(vg, x + width * 0.5f, y + inset);
    nvgLineTo(vg, x + width - inset, y + height - inset);
    nvgLineTo(vg, x + inset, y + height - inset);
    nvgClosePath(vg);
    nvgLineJoin(vg, NVG_ROUND);
    nvgStrokeWidth(vg, inset * 2);
    nvgStrokeColor(vg, ui::color_warn());
    nvgStroke(vg);
    nvgFillColor(vg, ui::color_warn());
    nvgFill(vg);
    nvgFontFaceId(vg, brls::Application::getDefaultFont());
    nvgFontSize(vg, height * 0.72f);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, brls::Application::getTheme()["brls/background"]);
    nvgText(vg, x + width * 0.5f, y + height * 0.62f, "!", nullptr);
}

WarnIcon* WarnIcon::on_row(brls::Box* row)
{
    if (!row) return nullptr;
    if (auto* icon = dynamic_cast<WarnIcon*>(row->getView(ICON_ID))) return icon;
    auto* icon = new WarnIcon();
    // After the title (which grows), so it sits right before the value.
    row->addView(icon, 1);
    return icon;
}

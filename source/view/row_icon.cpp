// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/row_icon.hpp"

#include "ui/ui.hpp"

static constexpr float SIZE = 26;
static constexpr const char* ICON_ID = "row_icon";

RowIcon::RowIcon(Kind kind)
    : kind(kind)
{
    this->setId(ICON_ID);
    this->setFocusable(false);
    this->setWidth(SIZE);
    this->setHeight(SIZE);
    this->setAlignSelf(brls::AlignSelf::CENTER);
    this->setMarginRight(12);
    this->setVisibility(brls::Visibility::GONE);
}

void RowIcon::set_shown(bool shown)
{
    this->setVisibility(shown ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

void RowIcon::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                   brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    const float cx = x + width * 0.5f;
    float       text_y;
    nvgBeginPath(vg);
    if (this->kind == Kind::Warning) {
        // Rounded corners: a stroke of the same colour along the filled triangle.
        const float inset = 2.5f;
        nvgMoveTo(vg, cx, y + inset);
        nvgLineTo(vg, x + width - inset, y + height - inset);
        nvgLineTo(vg, x + inset, y + height - inset);
        nvgClosePath(vg);
        nvgLineJoin(vg, NVG_ROUND);
        nvgStrokeWidth(vg, inset * 2);
        nvgStrokeColor(vg, ui::color_warn());
        nvgStroke(vg);
        nvgFillColor(vg, ui::color_warn());
        nvgFontSize(vg, height * 0.72f);
        text_y = y + height * 0.62f;
    } else {
        // In the value's colour: something to read, not something wrong.
        nvgCircle(vg, cx, y + height * 0.5f, height * 0.5f);
        nvgFillColor(vg, ui::color_neutral());
        nvgFontSize(vg, height * 0.7f);
        text_y = y + height * 0.53f;
    }
    nvgFill(vg);
    nvgFontFaceId(vg, brls::Application::getDefaultFont());
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, brls::Application::getTheme()["brls/background"]);
    nvgText(vg, cx, text_y, this->kind == Kind::Warning ? "!" : "i", nullptr);
}

RowIcon* RowIcon::on_row(brls::Box* row, Kind kind)
{
    if (!row) return nullptr;
    if (auto* icon = dynamic_cast<RowIcon*>(row->getView(ICON_ID))) return icon;
    auto* icon = new RowIcon(kind);
    // After the title (which grows), so it sits right before the value.
    row->addView(icon, 1);
    return icon;
}

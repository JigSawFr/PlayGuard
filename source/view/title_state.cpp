// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/title_state.hpp"

#include "action/fw_gate.hpp"
#include "activity/firmware_gate_activity.hpp"
#include "activity/main_activity.hpp"
#include "tab/help_tab.hpp"
#include "ui/ui.hpp"

using namespace brls::literals;

static constexpr float SIZE = 40;
static constexpr const char* READ_ONLY_ID = "title_state_read_only";

// Where the state is undone.
static void open_unlocked()
{
    // The PIN tab, on "Lock again".
    if (MainActivity* main = ui::main_activity()) main->open_tab(ui::tab::security, "sc_relock");
}

static void open_read_only()
{
    // Untested firmware: the screen that chose read-only, where the choice changes.
    if (fw_gate::needed()) {
        brls::Application::pushActivity(new FirmwareGateActivity());
        return;
    }
    // Else the developer switch turned it on: Help & diagnostics, on that switch.
    auto* help = new HelpTab();
    ui::open_screen(help, "playguard/prefs/help"_i18n);
    brls::sync([help]() {
        brls::View* cell = help->getView("hp_dev_read_only");
        if (cell && cell->getVisibility() == brls::Visibility::VISIBLE) brls::Application::giveFocus(cell);
    });
}

TitleState::TitleState()
{
    this->setId(READ_ONLY_ID);
    this->setFocusable(false);
    this->setWidth(SIZE);
    this->setHeight(SIZE);
    this->setAlignSelf(brls::AlignSelf::CENTER);
    this->setMarginLeft(14);
    // A tap goes where it is undone (hitTest finds it, focusable or not).
    this->addGestureRecognizer(new brls::TapGestureRecognizer(this, []() { open(Kind::READ_ONLY); }));
}

void TitleState::open(Kind kind)
{
    if (kind == Kind::UNLOCKED) open_unlocked();
    else open_read_only();
}

void TitleState::show(brls::AppletFrame* frame, bool shown)
{
    auto* label = frame ? frame->getView("brls/applet_frame/title_label") : nullptr;
    auto* row   = label ? dynamic_cast<brls::Box*>(label->getParent()) : nullptr;
    if (!row) return;
    brls::View* icon = row->getView(READ_ONLY_ID);
    if (!icon) row->addView(icon = new TitleState());
    icon->setVisibility(shown ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

brls::View* TitleState::hitTest(brls::Point point)
{
    // brls::View::hitTest skips a view that cannot take the focus.
    if (this->getAlpha() == 0.0f || this->getVisibility() != brls::Visibility::VISIBLE) return nullptr;
    return this->getFrame().pointInside(point) ? this : nullptr;
}

void TitleState::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                      brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    this->draw_read_only(vg, x, y, width, height);
}

void TitleState::draw_read_only(NVGcontext* vg, float x, float y, float width, float height)
{
    // A pencil (writing) crossed out, in the header's text colour.
    const NVGcolor ink = ui::color_text();
    const float cx = x + width / 2, cy = y + height / 2;
    nvgSave(vg);
    nvgTranslate(vg, cx, cy);
    nvgRotate(vg, NVG_PI / 4);
    const float len = height * 0.72f, w = height * 0.2f, tip = height * 0.18f;
    // The body, then the tip pointing down-left once rotated.
    nvgBeginPath(vg);
    nvgRect(vg, -w / 2, -len / 2, w, len - tip);
    nvgMoveTo(vg, -w / 2, len / 2 - tip);
    nvgLineTo(vg, w / 2, len / 2 - tip);
    nvgLineTo(vg, 0, len / 2);
    nvgClosePath(vg);
    nvgFillColor(vg, ink);
    nvgFill(vg);
    nvgRestore(vg);

    // The slash, the other way across, with the background on each side so it
    // reads over the pencil.
    const float s = height * 0.36f;
    nvgBeginPath(vg);
    nvgMoveTo(vg, cx - s, cy - s);
    nvgLineTo(vg, cx + s, cy + s);
    nvgLineCap(vg, NVG_ROUND);
    nvgStrokeColor(vg, brls::Application::getTheme()["brls/background"]);
    nvgStrokeWidth(vg, height * 0.16f);
    nvgStroke(vg);
    nvgStrokeColor(vg, ink);
    nvgStrokeWidth(vg, height * 0.07f);
    nvgStroke(vg);
}

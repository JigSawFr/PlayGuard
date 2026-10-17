// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/notice_bell.hpp"

#include <string>

#include "action/notices.hpp"
#include "activity/notices_activity.hpp"
#include "ui/ui.hpp"

static constexpr float SIZE = 40;
static constexpr const char* BELL_ID = "notice_bell";

NoticeBell::NoticeBell()
{
    this->setId(BELL_ID);
    this->setFocusable(false);
    this->setWidth(SIZE);
    this->setHeight(SIZE);
    this->setAlignSelf(brls::AlignSelf::CENTER);
    this->setMarginRight(20);
    // A tap opens the list, as Y does (hitTest finds it, focusable or not).
    this->addGestureRecognizer(new brls::TapGestureRecognizer(this, []() { NoticesActivity::open(); }));
    this->listener = notices::listen([this]() { this->update(); });
    this->update();
}

NoticeBell::~NoticeBell()
{
    notices::unlisten(this->listener);
}

brls::View* NoticeBell::hitTest(brls::Point point)
{
    // brls::View::hitTest skips a view that cannot take the focus.
    if (this->getAlpha() == 0.0f || this->getVisibility() != brls::Visibility::VISIBLE) return nullptr;
    return this->getFrame().pointInside(point) ? this : nullptr;
}

void NoticeBell::update()
{
    this->actions = notices::action_count();
    this->infos   = notices::info_count();
}

void NoticeBell::draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
                      brls::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    // The bell: a dome over a flared rim, and the clapper under it, in the
    // header's text colour (the notes' grey while nothing is there).
    const NVGcolor ink = this->actions || this->infos ? ui::color_text() : ui::color_note();
    const float cx = x + width * 0.42f, top = y + height * 0.14f;
    const float w = width * 0.62f, h = height * 0.6f;
    nvgBeginPath(vg);
    nvgMoveTo(vg, cx - w * 0.5f, top + h);
    nvgBezierTo(vg, cx - w * 0.38f, top + h * 0.78f, cx - w * 0.38f, top + h * 0.55f, cx - w * 0.36f, top + h * 0.42f);
    nvgBezierTo(vg, cx - w * 0.32f, top + h * 0.08f, cx - w * 0.12f, top, cx, top);
    nvgBezierTo(vg, cx + w * 0.12f, top, cx + w * 0.32f, top + h * 0.08f, cx + w * 0.36f, top + h * 0.42f);
    nvgBezierTo(vg, cx + w * 0.38f, top + h * 0.55f, cx + w * 0.38f, top + h * 0.78f, cx + w * 0.5f, top + h);
    nvgClosePath(vg);
    nvgFillColor(vg, ink);
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgCircle(vg, cx, top + h + height * 0.08f, height * 0.07f);
    nvgFill(vg);

    // What it holds: the number to do on an amber disc, else a dot for
    // information alone.
    const float bx = x + width * 0.76f, by = y + height * 0.24f;
    if (this->actions > 0) {
        const std::string n = this->actions > 9 ? "9+" : std::to_string(this->actions);
        const float r = height * 0.24f;
        nvgBeginPath(vg);
        nvgCircle(vg, bx, by, r);
        nvgFillColor(vg, ui::color_warn());
        nvgFill(vg);
        nvgFontFaceId(vg, brls::Application::getDefaultFont());
        nvgFontSize(vg, r * 1.45f);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, brls::Application::getTheme()["brls/background"]);
        nvgText(vg, bx, by + r * 0.06f, n.c_str(), nullptr);
    } else if (this->infos > 0) {
        nvgBeginPath(vg);
        nvgCircle(vg, bx, by, height * 0.12f);
        nvgFillColor(vg, ui::color_note());
        nvgFill(vg);
    }
}

// The main screen (its tab frame) holding `focus`: the bell goes first in
// the version badge's row, unless it has one.
static void bell_screen_of(brls::View* focus)
{
    if (!focus) return;
    while (focus->hasParent()) focus = focus->getParent();
    auto* frame = dynamic_cast<brls::AppletFrame*>(focus);
    if (!frame || !frame->getView("main_tabs")) return;
    auto* badge = dynamic_cast<brls::Box*>(frame->getView("version_badge"));
    if (!badge || badge->getView(BELL_ID)) return;
    badge->addView(new NoticeBell(), 0);
}

void NoticeBell::install()
{
    brls::Application::getGlobalFocusChangeEvent()->subscribe(bell_screen_of);
}

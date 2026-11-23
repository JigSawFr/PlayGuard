// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/notice_bell.hpp"

#include <charconv>
#include <cmath>
#include <cstring>

#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/ui/effects.hpp>
#include <aurore/cadran/ui/motion.hpp>
#include <aurore/cadran/view/header_settings.hpp>
#include <aurore/cadran/view/version_badge.hpp>

#include "action/notices.hpp"
#include "ui/app_hooks.hpp"
#include "ui/play_ui.hpp"

static constexpr float SIZE = 40;
static constexpr const char* BELL_ID = "notice_bell";
// The swing of a new entry (µs), and the pop of a changed number.
static constexpr float SWING_US = 700000.0f;
static constexpr float POP_US   = 380000.0f;

NoticeBell::NoticeBell()
{
    this->setId(BELL_ID);
    this->setFocusable(false);
    this->setWidth(SIZE);
    this->setHeight(SIZE);
    this->setAlignSelf(aurore::AlignSelf::CENTER);
    // A tap opens the list, as Y does (hitTest finds it, focusable or not).
    this->addGestureRecognizer(new aurore::TapGestureRecognizer(this, []() {
        if (ui::app_hooks::get().open_notices) ui::app_hooks::get().open_notices();
    }));
    this->listener = notices::listen([this]() { this->update(); });
    this->update();
}

NoticeBell::~NoticeBell()
{
    notices::unlisten(this->listener);
}

aurore::View* NoticeBell::hitTest(aurore::Point point)
{
    // aurore::View::hitTest skips a view that cannot take the focus.
    if (this->getAlpha() == 0.0f || this->getVisibility() != aurore::Visibility::VISIBLE) return nullptr;
    return this->getFrame().pointInside(point) ? this : nullptr;
}

void NoticeBell::update()
{
    // Everything listed counts, to do or for information (the parent marks
    // the latter as read); the disc is amber while something is to do.
    const int listed = notices::bell_count();
    const int to_do = notices::action_count() + notices::others_count();
    // Something new rings the bell once; a number that changed pops (not the
    // first reading: the bell is drawn as it is).
    if (this->counted && effects::motion()) {
        if (listed > this->total) this->swing_since = motion::now();
        if (listed != this->total) this->pop_since = motion::now();
    }
    this->total   = listed;
    this->actions = to_do;
    this->counted = true;
    if (listed > 9) {
        std::memcpy(this->label, "9+", 3);
    } else {
        const auto end = std::to_chars(this->label, this->label + sizeof(this->label) - 1, listed < 0 ? 0 : listed).ptr;
        *end = '\0';
    }
}

void NoticeBell::draw(NVGcontext* vg, float x, float y, float width, float height, aurore::Style style,
                      aurore::FrameContext* ctx)
{
    (void)style;
    (void)ctx;
    // The bell: a dome over a flared rim, and the clapper under it, in the
    // header's text colour (the notes' grey while nothing is there).
    const NVGcolor ink = this->total > 0 ? cadran::kit::color("cadran/text") : cadran::kit::color("cadran/text3");
    const float cx = x + width * 0.42f, top = y + height * 0.14f;
    const float w = width * 0.62f, h = height * 0.6f;
    // Swinging: the bell turns about its crown, less and less.
    nvgSave(vg);
    if (this->swing_since) {
        const float t = motion::progress(this->swing_since, SWING_US);
        if (t >= 1.0f) {
            this->swing_since = 0;
        } else {
            const float angle = 0.21f * std::sin(t * 3.0f * NVG_PI) * (1.0f - t);   // 12° at most
            nvgTranslate(vg, cx, top);
            nvgRotate(vg, angle);
            nvgTranslate(vg, -cx, -top);
        }
    }
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
    nvgRestore(vg);

    // What it holds: the number of entries on a disc, amber while one is
    // to do, grey for information alone. A changed number pops in.
    const float bx = x + width * 0.76f, by = y + height * 0.24f;
    nvgSave(vg);
    if (this->pop_since) {
        const float t = motion::progress(this->pop_since, POP_US);
        if (t >= 1.0f) {
            this->pop_since = 0;
        } else {
            const float scale = motion::back_out(t);
            nvgTranslate(vg, bx, by);
            nvgScale(vg, scale, scale);
            nvgTranslate(vg, -bx, -by);
        }
    }
    if (this->total > 0) {
        const float r = height * 0.24f;
        nvgBeginPath(vg);
        nvgCircle(vg, bx, by, r);
        nvgFillColor(vg, cadran::kit::color(this->actions > 0 ? "cadran/amber" : "cadran/text3"));
        nvgFill(vg);
        nvgFontFaceId(vg, aurore::Application::getDefaultFont());
        nvgFontSize(vg, r * 1.45f);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, aurore::Application::getTheme()["aurore/background"]);
        nvgText(vg, bx, by + r * 0.06f, this->label, nullptr);
    }
    nvgRestore(vg);
}

// The main screen (its tab frame) holding `focus`: the bell goes in the
// header's row at the right, after the language, theme and text size
// (Aurore's, which may be there first), unless it has one.
static void bell_screen_of(aurore::View* focus)
{
    if (!focus) return;
    while (focus->hasParent()) focus = focus->getParent();
    auto* frame = dynamic_cast<aurore::AppletFrame*>(focus);
    if (!frame || !frame->getView("main_tabs")) return;
    auto* tools = VersionBadge::header_tools(frame);
    if (!tools || tools->getView(BELL_ID)) return;
    size_t at = 0;
    for (size_t i = 0; i < tools->getChildren().size(); i++)
        if (dynamic_cast<HeaderSetting*>(tools->getChildren()[i])) at = i + 1;
    tools->addView(new NoticeBell(), at);
}

void NoticeBell::install()
{
    aurore::Application::getGlobalFocusChangeEvent()->subscribe(bell_screen_of);
}

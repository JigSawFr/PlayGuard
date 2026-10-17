// ui — dialogs, confirmations, a web page's QR code, the dropdown, keyboard prompts and the PIN dialog.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

#include "core/platform.h"
#include "util/config.hpp"
#include "util/duration.hpp"
#include "util/text.hpp"
#include "view/qr_view.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <yoga/Yoga.h>

using namespace brls::literals;

namespace ui
{

void on_cancel(brls::Dialog* dialog, std::function<void()> on_cancel)
{
    dialog->setCancelable(true);
    dialog->getAppletFrame()->registerAction(
        "hints/back"_i18n, brls::BUTTON_B,
        [dialog, on_cancel](brls::View*) {
            dialog->close([on_cancel]() { if (on_cancel) on_cancel(); });
            return true;
        },
        false, false, brls::SOUND_BACK);
}

// A dialog's text has to fit: borealis' Label stops wrapping when the height
// it is offered is below what its wrapped text needs, and draws the whole
// text on one line across the screen instead (label.cpp, labelMeasureFunc).
// So the font size is chosen from the text's real wrapped height, measured
// the way the Label measures it, never from a character count (French and
// German lines hold fewer characters than English ones).

// Height of the dialog's content above its button row (72 px).
static float content_height() { return brls::Application::contentHeight - 72; }

// Wrapped height of `text` at `size` in `width`, as labelMeasureFunc does it.
static float text_height(brls::Label* label, const std::string& text, float size, float width)
{
    NVGcontext* vg = brls::Application::getNVGContext();
    nvgFontSize(vg, size);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgFontFaceId(vg, label->getFont());
    nvgTextLineHeight(vg, label->getLineHeight());
    float bounds[4];
    nvgTextBoxBounds(vg, 0, 0, width, text.c_str(), nullptr, bounds);
    return bounds[3] - bounds[1];
}

// The largest of `sizes` at which `text` fits `height` in `width` (a few
// pixels spare), else the smallest (*fits false then).
static float fitting_size(brls::Label* label, const std::string& text, std::initializer_list<float> sizes,
                          float width, float height, bool* fits)
{
    *fits = true;
    for (float size : sizes)
        if (text_height(label, text, size, width) + 4 <= height) return size;
    *fits = false;
    return *(sizes.end() - 1);
}

static brls::Label* wrapping_label(const std::string& text)
{
    auto* label = new brls::Label();
    label->setText(text);
    label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    label->setSingleLine(false);
    return label;
}

// The smallest font is 18 px: a text still too tall for the dialog scrolls,
// in a frame of `height`, rather than going smaller. The frame holds the
// label (wrapped at its width, its own height unbounded); a finger drags it,
// and the D-pad's up and down scroll it half a frame (scroll_with), the
// buttons keeping the focus.
static brls::ScrollingFrame* scrolling_text(brls::Label* label, float height)
{
    auto* frame = new brls::ScrollingFrame();
    frame->setHeight(height);
    frame->setScrollingBehavior(brls::ScrollingBehavior::NATURAL);
    frame->setContentView(label);
    return frame;
}

static void scroll_with(brls::Dialog* dialog, brls::ScrollingFrame* frame, brls::View* text)
{
    for (const brls::ControllerButton button : { brls::BUTTON_NAV_UP, brls::BUTTON_NAV_DOWN }) {
        const bool down = button == brls::BUTTON_NAV_DOWN;
        dialog->getAppletFrame()->registerAction(
            "", button,
            [frame, text, down](brls::View*) {
                const float page = std::round(frame->getHeight() / 2);
                const float limit = std::max(0.0f, text->getHeight() - frame->getHeight());
                const float target = std::min(std::max(frame->getContentOffsetY() + (down ? page : -page), 0.0f), limit);
                frame->setContentOffsetY(target, true);
                return true;
            },
            true, true, brls::SOUND_NONE);
    }
}

brls::Dialog* dialog(const std::string& text)
{
    // borealis' own layout first (24 px, 65 / 115 px margins, as
    // brls::Dialog(text)); a longer text gets narrower margins and a
    // smaller font, down to 18 px, then scrolls. Never brls::Dialog(text)
    // itself: its label sits in a row, which Yoga may measure at a width of 0.
    auto* label = wrapping_label(text);
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);   // the dialog's width: the label wraps
    box->setJustifyContent(brls::JustifyContent::CENTER);
    const float h = content_height();
    brls::ScrollingFrame* frame = nullptr;
    if (text_height(label, text, 24, 720 - 2 * 115) + 4 <= h - 2 * 65) {
        label->setFontSize(24);
        box->setPadding(65, 115, 65, 115);
        box->addView(label);
    } else {
        bool fits = true;
        label->setFontSize(fitting_size(label, text, { 21, 19, 18 }, 720 - 2 * 40, h - 2 * 28, &fits));
        box->setPadding(28, 40, 28, 40);
        if (fits) box->addView(label);
        else box->addView(frame = scrolling_text(label, h - 2 * 28));
    }
    auto* dialog = new brls::Dialog(box);
    if (frame) scroll_with(dialog, frame, label);
    return dialog;
}

// The text and `extra` (a week chart) under it, the font as large as fits
// (20 px down to 18: German, with every note, then the text scrolls),
// `extra` measured first.
static brls::Dialog* dialog_with(const std::string& text, brls::View* extra)
{
    const float width = 720 - 2 * 40;
    extra->setMarginTop(18);
    YGNodeCalculateLayout(extra->getYGNode(), width, YGUndefined, YGDirectionLTR);
    const float extra_h = YGNodeLayoutGetHeight(extra->getYGNode()) + 18;
    auto* label = wrapping_label(text);
    const float text_h = content_height() - 26 - 18 - extra_h;
    bool fits = true;
    label->setFontSize(fitting_size(label, text, { 20, 18 }, width, text_h, &fits));
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(26, 40, 18, 40);
    brls::ScrollingFrame* frame = fits ? nullptr : scrolling_text(label, text_h);
    box->addView(frame ? (brls::View*)frame : label);
    box->addView(extra);
    auto* dialog = new brls::Dialog(box);
    if (frame) scroll_with(dialog, frame, label);
    return dialog;
}

static void open_confirm(const std::string& body, const std::string& confirm_label,
                         std::function<void()> on_yes, std::function<void()> on_no, bool danger,
                         brls::View* extra = nullptr)
{
    auto* dialog = extra ? dialog_with(body, extra) : ui::dialog(body);
    dialog->addButton("hints/cancel"_i18n, [on_no]() { if (on_no) on_no(); });
    dialog->addButton(confirm_label, [on_yes]() { if (on_yes) on_yes(); });
    on_cancel(dialog, on_no);
    if (danger)
        if (auto* button = dynamic_cast<brls::Button*>(dialog->getView("brls/dialog/button2")))
            button->setTextColor(color_bad());
    dialog->open();
}

void confirm(const std::string& body, const std::string& confirm_label,
             std::function<void()> on_yes, std::function<void()> on_no, bool danger)
{
    open_confirm(body, confirm_label, std::move(on_yes), std::move(on_no), danger);
}

void confirm_with(const std::string& body, brls::View* extra, const std::string& confirm_label,
                  std::function<void()> on_yes, std::function<void()> on_no, bool danger)
{
    open_confirm(body, confirm_label, std::move(on_yes), std::move(on_no), danger, extra);
}

void confirm_danger(const std::string& body, const std::string& confirm_label, std::function<void()> on_yes)
{
    open_confirm(body, confirm_label, std::move(on_yes), nullptr, true);
}

void info(const std::string& body)
{
    auto* dialog = ui::dialog(body);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

void qr_dialog(const std::string& body, const std::string& url)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setPadding(20, 32, 10, 32);
    brls::Label* label = text_label(body, 20, color_text());
    label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    label->setAlignSelf(brls::AlignSelf::STRETCH);
    box->addView(label);
    auto* qr = new QrView(url, 240);
    qr->setMarginTop(12);
    qr->setMarginBottom(10);
    box->addView(qr);
    brls::Label* link = text_label(text::short_url(url), 18, color_note());
    link->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    link->setAlignSelf(brls::AlignSelf::STRETCH);
    box->addView(link);
    auto* dialog = new brls::Dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

void pick(const std::string& title, const std::vector<std::string>& values, int selected,
          std::function<void(int)> on_pick)
{
    if (values.empty()) return;
    auto chosen = std::make_shared<int>(-1);
    auto* dropdown = new brls::Dropdown(
        title, values, [chosen](int index) { *chosen = index; },
        selected < 0 ? 0 : selected,
        [chosen, on_pick](int) {
            if (*chosen >= 0 && on_pick) {
                int index = *chosen;
                brls::sync([on_pick, index]() { on_pick(index); });
            }
        });
    brls::Application::pushActivity(new brls::Activity(dropdown));
}

void pick_actions(const std::string& title, const std::vector<Choice>& choices)
{
    std::vector<std::string> labels;
    for (const Choice& c : choices) labels.push_back(c.label);
    pick(title, labels, 0, [choices](int i) {
        if (i >= 0 && i < (int)choices.size()) choices[i].run();
    });
}

void prompt_minutes(const std::string& header, uint16_t current, std::function<void(uint16_t)> on_value)
{
    const std::string guide   = "playguard/numpad/guide"_i18n;
    const std::string initial = duration::format_hm(current == PT_DAY_NOLIMIT ? 60 : current);
    auto handle = [on_value](const std::string& text) {
        uint16_t minutes = 0;
        if (!duration::parse(text, &minutes)) {
            notify(rc_text(NXM_RC_INVALID_ARGUMENT));
            return;
        }
        on_value(minutes);
    };
    // The system number pad, with a ":" key (core/platform.h); borealis' own
    // text input where there is none.
    char out[16] = {};
    switch (platform_numpad(header.c_str(), guide.c_str(), initial.c_str(), 5, out, sizeof(out))) {
        case PLATFORM_INPUT_OK:        handle(out); return;
        case PLATFORM_INPUT_CANCELLED: return;
        case PLATFORM_INPUT_NONE:      break;
    }
    brls::Application::getImeManager()->openForText([handle](std::string text) { handle(text); },
                                                   header, guide, 5, initial);
}

void prompt_text(const std::string& header, const std::string& initial, int max_len,
                 std::function<void(std::string)> on_value)
{
    brls::Application::getImeManager()->openForText(
        [on_value](std::string text) { if (!text.empty()) on_value(text); },
        header, "", max_len, initial);
}

void offer_restart()
{
    auto* dialog = ui::dialog("playguard/common/restart_body"_i18n);
    dialog->addButton("playguard/common/later"_i18n, []() {});
    dialog->addButton("playguard/common/quit"_i18n, []() { brls::Application::quit(); });
    dialog->setCancelable(true);
    dialog->open();
}

namespace
{
template <size_t N>
int index_of(const char* const (&list)[N], const std::string& value)
{
    for (size_t i = 0; i < N; i++)
        if (value == list[i]) return (int)i;
    return 0;
}

// Saves `value` in `field`, then refreshes and asks to restart if it changed.
void choose(std::string& field, const char* value, std::function<void()> after)
{
    const bool changed = field != value;
    field = value;
    save_config();
    if (after) after();
    if (changed) offer_restart();
}
}   // namespace

void pick_language(std::function<void()> after)
{
    std::vector<std::string> labels;
    for (const char* l : config::LANGUAGES) labels.push_back(brls::getStr(std::string("playguard/tools/languages/") + l));
    pick("playguard/tools/language"_i18n, labels, index_of(config::LANGUAGES, config::get().language), [after](int i) {
        choose(config::get().language, config::LANGUAGES[i], after);
    });
}

void pick_theme(std::function<void()> after)
{
    std::vector<std::string> labels;
    for (const char* t : config::THEMES) labels.push_back(brls::getStr(std::string("playguard/tools/themes/") + t));
    pick("playguard/tools/theme"_i18n, labels, index_of(config::THEMES, config::get().theme), [after](int i) {
        choose(config::get().theme, config::THEMES[i], after);
    });
}

std::string language_name()
{
    return brls::getStr("playguard/tools/languages/" + std::string(config::LANGUAGES[index_of(config::LANGUAGES, config::get().language)]));
}

std::string theme_name()
{
    return brls::getStr("playguard/tools/themes/" + std::string(config::THEMES[index_of(config::THEMES, config::get().theme)]));
}

void quick_settings(std::function<void()> after)
{
    const std::vector<std::string> labels = { "playguard/tools/language"_i18n + " · " + language_name(),
                                              "playguard/tools/theme"_i18n + " · " + theme_name() };
    pick("playguard/quick/title"_i18n, labels, 0, [after](int i) {
        if (i == 0) pick_language(after);
        else pick_theme(after);
    });
}

namespace
{
void wipe(char* p, size_t n)
{
    volatile char* b = p;
    while (n--) *b++ = 0;
}
}   // namespace

bool show_pin_dialog(std::function<void()> on_close)
{
    char pin[16];
    Result rc = pctl_get_pin(pin, sizeof(pin));
    brls::Logger::info("pctl_get_pin returned 0x{:08X}", (unsigned)rc);
    if (R_FAILED(rc)) {
        notify_result(rc, "", "playguard/security/show_pin_err"_i18n);
        return false;
    }
    std::string spaced;   // "1 2 3 4": easier to read out and to type
    spaced.reserve(2 * sizeof(pin));   // no reallocation, so no stray copy
    for (const char* c = pin; *c; c++) {
        if (!spaced.empty()) spaced += ' ';
        spaced += *c;
    }
    wipe(pin, sizeof(pin));

    auto* title = new brls::Label();
    title->setText("playguard/security/show_pin_title"_i18n);
    title->setFontSize(22);
    title->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    title->setTextColor(color_note());
    title->setSingleLine(false);
    title->setAlignSelf(brls::AlignSelf::STRETCH);   // the dialog's width, not a guess (dialog())
    auto* digits = new brls::Label();
    digits->setText(spaced);
    digits->setFontSize(56);
    digits->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    digits->setMarginTop(16);
    wipe(&spaced[0], spaced.size());

    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setJustifyContent(brls::JustifyContent::CENTER);
    box->setPadding(40, 40, 40, 40);
    box->addView(title);
    box->addView(digits);

    auto* d = new brls::Dialog(box);
    d->addButton("hints/ok"_i18n, [on_close]() { if (on_close) on_close(); });
    on_cancel(d, on_close);
    d->open();
    return true;
}

}   // namespace ui

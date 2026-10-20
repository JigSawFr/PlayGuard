// ui — dialogs, confirmations, a web page's QR code, the dropdown, keyboard prompts and the PIN dialog.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

#include "action/upload_flow.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/cadran_text.hpp"
#include "ui/text_size.hpp"
#include "util/config.hpp"
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

void add_cancel(brls::Dialog* dialog, std::function<void()> cancel)
{
    dialog->addButton("hints/cancel"_i18n, cancel);
    on_cancel(dialog, cancel);
}

// Height of the dialog's content above its button row (its card keeps a
// margin from the screen's top and bottom).
static float content_height()
{
    return brls::Application::contentHeight - cadran::DIALOG_BUTTONS - 2 * cadran::DIALOG_MARGIN;
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

namespace
{
// A dialog's text that scrolls: its foot fades into the card while more
// follows below the buttons' row.
class DialogScroll : public brls::ScrollingFrame
{
  public:
    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style style,
              brls::FrameContext* ctx) override
    {
        brls::ScrollingFrame::draw(vg, x, y, width, height, style, ctx);
        cadran::kit::draw_more_below(vg, this, x, y, width, height, "cadran/surf");
    }
};
}   // namespace

// The text laid out by cadran::dialog_body, scrolling when taller than the
// dialog's room.
static brls::Dialog* body_dialog(const std::string& text)
{
    brls::Box* body = cadran::dialog_body(text);
    YGNodeCalculateLayout(body->getYGNode(), 720, YGUndefined, YGDirectionLTR);
    const float room = content_height();
    if (YGNodeLayoutGetHeight(body->getYGNode()) <= room) return new brls::Dialog(body);
    auto* frame = new DialogScroll();
    frame->setHeight(room);
    frame->setScrollingBehavior(brls::ScrollingBehavior::NATURAL);
    frame->setContentView(body);
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->addView(frame);
    auto* dialog = new brls::Dialog(box);
    scroll_with(dialog, frame, body);
    return dialog;
}

brls::Dialog* dialog(const std::string& text)
{
    return body_dialog(text);
}

static void open_confirm(const std::string& body, const std::string& confirm_label,
                         std::function<void()> on_yes, std::function<void()> on_no, bool danger)
{
    auto* dialog = body_dialog(body);
    dialog->addButton("hints/cancel"_i18n, [on_no]() { if (on_no) on_no(); });
    dialog->addButton(confirm_label, [on_yes]() { if (on_yes) on_yes(); });
    on_cancel(dialog, on_no);
    cadran::style_dialog(dialog, danger ? cadran::Emphasis::DANGER : cadran::Emphasis::PRIMARY);
    dialog->open();
}

void confirm(const std::string& body, const std::string& confirm_label,
             std::function<void()> on_yes, std::function<void()> on_no, bool danger)
{
    open_confirm(body, confirm_label, std::move(on_yes), std::move(on_no), danger);
}

void confirm_danger(const std::string& body, const std::string& confirm_label, std::function<void()> on_yes)
{
    open_confirm(body, confirm_label, std::move(on_yes), nullptr, true);
}

void show_failure(const cadran_text::Failure& f)
{
    const std::string& title = f.title;
    const std::string& rest = f.reason;
    const std::string& code = f.code;
    const bool report = f.report;
    const std::string body = title.empty() ? rest : rest.empty() ? title : title + "\n\n" + rest;
    auto* dialog = body_dialog(body.empty() ? code : body);
    if (!code.empty() && !body.empty()) {
        if (auto* box = dynamic_cast<brls::Box*>(dialog->getView("cadran_dialog_body"))) {
            brls::Label* small = cadran::kit::label(code, 15, "cadran/text3");
            small->setMarginTop(10);
            box->addView(small);
        }
    }
    if (report) {
        dialog->addButton("playguard/notices/send_report"_i18n, []() { upload_flow::choose(); });
    }
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::FAILURE);
    // A closes it, as before: the report is a choice, not the default.
    if (report)
        if (brls::View* ok = dialog->getView("brls/dialog/button2")) dialog->setLastFocusedView(ok);
    dialog->open();
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
    box->addView(centered_label(body, 20, color_text()));
    auto* qr = new QrView(url, 240);
    qr->setMarginTop(12);
    qr->setMarginBottom(10);
    box->addView(qr);
    box->addView(centered_label(text::short_url(url), 18, color_note()));
    auto* dialog = new brls::Dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

void qr_text_dialog(const std::string& body, const std::string& text)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setPadding(20, 32, 16, 32);
    box->addView(centered_label(body, 20, color_text()));
    auto* qr = new QrView(text, 400);
    qr->setMarginTop(12);
    box->addView(qr);
    auto* dialog = new brls::Dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
}

// borealis' Dropdown with the row `checked` (−1: none) checked, or each row
// `marks` sets (when given), and the focus on `focus`; in the Cadran look, a
// card and its rows (cadran_modal).
class Picker : public brls::Dropdown
{
  public:
    Picker(const std::string& title, const std::vector<std::string>& values, int checked, int focus,
           std::function<void(int)> on_pick, std::vector<std::string> details = {}, std::vector<bool> marks = {})
        : brls::Dropdown(
              title, values, [this](int index) { this->chosen = index; }, focus,
              [this, on_pick](int) {
                  if (this->chosen >= 0 && on_pick) {
                      const int index = this->chosen;
                      brls::sync([on_pick, index]() { on_pick(index); });
                  }
              }),
          labels(values), details(std::move(details)), marks(std::move(marks)), checked(checked)
    {
        // The rows Dropdown's constructor already made used its own
        // cellForRow (this one did not exist yet): set again here.
        this->restyle(this);
        cadran::style_picker(this);
    }

  private:
    std::vector<std::string> labels;
    std::vector<std::string> details;
    std::vector<bool> marks;
    int checked;
    int chosen = -1;

    bool is_checked(size_t index) const
    {
        if (this->marks.empty()) return (int)index == this->checked;
        return index < this->marks.size() && this->marks[index];
    }

    void row(brls::RadioCell* cell, bool on)
    {
        cell->setSelected(on);
        cadran::style_pick_row(cell, on);
    }

    // A row's detail, after its label, quieter (none: no detail at all).
    void detail(brls::RadioCell* cell, size_t index)
    {
        if (this->details.empty()) return;
        auto* text = dynamic_cast<brls::Label*>(cell->getView("pick_detail"));
        if (!text) {
            brls::Theme theme = brls::Application::getTheme();
            text = new brls::Label();
            text->setId("pick_detail");
            text->setFontSize(16);
            text->setSingleLine(true);
            text->setTextColor(theme["cadran/text2"]);
            text->setMarginRight(brls::getStyle()["brls/listitem/descriptionIndent"]);
            cell->addView(text, 1);
        }
        const std::string value = index < this->details.size() ? this->details[index] : "";
        text->setText(value);
        // None for this row: no room taken, its label has the row's width.
        text->setVisibility(value.empty() ? brls::Visibility::GONE : brls::Visibility::VISIBLE);
    }

    void restyle(brls::View* view)
    {
        if (auto* cell = dynamic_cast<brls::RadioCell*>(view)) {
            // A row made before this class existed: found by its label.
            const auto at = std::find(this->labels.begin(), this->labels.end(), cell->title->getFullText());
            const size_t index = (size_t)(at - this->labels.begin());
            if (!this->marks.empty())
                this->row(cell, at != this->labels.end() && this->is_checked(index));
            else
                this->row(cell, this->checked >= 0 && cell->getSelected());
            if (at != this->labels.end()) this->detail(cell, index);
        }
        if (auto* box = dynamic_cast<brls::Box*>(view))
            for (brls::View* child : box->getChildren()) this->restyle(child);
    }

    brls::RecyclerCell* cellForRow(brls::RecyclerFrame* recycler, brls::IndexPath index) override
    {
        auto* cell = (brls::RadioCell*)recycler->dequeueReusableCell("Cell");
        cell->title->setText(this->labels[index.row]);
        this->row(cell, this->is_checked((size_t)index.row));
        this->detail(cell, (size_t)index.row);
        return cell;
    }
};

void pick(const std::string& title, const std::vector<std::string>& values, int selected,
          std::function<void(int)> on_pick, bool checked, const std::vector<std::string>& details)
{
    if (values.empty()) return;
    const int focus = selected < 0 ? 0 : selected;
    auto* list = new Picker(title, values, checked ? focus : -1, focus, std::move(on_pick), details);
    brls::Application::pushActivity(new brls::Activity(list));
}

void pick_marked(const std::string& title, const std::vector<std::string>& values, const std::vector<bool>& marks,
                 int focus, std::function<void(int)> on_pick, const std::vector<std::string>& details)
{
    if (values.empty()) return;
    auto* list = new Picker(title, values, -1, focus < 0 ? 0 : focus, std::move(on_pick), details, marks);
    brls::Application::pushActivity(new brls::Activity(list));
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
    // The stick, not the number pad.
    cadran::prompt_minutes(header, current == PT_DAY_NOLIMIT ? 60 : current, std::move(on_value));
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

void pick_text_size(std::function<void()> after)
{
    std::vector<std::string> labels;
    for (const char* t : config::TEXT_SIZES) labels.push_back(brls::getStr(std::string("playguard/prefs/text_sizes/") + t));
    pick("playguard/prefs/text_size"_i18n, labels, index_of(config::TEXT_SIZES, config::get().text_size), [after](int i) {
        // At once, no restart: every screen is laid out again.
        config::get().text_size = config::TEXT_SIZES[i];
        save_config();
        text_size::apply();
        if (after) after();
    });
}

std::string text_size_name()
{
    return brls::getStr("playguard/prefs/text_sizes/" + std::string(config::TEXT_SIZES[index_of(config::TEXT_SIZES, config::get().text_size)]));
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
                                              "playguard/tools/theme"_i18n + " · " + theme_name(),
                                              "playguard/prefs/text_size"_i18n + " · " + text_size_name() };
    pick("playguard/quick/title_text"_i18n, labels, 0, [after](int i) {
        if (i == 0) pick_language(after);
        else if (i == 1) pick_theme(after);
        else pick_text_size(after);
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

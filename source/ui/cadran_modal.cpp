// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cadran_modal.hpp"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>
#include <yoga/Yoga.h>

#include "ui/cadran_kit.hpp"
#include "ui/cadran_text.hpp"
#include "ui/ui.hpp"
#include "view/cadran_digits.hpp"
#include "view/cadran_icon.hpp"

using namespace brls::literals;

namespace
{
// The pills' looks (theme and style keys, as borealis' own button styles):
// plain on the second surface with a hairline, the confirmation filled.
const brls::ButtonStyle PLAIN = {
    .shadowType              = brls::ShadowType::NONE,
    .hideHighlightBackground = true,
    .highlightPadding        = "",
    .borderThickness         = "cadran/hairline",
    .enabledBackgroundColor  = "cadran/surf2",
    .enabledLabelColor       = "cadran/text",
    .enabledBorderColor      = "cadran/line",
    .disabledBackgroundColor = "cadran/surf2",
    .disabledLabelColor      = "cadran/text3",
    .disabledBorderColor     = "cadran/line",
};
const brls::ButtonStyle PRIMARY = {
    .shadowType              = brls::ShadowType::NONE,
    .hideHighlightBackground = true,
    .highlightPadding        = "",
    .borderThickness         = "",
    .enabledBackgroundColor  = "cadran/teal_fill",
    .enabledLabelColor       = "cadran/navy",
    .enabledBorderColor      = "",
    .disabledBackgroundColor = "cadran/surf2",
    .disabledLabelColor      = "cadran/text3",
    .disabledBorderColor     = "",
};
const brls::ButtonStyle DANGER = {
    .shadowType              = brls::ShadowType::NONE,
    .hideHighlightBackground = true,
    .highlightPadding        = "",
    .borderThickness         = "",
    .enabledBackgroundColor  = "cadran/coral_fill",
    .enabledLabelColor       = "cadran/navy",
    .enabledBorderColor      = "",
    .disabledBackgroundColor = "cadran/surf2",
    .disabledLabelColor      = "cadran/text3",
    .disabledBorderColor     = "",
};

constexpr float PILL = 56;

void pill(brls::Button* button, const brls::ButtonStyle* style)
{
    button->setStyle(style);
    button->setFontSize(21);
    button->setCornerRadius(PILL / 2);
    button->setHighlightCornerRadius(PILL / 2);
}

brls::Button* button(brls::Dialog* dialog, const char* id)
{
    auto* b = dynamic_cast<brls::Button*>(dialog->getView(id));
    return b && b->getVisibility() == brls::Visibility::VISIBLE ? b : nullptr;
}

void hide(brls::View* parent, const char* id)
{
    if (brls::View* v = parent->getView(id)) v->setVisibility(brls::Visibility::GONE);
}
}   // namespace

namespace
{
// A label in the boards' display face (Sora, the other scripts from the
// app's fonts): a dialog's title.
class DisplayLabel : public brls::Label
{
  public:
    DisplayLabel()
    {
        const int face = brls::Application::getFont("cadran/digits");
        if (face != brls::FONT_INVALID) this->font = face;
    }
};

std::string trim(const std::string& text)
{
    const size_t first = text.find_first_not_of(" \n");
    if (first == std::string::npos) return "";
    return text.substr(first, text.find_last_not_of(" \n") - first + 1);
}

std::vector<std::string> split(const std::string& text, const std::string& by)
{
    std::vector<std::string> parts;
    size_t from = 0;
    while (true) {
        const size_t at = text.find(by, from);
        const std::string part = trim(text.substr(from, at == std::string::npos ? std::string::npos : at - from));
        if (!part.empty()) parts.push_back(part);
        if (at == std::string::npos) return parts;
        from = at + by.size();
    }
}

bool starts_with(const std::string& text, const std::string& start)
{
    return !start.empty() && text.compare(0, start.size(), start) == 0;
}

// The question that opens a paragraph ("Add 30 min today?", "Bloquer tous
// les jeux ? …", "…しますか？"), with its mark; "" when it does not open with one.
std::string question_of(const std::string& paragraph)
{
    size_t end = std::string::npos;
    for (const char* mark : { "?", "\xEF\xBC\x9F" }) {   // "?" and the full-width one
        const size_t at = paragraph.find(mark);
        if (at != std::string::npos && at + std::string(mark).size() < end) end = at + std::string(mark).size();
    }
    if (end == std::string::npos || end > 220 || paragraph.substr(0, end).find('\n') != std::string::npos) return "";
    return paragraph.substr(0, end);
}

// A note the app adds to its dialogs, and its icon (nullptr: not one): the
// linked phone app, the help it can give before unlinking, the unlock, a game
// about to be suspended, what the DNS's blocking changes.
const char* note_icon(const std::string& paragraph, bool* warn)
{
    *warn = false;
    if (paragraph == "playguard/pairing/note"_i18n) return "phone";
    if (paragraph == "playguard/pairing/decode_note"_i18n) return "code";
    if (paragraph == "playguard/cadran/today/dns_sync_note"_i18n) return "phone";
    const std::string clock = "playguard/cadran/today/dns_time_note"_i18n;
    if (starts_with(paragraph, clock.substr(0, clock.find("{}")))) return "globe";
    if (starts_with(paragraph, "playguard/play_timer/gate/body"_i18n)) return "unlock";
    const std::string suspend = "playguard/play_timer/suspend_warning"_i18n;
    if (starts_with(paragraph, suspend.substr(0, suspend.find("{}")))) {
        *warn = true;
        return "alert";
    }
    return nullptr;
}
}   // namespace

brls::Box* cadran::dialog_body(const std::string& text)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setId("cadran_dialog_body");
    body->setAlignItems(brls::AlignItems::STRETCH);
    body->setPadding(34, 40, 26, 40);

    std::vector<std::string> paragraphs = split(text, "\n\n");
    // The title: the opening question, else a short first line before more.
    std::string title;
    if (!paragraphs.empty()) {
        title = question_of(paragraphs[0]);
        if (!title.empty()) {
            paragraphs[0] = trim(paragraphs[0].substr(title.size()));
            if (paragraphs[0].empty()) paragraphs.erase(paragraphs.begin());
        } else if (paragraphs.size() > 1 && paragraphs[0].size() <= 90 && paragraphs[0].find('\n') == std::string::npos) {
            title = paragraphs[0];
            paragraphs.erase(paragraphs.begin());
        }
    }
    if (!title.empty()) {
        auto* head = new brls::Box(brls::Axis::ROW);
        head->setId("cadran_dialog_head");
        head->setAlignItems(brls::AlignItems::CENTER);
        auto* label = new DisplayLabel();
        label->setText(title);
        label->setFontSize(23);
        label->setTextColor(theme["cadran/text"]);
        label->setGrow(1.0f);
        label->setWidth(0);
        head->addView(label);
        body->addView(head);
    }

    struct Note
    {
        std::string text;
        const char* icon;
        bool warn;
    };
    std::vector<Note> notes;
    for (const std::string& paragraph : paragraphs) {
        bool warn = false;
        if (const char* icon = note_icon(paragraph, &warn)) {
            // Locked again by itself: the unlock in one line.
            const std::string unlock_auto = "playguard/play_timer/gate/body"_i18n + " " + "playguard/play_timer/gate/relock_auto"_i18n;
            notes.push_back({ paragraph == unlock_auto ? "playguard/cadran/dialog/unlock_short"_i18n : paragraph, icon, warn });
            continue;
        }
        std::vector<std::pair<std::string, std::string>> rows;
        if (cadran_text::table_rows(paragraph, &rows)) {
            // A week of lines: the days that change alike on one line
            // ("Monday – Friday  2 h → 0 min").
            rows = cadran_text::group_rows(rows);
            auto* table = new brls::Box(brls::Axis::COLUMN);
            table->setAlignItems(brls::AlignItems::STRETCH);
            table->setBackgroundColor(theme["cadran/surf2"]);
            table->setCornerRadius(14);
            table->setPadding(12, 20, 12, 20);
            table->setMarginTop(body->getChildren().empty() ? 0 : 18);
            for (size_t i = 0; i < rows.size(); i++) {
                auto* row = new brls::Box(brls::Axis::ROW);
                row->setAlignItems(brls::AlignItems::CENTER);
                row->setPaddingTop(7);
                row->setPaddingBottom(7);
                if (i + 1 < rows.size()) {
                    row->setLineBottom(1);
                    row->setLineColor(theme["cadran/line"]);
                }
                brls::Label* key = kit::label(rows[i].first, 17, "cadran/text2");
                key->setGrow(1.0f);
                key->setWidth(0);
                key->setMarginRight(16);
                row->addView(key);
                brls::Label* value = kit::label(rows[i].second, 17, "cadran/text");
                value->setHorizontalAlign(brls::HorizontalAlign::RIGHT);
                value->setMaxWidth(300);
                value->setShrink(0);
                row->addView(value);
                table->addView(row);
            }
            body->addView(table);
            continue;
        }
        brls::Label* label = kit::label(paragraph, 19, "cadran/text2");
        label->setMarginTop(body->getChildren().empty() ? 0 : 14);
        body->addView(label);
    }
    // A warning first, in an amber card under the title (a game about to be
    // suspended matters before the details); the other notes under a line,
    // each with its icon.
    size_t at = body->getView("cadran_dialog_head") ? 1 : 0;
    for (const Note& note : notes) {
        if (!note.warn) continue;
        auto* card = new brls::Box(brls::Axis::ROW);
        card->setAlignItems(brls::AlignItems::FLEX_START);
        card->setBackgroundColor(theme["cadran/amber_surf"]);
        card->setBorderColor(theme["cadran/amber_line"]);
        card->setBorderThickness(1);
        card->setCornerRadius(14);
        card->setPadding(12, 18, 12, 16);
        card->setMarginTop(at > 0 ? 14 : 0);
        CadranIcon* icon = kit::icon(note.icon, 20, "cadran/amber");
        icon->setMarginTop(2);
        icon->setMarginRight(12);
        icon->setShrink(0);
        card->addView(icon);
        brls::Label* label = kit::label(note.text, 17, "cadran/amber");
        label->setGrow(1.0f);
        label->setWidth(0);
        card->addView(label);
        body->addView(card, at++);
    }
    notes.erase(std::remove_if(notes.begin(), notes.end(), [](const Note& n) { return n.warn; }), notes.end());
    // What follows the warning, apart from it.
    if (at > 0 && at < body->getChildren().size()) body->getChildren()[at]->setMarginTop(14);
    for (size_t i = 0; i < notes.size(); i++) {
        auto* row = new brls::Box(brls::Axis::ROW);
        row->setAlignItems(brls::AlignItems::FLEX_START);
        if (i == 0 && !body->getChildren().empty()) {
            row->setMarginTop(20);
            row->setPaddingTop(16);
            row->setLineTop(1);
            row->setLineColor(theme["cadran/line"]);
        } else if (i > 0) {
            row->setMarginTop(12);
        }
        CadranIcon* icon = kit::icon(notes[i].icon, 18, notes[i].warn ? "cadran/amber" : "cadran/text3");
        icon->setMarginTop(2);
        icon->setMarginRight(12);
        icon->setShrink(0);
        row->addView(icon);
        brls::Label* label = kit::label(notes[i].text, 16, notes[i].warn ? "cadran/amber" : "cadran/text2");
        label->setGrow(1.0f);
        label->setWidth(0);
        row->addView(label);
        body->addView(row);
    }
    // Taller than the dialog's room, the body scrolls (ui::dialog): its lines
    // keep their height there. A label offered less than its wrapped text
    // needs draws one cut line, and borealis caps a label at its parent's
    // height (100 %), which a scrolling body does not settle before it.
    std::function<void(brls::View*)> unbounded = [&unbounded](brls::View* view) {
        view->setShrink(0);
        if (dynamic_cast<brls::Label*>(view)) YGNodeStyleSetMaxHeight(view->getYGNode(), YGUndefined);
        if (auto* box = dynamic_cast<brls::Box*>(view))
            for (brls::View* child : box->getChildren()) unbounded(child);
    };
    for (brls::View* child : body->getChildren()) unbounded(child);
    return body;
}

void cadran::style_dialog(brls::Dialog* dialog, Emphasis emphasis)
{
    if (!dialog || dialog->getView("cadran_dialog")) return;
    brls::Theme theme = brls::Application::getTheme();
    brls::AppletFrame* card = dialog->getAppletFrame();
    card->setCornerRadius(24);
    card->setBackgroundColor(theme["cadran/surf"]);
    card->setBorderColor(theme["cadran/line"]);
    card->setBorderThickness(1);
    auto* mark = new brls::Box();
    mark->setId("cadran_dialog");
    mark->setVisibility(brls::Visibility::GONE);
    card->addView(mark);
    // Something removed or stopped: a coral warning before the title (above
    // the text when it has none).
    auto* head = dynamic_cast<brls::Box*>(dialog->getView("cadran_dialog_head"));
    auto* body = dynamic_cast<brls::Box*>(dialog->getView("cadran_dialog_body"));
    const bool warns = emphasis == Emphasis::DANGER || emphasis == Emphasis::FAILURE;
    if (warns && !head && body) {
        head = new brls::Box(brls::Axis::ROW);
        head->setMarginBottom(14);
        body->addView(head, 0);
    }
    if (warns && head) {
        auto* badge = new brls::Box(brls::Axis::ROW);
        badge->setWidth(44);
        badge->setHeight(44);
        badge->setShrink(0);
        badge->setCornerRadius(22);
        badge->setBackgroundColor(theme["cadran/coral_soft"]);
        badge->setAlignItems(brls::AlignItems::CENTER);
        badge->setJustifyContent(brls::JustifyContent::CENTER);
        badge->setMarginRight(16);
        badge->addView(cadran::kit::icon("alert", 22, "cadran/coral"));
        head->addView(badge, 0);
    }

    // borealis' order: button3 alone above, button1 and button2 in a row.
    brls::Button* b1 = button(dialog, "brls/dialog/button1");
    brls::Button* b2 = button(dialog, "brls/dialog/button2");
    brls::Button* b3 = button(dialog, "brls/dialog/button3");
    brls::Button* last = b3 ? b3 : b2 ? b2 : b1;
    const std::string ok = "hints/ok"_i18n;
    for (brls::Button* b : { b1, b2, b3 }) {
        if (!b) continue;
        const brls::ButtonStyle* look = &PLAIN;
        if (b == last && emphasis == Emphasis::PRIMARY) look = &PRIMARY;
        else if (b == last && emphasis == Emphasis::DANGER) look = &DANGER;
        else if ((emphasis == Emphasis::NONE || emphasis == Emphasis::FAILURE) && b->getText() == ok) look = &PRIMARY;
        pill(b, look);
        // A says what the focused button does ("A Cancel"), not borealis' "OK".
        b->updateActionHint(brls::BUTTON_A, b->getText());
    }
    hide(dialog, "brls/dialog/button2/separator");
    hide(dialog, "brls/dialog/button3/separator");
    if (b2) b2->setMarginLeft(12);
    if (b3) {
        b3->setHeight(PILL);
        b3->setMargins(0, 24, 12, 24);
    }
    if (brls::Box* row = b1 ? b1->getParent() : nullptr) {
        row->setLineTop(0);
        row->setHeight(DIALOG_BUTTONS);
        row->setPadding(0, 24, DIALOG_BUTTONS - PILL, 24);
    }
}

// An arrow repeating more than this many times in a row steps by the hour.
static constexpr int HOLD_TO_HOURS = 5;

void cadran::prompt_minutes(const std::string& header, uint16_t initial, std::function<void(uint16_t)> on_value)
{
    namespace kit = cadran::kit;
    brls::Theme theme = brls::Application::getTheme();
    auto minutes = std::make_shared<int>(std::min<int>(initial, 24 * 60));
    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setAlignItems(brls::AlignItems::CENTER);
    body->setPadding(30, 40, 22, 40);
    auto* title = new DisplayLabel();
    title->setText(header);
    title->setFontSize(23);
    title->setTextColor(theme["cadran/text"]);
    title->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    body->addView(title);
    CadranIcon* up = kit::icon("caret-up", 30, "cadran/teal");
    up->setMarginTop(18);
    body->addView(up);
    auto* value = new CadranDigits();
    value->setFontSize(56);
    value->setSmallUnits(true);
    value->setTextColor(theme["cadran/text"]);
    value->setText(ui::fmt_played((uint16_t)*minutes));
    body->addView(value);
    CadranIcon* down = kit::icon("caret-down", 30, "cadran/teal");
    body->addView(down);
    // The keys, as the week's: the arrows 15 min, L and R 5 min.
    auto* keys = new brls::Box(brls::Axis::ROW);
    keys->setAlignItems(brls::AlignItems::CENTER);
    keys->setMarginTop(16);
    const auto key = [](const char* letter) {
        brls::Box* bubble = kit::key(letter);
        bubble->setMarginRight(4);
        return bubble;
    };
    keys->addView(kit::icon("caret-up", 18, "cadran/text3"));
    keys->addView(kit::icon("caret-down", 18, "cadran/text3"));
    brls::Label* quarter = kit::label(ui::fmt_played(15), 16, "cadran/text3");
    quarter->setMarginLeft(6);
    quarter->setMarginRight(22);
    keys->addView(quarter);
    keys->addView(key("L"));
    keys->addView(key("R"));
    brls::Label* five = kit::label(ui::fmt_played(5), 16, "cadran/text3");
    five->setMarginLeft(4);
    keys->addView(five);
    body->addView(keys);

    auto* dialog = new brls::Dialog(body);
    const auto step = [minutes, value](int delta) {
        *minutes = std::max(0, std::min(24 * 60, *minutes + delta));
        value->setText(ui::fmt_played((uint16_t)*minutes));
    };
    brls::View* frame = dialog->getAppletFrame();
    // An arrow held: 15 min at a time, then whole hours once it has repeated
    // for most of a second (borealis repeats every 100 ms once held): from
    // 30 min to 4 h without fourteen presses.
    auto held = std::make_shared<std::pair<int, brls::Time>>(0, 0);   // repeats in a row, the last one
    for (const auto& [button, delta] : { std::pair<brls::ControllerButton, int>{ brls::BUTTON_NAV_UP, 15 },
                                         { brls::BUTTON_NAV_DOWN, -15 }, { brls::BUTTON_RB, 5 }, { brls::BUTTON_LB, -5 } })
        frame->registerAction("", button, [step, held, minutes, delta = delta](brls::View*) {
            const brls::Time now = brls::getCPUTimeUsec();
            const bool repeat = now - held->second < 150000 && (held->first > 0) == (delta > 0);
            held->first = repeat ? held->first + (delta > 0 ? 1 : -1) : (delta > 0 ? 1 : -1);
            held->second = now;
            if (std::abs(delta) == 15 && std::abs(held->first) > HOLD_TO_HOURS) {
                // To the next whole hour that way.
                const int hour = delta > 0 ? (*minutes / 60 + 1) * 60 : ((*minutes + 59) / 60 - 1) * 60;
                step(hour - *minutes);
            } else {
                step(delta);
            }
            return true;
        }, true, true, brls::SOUND_FOCUS_CHANGE);
    up->addGestureRecognizer(new brls::TapGestureRecognizer(up, [step]() { step(15); }));
    down->addGestureRecognizer(new brls::TapGestureRecognizer(down, [step]() { step(-15); }));
    dialog->addButton("hints/cancel"_i18n, []() {});
    dialog->addButton("hints/ok"_i18n, [minutes, on_value]() {
        if (on_value) on_value((uint16_t)*minutes);
    });
    dialog->setCancelable(true);
    style_dialog(dialog, Emphasis::PRIMARY);
    // A value is set, then confirmed: A right after the arrows is OK, not Cancel.
    if (brls::View* ok = dialog->getView("brls/dialog/button2")) dialog->setLastFocusedView(ok);
    dialog->open();
}

brls::View* cadran::toast_icon(bool done)
{
    CadranIcon* icon = cadran::kit::icon(done ? "check" : "alert", 22, done ? "cadran/teal_fill" : "cadran/amber_fill");
    icon->setMarginRight(14);
    icon->setShrink(0);
    return icon;
}

void cadran::style_picker(brls::Dropdown* dropdown)
{
    brls::Theme theme = brls::Application::getTheme();
    if (brls::View* content = dropdown->getView("brls/dropdown/content")) {
        // 880 px wide, 20 px above the hints.
        content->setMarginLeft(200);
        content->setMarginRight(200);
        content->setMarginBottom(20);
        content->setCornerRadius(24);
        content->setBackgroundColor(theme["cadran/surf"]);
        content->setBorderColor(theme["cadran/line"]);
        content->setBorderThickness(1);
    }
    if (auto* header = dynamic_cast<brls::Box*>(dropdown->getView("brls/dropdown/header"))) {
        header->setMarginLeft(0);
        header->setMarginRight(0);
        header->setPaddingLeft(36);
        header->setPaddingRight(36);
    }
    if (auto* title = dynamic_cast<brls::Label*>(dropdown->getView("brls/dropdown/title_label"))) {
        title->setFontSize(22);
        title->setTextColor(theme["cadran/text"]);
    }
}

void cadran::style_pick_row(brls::RadioCell* cell, bool checked)
{
    brls::Theme theme = brls::Application::getTheme();
    cell->setCornerRadius(14);
    cell->setHighlightCornerRadius(14);
    // The row chosen: a soft fill and its check, the text as the others'
    // (borealis gives the focused row the teal).
    cell->setBackgroundColor(checked ? theme["cadran/teal_soft"] : theme["cadran/none"]);
    cell->title->setTextColor(theme["cadran/text"]);
    cell->checkbox->setVisibility(brls::Visibility::GONE);
    auto* check = dynamic_cast<CadranIcon*>(cell->getView("cadran_check"));
    if (!check) {
        check = new CadranIcon();
        check->setId("cadran_check");
        check->setIcon("check");
        check->setWidth(24);
        check->setHeight(24);
        check->setMarginRight(20);
        check->setColor(theme["cadran/teal"]);
        cell->addView(check);
    }
    check->setVisibility(checked ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
}

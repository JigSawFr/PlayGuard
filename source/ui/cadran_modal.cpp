// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/cadran_modal.hpp"

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
        else if (emphasis == Emphasis::NONE && b->getText() == ok) look = &PRIMARY;
        pill(b, look);
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
    cell->setBackgroundColor(checked ? theme["cadran/teal_soft"] : theme["cadran/none"]);
    cell->title->setTextColor(checked ? theme["cadran/teal"] : theme["cadran/text"]);
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

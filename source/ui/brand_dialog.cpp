// ui — dialogs in the brand colours (see ui.hpp: brand_confirm, brand_info).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

using namespace brls::literals;

namespace ui
{

namespace
{
// The brand palette (branding/*.svg), the same whatever the theme.
const NVGcolor NAVY  = nvgRGB(0x0B, 0x14, 0x24);
const NVGcolor TRACK = nvgRGB(0x1F, 0x33, 0x55);
const NVGcolor TEAL  = nvgRGB(0x2E, 0xC4, 0xA6);
const NVGcolor LIGHT = nvgRGB(0xE8, 0xEE, 0xF7);
const NVGcolor MUTED = nvgRGB(0xA9, 0xB6, 0xCC);
const NVGcolor RED   = nvgRGB(0xFF, 0x7A, 0x7A);   // dark theme's brand/bad: legible on navy

// borealis' borderless button without the light fill of its focus: the teal
// frame alone, so light text stays legible on navy. A style, not a setter:
// every colour change re-applies the button's style (Button::applyStyle).
const brls::ButtonStyle* button_style()
{
    static const brls::ButtonStyle style = [] {
        brls::ButtonStyle s = brls::BUTTONSTYLE_BORDERLESS;
        s.hideHighlightBackground = true;
        return s;
    }();
    return &style;
}

// The icon and the title side by side, a short teal rule, then the text.
brls::Box* content(const std::string& title, const std::string& body)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setPadding(34, 56, 30, 56);

    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    auto* icon = new brls::Image();
    icon->setImageFromRes("img/app_icon.jpg");
    icon->setDimensions(48, 48);
    icon->setCornerRadius(11);
    icon->setMarginRight(16);
    head->addView(icon);
    auto* heading = new brls::Label();
    heading->setText(title);
    heading->setFontSize(26);
    heading->setTextColor(LIGHT);
    heading->setSingleLine(true);
    head->addView(heading);
    box->addView(head);

    auto* rule = new brls::Rectangle(TEAL);
    rule->setDimensions(56, 3);
    rule->setCornerRadius(1.5f);
    rule->setMarginTop(18);
    rule->setMarginBottom(20);
    box->addView(rule);

    auto* text = new brls::Label();
    text->setText(body);
    // A long text in a smaller font rather than off the screen.
    text->setFontSize(body.size() > 360 ? 18.0f : 20.0f);
    text->setTextColor(MUTED);
    text->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    text->setSingleLine(false);
    box->addView(text);
    return box;
}

// borealis' dialog, repainted: a navy card with a teal edge, navy separators,
// and buttons in button_style().
brls::Dialog* make(const std::string& title, const std::string& body)
{
    auto* dialog = new brls::Dialog(content(title, body));
    if (brls::View* card = dialog->getView("brls/dialog/applet")) {
        card->setBackgroundColor(NAVY);
        card->setCornerRadius(14);
        card->setLineColor(TEAL);
        card->setLineTop(4);
    }
    for (const char* id : { "brls/dialog/button2/separator", "brls/dialog/button3/separator" })
        if (auto* sep = dynamic_cast<brls::Rectangle*>(dialog->getView(id))) sep->setColor(TRACK);
    if (brls::View* first = dialog->getView("brls/dialog/button1"))
        if (brls::View* row = first->getParent()) row->setLineColor(TRACK);
    for (const char* id : { "brls/dialog/button1", "brls/dialog/button2", "brls/dialog/button3" })
        if (auto* button = dynamic_cast<brls::Button*>(dialog->getView(id))) {
            button->setStyle(button_style());
            button->setTextColor(TEAL);
        }
    return dialog;
}

void paint(brls::Dialog* dialog, const char* id, NVGcolor color)
{
    if (auto* button = dynamic_cast<brls::Button*>(dialog->getView(id))) button->setTextColor(color);
}
}   // namespace

void brand_confirm(const std::string& title, const std::string& body, const std::string& confirm_label,
                   std::function<void()> on_yes, std::function<void()> on_no, bool danger)
{
    auto* dialog = make(title, body);
    dialog->addButton("hints/cancel"_i18n, [on_no]() { if (on_no) on_no(); });
    dialog->addButton(confirm_label, [on_yes]() { if (on_yes) on_yes(); });
    on_cancel(dialog, on_no);
    paint(dialog, "brls/dialog/button1", MUTED);
    paint(dialog, "brls/dialog/button2", danger ? RED : TEAL);
    dialog->open();
}

void brand_info(const std::string& title, const std::string& body)
{
    auto* dialog = make(title, body);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->setCancelable(true);
    dialog->open();
}

}   // namespace ui

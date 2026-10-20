// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/ha_links.hpp"

#include "app.hpp"
#include "ui/ui.hpp"
#include "view/qr_view.hpp"

using namespace brls::literals;

namespace ha_links
{

namespace
{
brls::Label* centered_label(const std::string& text, int font_size, NVGcolor color)
{
    auto* label = new brls::Label();
    label->setFontSize(font_size);
    label->setTextColor(color);
    label->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    label->setSingleLine(false);
    label->setAlignSelf(brls::AlignSelf::STRETCH);   // wraps at the card's width
    label->setText(text);
    return label;
}
}   // namespace

brls::Box* qr_card(const std::string& title, const std::string& url, const std::string& caption)
{
    auto* card = new brls::Box(brls::Axis::COLUMN);
    card->setAlignItems(brls::AlignItems::CENTER);
    card->setWidth(320);
    card->addView(new QrView(url, 220));
    brls::Label* name = centered_label(title, 20, ui::color_text());
    name->setMarginTop(6);
    card->addView(name);
    card->addView(centered_label(caption, 17, ui::color_note()));
    return card;
}

void show(const std::string& device)
{
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::CENTER);
    box->setPadding(24, 24, 12, 24);
    box->addView(centered_label("playguard/sync/ha_install_body"_i18n, 20, ui::color_text()));
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setJustifyContent(brls::JustifyContent::SPACE_EVENLY);
    row->setMarginTop(14);
    row->addView(qr_card("playguard/sync/ha_hacs"_i18n, app::HA_HACS_URL, "playguard/sync/ha_hacs_caption"_i18n));
    row->addView(qr_card("playguard/sync/ha_add"_i18n, app::HA_ADD_URL,
                         device.empty() ? "playguard/sync/ha_add_caption_none"_i18n
                                        : brls::getStr("playguard/sync/ha_add_caption", device)));
    box->addView(row);
    auto* dialog = new brls::Dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
    brls::Logger::info("sync: Home Assistant integration links shown");
}

}   // namespace ha_links

// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/ha_links.hpp"

#include <aurore/cadran/ui/cadran_kit.hpp>
#include <aurore/cadran/view/qr_view.hpp>

#include "app.hpp"
#include "ui/ui.hpp"

using namespace aurore::literals;

namespace ha_links
{

namespace
{
aurore::Label* centered_label(const std::string& text, float font_size, const char* color)
{
    auto* label = new aurore::Label();
    label->setFontSize(font_size);
    label->setThemeColor("textColor", color);
    label->setHorizontalAlign(aurore::HorizontalAlign::CENTER);
    label->setSingleLine(false);
    label->setAlignSelf(aurore::AlignSelf::STRETCH);   // wraps at the card's width
    label->setText(text);
    return label;
}
}   // namespace

aurore::Box* qr_card(const std::string& title, const std::string& url, const std::string& caption)
{
    auto* card = new aurore::Box(aurore::Axis::COLUMN);
    card->setAlignItems(aurore::AlignItems::CENTER);
    card->setWidth(320);
    card->addView(new QrView(url, 220));
    aurore::Label* name = centered_label(title, cadran::kit::TEXT_LEAD, "cadran/text");
    name->setMarginTop(6);
    card->addView(name);
    card->addView(centered_label(caption, cadran::kit::TEXT_NOTE, "cadran/text3"));
    return card;
}

void show(const std::string& device)
{
    auto* box = new aurore::Box(aurore::Axis::COLUMN);
    box->setAlignItems(aurore::AlignItems::CENTER);
    box->setPadding(24, 24, 12, 24);
    box->addView(centered_label("playguard/sync/ha_install_body"_i18n, cadran::kit::TEXT_LEAD, "cadran/text"));
    auto* row = new aurore::Box(aurore::Axis::ROW);
    row->setJustifyContent(aurore::JustifyContent::SPACE_EVENLY);
    row->setMarginTop(14);
    row->addView(qr_card("playguard/sync/ha_hacs"_i18n, app::HA_HACS_URL, "playguard/sync/ha_hacs_caption"_i18n));
    row->addView(qr_card("playguard/sync/ha_add"_i18n, app::HA_ADD_URL,
                         device.empty() ? "playguard/sync/ha_add_caption_none"_i18n
                                        : aurore::getStr("playguard/sync/ha_add_caption", device)));
    box->addView(row);
    auto* dialog = new aurore::Dialog(box);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->open();
    aurore::Logger::info("sync: Home Assistant integration links shown");
}

}   // namespace ha_links

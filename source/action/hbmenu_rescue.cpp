// hbmenu_lock's rescue sheet: the ways to undo the lock without the console,
// on screen and as a page to print.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/hbmenu_lock.hpp"

#include <borealis.hpp>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include "action/forwarder_flow.hpp"
#include "action/forwarders_logic.hpp"
#include "action/history_flow.hpp"
#include "action/notices.hpp"
#include "action/pin_lock.hpp"
#include "action/rescue.hpp"
#include "app.hpp"
#include "core/gate.h"
#include "core/hbl_lock.h"
#include "core/rescue.h"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/ui.hpp"
#include "util/diagnostics.hpp"
#include "util/forwarders.hpp"
#include "util/paths.hpp"
#include "util/rescue_page.hpp"
#include "view/qr_view.hpp"
#include "util/pctl_ops_c.hpp"

#include "action/hbmenu_lock_shared.hpp"

using namespace brls::literals;

namespace hbmenu_lock
{

using namespace detail;

namespace
{
// A step of the rescue sheet: its number in a bubble, its title, its text.
brls::Box* rescue_step(const char* number, const std::string& title, const std::string& text)
{
    namespace kit = cadran::kit;
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::FLEX_START);
    brls::Box* n = kit::key(number, 26);
    n->setMarginRight(14);
    n->setMarginTop(1);
    row->addView(n);
    auto* words = new brls::Box(brls::Axis::COLUMN);
    words->setGrow(1.0f);
    words->setWidth(0);
    words->addView(kit::label(title, 18, "cadran/text"));
    brls::Label* detail = kit::label(text, 15.5f, "cadran/text2");
    detail->setMarginTop(3);
    words->addView(detail);
    row->addView(words);
    return row;
}

constexpr const char* RESCUE_PAGE_NAME = "rescue.html";
std::string help_url()
{
    return std::string(app::repo_url()) + "/blob/main/docs/troubleshooting.md#homebrew-menu-locked";
}

// The rescue sheet as a page to print, in the data directory; false when it
// could not be written.
bool write_rescue_page()
{
    rescue_page::Text t;
    t.lang = brls::Application::getLocale();
    t.title = "playguard/hbmenu/rescue"_i18n;
    t.subtitle = "playguard/hbmenu/rescue_sub"_i18n;
    t.pc_title = "playguard/hbmenu/rescue_pc_title"_i18n;
    t.pc_text = brls::getStr("playguard/hbmenu/rescue_pc", HBL_LOCK_CONFIG_PATH);
    t.mod_title = "playguard/hbmenu/rescue_mod_title"_i18n;
    t.mod_text = rescue::installed() ? brls::getStr("playguard/hbmenu/rescue_mod", RESCUE_REQUEST_NAME)
                                     : "playguard/hbmenu/rescue_mod_none"_i18n;
    t.qr_caption = "playguard/hbmenu/rescue_qr"_i18n;
    t.footer = brls::getStr("playguard/hbmenu/rescue_written", ui::now_stamp());
    std::string err;
    if (paths::ensure_dir(paths::data_dir()) &&
        paths::atomic_write(in_data(RESCUE_PAGE_NAME), rescue_page::html(t, help_url()), &err))
        return true;
    brls::Logger::warning("hbmenu lock: {} not written: {}", RESCUE_PAGE_NAME, err);
    return false;
}
}   // namespace

void detail::rescue_sheet(bool locked)
{
    const bool page = write_rescue_page();
    namespace kit = cadran::kit;
    auto* body = new brls::Box(brls::Axis::COLUMN);
    body->setAlignItems(brls::AlignItems::STRETCH);
    body->setPadding(30, 40, 22, 40);
    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    brls::Box* badge = locked ? cadran::dialog_badge("check", "cadran/teal", "cadran/teal_soft")
                              : cadran::dialog_badge("shield", "cadran/teal", "cadran/teal_soft");
    badge->setMarginRight(16);
    head->addView(badge);
    auto* titles = new brls::Box(brls::Axis::COLUMN);
    titles->setGrow(1.0f);
    titles->setWidth(0);
    titles->addView(cadran::dialog_title(locked ? "playguard/hbmenu/locked_done"_i18n : "playguard/hbmenu/rescue"_i18n));
    brls::Label* sub = kit::label("playguard/hbmenu/rescue_sub"_i18n, 16, "cadran/text2");
    sub->setMarginTop(4);
    titles->addView(sub);
    head->addView(titles);
    body->addView(head);

    // The ways, numbered, beside the help's QR code.
    auto* main = new brls::Box(brls::Axis::ROW);
    main->setAlignItems(brls::AlignItems::FLEX_START);
    main->setMarginTop(20);
    auto* steps = new brls::Box(brls::Axis::COLUMN);
    steps->setGrow(1.0f);
    steps->setWidth(0);
    steps->setBackgroundColor(brls::Application::getTheme()["cadran/surf2"]);
    steps->setCornerRadius(14);
    steps->setPadding(16, 18, 16, 18);
    steps->addView(rescue_step("1", "playguard/hbmenu/rescue_pc_title"_i18n,
                               brls::getStr("playguard/hbmenu/rescue_pc", HBL_LOCK_CONFIG_PATH)));
    brls::Box* second = rescue_step("2", "playguard/hbmenu/rescue_mod_title"_i18n,
                                    rescue::installed() ? brls::getStr("playguard/hbmenu/rescue_mod", RESCUE_REQUEST_NAME)
                                                        : "playguard/hbmenu/rescue_mod_none"_i18n);
    second->setMarginTop(14);
    steps->addView(second);
    main->addView(steps);
    auto* side = new brls::Box(brls::Axis::COLUMN);
    side->setAlignItems(brls::AlignItems::CENTER);
    side->setMarginLeft(20);
    side->setShrink(0);
    auto* qr = new QrView(help_url(), 168);
    side->addView(qr);
    brls::Label* caption = kit::label("playguard/hbmenu/rescue_qr"_i18n, 13.5f, "cadran/text3");
    caption->setHorizontalAlign(brls::HorizontalAlign::CENTER);
    caption->setMaxWidth(168);
    caption->setMarginTop(8);
    side->addView(caption);
    main->addView(side);
    body->addView(main);
    // A copy to print, on the SD card.
    if (page) {
        auto* foot = new brls::Box(brls::Axis::ROW);
        foot->setAlignItems(brls::AlignItems::CENTER);
        foot->setMarginTop(14);
        CadranIcon* icon = kit::icon("file", 16, "cadran/text3");
        icon->setMarginRight(10);
        foot->addView(icon);
        foot->addView(kit::label(brls::getStr("playguard/hbmenu/rescue_print", "config/playguard/" + std::string(RESCUE_PAGE_NAME)),
                                 15, "cadran/text3"));
        body->addView(foot);
    }

    auto* dialog = ui::box_dialog(body);
    dialog->addButton("hints/ok"_i18n, []() {});
    dialog->setCancelable(true);
    cadran::style_dialog(dialog, cadran::Emphasis::NONE);
    dialog->open();
}

void show_rescue()
{
    rescue_sheet(false);
}

}   // namespace hbmenu_lock

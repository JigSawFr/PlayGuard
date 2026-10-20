// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/api_activity.hpp"

#include "action/api_flow.hpp"
#include "ui/cadran_kit.hpp"
#include "ui/ui.hpp"
#include "view/cadran_icon.hpp"

using namespace brls::literals;

ApiActivity::ApiActivity(const std::string& caller, const std::vector<Row>& rows, std::function<void(ApiActivity*)> start)
    : caller(caller), rows(rows), start(std::move(start))
{
}

void ApiActivity::onContentAvailable()
{
    from->setSingleLine(false);
    next->setSingleLine(false);
    // The name is the caller's own word for itself: said as such.
    from->setText(this->caller.empty() ? "playguard/api/unnamed_from"_i18n
                                       : brls::getStr("playguard/api/from", this->caller));
    for (const auto& r : this->rows) {
        auto* row = new brls::Box(brls::Axis::ROW);
        row->setAlignItems(brls::AlignItems::FLEX_START);
        row->setMarginBottom(16);
        CadranIcon* icon = cadran::kit::icon("timer", 26, "cadran/text3");
        icon->setMarginRight(16);
        icon->setMarginTop(2);
        row->addView(icon);
        auto* text = new brls::Box(brls::Axis::COLUMN);
        text->setGrow(1.0f);
        text->addView(ui::text_label(r.name, 22, ui::color_text()));
        text->addView(ui::text_label(r.route, 16, ui::color_note()));
        brls::Label* result = ui::text_label("", 20, ui::color_neutral());
        result->setMarginTop(4);
        text->addView(result);
        row->addView(text);
        list->addView(row);
        this->results.push_back(result);
        this->icons.push_back(icon);
    }
    go_on->registerClickAction([this](brls::View*) {
        if (this->finished) this->leave();
        return true;
    });
    allow->registerClickAction([this](brls::View*) {
        // The countdown stops: the parent is here, and goes back with Continue.
        this->counting = false;
        this->show_next();
        api_flow::open_settings();
        return true;
    });
    this->getContentView()->registerAction("hints/exit"_i18n, brls::BUTTON_B, [this](brls::View*) {
        if (this->finished) this->leave();
        return true;
    });
    brls::sync([this]() { this->start(this); });
}

void ApiActivity::set_line(size_t i, const std::string& text)
{
    if (i >= this->results.size()) return;
    this->results[i]->setText(text);
    this->results[i]->setTextColor(ui::color_neutral());
}

void ApiActivity::set_line(size_t i, const std::string& text, bool ok)
{
    if (i >= this->results.size()) return;
    this->results[i]->setText(text);
    this->results[i]->setTextColor(ok ? ui::color_ok() : ui::color_bad());
    this->icons[i]->setIcon(ok ? "check" : "alert");
    this->icons[i]->setColor(ok ? ui::color_ok() : ui::color_bad());
}

void ApiActivity::set_message(const std::string& text)
{
    list->clearViews();
    this->results.clear();
    this->icons.clear();
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setAlignItems(brls::AlignItems::FLEX_START);
    CadranIcon* icon = cadran::kit::icon("alert", 26, "cadran/text3");
    icon->setColor(ui::color_bad());
    icon->setMarginRight(16);
    row->addView(icon);
    brls::Label* label = ui::text_label(text, 22, ui::color_bad());
    label->setGrow(1.0f);
    row->addView(label);
    list->addView(row);
}

void ApiActivity::done(const std::string& return_to, bool offer_allow)
{
    this->finished = true;
    this->target   = return_to;
    const size_t slash = this->target.find_last_of('/');
    if (slash != std::string::npos) this->target = this->target.substr(slash + 1);
    ui::set_visible(go_on.getView(), true);
    ui::set_visible(allow.getView(), offer_allow);
    brls::Application::giveFocus(go_on.getView());
    this->counting     = true;
    this->seconds_left = LEAVE_S;
    this->show_next();
    brls::delay(1000, [this]() { this->tick(); });
}

void ApiActivity::show_next()
{
    if (!this->counting)
        next->setText(this->target.empty() ? "playguard/api/closing_wait"_i18n
                                           : brls::getStr("playguard/api/returning_wait", this->target));
    else
        next->setText(this->target.empty() ? brls::getStr("playguard/api/closing", this->seconds_left)
                                           : brls::getStr("playguard/api/returning", this->target, this->seconds_left));
}

void ApiActivity::tick()
{
    if (!this->counting || this->left) return;
    if (--this->seconds_left <= 0) {
        this->leave();
        return;
    }
    this->show_next();
    brls::delay(1000, [this]() { this->tick(); });
}

void ApiActivity::leave()
{
    if (this->left) return;
    this->left = true;
    brls::Application::quit();
}

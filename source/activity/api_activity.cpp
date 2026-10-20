// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/api_activity.hpp"

#include "ui/ui.hpp"

using namespace brls::literals;

ApiActivity::ApiActivity(const std::string& caller, const std::vector<std::string>& routes,
                         std::function<void(ApiActivity*)> start)
    : caller(caller), routes(routes), start(std::move(start))
{
}

void ApiActivity::onContentAvailable()
{
    from->setSingleLine(false);
    next->setSingleLine(false);
    from->setText(brls::getStr("playguard/api/from",
                               this->caller.empty() ? "playguard/api/unnamed"_i18n : this->caller));
    for (const auto& route : this->routes) {
        auto* row = new brls::Box(brls::Axis::COLUMN);
        row->setMarginBottom(14);
        row->addView(ui::text_label(route, 22, ui::color_text()));
        brls::Label* result = ui::text_label("", 20, ui::color_neutral());
        row->addView(result);
        rows->addView(row);
        this->results.push_back(result);
    }
    for (auto button : { brls::BUTTON_A, brls::BUTTON_B })
        this->getContentView()->registerAction(button == brls::BUTTON_A ? "hints/ok"_i18n : "hints/exit"_i18n, button,
                                               [this](brls::View*) {
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
}

void ApiActivity::set_message(const std::string& text)
{
    rows->clearViews();
    this->results.clear();
    rows->addView(ui::text_label(text, 22, ui::color_bad()));
}

void ApiActivity::done(bool ok, const std::string& return_to)
{
    (void)ok;
    this->finished = true;
    std::string name = return_to;
    const size_t slash = name.find_last_of('/');
    if (slash != std::string::npos) name = name.substr(slash + 1);
    next->setText(return_to.empty() ? "playguard/api/closing"_i18n : brls::getStr("playguard/api/returning", name));
    brls::delay(LEAVE_MS, [this]() { this->leave(); });
}

void ApiActivity::leave()
{
    if (this->left) return;
    this->left = true;
    brls::Application::quit();
}

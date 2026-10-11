// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/cadran_week.hpp"

#include <algorithm>

#include "ui/ui.hpp"

namespace
{
constexpr float COL_HEIGHT = 262.0f;
constexpr float BAR_MAX    = 170.0f;
constexpr float BAR_MIN    = 6.0f;
constexpr int   STEP       = 15;
}   // namespace

// A day: tells the view when it takes or loses the focus.
class CadranWeekView::Column : public brls::Box
{
  public:
    Column(CadranWeekView* owner, int day)
        : brls::Box(brls::Axis::COLUMN), owner(owner), day(day) {}
    void onFocusGained() override
    {
        brls::Box::onFocusGained();
        this->owner->focused = this->day;
        this->owner->render();
    }
    void onFocusLost() override
    {
        brls::Box::onFocusLost();
        if (this->owner->focused == this->day) this->owner->focused = -1;
        this->owner->render();
    }

  private:
    CadranWeekView* owner;
    int day;
};

CadranWeekView::CadranWeekView()
{
    brls::Theme theme = brls::Application::getTheme();
    this->setAxis(brls::Axis::ROW);
    this->setAlignItems(brls::AlignItems::FLEX_END);
    this->setJustifyContent(brls::JustifyContent::SPACE_BETWEEN);
    this->setHeight(COL_HEIGHT);
    this->setFocusable(false);

    for (int i = 1; i <= 7; i++) {   // Monday first, Sunday last
        const int d = i % 7;
        Col& c = this->cols[d];
        c.box = new Column(this, d);
        c.box->setGrow(1.0f);
        c.box->setWidth(0);
        c.box->setHeight(COL_HEIGHT);
        c.box->setAlignItems(brls::AlignItems::CENTER);
        c.box->setJustifyContent(brls::JustifyContent::FLEX_END);
        c.box->setPadding(10, 6, 8, 6);
        c.box->setCornerRadius(18);
        c.box->setFocusable(true);
        if (i < 7) c.box->setMarginRight(8);

        c.up = new CadranIcon();
        c.up->setIcon("caret-up");
        c.up->setWidth(22);
        c.up->setHeight(22);
        c.up->setColor(theme["cadran/teal"]);
        c.up->setMarginBottom(4);
        c.box->addView(c.up);
        c.value = new CadranDigits();
        c.value->setFontSize(20);
        c.value->setMarginBottom(8);
        c.box->addView(c.value);
        c.bar = new brls::Box();
        c.bar->setWidth(56);
        c.bar->setHeight(BAR_MIN);
        c.bar->setCornerRadius(12);
        c.bar->setMarginBottom(8);
        c.box->addView(c.bar);
        c.day = new brls::Label();
        c.day->setFontSize(16);
        c.day->setText(ui::day_name_in_text(d).substr(0, 3));
        c.box->addView(c.day);
        c.down = new CadranIcon();
        c.down->setIcon("caret-down");
        c.down->setWidth(22);
        c.down->setHeight(22);
        c.down->setColor(theme["cadran/teal"]);
        c.down->setMarginTop(4);
        c.box->addView(c.down);

        // The stick edits the day; the hints stay out of the footer.
        c.box->registerAction("", brls::BUTTON_NAV_UP, [this, d](brls::View*) { this->nudge(d, STEP); return true; }, true, true);
        c.box->registerAction("", brls::BUTTON_NAV_DOWN, [this, d](brls::View*) { this->nudge(d, -STEP); return true; }, true, true);
        c.box->registerAction("", brls::BUTTON_LB, [this, d](brls::View*) { if (this->on_copy) this->on_copy(d, false); return true; }, true);
        c.box->registerAction("", brls::BUTTON_RB, [this, d](brls::View*) { if (this->on_copy) this->on_copy(d, true); return true; }, true);
        c.box->registerAction("", brls::BUTTON_Y, [this, d](brls::View*) {
            if (this->on_change) this->on_change(d, PT_DAY_NOLIMIT);
            return true;
        }, true);
        c.box->registerClickAction([this, d](brls::View*) {
            if (this->on_pick) this->on_pick(d);
            return true;
        });
        this->addView(c.box);
    }
}

void CadranWeekView::nudge(int day, int delta)
{
    if (!this->valid || !this->on_change) return;
    const uint16_t cur = this->days[day];
    int v = cur == PT_DAY_NOLIMIT ? (delta > 0 ? 0 : 1440) : (int)cur;
    v = std::max(0, std::min(1440, v + delta));
    this->on_change(day, (uint16_t)v);
}

void CadranWeekView::show(const uint16_t days[7], const uint16_t live[7])
{
    for (int d = 0; d < 7; d++) {
        this->days[d] = days[d];
        this->live[d] = live[d];
    }
    this->valid = true;
    this->render();
}

void CadranWeekView::set_on_change(std::function<void(int, uint16_t)> f) { this->on_change = std::move(f); }
void CadranWeekView::set_on_pick(std::function<void(int)> f) { this->on_pick = std::move(f); }
void CadranWeekView::set_on_copy(std::function<void(int, bool)> f) { this->on_copy = std::move(f); }

void CadranWeekView::render()
{
    brls::Theme theme = brls::Application::getTheme();
    const int today = ui::today_weekday();
    uint16_t top = 180;
    for (int d = 0; d < 7; d++)
        if (this->days[d] != PT_DAY_NOLIMIT) top = std::max(top, this->days[d]);
    for (int d = 0; d < 7; d++) {
        Col& c = this->cols[d];
        const uint16_t m = this->days[d];
        const bool focused = d == this->focused;
        const bool changed = this->valid && m != this->live[d];
        c.value->setText(!this->valid ? "—" : m == PT_DAY_NOLIMIT ? "—" : ui::fmt_minutes(m));
        c.value->setTextColor(focused ? theme["cadran/teal"] : changed ? theme["cadran/amber"] : theme["cadran/text"]);
        c.bar->setHeight(!this->valid || m == PT_DAY_NOLIMIT ? BAR_MIN : std::max(BAR_MIN, BAR_MAX * (float)m / (float)top));
        c.bar->setBackgroundColor(focused ? theme["cadran/teal_fill"] : d == today ? theme["cadran/bar_today"] : theme["cadran/bar"]);
        c.day->setTextColor(d == today ? theme["cadran/teal"] : theme["cadran/text3"]);
        c.box->setBackgroundColor(focused ? theme["cadran/surf2"] : theme["cadran/none"]);
        c.up->setVisibility(focused ? brls::Visibility::VISIBLE : brls::Visibility::INVISIBLE);
        c.down->setVisibility(focused ? brls::Visibility::VISIBLE : brls::Visibility::INVISIBLE);
    }
}

brls::View* CadranWeekView::create() { return new CadranWeekView(); }

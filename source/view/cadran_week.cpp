// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/cadran_week.hpp"

#include <algorithm>
#include <fmt/format.h>

#include "ui/ui.hpp"

using namespace brls::literals;

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
        // Reached any other way than left / right while editing (the stick
        // from elsewhere, back from a dialog): not editing.
        if (!this->owner->moving) this->owner->set_edit(false);
        this->owner->render();
        if (this->owner->on_focus) this->owner->on_focus(this->day);
    }
    void onFocusLost() override
    {
        brls::Box::onFocusLost();
        if (this->owner->focused == this->day) this->owner->focused = -1;
        this->owner->render();
        if (this->owner->on_focus) this->owner->on_focus(this->owner->focused);
    }

  private:
    CadranWeekView* owner;
    int day;
};

namespace
{
// Monday first, Sunday last.
constexpr int ORDER[7] = { 1, 2, 3, 4, 5, 6, 0 };
}   // namespace

CadranWeekView::CadranWeekView()
{
    brls::Theme theme = brls::Application::getTheme();
    this->setAxis(brls::Axis::ROW);
    this->setAlignItems(brls::AlignItems::FLEX_END);
    this->setJustifyContent(brls::JustifyContent::SPACE_BETWEEN);
    this->setHeight(COL_HEIGHT);
    this->setFocusable(false);

    for (int i = 0; i < 7; i++) {
        const int d = ORDER[i];
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
        c.box->setHideHighlightBackground(true);
        c.box->setHighlightCornerRadius(18);
        if (i < 6) c.box->setMarginRight(8);

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
        c.bar->setAxis(brls::Axis::COLUMN);
        c.bar->setJustifyContent(brls::JustifyContent::FLEX_END);
        c.bar->setAlignItems(brls::AlignItems::STRETCH);
        c.fill = new brls::Box();
        c.fill->setCornerRadius(12);
        c.fill->setHeight(0);
        c.fill->setVisibility(brls::Visibility::GONE);
        c.bar->addView(c.fill);
        c.box->addView(c.bar);
        c.day = new brls::Label();
        c.day->setFontSize(16);
        c.day->setSingleLine(true);
        c.day->setText(brls::getStr(fmt::format("playguard/days_short/{}", d)));
        c.box->addView(c.day);
        c.down = new CadranIcon();
        c.down->setIcon("caret-down");
        c.down->setWidth(22);
        c.down->setHeight(22);
        c.down->setColor(theme["cadran/teal"]);
        c.down->setMarginTop(4);
        c.box->addView(c.down);

        // A starts and ends the edit; editing, the stick and the shoulders
        // edit the day (their hints are on the screen, under the week), and
        // B ends it. Not editing, they do what they do anywhere else.
        c.box->registerAction("playguard/cadran/week/edit"_i18n, brls::BUTTON_A, [this](brls::View*) {
            if (!this->edit && this->can_edit && !this->can_edit()) return true;
            this->set_edit(!this->edit);
            return true;
        });
        // X: told by the keys under the week, not in the footer (+ Save and
        // the global hints already fill it while a day is edited).
        c.box->registerAction("playguard/cadran/week/pick"_i18n, brls::BUTTON_X, [this, d](brls::View*) {
            if (this->can_edit && !this->can_edit()) return true;
            if (this->on_pick) this->on_pick(d);
            return true;
        }, true);
        c.box->registerAction("", brls::BUTTON_B, [this](brls::View*) {
            if (!this->edit) return false;
            this->set_edit(false);
            return true;
        }, true);
        c.box->registerAction("", brls::BUTTON_NAV_UP, [this, d](brls::View*) {
            if (!this->edit) return false;
            this->nudge(d, STEP);
            return true;
        }, true, true);
        c.box->registerAction("", brls::BUTTON_NAV_DOWN, [this, d](brls::View*) {
            if (!this->edit) return false;
            this->nudge(d, -STEP);
            return true;
        }, true, true);
        c.box->registerAction("", brls::BUTTON_NAV_LEFT, [this, d](brls::View*) { return this->step(d, -1); }, true, true);
        c.box->registerAction("", brls::BUTTON_NAV_RIGHT, [this, d](brls::View*) { return this->step(d, 1); }, true, true);
        c.box->registerAction("", brls::BUTTON_LB, [this, d](brls::View*) {
            if (!this->edit) return false;
            if (this->on_copy) this->on_copy(d, false);
            return true;
        }, true);
        c.box->registerAction("", brls::BUTTON_RB, [this, d](brls::View*) {
            if (!this->edit) return false;
            if (this->on_copy) this->on_copy(d, true);
            return true;
        }, true);
        c.box->registerAction("", brls::BUTTON_Y, [this, d](brls::View*) {
            if (!this->edit) return false;
            if (this->on_change) this->on_change(d, PT_DAY_NOLIMIT);
            return true;
        }, true);
        c.box->addGestureRecognizer(new brls::TapGestureRecognizer([this, d](brls::TapGestureStatus status, brls::Sound* sound) {
            if (status.state != brls::GestureState::END) return;
            *sound = brls::SOUND_CLICK;
            this->tap(d, status.position.y);
        }));
        this->addView(c.box);
    }
}

void CadranWeekView::tap(int day, float y)
{
    Column* box = this->cols[day].box;
    if (!this->edit || this->focused != day) {
        if (this->can_edit && !this->can_edit()) {
            brls::Application::giveFocus(box);
            return;
        }
        this->moving = this->edit;   // from another day being edited: the edit goes on
        brls::Application::giveFocus(box);
        this->moving = false;
        this->set_edit(true);
        return;
    }
    const float third = box->getHeight() / 3;
    if (y < box->getY() + third) this->nudge(day, STEP);
    else if (y > box->getY() + 2 * third) this->nudge(day, -STEP);
    else this->set_edit(false);
}

void CadranWeekView::set_edit(bool on)
{
    if (this->edit == on) return;
    this->edit = on;
    for (const Col& c : this->cols)
        c.box->updateActionHint(brls::BUTTON_A, on ? "playguard/cadran/week/done"_i18n : "playguard/cadran/week/edit"_i18n);
    brls::Application::getGlobalHintsUpdateEvent()->fire();
    this->render();
    if (this->on_mode) this->on_mode(on);
}

void CadranWeekView::focus_day(int day)
{
    if (day >= 0 && day <= 6) brls::Application::giveFocus(this->cols[day].box);
}

bool CadranWeekView::step(int day, int direction)
{
    if (!this->edit) return false;
    int at = 0;
    while (at < 7 && ORDER[at] != day) at++;
    const int next = at + direction;
    if (next < 0 || next > 6) return true;   // the edit stays in the week
    this->moving = true;
    brls::Application::giveFocus(this->cols[ORDER[next]].box);
    this->moving = false;
    return true;
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

void CadranWeekView::show_played(const uint32_t minutes[7], bool ok)
{
    for (int d = 0; d < 7; d++) this->played[d] = ok ? minutes[d] : 0;
    this->played_ok = ok;
    this->render();
}

void CadranWeekView::set_on_change(std::function<void(int, uint16_t)> f) { this->on_change = std::move(f); }
void CadranWeekView::set_on_pick(std::function<void(int)> f) { this->on_pick = std::move(f); }
void CadranWeekView::set_on_copy(std::function<void(int, bool)> f) { this->on_copy = std::move(f); }
void CadranWeekView::set_can_edit(std::function<bool()> f) { this->can_edit = std::move(f); }
void CadranWeekView::set_on_mode(std::function<void(bool)> f) { this->on_mode = std::move(f); }
void CadranWeekView::set_on_focus(std::function<void(int)> f) { this->on_focus = std::move(f); }

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
        c.value->setText(!this->valid ? "—" : m == PT_DAY_NOLIMIT ? "—" : ui::fmt_played(m));
        c.value->setTextColor(focused ? theme["cadran/teal"] : changed ? theme["cadran/amber"] : theme["cadran/text"]);
        const float bar = !this->valid || m == PT_DAY_NOLIMIT ? BAR_MIN : std::max(BAR_MIN, BAR_MAX * (float)m / (float)top);
        c.bar->setHeight(bar);
        c.bar->setBackgroundColor(focused ? theme["cadran/teal_fill"] : d == today ? theme["cadran/bar_today"] : theme["cadran/bar"]);
        // What was played that day, against its limit: up to the top, coral
        // once the limit is reached. Nothing on a day without one.
        const bool fills = this->valid && this->played_ok && m != PT_DAY_NOLIMIT && m > 0 && this->played[d] > 0;
        if (fills) {
            const bool over = this->played[d] >= m;
            c.fill->setHeight(std::max(6.0f, bar * std::min(1.0f, (float)this->played[d] / (float)m)));
            // On the focused bar (teal itself), the teal darkened.
            NVGcolor shade = theme["cadran/navy"];
            shade.a = 0.35f;
            c.fill->setBackgroundColor(over ? theme["cadran/coral_fill"] : focused ? shade : theme["cadran/teal_fill"]);
        }
        c.fill->setVisibility(fills ? brls::Visibility::VISIBLE : brls::Visibility::GONE);
        c.day->setTextColor(d == today ? theme["cadran/teal"] : theme["cadran/text3"]);
        // The day being edited on the soft teal, its arrows shown.
        const bool editing = focused && this->edit;
        c.box->setBackgroundColor(editing ? theme["cadran/teal_soft"] : focused ? theme["cadran/surf2"] : theme["cadran/none"]);
        c.up->setVisibility(editing ? brls::Visibility::VISIBLE : brls::Visibility::INVISIBLE);
        c.down->setVisibility(editing ? brls::Visibility::VISIBLE : brls::Visibility::INVISIBLE);
    }
}

brls::View* CadranWeekView::create() { return new CadranWeekView(); }

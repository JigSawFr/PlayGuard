// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "view/cadran_week.hpp"

#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>

#include <aurore/cadran/ui/effects.hpp>
#include <aurore/cadran/ui/motion.hpp>

#include "action/pt_logic.hpp"
#include "ui/ui.hpp"

using namespace aurore::literals;

namespace
{
constexpr float COL_HEIGHT = 262.0f;
// A bar's way to a new height (µs), and the gap between two bars moving
// together (a day copied to the others).
constexpr float BAR_MOVE_US   = 180000.0f;
constexpr int   BAR_STAGGER_MS = 35;
constexpr float BED_HEIGHT = 24.0f;   // the bedtimes' line, when shown
// The tallest bar still leaves room in its column (262 less its padding, 18,
// the arrows, 52, the value, 31, the day's name, 16, and 8 under the bar):
// taller, the day's name was squeezed to nothing.
constexpr float BAR_MAX    = 136.0f;
constexpr float BAR_MIN    = 6.0f;
constexpr int   STEP       = 15;
}   // namespace

// A day: tells the view when it takes or loses the focus.
class CadranWeekView::Column : public aurore::Box
{
  public:
    Column(CadranWeekView* week, int weekday)
        : aurore::Box(aurore::Axis::COLUMN), owner(week), day(weekday) {}
    void onFocusGained() override
    {
        aurore::Box::onFocusGained();
        this->owner->focused = this->day;
        // Reached any other way than left / right while editing (the stick
        // from elsewhere, back from a dialog): not editing.
        if (!this->owner->moving) this->owner->set_edit(false);
        this->owner->render();
        if (this->owner->on_focus) this->owner->on_focus(this->day);
    }
    void onFocusLost() override
    {
        aurore::Box::onFocusLost();
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
    this->setAxis(aurore::Axis::ROW);
    this->setAlignItems(aurore::AlignItems::FLEX_END);
    this->setJustifyContent(aurore::JustifyContent::SPACE_BETWEEN);
    this->setHeight(COL_HEIGHT);
    this->setFocusable(false);

    for (int i = 0; i < 7; i++) {
        const int d = ORDER[i];
        Col& c = this->cols[d];
        c.box = new Column(this, d);
        c.box->setGrow(1.0f);
        c.box->setWidth(0);
        c.box->setHeight(COL_HEIGHT);
        c.box->setAlignItems(aurore::AlignItems::CENTER);
        c.box->setJustifyContent(aurore::JustifyContent::FLEX_END);
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
        c.up->setThemeColor("color", "cadran/teal");
        c.up->setMarginBottom(4);
        c.box->addView(c.up);
        auto* value_row = new aurore::Box(aurore::Axis::ROW);
        value_row->setAlignItems(aurore::AlignItems::CENTER);
        value_row->setMarginBottom(8);
        c.over = new CadranIcon();
        c.over->setIcon("alert");
        c.over->setWidth(16);
        c.over->setHeight(16);
        c.over->setThemeColor("color", "cadran/coral");
        c.over->setMarginRight(5);
        c.over->setVisibility(aurore::Visibility::GONE);
        value_row->addView(c.over);
        c.value = new CadranDigits();
        c.value->setFontSize(20);
        value_row->addView(c.value);
        c.box->addView(value_row);
        c.bar = new aurore::Box();
        c.bar->setWidth(56);
        c.bar->setHeight(BAR_MIN);
        c.bar->setCornerRadius(12);
        c.bar->setMarginBottom(8);
        c.bar->setAxis(aurore::Axis::COLUMN);
        c.bar->setJustifyContent(aurore::JustifyContent::FLEX_END);
        c.bar->setAlignItems(aurore::AlignItems::STRETCH);
        c.fill = new aurore::Box(aurore::Axis::COLUMN);
        c.fill->setCornerRadius(12);
        c.fill->setHeight(0);
        c.fill->setJustifyContent(aurore::JustifyContent::FLEX_END);
        c.fill->setAlignItems(aurore::AlignItems::STRETCH);
        c.fill->setVisibility(aurore::Visibility::GONE);
        c.within = new aurore::Box();
        c.within->setCornerRadius(12);
        c.within->setHeight(0);
        c.within->setVisibility(aurore::Visibility::GONE);
        c.fill->addView(c.within);
        c.bar->addView(c.fill);
        c.box->addView(c.bar);
        c.day = new aurore::Label();
        c.day->setFontSize(16);
        c.day->setSingleLine(true);
        c.day->setText(ui::day_short(d));
        c.box->addView(c.day);
        c.down = new CadranIcon();
        c.down->setIcon("caret-down");
        c.down->setWidth(22);
        c.down->setHeight(22);
        c.down->setThemeColor("color", "cadran/teal");
        c.down->setMarginTop(4);
        c.box->addView(c.down);
        c.bed = new aurore::Box(aurore::Axis::ROW);
        c.bed->setAlignItems(aurore::AlignItems::CENTER);
        c.bed->setHeight(BED_HEIGHT - 4);
        c.bed->setMarginTop(4);
        c.bed->setVisibility(aurore::Visibility::GONE);
        CadranIcon* moon = new CadranIcon();
        moon->setIcon("moon");
        moon->setWidth(13);
        moon->setHeight(13);
        moon->setThemeColor("color", "cadran/text3");
        moon->setMarginRight(4);
        c.bed->addView(moon);
        c.bed_time = new aurore::Label();
        c.bed_time->setFontSize(14);
        c.bed_time->setSingleLine(true);
        c.bed_time->setThemeColor("textColor", "cadran/text2");
        c.bed->addView(c.bed_time);
        c.box->addView(c.bed);

        // A starts and ends the edit; editing, the stick and the shoulders
        // edit the day (their hints are on the screen, under the week), and
        // B ends it. Not editing, they do what they do anywhere else.
        c.box->registerAction("playguard/cadran/week/edit"_i18n, aurore::BUTTON_A, [this](aurore::View*) {
            if (!this->edit && this->can_edit && !this->can_edit()) return true;
            this->set_edit(!this->edit);
            return true;
        });
        // X: told by the keys under the week, not in the footer (+ Save and
        // the global hints already fill it while a day is edited).
        c.box->registerAction("playguard/cadran/week/pick"_i18n, aurore::BUTTON_X, [this, d](aurore::View*) {
            if (this->can_edit && !this->can_edit()) return true;
            if (this->on_pick) this->on_pick(d);
            return true;
        }, true);
        c.box->registerAction("", aurore::BUTTON_B, [this](aurore::View*) {
            if (!this->edit) return false;
            this->set_edit(false);
            return true;
        }, true);
        c.box->registerAction("", aurore::BUTTON_NAV_UP, [this, d](aurore::View*) {
            if (!this->edit) return false;
            this->nudge(d, STEP);
            return true;
        }, true, true);
        c.box->registerAction("", aurore::BUTTON_NAV_DOWN, [this, d](aurore::View*) {
            if (!this->edit) return false;
            this->nudge(d, -STEP);
            return true;
        }, true, true);
        c.box->registerAction("", aurore::BUTTON_NAV_LEFT, [this, d](aurore::View*) { return this->step(d, -1); }, true, true);
        c.box->registerAction("", aurore::BUTTON_NAV_RIGHT, [this, d](aurore::View*) { return this->step(d, 1); }, true, true);
        c.box->registerAction("", aurore::BUTTON_LB, [this, d](aurore::View*) {
            if (!this->edit) return false;
            if (this->on_copy) this->on_copy(d, false);
            return true;
        }, true);
        c.box->registerAction("", aurore::BUTTON_RB, [this, d](aurore::View*) {
            if (!this->edit) return false;
            if (this->on_copy) this->on_copy(d, true);
            return true;
        }, true);
        c.box->registerAction("", aurore::BUTTON_Y, [this, d](aurore::View*) {
            if (!this->edit) return false;
            if (this->on_change) this->on_change(d, PT_DAY_NOLIMIT);
            return true;
        }, true);
        c.box->addGestureRecognizer(new aurore::TapGestureRecognizer([this, d](aurore::TapGestureStatus status, aurore::Sound* sound) {
            if (status.state != aurore::GestureState::END) return;
            *sound = aurore::SOUND_CLICK;
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
            aurore::Application::giveFocus(box);
            return;
        }
        this->moving = this->edit;   // from another day being edited: the edit goes on
        aurore::Application::giveFocus(box);
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
    // Editing, A names the day whose edit it ends ("Done with Monday"): the
    // changes are saved with + or Save, not by it.
    for (int d = 0; d < 7; d++)
        this->cols[d].box->updateActionHint(
            aurore::BUTTON_A, on ? aurore::getStr("playguard/cadran/week/done_day", aurore::getStr(fmt::format("cadran/days_lower/{}", d)))
                               : "playguard/cadran/week/edit"_i18n);
    aurore::Application::getGlobalHintsUpdateEvent()->fire();
    this->render();
    if (this->on_mode) this->on_mode(on);
}

void CadranWeekView::focus_day(int day)
{
    if (day >= 0 && day <= 6) aurore::Application::giveFocus(this->cols[day].box);
}

bool CadranWeekView::step(int day, int direction)
{
    if (!this->edit) return false;
    int at = 0;
    while (at < 7 && ORDER[at] != day) at++;
    const int next = at + direction;
    if (next < 0 || next > 6) return true;   // the edit stays in the week
    this->moving = true;
    aurore::Application::giveFocus(this->cols[ORDER[next]].box);
    this->moving = false;
    return true;
}

void CadranWeekView::nudge(int day, int delta)
{
    if (!this->valid || !this->on_change) return;
    const uint16_t cur = this->days[day];
    const int v = cur == PT_DAY_NOLIMIT ? (delta > 0 ? 0 : 1440) : (int)cur;
    // Held: 15 min a press, then whole hours, as the minutes prompt.
    this->on_change(day, (uint16_t)pt_logic::held_step(this->held, v, delta, aurore::getCPUTimeUsec()));
}

void CadranWeekView::show(const uint16_t shown[7], const uint16_t saved[7])
{
    for (int d = 0; d < 7; d++) {
        this->days[d] = shown[d];
        this->live[d] = saved[d];
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

void CadranWeekView::show_bedtimes(const std::string texts[7])
{
    bool any = false;
    for (int d = 0; d < 7; d++) any = any || !texts[d].empty();
    const float height = COL_HEIGHT + (any ? BED_HEIGHT : 0);
    this->setHeight(height);
    for (int d = 0; d < 7; d++) {
        Col& c = this->cols[d];
        c.box->setHeight(height);
        c.bed_time->setText(texts[d]);
        c.bed->setVisibility(any ? aurore::Visibility::VISIBLE : aurore::Visibility::GONE);
    }
}

void CadranWeekView::set_on_change(std::function<void(int, uint16_t)> f) { this->on_change = std::move(f); }
void CadranWeekView::set_on_pick(std::function<void(int)> f) { this->on_pick = std::move(f); }
void CadranWeekView::set_on_copy(std::function<void(int, bool)> f) { this->on_copy = std::move(f); }
void CadranWeekView::set_can_edit(std::function<bool()> f) { this->can_edit = std::move(f); }
void CadranWeekView::set_on_mode(std::function<void(bool)> f) { this->on_mode = std::move(f); }
void CadranWeekView::set_on_focus(std::function<void(int)> f) { this->on_focus = std::move(f); }

void CadranWeekView::onThemeChanged()
{
    aurore::Box::onThemeChanged();
    if (this->bars_set) this->render();
}

void CadranWeekView::render()
{
    aurore::Theme theme = aurore::Application::getTheme();
    const int today = ui::today_weekday();
    uint16_t top = 180;
    for (int d = 0; d < 7; d++)
        if (this->days[d] != PT_DAY_NOLIMIT) top = std::max(top, this->days[d]);
    float target[7];   // each bar's height asked
    for (int d = 0; d < 7; d++) {
        Col& c = this->cols[d];
        const uint16_t m = this->days[d];
        const bool is_focused = d == this->focused;
        const bool changed = this->valid && m != this->live[d];
        const bool over = this->valid && this->played_ok && m != PT_DAY_NOLIMIT && m > 0 && this->played[d] >= m;
        c.value->setText(!this->valid ? "—" : m == PT_DAY_NOLIMIT ? "—" : ui::fmt_played(m));
        // A limit gone past in coral, as its bar (the key says so), after a
        // triangle; one not
        // saved in amber, its bar ringed in amber too.
        c.value->setThemeColor("textColor", is_focused ? "cadran/teal" : changed ? "cadran/amber" : over ? "cadran/coral" : "cadran/text");
        // Not the colour alone: a triangle before it too (the key shows one).
        c.over->setVisibility(over ? aurore::Visibility::VISIBLE : aurore::Visibility::GONE);
        const float bar = !this->valid || m == PT_DAY_NOLIMIT ? BAR_MIN : std::max(BAR_MIN, BAR_MAX * (float)m / (float)top);
        target[d] = bar;   // laid out below, on its way there
        c.bar->setThemeColor("backgroundColor", is_focused ? "cadran/teal_fill" : d == today ? "cadran/bar_today" : "cadran/bar");
        c.bar->setThemeColor("borderColor", "cadran/amber_fill");
        c.bar->setBorderThickness(changed ? 2 : 0);
        // What was played that day, against its limit: up to the top. Past
        // it, the bar full: the part within the limit as below it, the part
        // over it in coral at the top, as much of the bar as of the time
        // played (a third at least, to be seen). Nothing on a day without
        // a limit. Last week's days fainter.
        const bool fills = this->valid && this->played_ok && m != PT_DAY_NOLIMIT && m > 0 && this->played[d] > 0;
        this->bars[d].fill_share = fills ? std::min(1.0f, (float)this->played[d] / (float)m) : 0.0f;
        if (fills) {
            // On the focused bar (teal itself), the teal darkened.
            NVGcolor shade = theme["cadran/navy"];
            shade.a = 0.35f;
            NVGcolor played_color = is_focused ? shade : theme["cadran/teal_fill"];
            NVGcolor past = theme["cadran/coral_fill"];
            // Fainter: towards the bar's own colour, still opaque (the part
            // within the limit is drawn over the coral).
            if (last_week(d) && !is_focused) {
                const NVGcolor under = theme["cadran/bar"];
                played_color = nvgLerpRGBA(under, played_color, 0.45f);
                past = nvgLerpRGBA(under, past, 0.45f);
            }
            const float within = over ? std::min(2.0f / 3.0f, (float)m / (float)this->played[d]) : 1.0f;
            c.fill->setBackgroundColor(over ? past : played_color);
            this->bars[d].within_share = within;
            c.within->setBackgroundColor(played_color);
            c.within->setVisibility(over ? aurore::Visibility::VISIBLE : aurore::Visibility::GONE);
        }
        c.fill->setVisibility(fills ? aurore::Visibility::VISIBLE : aurore::Visibility::GONE);
        c.day->setThemeColor("textColor", d == today ? "cadran/teal" : "cadran/text3");
        // The day being edited on the soft teal, its arrows shown.
        const bool editing = is_focused && this->edit;
        c.box->setThemeColor("backgroundColor", editing ? "cadran/teal_soft" : is_focused ? "cadran/surf2" : "cadran/none");
        c.up->setVisibility(editing ? aurore::Visibility::VISIBLE : aurore::Visibility::INVISIBLE);
        c.down->setVisibility(editing ? aurore::Visibility::VISIBLE : aurore::Visibility::INVISIBLE);
    }
    // The bars whose height changed start moving: one after the other,
    // outward from the focused day, when several do (a copy); at once when
    // one does, or the first time, or with the motion off.
    const aurore::Time now = motion::now();
    const auto place = [](int d) { return (d + 6) % 7; };   // Monday first
    int changed = 0;
    for (int d = 0; d < 7; d++) changed += target[d] != this->bars[d].to ? 1 : 0;
    for (int d = 0; d < 7; d++) {
        BarMove& b = this->bars[d];
        if (target[d] == b.to) continue;   // there, or on its way there
        if (!this->bars_set || !effects::motion()) {
            b.from  = b.to = target[d];
            b.since = 0;
            continue;
        }
        b.from = this->bar_now(d);   // from where it is drawn now
        b.to   = target[d];
        const int gap = changed > 1 && this->focused >= 0 ? std::abs(place(d) - place(this->focused)) : 0;
        b.since = now + (aurore::Time)gap * BAR_STAGGER_MS * 1000;
    }
    this->bars_set = true;
    this->layout_bars();
}

float CadranWeekView::bar_now(int day) const
{
    const BarMove& b = this->bars[day];
    if (!b.since) return b.to;
    const float t = motion::ease_out(motion::progress(b.since, BAR_MOVE_US));
    return b.from + (b.to - b.from) * t;
}

void CadranWeekView::layout_bars()
{
    for (int d = 0; d < 7; d++) {
        Col& c = this->cols[d];
        BarMove& b = this->bars[d];
        const float h = this->bar_now(d);
        if (b.since && motion::progress(b.since, BAR_MOVE_US) >= 1.0f) b.since = 0;   // arrived
        c.bar->setHeight(h);
        if (b.fill_share > 0.0f) c.fill->setHeight(std::max(6.0f, h * b.fill_share));
        c.within->setHeight(h * b.within_share);
    }
}

void CadranWeekView::draw(NVGcontext* vg, float x, float y, float width, float height, aurore::Style style,
                          aurore::FrameContext* ctx)
{
    bool on_the_move = false;
    for (const BarMove& b : this->bars) on_the_move = on_the_move || b.since != 0;
    if (on_the_move) this->layout_bars();
    aurore::Box::draw(vg, x, y, width, height, style, ctx);
}

bool CadranWeekView::last_week(int day)
{
    // Monday first: Sunday (0) is the week's last day.
    const auto place = [](int d) { return (d + 6) % 7; };
    return place(day) > place(ui::today_weekday());
}

aurore::View* CadranWeekView::create() { return new CadranWeekView(); }

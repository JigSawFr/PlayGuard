// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/icon_sheet.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <map>
#include <memory>

#include "action/forwarder_flow.hpp"
#include "action/icon_logic.hpp"
#include "core/album.h"
#include "core/calendar.h"
#include "ui/cadran_kit.hpp"
#include "ui/cadran_modal.hpp"
#include "ui/format_text.hpp"
#include "ui/ui.hpp"
#include "util/forwarder_icon.hpp"
#include "view/cadran_icon.hpp"

using namespace brls::literals;

namespace icon_sheet
{
namespace
{
namespace kit = cadran::kit;

constexpr int   COLUMNS = 4;
constexpr float TILE = 116;          // a cell's icon
constexpr float CELL_PAD = 6;        // around it, the focus's room
constexpr float CELL_GAP = 8;
constexpr float GRID_PAD = 20;
constexpr float PANEL_W = 1040;
constexpr float PANEL_H = 548;
constexpr float NOW_PX = 88;         // what the HOME menu shows now
constexpr float AFTER_PX = 184;      // what it will show
// A screenshot kept small for its cell: as wide as gives a cell's square
// (thirty of them about 3.5 MB, the album's thumbnails being 320 wide).
constexpr int   THUMB_W = 228;
constexpr int   TILE_PX = 128;       // a cell's pixels (drawn at TILE)

struct Pixels
{
    std::vector<uint8_t> rgba;
    int w = 0, h = 0;
};

// One choice: the .nro's own icon (`shot` empty), or a screenshot.
struct Choice
{
    std::string shot;          // "sdmc:/Nintendo/Album/…"
    int         x = forwarders::ICON_X_MIDDLE;
    bool        seconds = false;   // another taken the same minute: its time to the second
    std::string game;          // the game it was taken in ("" unknown)
    bool        no_game = false;   // taken outside a game (the HOME menu, an applet)
    Pixels      small;         // its thumbnail (the .nro's own: its icon)
    brls::Image* image = nullptr;
    brls::Box*   cell = nullptr;
};

void close_sheet(std::function<void()> then)
{
    brls::Application::popActivity(brls::TransitionAnimation::FADE, [then]() {
        if (then) brls::sync(then);
    });
}

brls::Box* outside(float width, bool grow)
{
    auto* box = new brls::Box();
    if (grow) box->setGrow(1);
    else box->setWidth(width);
    box->setShrink(0);
    box->addGestureRecognizer(new brls::TapGestureRecognizer(box, []() { close_sheet(nullptr); }));
    return box;
}

int texture_of(const std::vector<uint8_t>& rgba, int px)
{
    if (rgba.size() != (size_t)px * px * 4) return 0;
    return nvgCreateImageRGBA(brls::Application::getNVGContext(), px, px, 0, rgba.data());
}

// `image` shows `texture`, its own: it frees the one before (its own too).
void set_texture(brls::Image* image, int texture)
{
    if (!image || !texture) return;
    image->setScalingType(brls::ImageScalingType::FIT);
    image->innerSetImage(texture);
    image->setFreeTexture(true);
}

// "Today", "Yesterday", the day's name in the last week, else its date.
std::string day_word(const forwarders_logic::ShotTime& t, bool caption)
{
    const std::string today = ui::today_date();
    int days = 99;
    if (today.size() == 10)
        days = forwarders_logic::days_before(t, std::atoi(today.substr(0, 4).c_str()), std::atoi(today.substr(5, 2).c_str()),
                                             std::atoi(today.substr(8, 2).c_str()));
    const std::string ymd = fmt::format("{:04}-{:02}-{:02}", t.year, t.month, t.day);
    if (days == 0) return "playguard/forwarders/icon_today"_i18n;
    if (days == 1) return "playguard/forwarders/icon_yesterday"_i18n;
    if (days > 1 && days < 7)
        return ui::day_name(calendar_weekday(calendar_days_from_civil(t.year, (unsigned)t.month, (unsigned)t.day)));
    return caption ? ymd : kit::long_date(ymd);
}

class Sheet : public brls::Box
{
  public:
    Sheet(const forwarders_logic::Row& row, std::vector<std::string> shots,
          std::function<void(const std::string&, int)> done)
        : brls::Box(brls::Axis::COLUMN), row(row), done(std::move(done))
    {
        // Its .nro's own first, then the screenshots; the one in use focused.
        this->choices.push_back(Choice{});
        const std::vector<bool> same = forwarders_logic::same_minute(shots);
        for (size_t i = 0; i < shots.size(); i++) {
            Choice c;
            c.shot = shots[i];
            c.seconds = same[i];
            if (row.custom_icon && c.shot == row.icon) {
                c.x = row.icon_x;
                this->focus = this->choices.size();
            }
            this->choices.push_back(std::move(c));
        }
        // In use: its .nro's own, or a screenshot (none of these when it is
        // older than the newest thirty).
        this->in_use = !row.custom_icon ? 0 : this->focus ? this->focus : this->choices.size();

        brls::Theme theme = brls::Application::getTheme();
        this->setId("cadran_sheet");
        this->setBackgroundColor(theme["brls/backdrop"]);
        this->setAlignItems(brls::AlignItems::STRETCH);
        this->addView(outside(0, true));

        auto* panel = new brls::Box(brls::Axis::COLUMN);
        panel->setWidth(PANEL_W);
        panel->setHeight(PANEL_H);
        panel->setCornerRadius(cadran::PICKER_CORNER);
        panel->setBackgroundColor(theme["cadran/surf"]);
        panel->setBorderColor(theme["cadran/line"]);
        panel->setBorderThickness(1);
        panel->setAlignItems(brls::AlignItems::STRETCH);
        panel->addView(this->header());

        auto* body = new brls::Box(brls::Axis::ROW);
        body->setGrow(1);
        body->setAlignItems(brls::AlignItems::STRETCH);
        body->addView(this->grid());
        body->addView(this->preview());
        panel->addView(body);

        auto* line = new brls::Box(brls::Axis::ROW);
        line->setJustifyContent(brls::JustifyContent::CENTER);
        line->setShrink(0);
        line->addView(outside(0, true));
        line->addView(panel);
        line->addView(outside(0, true));
        this->addView(line);
        this->addView(outside(20, false));   // the gap above the hints
        this->addView(new brls::BottomBar());

        this->registerAction("hints/back"_i18n, brls::BUTTON_B, [](brls::View*) {
            close_sheet(nullptr);
            return true;
        }, false, false, brls::SOUND_BACK);
        this->registerAction("playguard/forwarders/icon_own"_i18n, brls::BUTTON_Y, [this](brls::View*) {
            this->choose(0);
            return true;
        });
        this->registerAction("playguard/forwarders/icon_move"_i18n, brls::BUTTON_RB, [this](brls::View*) {
            this->move(1);
            return true;
        }, false, true);
        this->registerAction("playguard/forwarders/icon_move"_i18n, brls::BUTTON_LB, [this](brls::View*) {
            this->move(-1);
            return true;
        }, true, true);
#if !defined(__SWITCH__) && !defined(__EMSCRIPTEN__)
        // The desktop build: the keyboard has no Y, L or R; its y, q and e.
        this->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_Y), [this](brls::View*) {
            this->choose(0);
            return true;
        });
        this->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_Q), [this](brls::View*) {
            this->move(-1);
            return true;
        });
        this->registerAction(brls::BrlsKeyCombination(brls::BRLS_KBD_KEY_E), [this](brls::View*) {
            this->move(1);
            return true;
        });
#endif

        this->load();
    }

    bool isTranslucent() override { return true; }

    brls::View* getDefaultFocus() override
    {
        if (this->focus < this->choices.size() && this->choices[this->focus].cell) return this->choices[this->focus].cell;
        return brls::Box::getDefaultFocus();
    }

  private:
    const forwarders_logic::Row row;
    std::function<void(const std::string&, int)> done;
    std::vector<Choice> choices;
    size_t focus = 0, in_use = 0;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);

    // The one in focus, in full once read: a sharper "after" than its thumbnail.
    std::string full_shot;
    Pixels      full;
    bool        reading_full = false;

    brls::Image* now = nullptr;
    brls::Image* after = nullptr;
    brls::Label* title = nullptr;
    brls::Label* when = nullptr;
    brls::Box*   frame_row = nullptr;
    std::vector<brls::Box*> frame_dots;

    brls::Box* header()
    {
        brls::Theme theme = brls::Application::getTheme();
        auto* head = new brls::Box(brls::Axis::COLUMN);
        head->setPadding(20, 36, 18, 36);
        head->setLineColor(theme["cadran/line"]);
        head->setLineBottom(1);
        head->setShrink(0);
        brls::Label* name = kit::label("playguard/forwarders/icon_title"_i18n, 22, "cadran/text");
        name->setSingleLine(true);
        head->addView(name);
        brls::Label* sub = kit::label(this->row.name, kit::TEXT_META, "cadran/text2");
        sub->setSingleLine(true);
        sub->setMarginTop(5);
        head->addView(sub);
        return head;
    }

    brls::View* grid()
    {
        auto* frame = new brls::ScrollingFrame();
        frame->setWidth(GRID_PAD * 2 + COLUMNS * (TILE + 2 * CELL_PAD) + (COLUMNS - 1) * CELL_GAP);
        frame->setShrink(0);
        frame->setScrollingBehavior(brls::ScrollingBehavior::CENTERED);
        auto* column = new brls::Box(brls::Axis::COLUMN);
        column->setPadding(GRID_PAD, GRID_PAD, GRID_PAD, GRID_PAD);
        brls::Box* line = nullptr;
        for (size_t i = 0; i < this->choices.size(); i++) {
            if (i % COLUMNS == 0) {
                line = new brls::Box(brls::Axis::ROW);
                if (i) line->setMarginTop(CELL_GAP);
                column->addView(line);
            }
            brls::Box* cell = this->cell(i);
            if (i % COLUMNS) cell->setMarginLeft(CELL_GAP);
            line->addView(cell);
        }
        // Up and down stay in their column.
        for (size_t i = 0; i < this->choices.size(); i++) {
            if (i >= COLUMNS)
                this->choices[i].cell->setCustomNavigationRoute(brls::FocusDirection::UP, this->choices[i - COLUMNS].cell);
            if (i + COLUMNS < this->choices.size())
                this->choices[i].cell->setCustomNavigationRoute(brls::FocusDirection::DOWN, this->choices[i + COLUMNS].cell);
            else if (i / COLUMNS + 1 < (this->choices.size() + COLUMNS - 1) / COLUMNS)
                this->choices[i].cell->setCustomNavigationRoute(brls::FocusDirection::DOWN, this->choices.back().cell);
        }
        frame->setContentView(column);
        return frame;
    }

    brls::Box* cell(size_t index)
    {
        brls::Theme theme = brls::Application::getTheme();
        Choice& c = this->choices[index];
        auto* cell = new brls::Box(brls::Axis::COLUMN);
        cell->setAlignItems(brls::AlignItems::CENTER);
        cell->setPadding(CELL_PAD, CELL_PAD, CELL_PAD, CELL_PAD);
        cell->setCornerRadius(16);
        cell->setHighlightCornerRadius(18);
        cell->setFocusable(true);
        auto* slot = new brls::Box();
        slot->setWidth(TILE);
        slot->setHeight(TILE);
        auto* image = new brls::Image();
        image->setWidth(TILE);
        image->setHeight(TILE);
        image->setCornerRadius(kit::RADIUS_ICON + 4);
        image->setBackgroundColor(theme["cadran/well"]);
        slot->addView(image);
        // The one the HOME menu shows: its check on its corner.
        if (index == this->in_use) {
            auto* badge = new brls::Box();
            badge->setPositionType(brls::PositionType::ABSOLUTE);
            badge->setPositionTop(TILE - 30);
            badge->setPositionLeft(TILE - 30);
            badge->setWidth(26);
            badge->setHeight(26);
            badge->setCornerRadius(13);
            badge->setBackgroundColor(theme["cadran/teal"]);
            badge->setJustifyContent(brls::JustifyContent::CENTER);
            badge->setAlignItems(brls::AlignItems::CENTER);
            badge->addView(kit::icon("check", 16, "cadran/surf"));
            slot->addView(badge);
        }
        cell->addView(slot);
        brls::Label* caption = kit::label(this->caption(c), 14, index == this->in_use ? "cadran/teal" : "cadran/text2");
        caption->setSingleLine(true);
        caption->setMaxWidth(TILE);
        caption->setMarginTop(6);
        cell->addView(caption);
        cell->getFocusEvent()->subscribe([this, index](brls::View*) { this->select(index); });
        cell->registerClickAction([this, index](brls::View*) {
            this->choose(index);
            return true;
        });
        cell->addGestureRecognizer(new brls::TapGestureRecognizer(cell));
        c.image = image;
        c.cell = cell;
        return cell;
    }

    std::string caption(const Choice& c) const
    {
        if (c.shot.empty()) return "playguard/forwarders/icon_own"_i18n;
        forwarders_logic::ShotTime t;
        if (!forwarders_logic::shot_time(c.shot, t)) return forwarders_logic::file_title(c.shot);
        return day_word(t, true);
    }

    // Beside the grid: now, then after; the game and when; the frame's place.
    brls::Box* preview()
    {
        brls::Theme theme = brls::Application::getTheme();
        auto* side = new brls::Box(brls::Axis::COLUMN);
        side->setGrow(1);
        side->setWidth(0);
        side->setPadding(26, 32, 24, 28);
        side->setLineColor(theme["cadran/line"]);
        side->setLineLeft(1);
        auto* kicker = new brls::Label();
        kit::set_kicker(kicker, "playguard/forwarders/icon_preview"_i18n);
        kicker->setSingleLine(true);
        side->addView(kicker);

        auto* pair = new brls::Box(brls::Axis::ROW);
        pair->setAlignItems(brls::AlignItems::FLEX_END);
        pair->setMarginTop(18);
        auto figure = [&theme](float px, const std::string& label, brls::Image** out) {
            auto* box = new brls::Box(brls::Axis::COLUMN);
            box->setAlignItems(brls::AlignItems::CENTER);
            auto* image = new brls::Image();
            image->setWidth(px);
            image->setHeight(px);
            image->setCornerRadius(px * 0.14f);
            image->setBackgroundColor(theme["cadran/well"]);
            box->addView(image);
            brls::Label* l = kit::label(label, 14, "cadran/text3");
            l->setSingleLine(true);
            l->setMarginTop(8);
            box->addView(l);
            *out = image;
            return box;
        };
        pair->addView(figure(NOW_PX, "playguard/forwarders/icon_now"_i18n, &this->now));
        CadranIcon* arrow = kit::icon("chevron-right", 22, "cadran/text3");
        arrow->setMargins(0, 18, 26 + NOW_PX / 2 - 11, 18);
        pair->addView(arrow);
        pair->addView(figure(AFTER_PX, "playguard/forwarders/icon_after"_i18n, &this->after));
        side->addView(pair);

        this->title = kit::label("", 20, "cadran/text");
        this->title->setSingleLine(true);
        this->title->setMarginTop(22);
        side->addView(this->title);
        this->when = kit::label("", kit::TEXT_META, "cadran/text2");
        this->when->setSingleLine(true);
        this->when->setMarginTop(4);
        side->addView(this->when);

        // Where the square sits along the screenshot: five places, L and R.
        this->frame_row = new brls::Box(brls::Axis::ROW);
        this->frame_row->setAlignItems(brls::AlignItems::CENTER);
        this->frame_row->setMarginTop(18);
        brls::Box* l = kit::key("L", 24);
        l->setMarginRight(10);
        l->addGestureRecognizer(new brls::TapGestureRecognizer(l, [this]() { this->move(-1); }));
        this->frame_row->addView(l);
        for (int i = 0; i < 5; i++) {
            auto* dot = new brls::Box();
            dot->setWidth(10);
            dot->setHeight(10);
            dot->setCornerRadius(5);
            dot->setMarginRight(i < 4 ? 8 : 10);
            this->frame_dots.push_back(dot);
            this->frame_row->addView(dot);
        }
        brls::Box* r = kit::key("R", 24);
        r->addGestureRecognizer(new brls::TapGestureRecognizer(r, [this]() { this->move(1); }));
        this->frame_row->addView(r);
        brls::Label* hint = kit::label("playguard/forwarders/icon_move"_i18n, kit::TEXT_META, "cadran/text3");
        hint->setSingleLine(true);
        hint->setMarginLeft(12);
        this->frame_row->addView(hint);
        side->addView(this->frame_row);
        return side;
    }

    void select(size_t index)
    {
        if (index >= this->choices.size()) return;
        this->focus = index;
        const Choice& c = this->choices[index];
        if (c.shot.empty()) {
            this->title->setText("playguard/forwarders/icon_own"_i18n);
            this->when->setText("playguard/forwarders/icon_own_note"_i18n);
        } else {
            this->title->setText(!c.game.empty() ? c.game
                                 : c.no_game     ? "playguard/forwarders/icon_no_game"_i18n
                                                 : "playguard/forwarders/icon_shot"_i18n);
            forwarders_logic::ShotTime t;
            if (forwarders_logic::shot_time(c.shot, t)) {
                const std::string at = c.seconds ? fmt::format("{:02}:{:02}:{:02}", t.hour, t.minute, t.second)
                                                 : fmt::format("{:02}:{:02}", t.hour, t.minute);
                this->when->setText(brls::getStr("playguard/forwarders/icon_when", day_word(t, false), at));
            } else {
                this->when->setText(forwarders_logic::file_title(c.shot));
            }
        }
        this->frame_row->setVisibility(c.shot.empty() ? brls::Visibility::INVISIBLE : brls::Visibility::VISIBLE);
        for (size_t i = 0; i < this->frame_dots.size(); i++)
            this->frame_dots[i]->setBackgroundColor(kit::color((int)i == (c.x + 12) / 25 ? "cadran/teal" : "cadran/line"));
        this->draw_after();
        if (!c.shot.empty() && c.shot != this->full_shot) this->read_full(c.shot);
    }

    // The "after" of the one in focus: in full once read, else its thumbnail.
    void draw_after()
    {
        const Choice& c = this->choices[this->focus];
        const Pixels& p = !c.shot.empty() && c.shot == this->full_shot && !this->full.rgba.empty() ? this->full : c.small;
        if (p.rgba.empty()) {
            this->after->clear();
            return;
        }
        set_texture(this->after, texture_of(forwarder_icon::square_rgba(p.rgba.data(), p.w, p.h, c.x, 256), 256));
    }

    void draw_cell(size_t index)
    {
        const Choice& c = this->choices[index];
        if (c.small.rgba.empty()) return;
        set_texture(c.image, texture_of(forwarder_icon::square_rgba(c.small.rgba.data(), c.small.w, c.small.h, c.x, TILE_PX),
                                        TILE_PX));
        // What the HOME menu shows now: the one in use, its square where it was.
        if (index == this->in_use) {
            const int x = c.shot.empty() ? forwarders::ICON_X_MIDDLE : this->row.icon_x;
            set_texture(this->now, texture_of(forwarder_icon::square_rgba(c.small.rgba.data(), c.small.w, c.small.h, x, TILE_PX),
                                              TILE_PX));
        }
    }

    void move(int step)
    {
        if (this->focus >= this->choices.size()) return;
        Choice& c = this->choices[this->focus];
        if (c.shot.empty()) return;
        const int x = forwarders_logic::icon_x_step(c.x, step);
        if (x == c.x) return;
        c.x = x;
        this->draw_cell(this->focus);
        this->select(this->focus);
    }

    void choose(size_t index)
    {
        if (index >= this->choices.size()) return;
        const Choice& c = this->choices[index];
        const bool same = c.shot.empty() ? !this->row.custom_icon
                                         : this->row.custom_icon && c.shot == this->row.icon && c.x == this->row.icon_x;
        std::function<void()> then;
        if (!same) {
            const auto done = this->done;
            const std::string shot = c.shot;
            const int x = c.x;
            then = [done, shot, x]() {
                if (done) done(shot, x);
            };
        }
        close_sheet(then);
    }

    // Off the UI thread: the .nro's own icon, the games, then each
    // screenshot's thumbnail, each shown as it comes.
    void load()
    {
        std::weak_ptr<bool> weak = this->alive;
        std::vector<std::string> shots;
        for (const Choice& c : this->choices) shots.push_back(c.shot);
        const forwarders_logic::Row r = this->row;
        brls::async([this, weak, shots, r]() {
            Pixels own;
            own.rgba = forwarder_icon::rgba_of(forwarder_flow::own_icon(r), own.w, own.h);
            // The games, from caps:a; their names from ns, once each.
            std::vector<AlbumShot> found(512);
            found.resize(album_shots_of_games(found.data(), found.size()));
            std::map<std::string, u64> game_of;
            for (const AlbumShot& a : found) game_of[a.stamp] = a.app_id;
            std::map<u64, std::string> names;
            auto games = std::make_shared<std::vector<std::pair<std::string, bool>>>();
            for (const std::string& s : shots) {
                const auto it = s.empty() ? game_of.end() : game_of.find(forwarders_logic::shot_stamp(s));
                if (it == game_of.end()) {
                    games->push_back({ "", false });
                    continue;
                }
                if (!it->second) {
                    games->push_back({ "", true });
                    continue;
                }
                if (!names.count(it->second)) {
                    char name[0x201] = {};
                    names[it->second] = album_game_name(it->second, name, sizeof(name)) ? name : "";
                }
                games->push_back({ names[it->second], false });
            }
            brls::sync([this, weak, own, games]() {
                if (weak.expired()) return;
                this->choices[0].small = own;
                this->draw_cell(0);
                for (size_t i = 0; i < games->size() && i < this->choices.size(); i++) {
                    this->choices[i].game = (*games)[i].first;
                    this->choices[i].no_game = (*games)[i].second;
                }
                this->select(this->focus);
            });
            for (size_t i = 1; i < shots.size(); i++) {
                if (weak.expired()) return;
                Pixels p = small_of(shots[i]);
                brls::sync([this, weak, i, p]() {
                    if (weak.expired()) return;
                    this->choices[i].small = p;
                    this->draw_cell(i);
                    if (i == this->focus) this->draw_after();
                });
            }
        });
    }

    // A screenshot small: its Exif thumbnail, else itself shrunk to as wide.
    static Pixels small_of(const std::string& shot)
    {
        Pixels p;
        const std::vector<uint8_t> jpeg = forwarder_flow::shot_bytes(shot);
        if (jpeg.empty()) return p;
        std::vector<uint8_t> thumb = forwarder_icon::exif_thumbnail(jpeg);
        p.rgba = forwarder_icon::rgba_of(thumb.empty() ? jpeg : thumb, p.w, p.h);
        if (p.w > THUMB_W) {
            const int h = std::max(1, p.h * THUMB_W / p.w);
            p.rgba = icon_logic::shrink(p.rgba.data(), p.w, p.h, THUMB_W, h);
            p.w = p.rgba.empty() ? 0 : THUMB_W;
            p.h = p.rgba.empty() ? 0 : h;
        }
        return p;
    }

    // The one in focus in full, for its "after": one read at a time, the
    // last one asked read next.
    void read_full(const std::string& shot)
    {
        this->full_shot = shot;
        this->full = Pixels{};
        if (this->reading_full) return;
        this->reading_full = true;
        std::weak_ptr<bool> weak = this->alive;
        brls::async([this, weak, shot]() {
            Pixels p;
            p.rgba = forwarder_icon::rgba_of(forwarder_flow::shot_bytes(shot), p.w, p.h);
            brls::sync([this, weak, shot, p]() {
                if (weak.expired()) return;
                this->reading_full = false;
                if (shot != this->full_shot) {
                    this->read_full(this->full_shot);
                    return;
                }
                this->full = p;
                if (this->choices[this->focus].shot == shot) this->draw_after();
            });
        });
    }
};

}   // namespace

void open(const forwarders_logic::Row& row, std::vector<std::string> shots,
          std::function<void(const std::string& shot, int x)> done)
{
    brls::Application::pushActivity(new brls::Activity(new Sheet(row, std::move(shots), std::move(done))),
                                    brls::TransitionAnimation::FADE);
}

}   // namespace icon_sheet

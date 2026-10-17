// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_verify_flow.hpp"

#include <fmt/format.h>

#include "ui/ui.hpp"

using namespace brls::literals;

namespace game_verify_flow
{

// A plain progress bar: the track, then the part done in the icon's teal.
class ProgressBar : public brls::View
{
  public:
    ProgressBar()
        : track(ui::color_track()), done(ui::color_ok())
    {
        this->setHeight(12);
        this->setFocusable(false);
    }

    void set(float fraction)
    {
        this->fraction = fraction < 0 ? 0 : fraction > 1 ? 1 : fraction;
    }

    void draw(NVGcontext* vg, float x, float y, float width, float height, brls::Style, brls::FrameContext*) override
    {
        const float radius = height / 2.0f;
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, width, height, radius);
        nvgFillColor(vg, this->track);
        nvgFill(vg);
        if (this->fraction <= 0) return;
        float w = width * this->fraction;
        if (w < height) w = height;
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, height, radius);
        nvgFillColor(vg, this->done);
        nvgFill(vg);
    }

  private:
    float fraction = 0;
    NVGcolor track, done;
};

namespace
{
// "850 MB", "3.8 GB" (the language's decimal mark).
std::string size_text(u64 bytes)
{
    const u64 mb = bytes >> 20;
    if (mb < 1024) return brls::getStr("playguard/games/size_mb", (int)mb);
    std::string gb = fmt::format("{:.1f}", (double)bytes / (double)(1ULL << 30));
    const std::string mark = "playguard/games/decimal_mark"_i18n;
    const size_t dot = gb.find('.');
    if (dot != std::string::npos) gb.replace(dot, 1, mark);
    return brls::getStr("playguard/games/size_gb", gb);
}

// What the dialog says at `current` of `total` bytes: the share done, how
// much, and the time left once it can be told (a few seconds in).
void show_progress(State* s, u64 current, u64 total)
{
    if (!total) {
        s->bar->set(0);
        s->percent->setText("playguard/games/verify_preparing"_i18n);
        s->detail->setText("");
        return;
    }
    if (current > total) current = total;
    s->bar->set((float)((double)current / (double)total));
    s->percent->setText(fmt::format("{} %", (int)(current * 100 / total)));
    std::string detail = brls::getStr("playguard/games/verify_amount", size_text(current), size_text(total));
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - s->started).count();
    if (current > 0 && elapsed >= 3) {
        const double left = elapsed * (double)(total - current) / (double)current;
        detail += " · " + (left < 60 ? "playguard/games/verify_left_less"_i18n
                                      : brls::getStr("playguard/games/verify_left_min", (int)(left / 60 + 0.5)));
    }
    s->detail->setText(detail);
}

bool on_verify_progress(void* ctx, u64 current, u64 total)
{
    auto* state = (std::shared_ptr<State>*)ctx;
    const std::shared_ptr<State> s = *state;
    brls::sync([s, current, total]() {
        if (s->open) show_progress(s.get(), current, total);
    });
    return !s->cancel.load();
}
}   // namespace

std::shared_ptr<State> make_dialog()
{
    auto state = std::make_shared<State>();
    auto* box = new brls::Box(brls::Axis::COLUMN);
    box->setAlignItems(brls::AlignItems::STRETCH);
    box->setPadding(26, 36, 20, 36);

    auto* head = new brls::Box(brls::Axis::ROW);
    head->setAlignItems(brls::AlignItems::CENTER);
    state->image = new brls::Image();
    state->image->setScalingType(brls::ImageScalingType::FIT);
    state->image->setWidth(56);
    state->image->setHeight(56);
    state->image->setMarginRight(16);
    head->addView(state->image);
    auto* titles = new brls::Box(brls::Axis::COLUMN);
    titles->setGrow(1.0f);
    titles->setShrink(1.0f);
    state->title = ui::text_label("playguard/games/verify_title"_i18n, 18, ui::color_note());
    titles->addView(state->title);
    state->name = ui::text_label("", 22, ui::color_text());
    state->name->setSingleLine(true);   // a long name is cut, not wrapped over the dialog
    state->name->setMarginTop(2);
    titles->addView(state->name);
    head->addView(titles);
    box->addView(head);

    state->bar = new ProgressBar();
    state->bar->setMarginTop(22);
    box->addView(state->bar);
    auto* numbers = new brls::Box(brls::Axis::ROW);
    numbers->setAlignItems(brls::AlignItems::CENTER);
    numbers->setMarginTop(10);
    state->percent = ui::text_label("", 22, ui::color_text());
    state->percent->setSingleLine(true);
    state->percent->setMarginRight(14);
    numbers->addView(state->percent);
    state->detail = ui::text_label("", 18, ui::color_note());
    state->detail->setSingleLine(true);
    state->detail->setGrow(1.0f);
    state->detail->setShrink(1.0f);
    state->detail->setHorizontalAlign(brls::HorizontalAlign::RIGHT);
    numbers->addView(state->detail);
    box->addView(numbers);

    auto* note_label = ui::text_label("playguard/games/verify_note"_i18n, 18, ui::color_note());
    note_label->setMarginTop(16);
    box->addView(note_label);
    state->dialog = new brls::Dialog(box);
    auto cancel = [state]() {
        state->cancel = true;
        state->open = false;   // the dialog closes itself
    };
    state->dialog->addButton("hints/cancel"_i18n, cancel);
    ui::on_cancel(state->dialog, cancel);
    return state;
}

void show_game(State* s, const std::vector<unsigned char>& icon, const std::string& name, size_t at, size_t total)
{
    if (!icon.empty()) s->image->setImageFromMem(icon.data(), (int)icon.size());
    ui::set_visible(s->image, !icon.empty());
    s->title->setText(total == 1 ? "playguard/games/verify_title"_i18n
                                 : brls::getStr("playguard/games/verify_title_n", (int)at + 1, (int)total));
    s->name->setText(name);
    s->started = std::chrono::steady_clock::now();
    show_progress(s, 0, 0);
}

Result verify(const std::shared_ptr<State>& state, u64 app_id, bool* corrupt, Result* found)
{
    auto ctx = state;
    return gamecheck_verify(app_id, on_verify_progress, &ctx, corrupt, found);
}

void close(const std::shared_ptr<State>& state, std::function<void()> then)
{
    if (state->open) {
        state->open = false;
        state->dialog->close(then);
    } else {
        then();
    }
}

}   // namespace game_verify_flow

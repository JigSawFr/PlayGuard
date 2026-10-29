// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/game_verify_flow.hpp"

#include <fmt/format.h>

#include <aurore/cadran/ui/cadran_kit.hpp>

#include "ui/ui.hpp"

using namespace aurore::literals;

namespace game_verify_flow
{

// "850 MB", "3.8 GB" (the language's decimal mark).
std::string size_text(u64 bytes)
{
    const u64 mb = bytes >> 20;
    if (mb < 1024) return aurore::getStr("playguard/games/size_mb", (int)mb);
    std::string gb = fmt::format("{:.1f}", (double)bytes / (double)(1ULL << 30));
    const std::string mark = "playguard/games/decimal_mark"_i18n;
    const size_t dot = gb.find('.');
    if (dot != std::string::npos) gb.replace(dot, 1, mark);
    return aurore::getStr("playguard/games/size_gb", gb);
}

namespace
{
// What the dialog says at `current` of `total` bytes: the share done, how
// much, and the time left once it can be told (a few seconds in).
void show_progress(State* s, u64 current, u64 total)
{
    if (!total) {
        s->body.show(0, "");
        s->body.percent->setText("playguard/games/verify_preparing"_i18n);
        return;
    }
    if (current > total) current = total;
    std::string detail = aurore::getStr("playguard/games/verify_amount", size_text(current), size_text(total));
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - s->started).count();
    if (current > 0 && elapsed >= 3) {
        const double left = elapsed * (double)(total - current) / (double)current;
        detail += " · " + (left < 60 ? "playguard/games/verify_left_less"_i18n
                                      : aurore::getStr("playguard/games/verify_left_min", (int)(left / 60 + 0.5)));
    }
    s->body.show((float)((double)current / (double)total), detail);
}

bool on_verify_progress(void* ctx, u64 current, u64 total)
{
    auto* state = (std::shared_ptr<State>*)ctx;
    const std::shared_ptr<State> s = *state;
    aurore::sync([s, current, total]() {
        if (s->open) show_progress(s.get(), current, total);
    });
    return !s->cancel.load();
}
}   // namespace

std::shared_ptr<State> make_dialog(const std::vector<u64>& ids)
{
    auto state = std::make_shared<State>();
    state->body = cadran::progress_body("playguard/games/verify_title"_i18n, "", "playguard/games/verify_note"_i18n);
    if (ids.size() > 1) state->strip = cover_strip_above(state->body, ids);
    state->dialog = new aurore::Dialog(state->body.box);
    auto cancel = [state]() {
        state->cancel = true;
        state->open = false;   // the dialog closes itself
    };
    ui::add_cancel(state->dialog, cancel);
    return state;
}

void show_game(State* s, const std::vector<unsigned char>& icon, const std::string& name, size_t at, size_t total)
{
    if (s->strip) {
        s->strip->set_done((u32)at);
        cover_strip_read(s->strip);
    } else if (!icon.empty()) {
        s->body.image->setImageFromMem(icon.data(), (int)icon.size());
    }
    ui::set_visible(s->body.image, !s->strip && !icon.empty());
    s->body.heading->setText(total == 1 ? "playguard/games/verify_title"_i18n
                                 : aurore::getStr("playguard/games/verify_title_n", (int)at + 1, (int)total));
    s->body.title->setText(name);
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

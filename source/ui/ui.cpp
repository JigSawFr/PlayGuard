// ui — result texts, background tasks, toasts, screen changes and the main
// screen's state. The other ui/*.cpp hold the rest of ui.hpp by concern.
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "ui/ui.hpp"

#include "action/error_hint_logic.hpp"
#include "action/pt_log_flow.hpp"
#include "action/write_queue_logic.hpp"
#include "activity/main_activity.hpp"
#include "tab/tab_base.hpp"
#include "util/config.hpp"

#include <atomic>
#include <condition_variable>
#include <fmt/format.h>
#include <mutex>
#include <system_error>
#include <thread>

using namespace brls::literals;

namespace ui
{

static std::string hint_for(Result rc)
{
    const std::string key = error_hint_logic::hint_key(rc);
    return key.empty() ? std::string() : brls::getStr("playguard/error/" + key);
}

std::string rc_text(Result rc, bool next_step)
{
    std::string hint = hint_for(rc);
    if (NXM_IS_APP_RESULT(rc) && !hint.empty()) {
        if (hint[0] >= 'a' && hint[0] <= 'z') hint[0] = (char)(hint[0] - 'a' + 'A');
        return hint;
    }
    std::string code = fmt::format("0x{:08X}", (unsigned)rc);
    std::string text = hint.empty() ? brls::getStr("playguard/error/code", code)
                                    : brls::getStr("playguard/error/code_hint", code, hint);
    // A system code alone leaves the parent stuck: say what to do next. App
    // results already explain themselves.
    if (next_step && !NXM_IS_APP_RESULT(rc)) text += "\n\n" + "playguard/error/next_step"_i18n;
    return text;
}

namespace
{
std::mutex s_bg_lock;
std::condition_variable s_bg_done;
int s_bg_running = 0;   // tasks of in_background on their own thread
std::atomic<bool> s_quitting{ false };
write_queue_logic::Queue s_writes;
std::atomic<bool> s_config_saved{ true };   // the last config.json write went through
}   // namespace

void in_background(const char* what, std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(s_bg_lock);
        s_bg_running++;
    }
    try {
        std::thread([task]() {
            task();
            std::lock_guard<std::mutex> lock(s_bg_lock);
            s_bg_running--;
            s_bg_done.notify_all();
        }).detach();
    } catch (const std::system_error& e) {
        {
            std::lock_guard<std::mutex> lock(s_bg_lock);
            s_bg_running--;
        }
        brls::Logger::warning("{}: no thread ({}), queued instead", what, e.what());
        brls::async(task);
    }
}

bool quitting()
{
    return s_quitting;
}

bool finish_background(std::chrono::milliseconds max)
{
    s_quitting = true;
    const auto until = std::chrono::steady_clock::now() + max;
    // The SD-card writes first: what the parent changed last is not lost.
    if (!s_writes.flush(max)) brls::Logger::warning("quitting with SD-card writes still running");
    std::unique_lock<std::mutex> lock(s_bg_lock);
    const bool done = s_bg_done.wait_until(lock, until, []() { return s_bg_running == 0; });
    if (!done) brls::Logger::warning("quitting with {} background task(s) still running", s_bg_running);
    return done;
}

static uint64_t push_write(const char* what, std::function<void()> job, const std::string& key)
{
    bool start = false;
    const uint64_t ticket = s_writes.push(key, std::move(job), &start);
    if (start) in_background(what, []() { s_writes.run(); });
    return ticket;
}

void write_later(const char* what, std::function<void()> job, const std::string& key)
{
    push_write(what, std::move(job), key);
}

void settle_writes()
{
    s_writes.settle();
}

void notify(const std::string& text)
{
    // Logged too: the desktop smoke test checks what the user was told.
    brls::Logger::info("toast: {}", text);
    brls::sync([text]() { brls::Application::notify(text); });
}

void error(const std::string& text)
{
    brls::Logger::info("error: {}", text);
    brls::sync([text]() { info(text); });
}

void replace_screen(brls::Activity* next)
{
    const auto none = brls::TransitionAnimation::NONE;
    if (!brls::Application::popActivity(none, [next, none]() { brls::Application::pushActivity(next, none); }))
        brls::Application::pushActivity(next, none);
}

// config.json as it is now, queued (*ticket to wait for). False when it
// cannot be saved at all.
static bool queue_config(const std::string& saved_text, const std::string& failed_text, uint64_t* ticket)
{
    const std::string failed = failed_text.empty() ? "playguard/toast/config_err"_i18n : failed_text;
    std::string text;
    if (!config::snapshot(text)) {
        notify(failed);
        return false;
    }
    *ticket = push_write("config", [text, saved_text, failed]() {
        const bool saved = config::write(text);
        s_config_saved = saved;
        if (!saved) notify(failed);
        else if (!saved_text.empty()) notify(saved_text);
    }, "config");
    return true;
}

bool save_config(const std::string& saved_text, const std::string& failed_text)
{
    uint64_t ticket;
    return queue_config(saved_text, failed_text, &ticket);
}

bool save_config_now()
{
    uint64_t ticket;
    if (!queue_config("", "", &ticket)) return false;
    s_writes.wait(ticket);
    return s_config_saved;
}

void notify_result(Result rc, const std::string& ok_text, const std::string& error_prefix)
{
    if (R_SUCCEEDED(rc)) notify(ok_text);
    else error(error_prefix + " — " + rc_text(rc));
}

static bool s_unlocked = false;

bool known_unlocked() { return s_unlocked; }

void note_unlocked(bool valid, bool unlocked)
{
    if (!valid || unlocked == s_unlocked) return;
    s_unlocked = unlocked;
    // Next frame: at start-up the first tab is read before the main screen is
    // on the activity stack.
    brls::sync([]() {
        for (brls::Activity* activity : brls::Application::getActivitiesStack())
            if (auto* main = dynamic_cast<MainActivity*>(activity)) main->update_title();
    });
}

void on_mode_changed()
{
    pt_log_flow::apply();   // the recorder only runs in developer mode
    for (brls::Activity* activity : brls::Application::getActivitiesStack())
        if (auto* main = dynamic_cast<MainActivity*>(activity)) main->update_title();
    TabBase::refresh_shown();
}

}   // namespace ui

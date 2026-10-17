// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/dev_build_flow.hpp"

#include <algorithm>
#include <atomic>
#include <borealis.hpp>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <vector>

#include "action/github_login_flow.hpp"
#include "action/pin_lock.hpp"
#include "action/rollback_flow.hpp"
#include "activity/dev_build_activity.hpp"
#include "activity/dev_builds_activity.hpp"
#include "app.hpp"
#include "core/platform.h"
#include "ui/ui.hpp"
#include "util/dev_builds.hpp"
#include "util/github_auth.hpp"
#include "util/launcher.hpp"
#include "util/paths.hpp"
#include "util/rollback.hpp"

using namespace brls::literals;

namespace dev_build_flow
{

namespace
{
bool s_busy = false;   // one list or download at a time

// The last list fetched (dev_builds::Cache), read from the SD card on the
// first opening of this run.
dev_builds::Cache s_cache;
bool              s_cache_read = false;

std::string cache_file() { return paths::cache_dir() + "/dev_builds.json"; }

void read_cache()
{
    if (s_cache_read) return;
    s_cache_read = true;
    std::string text;
    if (paths::read_file(cache_file(), text) && !dev_builds::decode_cache(text, &s_cache)) s_cache = {};
}

void keep(const dev_builds::Cache& cache)
{
    s_cache = cache;
    std::string err;
    // Only saves time on the next start: a write that fails is just logged.
    if (!paths::ensure_dir(paths::cache_dir()) ||
        !paths::atomic_write(cache_file(), dev_builds::encode_cache(cache), &err))
        brls::Logger::warning("dev builds: cannot keep the list ({})", err);
}

// The .nro to replace, from the SD card root: the running one. On desktop,
// where nothing runs from an SD card, the usual place on the simulated one.
std::string sd_path()
{
#ifdef __SWITCH__
    return app::self_path();
#else
    return "/switch/playguard/playguard.nro";
#endif
}

// That file for the C library ("/switch/…" on the console, under
// ./playguard_data/sd on desktop).
std::string file_path(const std::string& sd)
{
    const std::string root = paths::sd_root();
    return root == "/" ? sd : root + sd;
}

// The download's dialog: a bar, "3 of 7 MB" and Cancel (B too), updated
// from the download thread (at most once per percent, through brls::sync).
// Cancel sets the flag the download checks (it stops within a second) and
// closes the dialog at once; nothing touches the dialog after it closed,
// whichever way (Cancel, B, the download's end) it did.
class DownloadProgress
{
  public:
    explicit DownloadProgress(uint64_t size) : size(size)
    {
        this->label = ui::centered_label("", 22, ui::color_text());
        this->label->setMarginBottom(24);
        auto* track = new brls::Box(brls::Axis::ROW);
        track->setWidth(BAR_WIDTH);
        track->setHeight(12);
        track->setCornerRadius(6);
        track->setBackgroundColor(nvgTransRGBA(ui::color_neutral(), 50));
        this->fill = new brls::Rectangle(ui::color_neutral());
        this->fill->setWidth(0);
        this->fill->setHeight(12);
        this->fill->setCornerRadius(6);
        track->addView(this->fill);
        // Its own button, not the dialog's: a dialog's button acts once the
        // dialog is gone, and the download must stop updating it before.
        auto* cancel = new brls::Button();
        cancel->setText("hints/cancel"_i18n);
        cancel->setWidth(260);
        cancel->setMarginTop(28);
        cancel->registerClickAction([this](brls::View*) {
            this->cancel();
            return true;
        });
        auto* box = new brls::Box(brls::Axis::COLUMN);
        box->setAlignItems(brls::AlignItems::CENTER);
        box->setJustifyContent(brls::JustifyContent::CENTER);
        box->setPadding(40, 40, 32, 40);
        box->addView(this->label);
        box->addView(track);
        box->addView(cancel);
        this->dialog = new brls::Dialog(box);
        this->dialog->setCancelable(false);   // B: below, through cancel()
        this->dialog->getAppletFrame()->registerAction(
            "hints/cancel"_i18n, brls::BUTTON_B, [this](brls::View*) {
                this->cancel();
                return true;
            },
            false, false, brls::SOUND_BACK);
        this->show(0, size);
        this->dialog->open();
    }

    // From the download thread.
    void received(uint64_t done, uint64_t total)
    {
        if (!total) total = this->size;
        const int percent = total ? (int)std::min<uint64_t>(100, done * 100 / total) : 0;
        if (percent == this->percent.exchange(percent)) return;
        // `this` outlives every update: the shared_ptr is held by the
        // close() task, queued after them.
        brls::sync([this, done, total]() { this->show(done, total); });
    }

    // What the download checks.
    const std::atomic<bool>* cancel_flag() const { return &this->cancelled; }
    bool was_cancelled() const { return this->cancelled.load(); }

    void close()
    {
        if (this->dialog) this->dialog->close();
        this->dialog = nullptr;
    }

  private:
    static constexpr float BAR_WIDTH = 480.0f;
    const uint64_t size;
    std::atomic<int> percent{ -1 };
    std::atomic<bool> cancelled{ false };
    brls::Dialog* dialog = nullptr;   // null once it is closing: never touched again
    brls::Label* label = nullptr;
    brls::Rectangle* fill = nullptr;

    void cancel()
    {
        if (!this->dialog) return;
        this->cancelled = true;
        this->close();
    }

    void show(uint64_t done, uint64_t total)
    {
        if (!this->dialog) return;
        const float part = total ? std::min(1.0f, (float)done / (float)total) : 0.0f;
        this->fill->setWidth(BAR_WIDTH * part);
        // Whole MB, the total rounded up (never "0 of 0"), what came in down.
        const uint64_t mb = 1024 * 1024;
        this->label->setText(brls::getStr("playguard/dev_build/downloading", (int)(done / mb), (int)((total + mb - 1) / mb)));
    }
};

void installed_now(const dev_builds::Build& b)
{
    // The build replaced is kept, and the next start checked (rollback_flow).
    rollback_flow::installed(label(b));
    if (launcher::can_launch() && launcher::launch_nro(sd_path())) {
        // hbloader starts the new .nro as soon as this one exits.
        brls::Application::quit();
        return;
    }
    ui::info(brls::getStr("playguard/dev_build/done_manual", label(b)));
}

void install(const dev_builds::Build& b)
{
    if (sd_path().empty()) {
        ui::info("playguard/dev_build/no_path"_i18n);
        return;
    }
    if (s_busy) return;
    s_busy = true;
    const std::string target = file_path(sd_path());
    auto progress = std::make_shared<DownloadProgress>(b.size);
    // Not behind the play log or the game icons (ui::in_background).
    ui::in_background("dev build download", [b, target, progress]() {
        std::string err;
        const std::string fresh = target + ".new";
        bool ok = dev_builds::download(b, fresh, &err, [progress](uint64_t done, uint64_t total) {
            progress->received(done, total);
        }, progress->cancel_flag());
        brls::sync([b, ok, err, target, fresh, progress]() mutable {
            // Cancelled: nothing replaced, even when the download had just
            // ended; said with a toast (the dialog is already gone).
            if (progress->was_cancelled()) {
                std::remove(fresh.c_str());
                s_busy = false;
                ui::notify("playguard/dev_build/cancelled"_i18n);
                return;
            }
            if (ok) {
                // The running .nro is held open by romfs:/ (the app's
                // resources), and the SD card will not rename an open file:
                // released for the two renames, here on the UI thread so that
                // nothing reads romfs:/ meanwhile, then mounted again.
                platform_romfs_release();
                ok = dev_builds::replace(target, fresh, &err, rollback::previous_file(target));
                platform_romfs_mount();
                if (!ok) std::remove(fresh.c_str());
            }
            s_busy = false;
            progress->close();
            // The one reason a user can act on, said in their language.
            if (err == dev_builds::NO_DIGEST) err = "playguard/dev_build/no_digest"_i18n;
            if (!ok) ui::info(brls::getStr("playguard/dev_build/failed", err));
            else installed_now(b);
        });
    });
}

}   // namespace

std::string nro_sd_path()
{
    return sd_path();
}

std::string nro_file()
{
    const std::string sd = sd_path();
    return sd.empty() ? "" : file_path(sd);
}

int64_t now()
{
#ifndef __SWITCH__
    // Desktop simulation: the console's frozen time, so ages stay the same in screenshots.
    if (const char* frozen = std::getenv("PLAYGUARD_SIM_NOW"))
        if (const long long v = std::atoll(frozen)) return v;
#endif
    return (int64_t)std::time(nullptr);
}

std::string age_text(const std::string& iso)
{
    const dev_builds::Age age = dev_builds::age_of(iso, now());
    switch (age.unit) {
        case dev_builds::Age::None: break;
        case dev_builds::Age::Now: return "playguard/dev_build/age_now"_i18n;
        case dev_builds::Age::Hours: return brls::getStr("playguard/dev_build/age_hours", age.count);
        case dev_builds::Age::Days: return brls::getStr("playguard/dev_build/age_days", age.count);
    }
    return "";
}

std::string label(const dev_builds::Build& b)
{
    std::string text;
    switch (b.kind) {
        case dev_builds::Kind::Release:
            text = release_text(b);
            break;
        case dev_builds::Kind::Main:
            text = brls::getStr("playguard/dev_build/main", b.commit, dev_builds::short_date(b.date));
            break;
        case dev_builds::Kind::PullRequest:
            text = brls::getStr("playguard/dev_build/pr", b.pr, b.title, b.commit);
            break;
        case dev_builds::Kind::Commit:
            text = brls::getStr("playguard/dev_build/commit", b.commit, b.branch, dev_builds::short_date(b.date));
            break;
    }
    if (installed(b)) text += " · " + "playguard/dev_build/installed"_i18n;
    return text;
}

const dev_builds::Build* running_build(const std::vector<dev_builds::Build>& all)
{
    for (const dev_builds::Build& b : all)
        if (b.kind != dev_builds::Kind::Release && installed(b)) return &b;
    return nullptr;
}

bool installed(const dev_builds::Build& b)
{
    // A release is told by its version (a release build has no commit).
    if (b.kind == dev_builds::Kind::Release) return app::is_release() && b.version == app::version();
    return !app::commit().empty() && b.commit == app::commit();
}

std::string release_text(const dev_builds::Build& b)
{
    std::string text = brls::getStr("playguard/dev_build/release", b.version);
    if (b.prerelease) text += " · " + "playguard/dev_build/prerelease"_i18n;
    return text;
}

std::string checked_text(const dev_builds::Cache& cache)
{
    const int64_t age = now() - cache.fetched_at;
    return brls::getStr("playguard/dev_build/checked", (int)(age > 0 ? age / 60 : 0));
}

void choose(const dev_builds::Build& b, bool newer)
{
    DevBuildActivity::show(b, newer);
}

void install_now(const dev_builds::Build& b)
{
    // Nothing to check it against: refused before the PIN and the download
    // (its screen hides Install then).
    if (b.sha256.empty()) {
        ui::info("playguard/dev_build/no_digest"_i18n);
        return;
    }
    // Replacing PlayGuard can undo everything it guards: the PIN is asked
    // whenever one is set, whatever Protect PlayGuard with the PIN says
    // (unless it was entered since PlayGuard opened).
    if (!pin_lock::confirm()) {
        ui::notify(pin_lock::refusal_text());
        return;
    }
    install(b);
}

void sign_in()
{
    github_login_flow::sign_in([]() { open(); });
}

void go_back()
{
    std::string previous;
    if (s_busy || !rollback_flow::previous(&previous)) return;
    std::string running = app::version();
    if (!app::commit().empty()) running += " (" + app::commit() + ")";
    ui::confirm(brls::getStr("playguard/dev_build/back_body", running, previous), "playguard/dev_build/back_confirm"_i18n,
                []() {
                    // As for an install: whatever Protect PlayGuard with the PIN says.
                    if (!pin_lock::confirm()) {
                        ui::notify(pin_lock::refusal_text());
                        return;
                    }
                    rollback_flow::go_back_now();
                });
}

void find()
{
    if (s_busy) {
        ui::notify("playguard/dev_build/busy"_i18n);
        return;
    }
    ui::prompt_text("playguard/dev_build/find_header"_i18n, "", 40, [](std::string text) {
        dev_builds::Query q;
        if (!dev_builds::parse_query(text, &q)) {
            ui::info(brls::getStr("playguard/dev_build/find_invalid", text));
            return;
        }
        if (s_busy) return;
        s_busy = true;
        const std::string what = q.pr ? "#" + std::to_string(q.pr) : q.commit.substr(0, 7);
        // A few requests, a few seconds: the bar fills at a steady pace meanwhile.
        DevBuildsActivity::loading(brls::getStr("playguard/dev_build/finding", what), 0.0f, true);
        ui::in_background("dev build find", [q]() {
            dev_builds::Build b;
            std::string err;
            const bool ok = dev_builds::find(q, &b, &err);
            brls::sync([ok, b, err]() {
                s_busy = false;
                DevBuildsActivity::loaded();
                if (ok) choose(b);
                else ui::info(brls::getStr("playguard/dev_build/find_failed", err));
            });
        });
    });
}

void open(bool force)
{
    if (s_busy) {
        ui::notify("playguard/dev_build/busy"_i18n);
        return;
    }
    // A list fetched a few minutes ago, in the same sign-in state, is shown at
    // once; an older one is fetched again (and still shown if GitHub cannot
    // be reached).
    read_cache();
    const bool needs_login = github_auth::token().empty();
    if (!force && dev_builds::cache_fresh(s_cache, now(), needs_login)) {
        DevBuildsActivity::show(s_cache);
        return;
    }
    s_busy = true;
    brls::Logger::info("dev builds: fetching the list");
    // The screen opens at once, the list kept (if any) under a bar that shows
    // how far the fetch is: a toast alone looked like nothing was happening.
    // Nothing kept yet: an empty list, Sign in already there when signed out.
    dev_builds::Cache shown = s_cache;
    if (shown.builds.empty()) shown.needs_login = needs_login;
    DevBuildsActivity::show(shown);
    DevBuildsActivity::loading("playguard/dev_build/step_releases"_i18n, 0.0f);
    // Not behind the play log or the game icons (ui::in_background).
    ui::in_background("dev build list", []() {
        dev_builds::Cache fresh;
        std::string err;
        const bool ok = dev_builds::fetch(&fresh.builds, &fresh.needs_login, &err, [](float done, dev_builds::Step step) {
            brls::sync([done, step]() {
                const char* key = step == dev_builds::Step::Releases    ? "playguard/dev_build/step_releases"
                                  : step == dev_builds::Step::Artifacts ? "playguard/dev_build/step_artifacts"
                                  : step == dev_builds::Step::Pulls     ? "playguard/dev_build/step_pulls"
                                                                        : "playguard/dev_build/step_runs";
                DevBuildsActivity::loading(brls::getStr(key), done);
            });
        });
        fresh.fetched_at = now();
        brls::sync([ok, fresh, err]() {
            s_busy = false;
            DevBuildsActivity::loaded();
            if (ok) {
                keep(fresh);
                DevBuildsActivity::show(fresh);
            } else if (!s_cache.builds.empty()) {
                ui::notify(brls::getStr("playguard/dev_build/list_stale", err));
                DevBuildsActivity::show(s_cache);
            } else if (fresh.needs_login) {
                DevBuildsActivity::show(fresh);   // signing in may still list them
            } else {
                DevBuildsActivity::failed(brls::getStr("playguard/dev_build/list_failed", err));
            }
        });
    });
}

}   // namespace dev_build_flow

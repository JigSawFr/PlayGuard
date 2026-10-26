// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/profile_flow.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "action/pt_flow.hpp"
#include "activity/profiles_activity.hpp"
#include "ui/play_text.hpp"
#include "ui/ui.hpp"
#include "util/pctl_ops_c.hpp"

using namespace brls::literals;

namespace profile_flow
{
void apply(const profiles::Profile& p, std::function<void()> after)
{
    const std::array<uint16_t, 7> days = p.days;
    // Already the console's limits: nothing to write, nothing to ask.
    PtState live;
    pctl_play_timer_query(&live);
    if (live.valid && std::equal(days.begin(), days.end(), live.day_min)) {
        ui::notify(brls::getStr("playguard/play_timer/profile_already", p.name));
        return;
    }
    // The week as the profile sets it is drawn under the question.
    pt_flow::confirm_write(brls::getStr("playguard/play_timer/profile_apply", p.name),
                           "playguard/play_timer/confirm_set"_i18n, [days, name = p.name, after](bool did_unlock) {
                               Result rc = pt_flow::write_days(days.data(), "profile", name);
                               pt_flow::finish_write(rc, did_unlock, "playguard/play_timer/written_days"_i18n,
                                                     "playguard/play_timer/write_err"_i18n, after);
                           }, days.data());
}

void pick(std::function<void()> after)
{
    const std::vector<profiles::Profile> all = profiles::list();
    PtState live;
    pctl_play_timer_query(&live);
    const std::string current = live.valid ? profiles::match(live.day_min) : "";
    std::vector<std::string> labels, details;
    int selected = -1;
    for (size_t i = 0; i < all.size(); i++) {
        labels.push_back(all[i].name);
        details.push_back(ui::days_summary(all[i].days.data()));
        if (all[i].name == current) selected = (int)i;
    }
    labels.push_back("playguard/cadran/today/profiles_manage"_i18n);
    details.push_back("");
    ui::pick("playguard/cadran/today/profiles_title"_i18n, labels, selected, [all, after](int i) {
        if (i == (int)all.size()) brls::Application::pushActivity(new ProfilesActivity());
        else if (i >= 0 && i < (int)all.size()) apply(all[i], after);
    }, selected >= 0, details);
}
}   // namespace profile_flow

// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/backup_logic.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

#include "util/pt_block.hpp"

namespace backup_logic
{

backup::Snapshot snapshot_of(const PctlStatus& st, const PtState& pt)
{
    backup::Snapshot s;
    if (st.safety_level_ok && st.safety_level <= PctlSafetyLevel_Teen) s.level = st.safety_level;
    if (st.settings_ok && st.settings.rating_age <= 21)
        s.custom = backup::Custom{ st.settings.rating_age, st.settings.sns_post_restriction,
                                   st.settings.free_communication_restriction };
    if (st.stereo_vision_ok) s.vr_restricted = st.stereo_vision_restricted;
    if (st.rating_org_ok) s.rating_org = st.rating_org;

    bool days_ok = pt.fw_supported && pt.valid;
    std::array<uint16_t, 7> days{};
    for (int d = 0; d < 7 && days_ok; d++) {
        days[d] = pt.day_min[d];
        if (days[d] != PT_DAY_NOLIMIT && days[d] > 1440) days_ok = false;
    }
    if (days_ok) {
        s.days = days;
        pt_block::Block b;
        std::copy(std::begin(pt.block), std::end(pt.block), b.begin());
        s.raw_block = pt_block::to_hex(b);
        if (pt.alarm_disabled_valid) s.alarm_disabled = pt.alarm_disabled;
    }
    return s;
}

std::string list_label(const std::string& name, const backup::Snapshot& s)
{
    if (s.created.empty()) return name;
    return s.firmware.empty() ? s.created : s.created + " · " + s.firmware;
}

void Tally::add(Result rc, const std::string& what)
{
    tried++;
    if (R_SUCCEEDED(rc)) return;
    if (R_SUCCEEDED(first)) first = rc;
    failed.push_back(what);
}

std::string Tally::failed_list() const
{
    std::string what;
    for (const auto& f : failed) what += (what.empty() ? "" : ", ") + f;
    return what;
}

const std::vector<std::string>& Listing::names(const std::function<std::vector<std::string>()>& list)
{
    if (!listed) set(list());
    return kept;
}

void Listing::set(std::vector<std::string> fresh)
{
    kept = std::move(fresh);
    listed = true;
}

}   // namespace backup_logic

// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/backup_logic.hpp"

#include <algorithm>
#include <iterator>

#include "util/pt_block.hpp"

namespace backup_logic
{

backup::Snapshot snapshot_of(const PctlStatus& st, const PtState& pt)
{
    backup::Snapshot s;
    s.level_ok        = st.safety_level_ok && st.safety_level <= PctlSafetyLevel_Teen;
    s.level           = st.safety_level;
    s.custom_ok       = st.settings_ok && st.settings.rating_age <= 21;
    s.rating_age      = st.settings.rating_age;
    s.sns_restricted  = st.settings.sns_post_restriction;
    s.comm_restricted = st.settings.free_communication_restriction;
    s.vr_ok           = st.stereo_vision_ok;
    s.vr_restricted   = st.stereo_vision_restricted;
    s.rating_org_ok   = st.rating_org_ok;
    s.rating_org      = st.rating_org;

    s.days_ok = pt.fw_supported && pt.valid;
    for (int d = 0; d < 7 && s.days_ok; d++) {
        s.days[d] = pt.day_min[d];
        if (s.days[d] != PT_DAY_NOLIMIT && s.days[d] > 1440) s.days_ok = false;
    }
    if (s.days_ok) {
        pt_block::Block b;
        std::copy(std::begin(pt.block), std::end(pt.block), b.begin());
        s.raw_block = pt_block::to_hex(b);
    }
    s.alarm_ok       = s.days_ok && pt.alarm_disabled_valid;
    s.alarm_disabled = pt.alarm_disabled;
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

}   // namespace backup_logic

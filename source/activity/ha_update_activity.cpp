// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "activity/ha_update_activity.hpp"

#include "action/ha_update_flow.hpp"
#include "app.hpp"
#include "view/ha_links.hpp"
#include "view/release_notes.hpp"

using namespace aurore::literals;

void HaUpdateActivity::onContentAvailable()
{
    if (auto* applet = dynamic_cast<aurore::AppletFrame*>(this->getContentView()))
        applet->setTitle("playguard/title"_i18n);
    const std::string installed = ha_update_flow::installed();
    header->setTitle(aurore::getStr("playguard/notices/ha_integration", ha_update_flow::newer()));
    body->setSingleLine(false);
    body->setText(aurore::getStr("playguard/ha_update/body", installed));
    qr->addView(ha_links::qr_card("playguard/ha_update/hacs"_i18n, app::HA_HACS_URL,
                                  "playguard/ha_update/hacs_caption"_i18n));

    const auto lines = ha_update_flow::changes();
    changes->setTitle(aurore::getStr("playguard/ha_update/changes", installed));
    release_notes::fill(notes, lines);
    // Nothing else to focus: the notes take it (with no highlight), so the
    // stick and the D-pad scroll them.
    notes->setFocusable(true);
    notes->setHideHighlight(true);
    note->setSingleLine(false);
    note->setText(aurore::getStr(lines.empty() ? "playguard/ha_update/no_notes" : "playguard/ha_update/note",
                               std::string(app::HA_RELEASES_URL)));
    aurore::Logger::info("ha integration: update screen ({} lines of changes)", lines.size());
}

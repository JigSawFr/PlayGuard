// DevBuildsActivity — Developer tools › Install another build
// (action/dev_build_flow.hpp): a tab each for the last releases, main's
// last commits and the open pull requests (a long list of one kind never
// hides the next), the builds of the chosen one below, the installed one
// marked. Ⓛ / Ⓡ move from one tab to the next; the tab opened first is the
// installed build's, then the last one chosen in this run. Ⓨ sorts the pull
// requests (the newest build first, or by number), remembered in
// config.json. A on a build offers to install it; below the list, signing
// in to GitHub (signed out, only the releases are listed) and how old the list
// is; Ⓧ fetches it again.
// A list fetched again while this screen is open replaces its content in
// place: the cells are reused, never freed (one of them may have the focus).
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "util/dev_builds.hpp"

class DevBuildsActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/dev_builds.xml");

    // Shows `cache`: in this screen when it is open, else in a new one.
    static void show(const dev_builds::Cache& cache);

    explicit DevBuildsActivity(const dev_builds::Cache& cache);
    ~DevBuildsActivity() override;

    void onContentAvailable() override;

  private:
    void update(const dev_builds::Cache& cache);
    // The tabs and the chosen one's builds from this->cache, in the order chosen.
    void fill();
    // The builds of `kind`, in the order chosen.
    std::vector<dev_builds::Build> builds(dev_builds::Kind kind) const;
    // Shows `kind`'s builds, the focus on the first one when `focus_list`.
    void show_tab(dev_builds::Kind kind, bool focus_list);
    // The next tab with builds, `step` 1 or -1 (Ⓡ, Ⓛ), the focus going along.
    void next_tab(int step);
    void sort();

    dev_builds::Cache cache;
    dev_builds::Kind  tab = dev_builds::Kind::Release;

    BRLS_BIND(brls::Label,      note,           "db_note");
    BRLS_BIND(brls::Box,        tabs,           "db_tabs");
    BRLS_BIND(brls::Button,     tab_release,    "db_tab_release");
    BRLS_BIND(brls::Button,     tab_main,       "db_tab_main");
    BRLS_BIND(brls::Button,     tab_pulls,      "db_tab_pulls");
    BRLS_BIND(brls::Box,        list,           "db_list");
    BRLS_BIND(brls::DetailCell, sign_in,        "db_sign_in");
    BRLS_BIND(brls::Label,      checked,        "db_checked");
};

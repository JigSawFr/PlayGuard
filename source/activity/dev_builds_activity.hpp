// DevBuildsActivity — Developer tools › Install another build
// (action/dev_build_flow.hpp): the latest release, main's last commits and
// the open pull requests, each kind under its own header, the installed
// one marked. Ⓨ sorts the pull requests (the newest build first, or by
// number), remembered in config.json. A on a build offers to install it;
// below the lists, signing in to GitHub (signed out, only the release is
// listed) and refreshing the list.
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
    // The lists from this->cache, in the order chosen.
    void fill();
    // `builds` in the cells of `box`, reusing them; the extra ones hidden.
    void fill(brls::Box* box, const std::vector<dev_builds::Build>& builds);
    void sort();

    dev_builds::Cache cache;

    BRLS_BIND(brls::Label,      note,           "db_note");
    BRLS_BIND(brls::Header,     release_header, "db_release_header");
    BRLS_BIND(brls::Box,        release_list,   "db_release");
    BRLS_BIND(brls::Header,     main_header,    "db_main_header");
    BRLS_BIND(brls::Box,        main_list,      "db_main");
    BRLS_BIND(brls::Header,     pulls_header,   "db_pulls_header");
    BRLS_BIND(brls::Box,        pulls_list,     "db_pulls");
    BRLS_BIND(brls::DetailCell, sign_in,        "db_sign_in");
    BRLS_BIND(brls::DetailCell, refresh,        "db_refresh");
};

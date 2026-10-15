// DevBuildActivity — one build of Install another build
// (activity/dev_builds_activity.hpp), before it is installed: what it is,
// when it was built and how long ago, its size, how its checks ended, when
// GitHub deletes it (an artifact), whether it came after the installed one,
// what it replaces, then Install (the PIN first when one is set) and its
// notes: a release's changelog, a pull request's description. B goes back
// to the list.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <borealis.hpp>

#include "util/dev_builds.hpp"

class DevBuildActivity : public brls::Activity
{
  public:
    CONTENT_FROM_XML_RES("activity/dev_build.xml");

    // `newer`: it came after the running build (the list knows).
    static void show(const dev_builds::Build& build, bool newer = false);

    DevBuildActivity(const dev_builds::Build& build, bool newer) : build(build), newer(newer) {}

    void onContentAvailable() override;

  private:
    dev_builds::Build build;
    bool              newer;

    BRLS_BIND(brls::Label,      headline,     "dbd_headline");
    BRLS_BIND(brls::Label,      facts,        "dbd_facts");
    BRLS_BIND(brls::Label,      warning,      "dbd_warning");
    BRLS_BIND(brls::DetailCell, install,      "dbd_install");
    BRLS_BIND(brls::Header,     notes_header, "dbd_notes_header");
    BRLS_BIND(brls::Box,        notes,        "dbd_notes");
};

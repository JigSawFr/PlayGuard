// modules — PlayGuard's optional sysmodules on the SD card: the recovery
// module (sysmodule/rescue/, acts once at boot) and the remote-link agent
// (sysmodule/agent/, stays running). Installing, updating with a rollback and
// removing them is the kit's (util/sysmodule.hpp, host-tested there); this is
// what PlayGuard adds: which modules it carries and their names, and the
// recovery module's playguard-rescue.txt. Starting and stopping a module is
// core/modules_nx.h.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#include <vector>

#include "util/sysmodule.hpp"

namespace modules
{

enum class Id
{
    Rescue,
    Agent,
};

// A kit module with PlayGuard's name for it (`name`: the romfs folder and i18n keys).
struct Module : sysmodule::Module
{
    Id id;
};

const std::vector<Module>& all();
const Module& get(Id id);

// What the kit does, under this namespace as PlayGuard's code has always called it.
using sysmodule::Applied;
using sysmodule::Bundle;
using sysmodule::Installed;
using sysmodule::Offer;
using sysmodule::State;
using sysmodule::Step;
using sysmodule::bundled;
using sysmodule::confirm;
using sysmodule::dir;
using sysmodule::first_step;
using sysmodule::install;
using sysmodule::next_step;
using sysmodule::offer;
using sysmodule::rollback;
using sysmodule::set_at_boot;
using sysmodule::state;
using sysmodule::step_final;
using sysmodule::step_name;
using sysmodule::toolbox_json;
using sysmodule::uninstall;

// The recovery module's playguard-rescue.txt, as Makefile's dist-rescue
// writes it in the zip: "<PlayGuard version> <revision>" (tools/rescue_rev.cmake),
// which action/rescue compares with its own to tell a module out of date.
// Nothing is written without a revision (a build that has none).
Applied write_rescue_note(const std::string& sd_root, const std::string& version, const std::string& revision);

}   // namespace modules

// The desktop and web simulators' PLAYGUARD_SIM_* variables (CONTRIBUTING.md,
// web/README.md): the environment's, unless the web page changed one while
// PlayGuard runs. Read through sim_env(), never getenv(): the page's changes
// do not touch the process environment, which other threads read without a
// lock. On the console, there is none.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SWITCH__
static inline const char *sim_env(const char *name)
{
    (void)name;
    return 0;
}
#else
// The variable's value, NULL when unset. The string stays valid as long as
// the program runs.
const char *sim_env(const char *name);
// The web page's change (source/sim/web_page.cpp): `value` NULL or empty
// removes it, whatever the environment says.
void sim_env_set(const char *name, const char *value);
#endif

#ifdef __cplusplus
}
#endif

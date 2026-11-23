# PlayGuard's optional sysmodules

Two small Atmosphère sysmodules, both **optional**: PlayGuard works fully
without them. PlayGuard carries a copy of each in its romfs and installs,
updates, starts at boot or removes them from *Console › Optional modules*
(`source/util/modules.hpp`, `source/activity/modules_activity.cpp`); each is
also a download of its own on the releases page.

| Folder | Program id | What it does |
|---|---|---|
| [`rescue/`](rescue/README.md) | `4200000000505247` | At boot, only when a `RESCUE` file asks it to: unlocks parental controls with the stored PIN (or deletes them), for a forgotten PIN with a 0-minute limit; and lifts the homebrew menu's lock PlayGuard set (a `RESCUE` request, or Atmosphère updated since). Then exits. |
| [`agent/`](https://github.com/JigSawFr/boreale-link/blob/main/agent/README.md) | `4200000000504741` | Keeps the remote link (MQTT, Home Assistant) up while PlayGuard is closed; PlayGuard talks to it over `pg:agent` while it runs. It is the Boréale agent: its code lives in `library/boreale-link/agent/` (the shared sources, with PlayGuard's module in `library/boreale-link/modules/playguard/`) and PlayGuard builds it from there. |

Both are built with devkitPro's libnx rules, the ARM flags and `out/<name>.nsp`
(`sysmodule/common.mk` for the recovery module, the same file in `library/boreale-link/agent/` for the agent):

```sh
make rescue          # sysmodule/rescue/out/playguard-rescue.nsp
make dist-rescue     # playguard-rescue.zip
make agent           # library/boreale-link/agent/out/boreale-link.nsp
make dist-agent      # boreale-link.zip
make                 # the .nro, with the modules built first and bundled (cmake/bundle_sysmodules.cmake)
```

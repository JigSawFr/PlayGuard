# playguard-agent — the remote link in the background

An optional Atmosphère sysmodule that keeps PlayGuard's remote link (MQTT,
Home Assistant: [`docs/home-assistant.md`](../../docs/home-assistant.md)) up
while PlayGuard is closed. Without it the link only runs while PlayGuard is
open; with it, the console stays online, publishes its state and play time,
and carries out the orders Home Assistant sends, whatever is running.

It is **optional**: PlayGuard carries a copy and installs it from *Console ›
Optional modules*, which also starts it, stops it, updates it and removes it;
it is also its own download (`playguard-agent.zip`, to extract at the SD-card
root). It does nothing until *Console › Remote access* is set up.

When the agent on the SD card is not the one PlayGuard carries (another
build, or another protocol of `pg:agent`), PlayGuard offers the update at
start-up (*Later* skips that build). An update asks the agent to leave the
broker (`PrepareShutdown`), stops it, keeps the previous `exefs.nsp` as
`exefs.nsp.bak`, starts the new one and waits up to 10 s for its `Hello`; when
it does not answer, the new one is stopped and the previous one put back and
started again (`source/action/agent_update.cpp`, the steps host-tested in
`tests/modules/`).

## What it does

- Connects to the broker set in `sd:/config/playguard/sync.conf` (the same
  settings as PlayGuard, same session engine: `source/sync/sync_engine.c` on
  `source/link/bl_session.c`),
  as the client `pg-<id>-agent`, with `offline` as its last will. `<id>` is
  this system's: on an emuMMC it differs from the sysMMC's, so the two are
  two devices ([`docs/sync-protocol.md`](../../docs/sync-protocol.md)).
- Every `poll_s` seconds, reads the parental controls and the play timer
  through PlayGuard's own service layer (`source/core/`), and the play log of
  the last 7 days (`pdm:qry`, no game names: those come from `names.txt`,
  which PlayGuard writes), and publishes `state`, `activity` and, after
  midnight, the finished day.
- Publishes the icon of the game played when the broker does not have it
  yet and PlayGuard's icon cache on the SD card does
  (`config/playguard/cache/icons/`), streamed from the file a piece at a time;
  every other icon is PlayGuard's to publish (no `ns` here to read one).
- Carries out orders **only under the *auto* policy**, through the same gate,
  unlock and relock as PlayGuard (`source/sync/sync_exec.c`). Under *ask*,
  orders wait on the broker until PlayGuard opens and asks on the console.
- Puts the usual limit back by itself the day after extra time, as PlayGuard
  does (not while the lower limit would stop the game in progress, on the
  same weekday a week later). The day after "no more play today", or when
  the usual limit was "no limit", it leaves the question to PlayGuard.
- Changes nothing when PlayGuard was in read-only mode, or when the firmware
  is not the one PlayGuard last ran on (a system update PlayGuard has not
  checked): `nro_state.txt`.

## With PlayGuard open

The agent stays the only MQTT client, so Home Assistant never sees the
console go offline. PlayGuard holds a session on the agent's IPC service
`pg:agent` ([`agent_ipc.h`](../../source/sync/agent_ipc.h)): in the
foreground it is the only one reading the parental controls and pushes what it
reads; orders go to PlayGuard, which applies its policy, its PIN check and its
change history. When PlayGuard closes or goes to the background, the agent
reads the console again by itself. *Console › Remote access* then says
*Online · through the agent*; when PlayGuard opens, it adopts the agent's
records and imports what the agent changed into its history. Started or
stopped from *Console › Optional modules*, the link moves between the agent and
PlayGuard's own session at once.

## Files

In `sd:/config/playguard/`, as PlayGuard's: it reads `sync.conf` and
`sync/profiles.txt`, shared by every system on the SD card; in this system's
folder, `nand/<system>/sync/` (`sysmmc`, `emummc` or `unknown`, as PlayGuard
names it), it reads `nro_state.txt`, `names.txt` and `icons.txt` and writes
`agent_icons.txt` (the icons it published itself),
`agent_state.txt` (what it changed: extra time, console lock, a relock
pending), `agent_events.log` (the orders it carried out, imported into
PlayGuard's history) and `agent_watchdog.log` (where its loop stopped, when
it stopped; PlayGuard's report shows the last lines); a report asked for from
Home Assistant goes to
`logs/reports/` with PlayGuard's. At boot it waits up to 10 s for the SD card,
then tries again every 30 s.
Details: [`docs/sync-protocol.md`](../../docs/sync-protocol.md).

## Build

```sh
make agent                 # -> sysmodule/agent/out/playguard-agent.nsp (needs devkitPro, DEVKITPRO set)
make dist-agent            # -> playguard-agent.zip  (SD-card layout)
make                       # the .nro, with this module bundled in its romfs
```

It links PlayGuard's C service layer and the remote link's core unchanged
(`source/core/`, `source/sync/`); its own code is `source/main.c` (the loop),
`source/agent_core.c` (what the loop and the IPC thread share, host-tested in
`tests/agent/`) and `source/agent_ipc.c` (the HIPC server).

Program ID `0x4200000000504741`, in the private range homebrew sysmodules use.
Static heap 1.5 MiB, 256 KiB of it the network service's room for the socket
buffers; one TCP socket, on two `bsd` sessions (the watchdog shuts it down
when a call on it hangs); TLS through the console's `ssl` service.

## Not tested on a console yet

Everything here builds and its logic is host-tested, but the IPC server, the
start without a reboot from PlayGuard (`pm:shell`), TLS and the share of the
`pctl:a` service with the HOME menu's PIN prompt still have to be tried on
hardware.

## License

GPLv3 (see [`../../LICENSE`](../../LICENSE)).

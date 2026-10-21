# playguard-rescue — the recovery sysmodule

A tiny Atmosphère boot sysmodule: the **last chance** for a parent who set a
play-time limit of 0 minutes (or any limit that suspends every program) and
then **forgot the PIN**. When the timer blocks everything, the HOME menu, the
Album and PlayGuard itself cannot start — so no homebrew can undo it. This
module runs before any of that, from the SD card.

It is **optional** and ships as its own download (`playguard-rescue.zip`), never
as part of the normal PlayGuard install. Install it only if you want this safety
net. PlayGuard works fully without it; it simply shows a recovery screen when
the module leaves a report.

## What it does

At boot the module looks for a file named `RESCUE` (or `RESCUE.txt`) in
`sd:/config/playguard/` (where PlayGuard keeps its files):

- **present, empty or any text** → it unlocks parental controls temporarily,
  using the PIN the console already stores, exactly as the system PIN screen
  would. Everything can start again, so you can open PlayGuard and fix things
  properly (set a new PIN, adjust the limit, or remove the controls).
- **a line that reads just `delete`** (any case; spaces around it and a
  byte-order mark are ignored) → it deletes every parental control (PIN and all
  restrictions). Irreversible; use it only if you are giving up on the PIN.
  The word inside other text (`delete everything`, `don't delete`) is not
  enough: that file only unlocks.

The request file is **removed before anything is done** (it acts once). Should
the card refuse that, it is renamed `RESCUE.done` instead; should that fail too,
the file is still there, so a delete is **not done** (it would repeat at every
boot) while an unlock still is — and PlayGuard's recovery screen tells you to
remove the file from a computer. The outcome is written to
`rescue_report.txt`. The next time PlayGuard opens it
shows what happened, lets you finish, records it in the change history, and
removes the report. A report it cannot read is removed too, and PlayGuard says
so (check the PIN and the play timer yourself). Should the SD card not be
mounted at boot (the module retries for about 10 s), nothing is done and no
report is left.

The report is a plain file, so PlayGuard does not take its word for it. It
opens the recovery actions (set a new PIN, delete everything) only when the
console's state agrees with the report and, after an unlock, when the report
carries the module's **proof**: PlayGuard keeps a random challenge in
`config/playguard/rescue_nonce` (a new one at every start, so a report is
believed only at the first start after the boot that wrote it), and the
module adds to its report that challenge and an HMAC-SHA256 of the report
keyed with the PIN the console stores, which PlayGuard computes again. A
report written by hand, copied from an earlier recovery, or left by a module
older than this check is shown without the actions, and *Open PlayGuard*
goes through the lock screen as on any start. The PIN itself is never
written; the proof is made from it, so like the report it is removed at the
next start of PlayGuard.

## The homebrew menu's lock

When PlayGuard has locked the homebrew menu (*Console › Forwarders ›
Homebrew menu*: the album and a game started while R is held no longer open
it, see `source/core/hbl_lock.h`), the module is also a way out of it. At
boot it lifts the lock — PlayGuard's block taken away from
`atmosphere/config/override_config.ini`, the rest of the file left as it is —
when:

- **a `RESCUE` request is there** (either mode): the request that unlocks
  parental controls opens the homebrew menu too;
- **Atmosphère was updated since it was locked** (the version PlayGuard wrote
  in `config/playguard/hbmenu_lock.txt` is not the one running, read from
  exosphère): its signature patches may lag behind, and no forwarder starts
  without them, PlayGuard's own included.

Neither is something a child can do from the console. Deleting a forwarder,
which anyone can do in System Settings, lifts nothing. The outcome goes to
`config/playguard/hbmenu_lock_report.txt`; PlayGuard tells it at its next
start, records it in the change history, offers to lock again and removes
the report. A file without PlayGuard's block is never written. At the boot
after an unlock, the module also takes away the block of Atmosphère's
defaults the unlock wrote back (needed only until the console restarts).

The lock does not need the module: without it, deleting the file from a
computer (or putting PlayGuard's backup, `config/playguard/override_config.ini.backup`,
back) and restarting undoes it.

## How to use it

1. Install the module once: extract `playguard-rescue.zip` to the SD-card root
   (it lands in `atmosphere/contents/4200000000505247/`). Reboot. Its
   `playguard-rescue.txt` says which PlayGuard it comes with and its revision
   (`tools/rescue_rev.cmake`, a hash of its sources): while it is not the one
   the running PlayGuard ships with, the notification bell says so, and its
   *Update* puts in the module the `.nro` carries (`romfs:/rescue/exefs.nsp`,
   built with it) and writes `playguard-rescue.txt`; it runs from the next
   boot. Until then the bell says to restart the console: PlayGuard leaves
   `restart_pending` next to the module, and the module removes it at boot. PlayGuard never installs it when it is not there already.
2. When you are locked out: put the SD card in a computer, create an empty file
   `config/playguard/RESCUE` (or `RESCUE.txt`; to delete everything instead,
   write `delete` on a line of its own), put the card back, turn the
   console on. Open PlayGuard and finish from the recovery screen. (PlayGuard
   must have been opened once since the module was installed: that leaves the
   challenge the module answers.)
3. Remove `atmosphere/contents/4200000000505247` when you no longer need it:
   while it is installed, anyone who can edit the SD card can use it.

Prefer the empty `RESCUE` file, then *Delete all parental controls* on the
recovery screen, to the `delete` request: PlayGuard saves a backup of the
settings first, while `delete` erases everything at boot with no backup.

## What it is not

- **Not a security hole beyond what already exists.** Anyone who can edit the
  SD card can already turn off PlayGuard's own PIN prompt by editing
  `config.json`, or wipe parental controls with any pctl tool. The module needs
  the same physical access to the card. It changes nothing a determined person
  with the card could not already do; it just makes the honest "I forgot the
  PIN" case recoverable without a computer-side guide.
- **Not for stock consoles.** Like all of PlayGuard it needs Atmosphère.

## Build

```sh
make -C sysmodule          # -> sysmodule/out/playguard-rescue.nsp (needs devkitPro, DEVKITPRO set)
make dist-rescue           # -> playguard-rescue.zip  (SD-card layout, from the repo root)
```

The module shares `source/core/rescue.c` with the app (the request / report
file format, host-tested in `tests/rescue/`), and `source/core/hbl_lock.c`
(the homebrew menu's lock, its state and report files, the boot decision,
host-tested in `tests/hbl_lock/`). Its pctl calls are the app's,
from `source/core/pctl_ops_internal.h` (the typed reads, the PIN read and the
unlock with it, all `static inline`): the sysmodule links nothing else of the
app's service layer.

Program ID `0x4200000000505247` is in the private range homebrew sysmodules use,
so it never collides with a real title.

## License

GPLv3 (see [`../LICENSE`](../LICENSE)).

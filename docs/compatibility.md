# Compatibility

Supported firmware, Atmosphère and launchers, emuMMC and sysMMC, and how PlayGuard compares with a replacement sysmodule. Back to the [README](../README.md).

## Supported versions

| | Supported | Notes |
|---|---|---|
| **Firmware** | **21.0.0 → 22.5.0** | The play-time limit layout (0x44 bytes) exists since 21.0.0; below that, every tab works except the play timer. 23.x goes through the [newer firmware](#newer-firmware) screen until it is tested on a console. |
| **Atmosphère** | **1.11.x → 1.12.0** | 1.12.0 adds 23.0.0 support. The app shows the detected version. |
| **Launchers** | hbmenu, **sphaira**, **Homebrew App Store** | Launching over a game (title override) is recommended. The app says whether it runs as an application or as an applet (album). Over a game, the console counts PlayGuard's time as that game's. In the activity, it goes to the user picked at launch: open it with a parent's user, not a child's. The play timer is the console's, the same for every user: it counts that time whoever opened PlayGuard (unless the timer is off). |
| **Tested on hardware** | 22.0.0, 22.1.0, 22.5.0 | 23.0.1 / 1.12.0: the command table ([switchbrew](https://switchbrew.org/wiki/Parental_Control_services)) has the same commands, but nothing was run on a console yet, so PlayGuard asks first (read-only until you choose) — reports are welcome. |
| **Nintendo Switch 2** | Not supported | There is no Atmosphère for it. |

<a id="newer-firmware"></a>**Newer firmware?** Above the newest firmware tested on a console, PlayGuard opens **read-only** and checks whether a newer release supports it. If one does, it offers to update (in PlayGuard itself, through sphaira or the Homebrew App Store). Otherwise you choose: read-only, read-only with the developer tools (to investigate the firmware), or every feature at your own risk. The choice can be remembered for that firmware and app version; *Tools › Compatibility* brings the screen back.

**emuMMC and sysMMC.** Each has its own parental controls, and PlayGuard only sees those of the system it runs on. On emuMMC, *Tools* and *First steps* say so; on sysMMC too, when an emuMMC is set up on the SD card. Set the controls on both, or hide the hekate boot menu so that a child cannot pick the other system. PlayGuard keeps each system's history, backups and state apart (the sysMMC and the emuMMC, in `nand/`); the preferences and profiles are shared.

**Can it get the console banned?** PlayGuard never contacts Nintendo: online, it only reaches public time servers, GitHub (updates, and the reports you send). What gets consoles banned is custom firmware or an emuMMC going online to Nintendo ([switchbrew FAQ](https://switchbrewdocs.readthedocs.io/en/latest/faq.html)). No ban tied to parental-control or clock changes is documented; that is not a guarantee.

**A console without Internet?** That is what PlayGuard is for, and the play timer still needs a network clock that has been set. With no time server to ask, *Network clock › Measure* offers to set it to the console clock instead: check the console clock first, with *Synchronize Clock via Internet* off, and turn that setting on only afterwards (see [Network clock](features.md#network-clock--the-clock-the-play-timer-relies-on)).

**Phone notifications.** Since 22.0.0, a console still linked to the phone app may notify the parent when the PIN is entered on the console ([switchbrew](https://switchbrew.org/wiki/22.0.0)). Whether PlayGuard's own unlocks trigger it is unknown.

## Why PlayGuard

Children love video games, and video games mean screens: a limit is part of looking after them. On an unmodified Switch, Nintendo's phone app took care of it. Once the console runs custom firmware, a whole new world opens up — and parental control goes out of the window, because the phone app no longer reaches the console. Knowing how long they played, setting a limit, getting the game stopped without a fight or one more "five more minutes" became hard.

The few tools that existed did not do the job: activity reports that were barely maintained and not very detailed, parental controls that were rough and unfinished. And I did not want a custom replacement either, or a sysmodule always running in the background. The idea was to **reuse the console's own parental controls as much as possible**, with everything they already do — the limit, the PIN, the warnings, the suspension — and to bring their settings back onto the console. That is PlayGuard.

Driving the console's own controls also opens the door to much more: reporting play time to a home server, taking orders from it, Home Assistant, automations. Where that could go is in the [roadmap's horizon](../ROADMAP.md#horizon).

## PlayGuard or a replacement sysmodule?

PlayGuard drives the parental controls built into the console (the `pctl` service), offline. The other approach replaces them with a sysmodule, for instance [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (a sysmodule and a Tesla / Ultrahand overlay).

| | PlayGuard | A replacement sysmodule |
|---|---|---|
| Play-time limit | One for the console (the console's own timer) | One per user account |
| PIN, warnings, suspension | The console's | Its own |
| In the background | Nothing (the optional recovery sysmodule only acts at boot, when a `RESCUE` file asks it to) | A sysmodule, from boot |
| After a firmware update | The console's controls keep working; PlayGuard opens read-only until a version supports the firmware | The controls depend on the sysmodule still working |
| Deleted from the SD card | The limits stay on the console | The controls go with it |

**Need a different limit for each child?** Pick a replacement sysmodule — the console's timer has one limit for the whole console. Otherwise, PlayGuard keeps Nintendo's controls and just brings their settings to the console.

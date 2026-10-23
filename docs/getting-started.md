# Getting started

From the SD card to a console set up for your child, in about ten minutes, then the everyday tasks. Back to the [README](../README.md).

## Before you start

- **A console running Atmosphère.** PlayGuard drives the console's own parental controls through a restricted service: it needs custom firmware. Not modded? It cannot help ([troubleshooting](troubleshooting.md#console-not-modded)).
- **A supported firmware.** 21.0.0 to 23.0.1 for every feature; older ones, everything but the play timer; a newer one opens read-only until a release supports it ([compatibility](compatibility.md)).
- **One limit for the whole console.** The play timer is the console's: it is the same for every user. Different limits per child need another tool ([why](compatibility.md#playguard-or-a-replacement-sysmodule)).

Want to look around first? [Try PlayGuard in your browser](https://playguard.link/simulator/?not_set_up=1), against a simulated console on its first start.

## 1. Install it

Extract `playguard.zip` from the [latest release](https://github.com/JigSawFr/PlayGuard/releases/latest) to the root of the SD card ([install](install.md)). PlayGuard is not in the Homebrew App Store or sphaira yet: its listing is still awaiting approval.

> [!TIP]
> Also extract `playguard-rescue.zip`, the optional recovery sysmodule. It does nothing until you need it, and it is the only way back in if a 0-minute limit blocks everything and the PIN is forgotten ([Locked out?](troubleshooting.md#locked-out)).

## 2. Open it the right way

Open PlayGuard **over a game** (hold R while starting a game, with hbmenu, sphaira or the Homebrew App Store), with **a parent's user**:

- over a game, PlayGuard runs as an application, with all its memory; from the album it runs as an applet, with less;
- the console counts the time spent in PlayGuard as that game's, and gives it to the user picked at launch: a parent's, not a child's.

## 3. Follow *First steps*

On the first start, the *First steps* guide comes up (later: *Preferences › First steps*). *Today* shows the same steps as a card until they are done (*1 of 3*):

1. **The PIN.** The console's own PIN, asked by the HOME menu to start a game past the limit or to change the settings. Avoid 0000, 1234 or 2580: PlayGuard flags a PIN anyone would try first.
2. **A daily limit.** The same every day, or one for weekdays and one for the weekend. Change any single day later on *Play timer*.
3. **The network clock.** The play timer counts with it. A console that never reaches Nintendo's servers keeps it inaccurate: see step 5.

## 4. Unlink the companion app

Still linked to Nintendo's *Parental Controls* phone app, the companion app (or a second-hand console linked to someone else's)? *Security › Unlink the companion app*. Otherwise the companion app's next sync overwrites what you set in PlayGuard. It works offline.

## 5. Set the network clock

*Console › Network clock › Measure the time difference* asks three public time servers, then *Set the network clock*. Setting the clock starts today's play time over, so do it before the child plays.

A console without Internet: when no server answers, PlayGuard offers to set the network clock to the console clock instead. Check the console clock against a phone or a watch first, with *System Settings › System › Date and Time › Synchronize Clock via Internet* off ([details](features.md#network-clock)).

## 6. Choose what happens when time is up

*Play timer › When the time is up*: the console shows "Time's up", then either **suspends the game** or **only warns**. Suspending stops play without a fight, but unsaved progress may be lost; warning leaves it to the child. Add a **bedtime alarm** on the same tab if you want one.

## 7. Protect PlayGuard itself

*Security › Protect PlayGuard with the PIN*:

- **Before a change** (the default) — anyone can look, only the parent changes something;
- **To open PlayGuard** — a lock screen at start;
- **Never** — PlayGuard flags it on the bell while a PIN is set.

## 8. Hand the console over

*Today › Show the child* shows only the time left and when play ends, large enough to read from across the room. Ⓐ and the PIN bring the parent's screens back.

> [!IMPORTANT]
> **A game left open on the HOME menu keeps counting.** Close it, or put the console in sleep mode ([what counts](features.md#what-counts-as-play-time)).

## Everyday tasks

| To… | Where |
|---|---|
| Give more time today | *Today › Extra time* (+15 min, +30 min, +1 h). It comes back by itself the next day. |
| Give more time on another day | *Today › Extra time › Another day…* |
| Stop for today | *Today › Stop for today*: now, or in 5 minutes to save. |
| Change one day's limit | *Play timer*: Ⓐ on the day, ↑/↓, then **+** to save. |
| Switch between term time and holidays | *Play timer › Profiles*; *Scheduled profile* for a period picked ahead. |
| See who played what | *Activity*: per game, per account, today, 7 days, all time; export or a QR code. |
| Block every game for now | *Play timer › Block all games*. Turned off, the previous limits come back. |
| Undo a change | **ZR** *Undo* right after it, or *History › Changes made with PlayGuard* any time. |
| Keep a copy of the settings | *History › Back up the settings* (never the PIN). |
| Know what needs you | The bell (Ⓨ): what to do in amber, what to know with an "i". |

## Next

- Every tab and setting: [features](features.md).
- Something does not work as expected: [troubleshooting](troubleshooting.md#common-problems).
- What the console does, and how sure we are of it: [how parental controls work](parental-controls.md).

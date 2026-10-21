# Testing on a console: forwarders and the homebrew menu

What the desktop and web simulators cannot show: the console's own HOME menu, its PIN screen, Atmosphère reading `override_config.ini`, the rescue module at boot. Go through it on a console after a change to *Console › Forwarders* (the files under `source/action/forwarder*`, `hbmenu_lock*`, `source/core/gate*`, `hbl_lock*`, `gate/`, `sysmodule/`), and note in the pull request what was checked, on which firmware and Atmosphère.

Before starting: a PIN on the console, sys-patch (or sigpatch files), sphaira in `/switch/sphaira/`, a computer with an SD card reader at hand, and preferably the rescue module installed (`playguard-rescue.zip`). Use a console you can put back as it was.

Each step: what to do, then what must happen. ☐ is to tick.

## 1. Forwarders

- ☐ *Forwarders › PlayGuard*: on the HOME menu with PlayGuard's icon; it starts PlayGuard.
- ☐ *Another homebrew › sphaira*: the preview shows sphaira's icon and name; once added, the HOME menu has the same icon and name, and it starts sphaira.
- ☐ A homebrew whose `.nro` has a PNG icon, or one of another size: its icon on the HOME menu (not a “?”).
- ☐ The console set to another language (System Settings › System › Language): the icons stay.
- ☐ *Change the icon* on one, a screenshot of the album: the HOME menu shows the screenshot's middle square.
- ☐ *Delete*, then ZR within 8 seconds: back on the HOME menu, with the same icon.

## 2. The PIN gate

- ☐ *Protect with the PIN* on sphaira, then start it from the HOME menu: the gate's screen (night blue, *Protected by PlayGuard*, sphaira's icon and name, in the console's language), then the console's PIN screen.
- ☐ The PIN entered: sphaira starts. In PlayGuard: *History* has *Forwarder opened with the PIN*, and sphaira's row says *seen working*.
- ☐ B on the PIN screen: back to the HOME menu, the gate saying *PIN not entered*, then *Refused today: 1*; once more: *Refused today: 2* while it asks.
- ☐ Refused five times in a row (the default pause): the fifth says *It asks again at …*; started again before then, it says it is paused and never shows the PIN screen. In PlayGuard: the bell has the pause; *Lift the pause* (the PIN asked) and it asks again at once.
- ☐ *Hours without the PIN* now included: it starts without the PIN, the gate saying so; its row and the gate's screen name the hours.
- ☐ *Hours it never opens* now included: the gate says *Not at this time* and the hours, even with a PIN on the console; outside them, the PIN again.
- ☐ *Pause after refused PINs › Never*: refused ten times, no pause.
- ☐ *Hours without the PIN › Monday to Friday, and the weekend apart*: on a weekday in its span, no PIN; at the weekend, the weekend's span.
- ☐ *Close it once today's play time is used up*, a play timer with no time left today: the gate says *Today's play time is used up*, even with a PIN entered; time added (PlayGuard or the parent's app), it asks for the PIN again.
- ☐ From a computer, `config/playguard/gate_rules.txt` written with a line giving sphaira every hour: the gate still asks for the PIN (its rules are in the forwarder).
- ☐ From a computer, `config/playguard/gate-….nro` replaced by another `.nro` (hbmenu's): sphaira's forwarder ends with error 2347-0100 and does not start it; PlayGuard's next start writes the gate back.
- ☐ A forwarder protected by the previous PlayGuard: the bell says to make it again; *Make again*, then its hours without the PIN work.

## 3. Locking the homebrew menu

- ☐ *Lock the homebrew menu* before sphaira was started through its gate: the steps, the last one asking to start it from the HOME menu.
- ☐ The steps done, *Lock*: a short press on A does nothing; A held 2 seconds fills the button and locks it; a finger held on it does the same.
- ☐ The rescue sheet; on a computer, `config/playguard/rescue.html` opens and prints, its QR code readable by a phone.
- ☐ The album: it no longer opens the homebrew menu (the album itself opens). A game started while R is held: the game, not the homebrew menu. **No restart needed.**
- ☐ sphaira from the HOME menu, the PIN entered: sphaira opens, and starts any homebrew (PlayGuard among them).
- ☐ While locked, *Delete* on sphaira (the only protected menu) or PlayGuard's forwarder: refused, *Unlock first*.
- ☐ *Unlock*: the album opens the homebrew menu again at once, R on a game too.

## 4. Ways back

- ☐ Locked, sphaira's forwarder deleted in System Settings › Data Management: at the next start, the bell says a way back is missing; the lock stays.
- ☐ From a computer: `atmosphere/config/override_config.ini` deleted, the console restarted: the album opens the homebrew menu; PlayGuard's next start tells the lock was taken away (bell, history).
- ☐ With the rescue module: locked again, an empty `config/playguard/RESCUE` file, a restart: unlocked at boot, PlayGuard telling so at its next start.
- ☐ Atmosphère updated while locked (rescue module installed): unlocked at boot, PlayGuard telling so.
- ☐ With an emuMMC: locking from one system warns that the lock is on the SD card; booted into the other, the album no longer opens the homebrew menu there either, and that system has none of the forwarders.

## 5. Backups

- ☐ *History › Tries this week*: the refusals of section 2, by day.
- ☐ *History › Save a backup* with forwarders made: the file (`config/playguard/backups/`) has a `forwarders` part.
- ☐ A forwarder deleted in System Settings, the backup restored: PlayGuard offers to put it back; once put back, it is on the HOME menu, protected as it was.

## Afterwards

Unlock the homebrew menu, delete the forwarders made for the test (or keep them), and say in the pull request which steps were checked and which were not.

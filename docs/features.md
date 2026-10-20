# Features

What each tab does, how PlayGuard writes safely, and what the console counts as play time. New to PlayGuard? Start with [getting started](getting-started.md). Back to the [README](../README.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

The screenshots show the dark theme, PlayGuard's default.

| Today | Console |
|---|---|
| ![Today](../images/screenshots/dashboard.png) | ![Console](../images/screenshots/console.png) |
| **Play timer** | **Play timer › editing a day** |
| ![Play timer](../images/screenshots/play_timer.png) | ![Editing a day](../images/screenshots/per_day.png) |
| **Activity** | **A game** |
| ![Activity](../images/screenshots/activity.png) | ![A game](../images/screenshots/activity_game.png) |
| **Restrictions** | **Console › Network clock** |
| ![Restrictions](../images/screenshots/restrictions.png) | ![Network clock](../images/screenshots/clock.png) |
| **PIN** | **Preferences** |
| ![PIN](../images/screenshots/security.png) | ![Preferences](../images/screenshots/preferences.png) |

## Controls

| Button | What it does |
|---|---|
| ↑/↓ | Move |
| Ⓐ | Confirm |
| Ⓑ | Back or cancel (twice on the sidebar to exit) |
| Ⓧ | Refresh (on the tabs that read the console) |
| Ⓨ | Notifications, from any tab |
| **−** | Language and theme, from any tab (the footer leaves them out; the header's bell, language and theme show them) |
| **+** | Saves per-day limits (and the week edited on *Play timer*) |
| **ZR** | *Undo*, for a few seconds after a change is saved |

The app is organised in tabs, like System Settings; each has its section below.

- **What failed is said in a dialog, what worked in a toast.** A dialog only says what applies to this console now: that the game in progress would be suspended, only while a game is open; that the companion app could overwrite a change, only while it is linked and can sync.
- **The header** says, on every tab, while parental controls are temporarily unlocked (its pill), and shows a crossed-out pencil right of the title when the app is read-only; a tap on either goes where it is undone (*PIN › Turn parental controls back on*, the firmware screen).
- **In read-only mode**, actions stay in place, greyed, and say why when pressed.

## The look: Cadran

PlayGuard's look is called Cadran: an icon rail for the tabs, a navy or cool-white palette (with the theme), cards instead of lists. Each tab opens on what matters most (a ring, tiles, cards, chips), its other settings underneath.

- **The header's pill** says what the tab is about (*Parental controls on*, *Play timer on*…); while parental controls are unlocked it says so on every tab, and a tap on it opens *PIN › Turn parental controls back on*. Secondary text is never under 15 px, to read from the couch on a TV.
- **Dialogs** are cards with their buttons as pills, the confirmation in teal (in coral when it removes or stops something).
  - The choices open as a card above the hints, the current one on a soft teal with its check. The teal ring is the focus's alone, everywhere: a level, an account, a choice in force keeps the soft fill.
  - A long dialog fades at its foot while more follows, and its warning (a game about to be suspended) comes first, in an amber card.
  - A failure names what failed as its title, the error code small under the reason, and offers *Send a report* when one helps.
  - Messages appear as cards under the header, with a teal check for what was done and an amber mark for what was refused.
- **The other screens** take the same look: a profile's per-day editor shows the week as on *Play timer* (edited the same way, the quick fills as tiles); *Check the installed games* and *All games* put their lines in cards; *Profiles* and *Help & diagnostics* give each line its icon.
- **Text size** (*Preferences*, or **AA** in the header):
  - *Automatic*, the default, makes every text 25 % bigger while the console is docked, on the TV, and puts it back in handheld mode; *Larger* keeps it bigger, *Normal* never.
  - The header, the sidebar and the footer keep their size (they have no room for more), and so do the Restrictions levels' names, large already.
  - It changes at once, on every screen, with no restart.
  - The 25 % come from a 55" TV in 4K: the smallest text (15 px) is then 14.6 mm high in capitals, 20′ of arc from 2.5 m, where reading is comfortable (16′ in normal size, the bare minimum).
- **Visual effects** (*Preferences*): *Lighter on battery*, the default, leaves out the background glow, the beat of the child screen's ring in its last minutes and the arc's sweep to a new value while no charger is connected (handheld, unplugged); *Always full* keeps them. Read again every 5 seconds, so plugging the console in brings them back.
- **Undo:** after a change is saved, **ZR** *Undo* stays in the footer for a few seconds: it opens that change in the change history, to put the previous value back. An empty change history says so in a card.

## Notifications — what needs you, from any tab

A bell in the header, left of the version, gathers what needs the parent; Ⓨ opens the list from any tab (the bell's badge says *2* when two things are to do).

Left of the bell, the language (its code, *FR*, *EN*…) and the theme (a sun for light, a moon for dark, a half-filled circle when it follows the console), then the text size (**AA**, its large A in teal while the text is larger): a tap opens the same list as *Preferences*, and **−** offers them from any tab. A new theme or language takes effect at once, every screen made again on the same tab with the focus on the same line, Chinese and its own font included.

### To do

Each with an amber warning sign, counted on the bell in amber; it stays until it is fixed:

- no PIN yet, or a PIN anyone would try first (0000, 1234, 1212, 2580…: judged on the console, never shown);
- a PIN on the console while PlayGuard itself opens without it (*Protect PlayGuard with the PIN* on *Never*);
- parental controls temporarily unlocked (*Turn back on*);
- extra time from an earlier day still to put back;
- the "time's up" alarm off;
- changes made outside PlayGuard;
- the console not counting play time;
- an inaccurate network clock.

### For information

Each with an "i", a dot on the bell; it can be marked as read:

- a limit still lowered by *No more play today* on an earlier day (*Put back*);
- a new version installed (*What's new*; after an update in place, the previous build kept), or a newer version of PlayGuard;
- a report sent online (*Show the link*), or PlayGuard closed unexpectedly last time (*Send a report*);
- the week's play time (every 7 days until read; with how it compares with the week before, per day; *Show its QR code* opens the week's summary on *Activity*), and a limit passed on two or more of the last 7 days while the console only warns;
- a settings backup to make (none yet, or none for 30 days);
- all games blocked for a week;
- a new user account;
- the companion app linked;
- the network clock found off at start-up.

### Console

The entries about the console rather than the parental controls come last, on one line that Ⓐ opens (open at once when nothing else is there). Those to do are counted on the Console tab's badge, not on the bell:

- installed games that cannot start (*Check the installed games*), game patches that need a look;
- the SD card almost full (under 500 MB);
- one of PlayGuard's own files that could not be put aside or written to the SD card;
- the recovery sysmodule out of date (not the one this version ships with, or installed before it said its version; *Update* puts in the one this version carries), then the console to restart for it, until it has;
- a build that did not finish starting and was gone back from (*Send a report*).

Those for information: a build gone back from, a firmware not tested with this version, a newer Atmosphère, hekate or firmware (checked with *Check for updates every day*; the firmware's entry says to wait until Atmosphère supports it), the serial number not blanked on emuMMC, the other system's own parental controls, one of PlayGuard's own files found damaged and put aside (a toast at start-up points to it).

### How entries behave

- Each section lists its entries in a fixed order, whichever check finished first.
- **In the sidebar**, an amber badge by a tab's name counts its entries to do, where they are fixed: the PIN tab (no PIN, a weak PIN, PlayGuard without its PIN, temporarily unlocked), the Play timer (the alarm off, the console not counting), Today (changes made outside PlayGuard, extra time to put back), the Console (its entries above, and the network clock). Ignored entries and those for information are not counted.
- In the tab, the same badge is on the row where each is fixed (*Network clock*, *Check the installed games*, *Storage*, *Game patches*; *Set or change the PIN*, *Protect PlayGuard with the PIN*, *Turn parental controls back on*; the Play timer's *Advanced* for the alarm), and the tab opens on the first of them.
- **Ⓐ** on an entry does the fix (*Turn it on*, *Set it*, *Update*…) or opens the tab where it is made. An entry goes by itself once what it says is fixed.
- **Ⓧ** marks an entry for information as read; **Ⓨ** marks them all. An entry marked as read stays read at the next start, until its text changes.
- **Ignoring:** a few entries to do can be ignored with Ⓧ, after a confirmation and the PIN: a PIN anyone would try first, PlayGuard opening without the PIN, the "time's up" alarm off, the SD card almost full (never no PIN, unlocked controls, the clock, the play timer not counting or PlayGuard's own files). An ignored entry is no longer counted, and is listed last under *Ignored*, where Ⓐ counts it again. It comes back by itself 30 days later (unless *Count ignored warnings again after 30 days* is off), once what it says is fixed and then happens again, or, for the PIN, for a new PIN of another length.
- *Preferences › Notifications* turns off three kinds: Atmosphère, hekate and firmware releases; the week's play time; reminders (backups, the other system, all games blocked).
- The state behind it is read when PlayGuard starts, every minute on the main screen, and every 5 s while *Today* is shown.

## Today — the day at a glance

*[Try it in the simulator ›](https://playguard.link/simulator/)*

### The ring and the line

**The time left** as a ring: a day without play says *No play*, not *0 min*; the extra time given today shows as a lighter segment at its end, and the ring moves to each new value.

Under it, what stops play first:

- before the time up or the bedtime alarm, the rest of the day as a line, now at its left, each of them a mark with its time (*16:40 time is up*, *21:00 bedtime*), teal up to the first, amber in its last 15 minutes;
- otherwise in words: the bedtime alarm once the day's time is spent, when games are allowed again; while parental controls are unlocked, today's limit on hold and why.

### The actions

- **Extra time**, with its amounts one press away: an amount focused moves the time up on the line (*with 15 min more*), or says on the bedtime alarm's mark that it still comes first; the first one stands out in teal once 15 min or less are left, or none.
- **Stop for today**, **Show the child**.
- **Today's limit…**: today's limit picked there and then, written after one confirmation.
- **The profile in force**, by its name, once a profile is saved (*Profiles…* when none matches): Ⓐ lists them, the one in force ticked, to put another in force after one confirmation (the days that change), or *Manage the profiles…*.
- While parental controls are unlocked, **Turn back on** comes first, the row's whole width, and *Extra time* steps aside.
- With no limit on any day, **Set the week's limits** opens the week on *Play timer*.

### The cards and pills

- The **bedtime alarm** (when games are allowed again, or in how long it rings when the console does not say; off, *Off* in large and Ⓐ *Set it*), **parental controls** and the **network clock**, as cards.
- The console's state as **pills**:
  - one says whether Atmosphère's DNS redirection blocks Nintendo's servers, and Ⓐ on it which ones and what that changes: no sync for the companion app, no network clock setting itself;
  - another, opened over a game, that the time spent in PlayGuard counts, Ⓐ saying how;
  - a game left open on the HOME menu shows as an amber pill (*still counts*, Ⓐ saying why), when the pill about PlayGuard opened over a game does not say it already.
- The pills put what needs the parent first (amber, with a triangle), then what is on (teal, with a check), then the rest (grey, with a dot): the shape says it as well as the colour.
- Under the pills, the extra time still to put back (Ⓐ puts it back) and the "time's up" alarm while it is off (Ⓐ turns it back on).

### First steps

Until the PIN and the week's limits are set, a card above the cards lists the three steps (the PIN, a daily limit, the network clock), each ticked in teal once done or ringed in amber, with how many are done (*1 of 3*); Ⓐ on one does it there.

### Is the console counting?

PlayGuard's own time counts while it runs as an application, so with a limit today the time left should go down while it is open. When it has not moved for about 90 s on screen (timer on, a limit today, not unlocked, not opened from the album), an amber line under the time played says *The console is not counting play time right now*, with the likely causes:

- the clock was set back (it counts again later);
- the network clock was never set (*Console › Network clock*);
- a limit was just written (start a game for a minute, then check).

It goes as soon as the time left moves again. The Play timer says it too.

### Changed outside PlayGuard

Today's play time starting over on the same day (a clock change does that), the console clock moved by more than 5 minutes, limits that differ from the ones PlayGuard last saw (the phone app, another tool): an amber card above the ring until dismissed (Ⓐ) or the next day, and an entry in the change history. PlayGuard does not run in the background: it only sees what changed while it was open, or since it last was, when it reads the console again.

### At the limit

The console shows "Time's up", then suspends the game (no "continue") or only warns: *When the time is up* shows which and changes it, once a limit or a bedtime is set ([details](parental-controls.md#when-the-time-runs-out)).

### Extra time and no more play today

- **Extra time today**: +15 min, +30 min, +1 h by default.
- **No more play today**: limit 0 for today only, *now* or *in 5 minutes* when the time played is known (the limit then becomes the time played + 5 min, time to save). At the lock the console's "Time's up" comes up, and unsaved progress may be lost — the confirmation says so.
- **The next day** — at start-up, or at midnight if the app is open — PlayGuard puts the usual limit back:
  - after extra time, by itself, with no question and no PIN (it only lowers the limit, as turning parental controls back on does), unlocking for a moment if the timer is counting and locking again at once. It waits while it cannot write (read-only mode, a firmware it does not support, the console lock on) and, a week later on the same day, while the lower limit would stop the game in progress (then the next day). To keep more time on a day, change that day's limit;
  - after *No more play today*, it asks whether to keep 0 or put the limit back (through the PIN, as any change that raises a limit), or puts it back as any change (*Play timer › Put the limit back by itself after "No more play today"*); "no limit" to put back is always asked.
- **Extra time on another day** (*Extra time › Another day…*, at the end of its list): tomorrow or one of the five days after, each listed with its limit, then the amount. That day's limit is raised now, for that date only, and put back by itself (it only lowers the limit) the next time PlayGuard is opened once the day is over — not on a console PlayGuard is not opened on, where the next same day keeps it too. One at a time: the list then offers to remove it (its limit back at once). A change of that day's limit meanwhile (the week, a profile…) wins: nothing is put back.

### The child screen

*Show the child* on *Today*, to hand the console over:

- only the time left (while parental controls are unlocked, today's limit, on hold), when play ends (*Ends around 16:40*; when the bedtime alarm comes first, the ring counts down to it instead, *45 min until bedtime*, as it does in the last hour of a day without a limit), the bedtime and what happens when time is up;
- sized to be read from across the room, on the TV; its text keeps its size whatever the text size;
- in the last 5 minutes (of play or before the bedtime), the end turns amber and the ring pulses slowly; the bedtime turns amber in its last 30;
- Ⓐ and Ⓨ, the parent's keys, are said small at the top right, so the footer shows the child's alone;
- Ⓐ goes back to the parent's screens with the PIN; Ⓨ gives extra time from there, after the PIN (the amounts of *Extra time*, the child screen staying); Ⓑ leaves PlayGuard (asking first to turn parental controls back on, as quitting does).

In larger text, Today still fits on one screen: the line above the headline says only the day and its limit (*THURSDAY · 2 h*), the buttons and the cards are a little tighter. It refreshes every 5 s (Ⓧ refreshes now).

## Play timer — the week is the editor

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=play_timer)*

### Editing the week

Ⓐ on a day edits it:

| Button | In the edit |
|---|---|
| ↑/↓ | Change it by 15 min, by the hour once held |
| ←/→ | Move to the next day |
| Ⓛ | Copy it to Monday–Friday |
| Ⓡ | Copy it to the weekend |
| Ⓨ | No limit |
| Ⓧ | An exact time (*Enter a duration…*: ↑/↓ by 15 min, by the hour once held, Ⓛ/Ⓡ by 5 min, OK focused) |
| Ⓐ or Ⓑ | End the edit |

- By touch, a day tapped is edited, then a tap on its top or bottom changes it by 15 min.
- The changes are saved together, with **+** or *Save*, as in the per-day editor (the confirmation lists the days changed, before → after); *Discard* puts them back.
- Inside each day's bar, the time played on its last occurrence (today's included, every account), in coral once the limit was reached, its limit too, and a key under the week says so (*over the limit*).
- A day changed and not saved yet has its limit in amber and its bar ringed in amber; under the week, the focused day's figures (*Thursday: 1 h 20 played of 2 h*).
- Above the week, in amber, a limit reached and what the console does then, or the console not counting play time.
- `0` minutes means *no play that day*; *Remove the play-time limit* turns the timer off.

### Chips and quick settings

**Chips** above and below the week open their choices at once: the profiles (how many are saved), the same limit every day, the bedtime alarm, *When the time is up* and the removal.

- **Same limit every day:** a quick list or an exact duration (*Enter a duration…*: ↑/↓ by 15 min, by the hour once held, Ⓛ/Ⓡ by 5 min).
- **Weekdays and weekend ›:** the editor, asking for Monday–Friday then for the weekend.
- **Extra time today** and **no more play today**, as on *Today*. A note under them says what the console does at the limit. While parental controls are temporarily unlocked, play time *may* not be counted: that is still open ([docs](parental-controls.md#still-open)).
- Every confirmation lists the days that change, before → after.

### Profiles

- **Profiles** saved on the SD card: apply (a profile already in force just says so), edit, rename or delete one; save the current limits or make a new one.
- Any name, in any script (*École*, *周末*, *Выходные* …) — a name without Latin letters gets a file name of its own, and two names that would map to the same file are caught.
- **Scheduled profile** (*Profiles › Scheduled profile*): a profile for a period — holidays, a trip — its first and last day picked from the next 60 days.
  - When the period begins, PlayGuard asks to put it in force (the days it changes drawn under the question); once it is over, to put back the limits it replaced, or to keep them.
  - Asked once a day, on the main screen, when nothing else is in front: PlayGuard acts only when it is opened, so a console it is not opened on keeps the limits it has. A period that went by with the profile never put in force is forgotten.
  - One at a time; the line changes, removes or ends it.

### Bedtime alarm

- The alarm time (16:00 to 23:45, or off) and when play is allowed again (05:00 to 09:00), the same every day.
- Or, from *By day of the week…* at the end of the alarm's list, Monday to Friday, the weekend or one day, the other days keeping theirs (each set of days listed with its time). When the days differ, the tab says *Varies by day*, and the week on *Play timer* shows each day's alarm under its name.
- Its place in the play-timer settings, worked out from the companion app's settings, is verified on a console (22.0.0); PlayGuard still changes it only once the console reports what PlayGuard reads there, checks the console's answer after the change and puts the previous settings back if it differs.

### Extra time settings and Advanced

- **Extra time:** the amounts offered (+15/+30/+1 h, +10/+20/+30 min …) and putting the usual limit back without asking after *No more play today* (extra time always comes back without asking). Either is done the next time PlayGuard is opened, from the next day on: a console it is not opened on keeps the limit of that day.
- **Advanced ›**, a screen of its own: the switch for the debug-class actions and, while it is on, the "time's up" alarm on/off and pause / resume the countdown. The same switch lets a backup or the history put the alarm back.

## Activity — who played what, and how long

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=activity)*

Time per game **today**, in the **last 7 days** and **in all**, from the console's own activity log — for every account or **one user account** (the accounts as chips at the top, when the console has several). PlayGuard starts on the account shown last, or on the one set under *Account by default* (every account, or one).

### At the top

- The accounts as chips, the period and the export.
- Today, the last 7 days and all time as figures. Under the last 7 days, how they compare with the 7 before, per day: ▲ *+20 min/day vs last week* in amber, ▼ less in teal, *same as last week* within 5 minutes.
- The four games most played as cards (Ⓐ opens one); over the last 7 days, the one played most more than the week before says by how much, from half an hour (▲ *+2 h*).
- Nothing played over the period: a card says so, with *See all time* when there is something to see.

### Charts and summary

- **A chart of the last seven days** (today on the right, a legend under it), with this week's limit for each day as a line and the time over it in amber, marked !; today's, the week's and all-time totals.
- **A summary** over the chosen period (today, the last 7 days, the **last 30 days** or all time), for the account shown: average per day, the most played game, the days played and the busiest one (last 7 days), the average session (all time).
- **Hours played, last 7 days**: a square for each hour of each day, Monday first, the stronger the more was played (every game, the account shown); then the sessions (play with breaks under 5 minutes is one session, a game changed included) and their average length, the longest one and its day, the two busiest hours in a row, and the time played between 21:00 and 06:00.

### All games and a game's page

- **All games**, on a screen of their own: every game played over the period, most played first, as a **grid of covers** or a **list** (Ⓨ switches, and the choice is kept), with the selected game's figures always on top: its icon and name, the period's play time, all time, launches and last play. Ⓧ picks the period (the Activity tab follows).
- Icons are read around the selected game (not in applet mode, to spare memory); a grey controller stands in until one arrives, and for a game without one.
- A large library shows its first 50 games, then *Show every game* (100 games at most in applet mode); the export always holds them all. PlayGuard reads 256 games at most (this week's first): beyond that, the Activity tab, All games and the export (not CSV) say that some are missing, and the all-time total leaves them out.
- **Ⓐ on a game:** its last seven days as bars, launches, first and last play, time per user account. Deleted games keep their all-time figures, even one deleted before PlayGuard was installed as long as the console's play log still names it, and their name once PlayGuard has seen it (else *Deleted game* and its title ID).

### Accounts

- A game's time that no user account holds (some games never ask for one, or the account was deleted since) shows as *No account*, from a minute up.
- A user account PlayGuard never saw before is flagged in amber on the tab for 30 days (*New account since …*, a fresh account is a known way to start over) and recorded in the change history; the accounts there when PlayGuard first lists them are not flagged.

### Export and sharing

- **Export to the SD card** (the *Export* chip) as CSV, JSON, XLSX (Excel) or PDF, one column per day, then the last 7 days, the last 30 days and all time.
- **The week's summary as a QR code:** a phone's camera reads it as a few lines of text, in the language shown — the last 7 days and their average, the trend, each day, the three games most played, the sessions, the longest one and the time played between 21:00 and 06:00. Nothing goes online.
- Opens at once on the last figures read (kept on the SD card between runs), refreshed in the background when over a minute old; Ⓧ reads them again now, with a spinner.
- Times are approximate if the console clock was changed.

## Restrictions — level, age rating, communication

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=restrictions)*

- **The restriction level:** Young child, Child, Teen and Custom side by side as tiles, each with what it restricts (a padlock for restricted, a check for allowed, and the installed games it blocks; Ⓐ sets it; on *Custom* in force, Ⓐ goes to its settings below); *None* under them. What a level restricts (age limit, posting, communication) is said again before it is set.
- **In Custom:** age rating, social-media posting, communication with others.
- **VR mode** and the **rating organisation** (PEGI, ESRB, USK, CERO …): the list says the countries each one rates for (PEGI: most of Europe, ESRB: the United States, Canada and Latin America, USK: Germany, PEGI BBFC: the United Kingdom…).
- **The number of games allowed to communicate** (the list itself is in the diagnostic report, raw, until its layout is known).
- **Installed games it blocks:** how many of the installed games the level in force blocks by their age rating (*3 of 15*), Ⓐ listing them with their rating (*PEGI 16*); each level's tile says it too (*Games blocked · 3*).
  - The ratings are each game's own (its control data, read by the check of the installed games at start-up or from *Console › Check the installed games*), by the organisation the console uses.
  - A game that organisation does not rate is not counted, nor the exceptions set in the companion app.
  - A game's screen in Activity shows its rating, and *blocked at the current level* when it is.

## PIN — the PIN, unlocking, blocking games, companion app

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=security)*

- **At the top**, three tiles: the PIN, the restrictions and the temporary unlock, each with a teal check when it is as it should be (a PIN set, restrictions on, locked) or an amber warning when it needs you.
- **Set / change the PIN** (system PIN screen).
- **Show the PIN**: after a warning and the PIN itself, asked every time; recorded in the history, then a new PIN is offered.
- **Unlock temporarily**, **turn parental controls back on**, and whether PlayGuard turns them back on by itself right after a change that needed the unlock (on by default).

### Protect PlayGuard with the PIN

The PIN in PlayGuard itself:

| Setting | Effect |
|---|---|
| *Never* | No PIN in PlayGuard (the bell flags it while the console has one) |
| *Before a change* (default) | Anyone can look, only the parent changes something |
| *To open PlayGuard* | A lock screen at start |

- Once entered, the PIN holds until PlayGuard closes; *Ask for the PIN again after 5 minutes* (off by default) asks again after them, and brings *To open PlayGuard*'s lock screen back when PlayGuard returns after 5 min or more out of focus.
- Checked in the service layer, so no change skips it; locking again never asks. *Show the PIN* asks for it every time, in every mode and even right after it was entered, and a missing or damaged `config.json` counts as *Before a change*.
- **Quitting** while parental controls are still unlocked asks whether to lock them again (*Turn back on and quit* also closes the game PlayGuard was opened over, back to the HOME menu rather than hbmenu); closed from the HOME menu instead, PlayGuard locks them again the next time it starts, and a restart of the console locks them by itself.

### Block all games

One switch that sets every day's limit to 0, so a PIN is needed to start a game — a light block without age ratings or communication limits.

- It blocks starting games, not the HOME menu, and needs a PIN.
- The previous limits come back when it is turned off.
- While it is on, *extra time* and *no more play today* are refused, and limits set another way (a profile, a backup, the history…) replace it.

### Companion app

- Whether the Nintendo Switch Parental Controls app is linked, its last sync, and **unlink** (otherwise its next sync overwrites the limits set here: while it is linked, every change PlayGuard asks to confirm says so).
- When Atmosphère's DNS redirection (`atmosphere/hosts`) sends Nintendo's parental-control server elsewhere, the app cannot synchronize: PlayGuard then says *sync blocked (DNS)* here and mentions the app nowhere else (no note in the dialogs, no bell entry, no first-steps step, no pill on *Today*, no help to decode, and unlinking asks without saying what the app would overwrite).
- While it is linked and can sync, *Before unlinking: help decode…* opens the block comparison (see [CONTRIBUTING.md](../CONTRIBUTING.md#no-code-needed)).

### Delete all parental controls

Two confirmations, irreversible; a backup of the settings is saved first.

## Console — the network clock, the installed games, what the console runs

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=console)*

**At the top**, what the console runs as tiles (the firmware, Atmosphère, and hekate when it is on the SD card; Ⓐ opens its release notes), then the diagnostics, then the console's lines, compatibility first (a teal check alone when PlayGuard was checked on this firmware, else a warning and what is wrong).

### Network clock

*Network clock ›*, a screen of its own — the clock the play timer relies on. A console that never reaches Nintendo's servers keeps this clock inaccurate, which skews the play timer.

- **Console and network clocks**, time zone, accuracy. When Atmosphère's DNS redirection (`atmosphere/hosts`) sends Nintendo's time server (`aauth-lp1.ndas.srv.nintendo.net`) elsewhere, the console can no longer set its network clock itself: this screen, the bell's entry and *Today*'s card say so and point to the measurement below.
- **Pick a public NTP server** (≈ 50 built in, by region, or your own).
- **Measure** against 3 servers (median; an amber warning when they disagree), then **set the network clock** — a measurement stays usable for 2 minutes, with a countdown. The confirmation says it: changing the clock starts today's play time over, so the whole limit is available again.
- **A network-clock check at start-up**: a toast when it is more than a minute off; it never sets the clock.

#### No server answers

On a console without Internet, *Set the network clock to the console clock* appears under the result (and on *Today*'s network clock card, after its measurement fails), behind the same confirmation.

- PlayGuard has nothing else to compare the console clock with: the confirmation shows its time and asks you to check it against a phone or a watch first, and a console clock before 2026 (never set) is refused.
- With *Synchronize Clock via Internet* off, the console clock stays where it is, and turning the setting on afterwards no longer makes it jump; with it on, the console clock already follows the network clock, so the confirmation says to turn it off and set the time by hand if it is wrong.
- It writes the clock only, not the parental controls, so it needs no unlock (the PIN of *Protect PlayGuard with the PIN* still applies); like any clock change, it starts today's play time over.
- Not tried on a console yet.

### Installed games

*Check the installed games* finds the games the HOME menu cannot start or draw — an icon that keeps loading, a "!", a dotted frame — with what is wrong and what to do, under the game's icon, and how many installed games have a problem.

**What it finds:** name and icon unreadable, the game itself missing behind its update, nothing installed any more, files missing, corrupt data the console found, a newer firmware needed by the game or only by its update, keys too new for the firmware, a missing license (ticket), the console asking for an update that was removed, the SD card unreadable, a launch the console refuses.

**The list has a section per way out**, each with its count and what to do, the games with the same problem together (two games with the same name are told apart by their version or title ID):

| Section | What it holds |
|---|---|
| **Fix here** | What PlayGuard repairs |
| **To check** | Data the console suspects: the check says whether to install the game again |
| **Reinstall** | Files gone or unusable, or data found corrupt: remove, then install again |
| **Firmware or SD card** | The game is fine, the console is not |

- **Checking data:** what the data check found is kept with the game until another version is installed: corrupt, it moves to *Reinstall* as *Corrupt (checked)*, with the date and the console's code; intact, it leaves the list. *Check these games* checks the whole of *To check* one game after the other in one dialog (*game 2 of 5*), then says how many are corrupt, intact or could not be checked, with removing the corrupt ones at hand.
- **Several games at once:** Ⓨ picks games (a check mark in front), and *Several games at once*, above the sections, picks a whole section; then one action for all of them: check their data, or remove them (their list saved first, one question, one PIN).
- **Under *Reinstall*,** their list can be saved (JSON and text, in the exports folder: name, title ID, version, problem), and all of them removed at once after one question and one PIN: the list is saved first, then the games are removed so an installer no longer counts them as installed.
- **The actions:** when one action is the fix, it is the dialog's first button; the others follow: accept the installed version again (when the console asks for a removed update), remove only the update or an add-on, check the data for corruption (the console's own check, with its progress and Cancel), or remove the game.
- Versions are shown as the game shows them (*version 1.4.0*, *update #4*); after a repair, a toast says whether the game works again. Save data always stays; every change is in the change history.
- **When it checks:** the last check shows at once; then only the games whose entry changed are checked again (every game once a day, after a firmware update, or with Ⓧ *Check again*), each shown as soon as it is checked.
- Several broken games at once points to the SD card or to a game installed from the other storage (sysMMC / emuMMC).
- *View*, above the sections, shows the games as a **list** or as **covers** (icon, name and problem; the choice is kept), as *All games* does.

### Forwarders

*Forwarders ›*, a screen of its own under *Homebrew* — a homebrew on the HOME menu, with its icon and its name, started from there like a game (an NRO forwarder, as sphaira makes them).

- **A card at the top** says where the homebrew menu stands: locked or open, its ways back (PlayGuard's forwarder, the protected menus, those never started from the HOME menu yet), what still opens it without the PIN, and whether the rescue module is installed. Ⓐ on it lists what to do about it, each a press away (the next step to a lock, protecting or deleting a forwarder that opens it without the PIN, the check, the rescue sheet).
- **The list by group:** PlayGuard's own, those protected with the PIN, those without it, those another app made, those no longer on the HOME menu. A protected one has a padlock on its icon (a check for a menu seen working through its gate), one made here without the PIN an open padlock in amber.
- **Ⓨ** opens this part of the guide (a QR code). After deleting or forgetting a forwarder, or changing its hours without the PIN, **ZR** puts it back for a few seconds (installed again with the same program ID, or the hours as they were).
- **PlayGuard** in one press: its own icon and name, the forwarder starting the `.nro` PlayGuard was opened from.
- **Another homebrew**: any `.nro` of the SD card's `/switch` folder (and one folder down), its name asked first (the `.nro`'s own, to keep or change), then confirmed with what the HOME menu will show: its icon, its name and its publisher. Its icon is the `.nro`'s, made the 256 × 256 JPEG the HOME menu draws when it is another size or a PNG (a controller in its place when the `.nro` has none), as sphaira does. A path with a space is handed to the homebrew quoted.
- **The forwarders on the HOME menu**, each with its icon: those PlayGuard made (Ⓐ: details, rename, change the publisher, install again, delete) and those another app made (details, delete). One whose `.nro` is gone says so; one deleted in System Settings stays listed until it is installed again or forgotten. Installing again keeps its program ID, so the HOME menu's entry is replaced, not doubled; a forwarder another tool made for the same `.nro` is replaced too.
- **Protected with the PIN** (Ⓐ › *Protect with the PIN*, for one PlayGuard made other than its own): started from the HOME menu, the forwarder first opens a small gate (`config/playguard/gate.nro`, a few hundred kilobytes that PlayGuard writes from its own files), which shows the console's PIN screen at once, then the homebrew once the PIN is right; B there goes back to the HOME menu. Around the PIN screen, the gate's own: the homebrew's icon and name, *Protected by PlayGuard*, and how it went (starting, in its free hours, PIN not entered), in the console's language, on Cadran's night blue (`make gate-preview` draws it on a computer). It keeps its program ID and its icon, the row says *PIN*, and the change history has it. It needs a PIN on the console. *No longer ask for the PIN* asks for it first, whatever *Protect PlayGuard with the PIN* says. It guards the HOME menu's icon only: the `.nro` still starts from the homebrew menu, unless that is locked (below). A forwarder protected by an earlier PlayGuard starts PlayGuard itself as its gate, which still works; *Install them all again* moves it to the small gate.
  - **Hours without the PIN** (Ⓐ on a protected one): every day, Monday to Friday or the weekend, from one hour to another (past midnight too); the row says when. Setting them asks for the PIN.
  - **Hours it never opens** (Ⓐ on a protected one), set the same way: then the gate says *Not at this time* and the hours, and goes back to the HOME menu, even with a PIN entered (bedtime for homebrew). They come before the hours without the PIN.
  - **A pause after refused PINs** (*Pause after refused PINs*, under *Homebrew menu*): five refusals in a row (B on the PIN screen) and the forwarder does not ask again for 15 minutes, the gate saying until when; 3, 5 or 10 tries and 15 minutes or an hour, or never. The bell says which forwarder is paused, its row too; *Lift the pause* (Ⓐ on it, the PIN asked) makes it ask again at once. Each pause and each lifted one is in the change history.
  - **Every try is in the change history**: opened with the PIN, opened in its hours, or refused (B on the PIN screen), at the time it happened; the tries of one kind on one day for one forwarder make one entry, with their times. The gate notes them on the SD card; PlayGuard reads them at its next start. *History › Show › Homebrew and forwarders* lists only these, the forwarders and the lock.
  - **The gate's screen says more:** its hours without the PIN while it asks, and the day's refusals so far (in amber) while it asks and once refused, under *Ask a parent to enter the PIN*.
- **Its icon from the album** (Ⓐ › *Change the icon*): a screenshot of the album (the newest 30, by date), its middle square made the HOME menu's 256 × 256 JPEG; or the `.nro`'s own again. Installing again keeps it.
- **Icons and languages:** an icon of another size, a PNG, or a JPEG with EXIF data or progressive coding (the HOME menu shows those as a “?”) is made a plain 256 × 256 JPEG. The forwarder's control data lists American English alone, the language of its one icon: so the icon shows on a console set to any language.
- **Protect every forwarder with the PIN** and **Install them all again** (a new `.nro`'s icon and version, the gate as this PlayGuard carries it, one deleted from the HOME menu put back): in one press each.
- **Lock the homebrew menu** (*Homebrew menu*, last on the screen): neither the album nor a game started while R is held opens the homebrew menu any more, so a homebrew starts only from its forwarder, which can ask for the PIN.
  - **Step by step:** until it can be locked, Ⓐ lists the steps, numbered, the next one in amber and a press away (*Next step*): a PIN, the signature patches, PlayGuard's forwarder, a homebrew menu protected with the PIN (sphaira found in `/switch`, else hbmenu, made protected in one go), started once from the HOME menu; the list comes back after each step that makes a forwarder.
  - It takes two ways back to PlayGuard from the HOME menu: its own forwarder, and a forwarder of a homebrew menu (sphaira or hbmenu, by the `.nro`'s name) protected with the PIN, which keeps every homebrew, a homebrew store included, a PIN away. And a PIN on the console, and the signature patches forwarders need. Until then, Ⓐ says what is missing.
  - The menu's way back counts once it has been seen working: started once from the HOME menu, the PIN entered at its gate (the gate's log says so, read when PlayGuard opens or locks). A menu protected and never started that way is listed as such.
  - Locking shows the checks as a list (a teal check for what is ready, an amber or coral triangle for what is not), and asks for Ⓐ held 2 seconds on *Lock* (or a finger held on it), a bar filling meanwhile; let go sooner, nothing changes. Then comes the **rescue sheet** (also written to `config/playguard/rescue.html`, a page to open and print on a computer, in the UI's language, with the QR code): how to undo it from a computer, or with the rescue module when it is installed, and a QR code to the help ([below](troubleshooting.md#homebrew-menu-locked)); *Check the homebrew menu* has it too.
  - On emuMMC (or a sysMMC with an emuMMC set up), it warns that the lock is on the SD card, so both systems read it, while the forwarders are each system's: the other system has none of these ways back.
  - While it is locked, deleting PlayGuard's own forwarder, or the last protected menu (or taking its PIN away), is refused: unlock first. Deleting one of several protected menus asks first, in coral, naming those that remain.
  - Before locking, it offers to protect the forwarders PlayGuard made that are not protected yet, in one go, and says which would still start homebrew without the PIN (a menu not protected, or one another app made), which write to the SD card (a file manager, an installer such as Cyberfoil or Goldleaf, an FTP server: they could change the lock's file), and when uLaunch is installed (it starts homebrew its own way, which the lock does not reach).
  - The PIN refused three times or more on one day at a forwarder's gate is told in the bell (which forwarder, the day, how many times).
  - When writing a file fails, the error says why when it can tell: the SD card full, or nothing written to it at all (its lock switch, or the archive bit hekate's *Tools* fixes).
  - *Check the homebrew menu* shows what Atmosphère will read (whether the album and a game started with R open the homebrew menu), the ways back, the signature patches, the Atmosphère version it was locked on and the rescue module; the diagnostic report has the same.
  - It is a few lines at the end of `atmosphere/config/override_config.ini`, between two of PlayGuard's comments; the rest of the file is left as it is, and kept as it was in `config/playguard/override_config.ini.backup`. Atmosphère reads the file at every start of a program: no restart. Unlocking takes the lines away, and writes back Atmosphère's own values for the keys the rest of the file does not set (Atmosphère keeps what it read until the console restarts).
  - Locking and unlocking ask for the PIN whatever *Protect PlayGuard with the PIN* says, and are in the change history. While it is locked, PlayGuard checks at every start that both ways back are still there; the bell says when one is gone. A lock taken away from the file outside PlayGuard (a computer, a homebrew) is told in the bell and the change history; PlayGuard never puts it back by itself, deleting the file being the way back from a computer. Deleting a forwarder (System Settings › Data Management, where no PIN is asked) never lifts the lock: it only takes a way back away.
  - From a computer: delete `atmosphere/config/override_config.ini` (or put the backup back), then restart the console.
  - With the rescue module installed (optional, [sysmodule/README.md](../sysmodule/README.md)), a `RESCUE` request lifts it at boot too, and so does an update of Atmosphère since it was locked (its signature patches may lag, and no forwarder starts without them): the next start of PlayGuard tells it, and offers to lock it again.
- **A forwarder deleted outside PlayGuard** (in System Settings): told once in the change history, and in the bell while it lasts, with *Install again* or *Forget* in its row.
- **Signature patches:** the console starts a forwarder only with fs and loader signature patches (sys-patch, or sigpatch files): without them, the screen says so above the list (*Console › Game patches* has the details). The `.nro` stays where it is; deleting a forwarder never touches it.
- **Play time:** started from its icon, a homebrew runs as a game: the play timer counts it and suspends it like any game once the time is up, and Activity lists it. PlayGuard stays at hand from the homebrew menu.
- **In the settings backups** (*History › Backups*): what PlayGuard made and whether the homebrew menu was locked. Restoring a backup offers, after the rest, to put back the forwarders no longer on the HOME menu (protected as they were, their hours and icon); a lock is only said, to be done again from here.
- Every change asks for the PIN when *Protect PlayGuard with the PIN* does, is refused in read-only mode, and is in the change history. What PlayGuard made is kept per system (sysMMC / emuMMC), in `forwarders.json`.

### The console's lines

- **Firmware, Atmosphère and hekate** (when its `bootloader/update.bin` is on the SD card), each with the newer release the last update check found, in amber (*22.0.0 · 23.0.1 available*, and how many versions behind when more than one), and a warning icon on the firmware's row when the newest firmware needs a newer Atmosphère or hekate first, or is not supported by them yet (the firmware's release notes say why: their release notes say which firmware they support).
- **Release notes:** Ⓐ on each opens them: a tab for the installed version, one for each newer one (at most five) and older ones in the tabs left (six in all; Ⓛ / Ⓡ move between them), opening on the next one, with tags (installed or newer, the newest firmware it supports, security fixes, parental controls changed), the notes as published (in English), *See online* (a QR code) and when they were fetched. Atmosphère's and hekate's come from their GitHub releases, the firmware's from switchbrew; they are kept on the SD card for offline reading, fetched again once a day (Ⓧ now).
- **Compatibility**, then **storage** (emuMMC or sysMMC, with an info icon when the other system has its own parental controls: Ⓐ explains).
- Whether Atmosphère **blanks the serial number** (partly hidden until Ⓐ; on emuMMC, when it is not, a warning icon on the row above, which Ⓐ explains).
- **Game patches** (sys-patch or sigpatch files, recommending sys-patch when only files are used: an info icon, or a warning one when something needs a look, which Ⓐ explains).

## History — changes and backups

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=history)*

### Change history

- The newest 500 changes PlayGuard made (limits, restriction level, PIN, unlocks, unlinking, the clock, restores, pausing or resuming the countdown …) and what it saw changed outside it, grouped by day (*Today*, *Yesterday*, then the day and date), with the time and from where; a limit changed on one day names that day.
- A change the console **refused** is listed too, in amber, with the console's error and what was tried (never put back: nothing changed).
- *Show* keeps one kind (play time, restrictions, security and app, seen outside PlayGuard, refused by the console); *Export what is shown* writes it to `exports/` as CSV, JSON, XLSX or PDF.
- Ⓐ on a change shows it and, for a value, **puts the previous one back** — through the same unlock and PIN as any change, saying if it changed since.
- Each change keeps the PlayGuard version that made it; a change recorded by a newer version of PlayGuard shows as *Other change*.

### Backups

- **Back up / restore the settings** on the SD card: restriction level, custom settings, VR mode, rating organisation, daily limits, the "time's up" alarm (with the advanced actions on), and the raw play-timer block for the record — never the PIN.
- A restore lists only what would change (nothing to change: it just says so, with no unlock and nothing written), and says when the backup had a bedtime alarm on (it is not written back).
- Choose how many backups to keep.

## Preferences

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=preferences)*

- What sets PlayGuard itself up:
  - Language (every console language, or the console's own), theme (dark by default, light, or the console's), text size (automatic / normal / larger): all three at once.
  - The tab to start on.
  - **The intro** at start-up: the loading screen for 2 seconds (on by default; Ⓐ or Ⓑ skips it; off, PlayGuard opens as soon as it is ready).
- The messages it shows:
  - The kinds of notifications (see [above](#how-entries-behave)).
  - A **monthly reminder to support PlayGuard** (on by default, never in the first month nor right after an update; *Don't show again* on the reminder or this switch turns it off for good, updates included).
- A setting that shapes one feature sits with it: extra time in *Play timer*, turning parental controls back on in *PIN*, the clock check in *Console › Network clock*. The updates and *Help & diagnostics* are in *About*.

## About

*[Try it in the simulator ›](https://playguard.link/simulator/?tab=about)*

### Version and updates

- The installed PlayGuard: version (in amber when a newer one is out; Ⓐ opens its **release notes**), launch mode and data folder.
- **Check for updates** now; *Check for updates every day* at start-up.
- *Update with*: in PlayGuard, which downloads the release, checks it, replaces itself and restarts, or sphaira, Homebrew App Store or by hand (asked on the first update when none is chosen). Installed from the Homebrew App Store or sphaira's App Store, the store is the way recommended there, so it keeps track of the version; when its catalogue does not have the new release yet (it publishes one after a review, often a few days later), PlayGuard says so and offers to update in place instead.
- While a newer release is known, PlayGuard opens on **PlayGuard X.Y.Z is available** (its date and notes): *Update*, or *Later* for 1 day, 1 week or 30 days at most.
- The release notes are PlayGuard's own, on the same screen as Atmosphère's and hekate's in *Console*: from its GitHub releases, a tab per version (the installed one, the newer ones, the older ones), each with its highlights as cards and every change, kept on the SD card for offline reading (the installed version's are in the app too, for when nothing was fetched yet).
- After an update, PlayGuard opens once on **What's new in X.Y.Z** (the same highlights, then the QR codes).

### Help & diagnostics

*Help & diagnostics ›*, a screen of its own, below *Update with*:

- **First steps** opens the guide again (with an *Unlink the companion app* step while linked, a *Turn the "Time's up" alarm back on* step while it is off, a *Protect PlayGuard* step once a PIN is set — the *Protect PlayGuard with the PIN* choice —, and a switch to stop it coming up at start-up). Below *Close*, *Support PlayGuard* shows the funding QR codes.
- **Export a diagnostic report**, or **send one online** (see [Reporting a bug](troubleshooting.md#reporting-a-bug)); how many saved reports, crash files, session logs and links of reports sent online are kept, by number, age or size.

### Credits

The credits, how to **support PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), shown as QR codes to scan with a phone), and a small *Made in France* 🇫🇷.

## Safety by design

**Writing the play-time limit safely.** Overwriting the timer's configuration while it counts down destabilises Atmosphère. So PlayGuard checks the state first; if the timer is active, the confirmation says parental controls will be unlocked temporarily — with the stored PIN, **you don't need to remember it**. One press then unlocks, checks that the system really reports the unlock, writes, and **locks again right away** (or offers to, if *Turn parental controls back on after a change* is off). The service layer re-checks the state just before writing, so no screen can skip the safeguard.

> [!CAUTION]
> A limit below the time already played today suspends the game as soon as parental controls are locked again. The confirmation says so when that would happen.

**No more 22.5 crashes.** `pctl:a`, the privileged parental-control service, accepts a **single session**. Older builds of the original app kept it open, so the HOME-menu PIN prompt (or the PIN applet) could not get it and Atmosphère could crash. PlayGuard opens the session for each action and releases it immediately; periodic refreshes pause while the app is in the background. *(Diagnosis by [anbingxi's fork](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Nothing runs in the background.** The limit, the PIN, the warnings and the suspension are the console's own; PlayGuard only changes their settings.

## What counts as play time

The play timer is the console's: PlayGuard reads it, it does not count anything itself. What is known, and how sure it is ([parental-controls.md](parental-controls.md#time-spent-and-time-left) has the measurements):

| Situation | Counts? | How sure |
|---|---|---|
| A game running | Yes | Measured (22.0.0) |
| **A game left open on the HOME menu** (suspended in the background) | **Yes**: close it, or put the console in sleep mode | Reported by parents ([Arqade](https://gaming.stackexchange.com/questions/398954/)); fits the measurements (the console counts while an application is open) |
| Sleep mode | No | Reported ([Arqade](https://gaming.stackexchange.com/questions/398954/)); not measured by PlayGuard |
| **PlayGuard opened over a game** (as an application) | **Yes**, as that game | Measured (22.0.0): open it with a parent's user, or from the album for a quick look |
| PlayGuard opened from the album (applet mode) | No | Inferred: it is not an application, and time outside applications did not seem to count |
| While parental controls are temporarily unlocked | Maybe not | Not measured: still open |
| After the clock was changed | The day starts over; after a jump back the count stops until the clock passes the old time | Observed (22.0.0); *Today* says when the console is not counting |

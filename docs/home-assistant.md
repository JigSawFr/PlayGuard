# PlayGuard and Home Assistant

**English** · [Français](home-assistant.fr.md)

PlayGuard can publish the console's parental-control state and play time to an
MQTT broker, and take orders from it. Home Assistant then shows the console as
a device (limits, time left, play time per game, bedtime, restrictions) and
can change the limits, add extra time or lock the console from a dashboard or
an automation. The link is **optional and off by default**, stays on your
local network, and works with any MQTT broker; the Mosquitto app of Home
Assistant is the easy path.

> [!NOTE]
> On its own the link runs **while PlayGuard is open** on the console: orders
> sent while it is closed or the console sleeps wait on the broker and are
> carried out the next time PlayGuard opens. The optional **agent** keeps it up
> all the time ([below](#keep-the-link-up-when-playguard-is-closed)).

## What you need

- Home Assistant **2026.5 or later** (the bedtime's `time` entities: an older
  one refuses the whole device), with the **MQTT** integration set up, and a broker: the
  **Mosquitto broker** app (Settings › Apps; an add-on in older Home Assistant) or any other MQTT broker,
  in version 5.0 or 3.1.1 (EMQX, HiveMQ, NanoMQ, VerneMQ …): *Remote access ›
  MQTT version* is automatic (5.0, else 3.1.1).
- A broker user for PlayGuard. With the Mosquitto app, any Home Assistant
  user can log in: create one named `playguard` (Settings › People › Users,
  "can only log in from the local network"), or add a login in the app's
  configuration.
- The console on the same network as the broker, with a PlayGuard that has
  *Console › Remote access*.

## Set it up on the console

1. Open PlayGuard › **Console › Remote access › MQTT / Home Assistant**.
2. **Broker address**: the IP address or name of the machine running the
   broker (Home Assistant's, for the app). **Port**: 1883 (8883 for TLS,
   which turns *Encrypted connection* on).
3. **User name** and **Password**: the broker user created above. A broker
   that accepts anonymous clients: *User name › None*.
4. **Name in Home Assistant**: the device's name (*Living room*, *Léa's
   Switch* …). Left empty, it is the console's nickname (System Settings ›
   System › Console Nickname), or *Nintendo Switch* when *Publish the
   versions and hardware* is off.
5. Turn **Remote link** on. The console gets a random id (8 characters, never
   the serial number), and **Status** reads *Online* within a few seconds.

Changing any of these asks for the PIN when *Security › Protect PlayGuard with the PIN* is
set to *Before a change*: the link can change the console.

## Keep the link up when PlayGuard is closed

The **agent** is a small optional background module (an Atmosphère
sysmodule, [`sysmodule/agent`](../sysmodule/agent/README.md)) that holds the
same link while PlayGuard is closed, with the same settings:

1. PlayGuard › **Console › Optional modules › Remote link agent › Install**
   (PlayGuard carries it; or extract `playguard-agent.zip` from the release at
   the SD card's root). It starts at once, and at every boot.
2. Nothing else to set up: it reads *Remote access*. Its **Status** then reads
   *Online · through the agent*.
3. After an update of PlayGuard, it offers to update the agent at start-up;
   the previous agent is put back if the new one does not answer.

While PlayGuard is open, the agent stays the console's only MQTT client
(Home Assistant never sees it go offline); PlayGuard passes it what it reads
and carries out the orders, with its confirmation, PIN check and history as
usual. While PlayGuard is closed, the agent reads the console itself every
`poll_s` seconds and carries out orders **only under *Carry them out at
once***: under *Ask on the console* they wait for PlayGuard. It changes
nothing when PlayGuard was in read-only mode or after a system update
PlayGuard has not checked yet. What it changed shows up in PlayGuard's change
history, with its time, the next time PlayGuard opens.

## What appears in Home Assistant

Settings › Devices & services › MQTT: a device with the console's name,
created by MQTT discovery (turn *Create the device in Home Assistant* off to
handle the topics yourself). It is the console as Nintendo names it, its model
(*Nintendo Switch – OLED Model*, HEG-001) and its system as its firmware
(*23.0.1 · Atmosphère 1.10.1*); PlayGuard is where it comes from.

| Kind | Entities |
|---|---|
| Sensors | Play time today (the play log: every game, with or without a limit), Used screen time (the play timer's count, off), Screen time remaining, Extended screen time, Now playing (unknown while PlayGuard is open: nobody plays then), Age rating, Firmware, Atmosphère, Last order |
| Binary sensors | Parental controls set up, PIN set, Companion app linked, Play timer, Playing blocked (its *reason*: `limit` or `bedtime`), Network clock accurate |
| Lock | Parental controls: locked while they are in effect; unlock it to unlock them for now (with *Orders may change the play timer* on), lock it to turn them back on |
| Numbers | Limit Sunday … Saturday (off), Limit every day, Max screentime today (0 to 1440 minutes; 1440 means no limit) |
| Time | Bedtime, Wake-up time |
| Switches | Block all games, Time's up alarm, Bedtime alarm, VR mode restricted, Posting to social media restricted, Communication restricted |
| Selects | Restriction level (*unrestricted*, young child, child, teen, custom), Profile (your saved profiles) |
| Buttons | Sync now, Extra time +15 / +30 min / +1 h, No more play today, Remove the limit, Export a diagnostic report |
| Event | Events: every order applied, refused (with the reason) or kept waiting for the console; and, from the agent, what the console does on its own: a game started or stopped, 10 minutes left of today's limit, games blocked (the limit or bedtime) |
| The console | Model, Console nickname, Region, hekate, System (sysMMC / emuMMC), PRODINFO blanked, Serial number (off, see below), SD card free space and size, Internal storage free space and size, Battery, Charging, Docked, Temperature, Connection, IP address, Wi-Fi signal, Wi-Fi network (off), Last started |
| Games | Games installed, Play time (all time), User accounts; the integration below lists every game on *Games installed*, with its play time, and keeps each game's history in a long-term statistic |

What a dashboard needs is on; the rest is off in Home Assistant until you turn
it on, on the device's page (*entities not shown*): Used screen time (the
play timer's), each day's limit, Age rating, Firmware and Atmosphère (the
device's info shows both), Last order, Network clock accurate, Export a diagnostic
report, User accounts, and the console's own but the free space, Battery,
Charging and Docked.

**What is published** (*Remote access › What is published*, behind the PIN):
the parental controls and the play timer always are; the play time, the
games, the games' icons, the console's state (battery, storage, dock), the network (IP address,
Wi-Fi), the versions and hardware (model, firmware, Atmosphère) and the
diagnostic reports each have a switch, on by default but the reports. What
you turn off is cleared from the broker and its entities go from Home
Assistant (the integration below follows too); turn it on again and they
come back.

The console's own entities read `playguard/<id>/system`, which the agent
keeps fresh while PlayGuard is closed; the games ones read
`playguard/<id>/games`, which PlayGuard publishes after each read of the play
log. Every game's icon (installed or played) goes to
`playguard/<id>/icon/<game id>`, each sent once by PlayGuard while it is open
and the link online; the agent adds the one of the game being played when
PlayGuard has read it but not sent it. A game installed since PlayGuard last
ran gets its icon the next time PlayGuard is opened. Only games are listed
(their names, icons, play time per game): a homebrew (a forwarder on the HOME
menu, PlayGuard's own or another) counts in the play time as one total, the
homebrews and forwarders as numbers in the games document, and the one in
front is named in *Now playing*, game or homebrew. **The serial number** is only published once *Remote access › What is published › Publish
the serial number* is on (off by default): it then goes to the broker, and
never into a report. On a console whose PRODINFO is blanked, it is the real
one, from Atmosphère's backup of it on the SD card. Its entity, and the
Wi-Fi network's name, are off in Home Assistant until you turn them on.

The names follow Home Assistant's own *Nintendo Switch Parental Controls*
integration where they mean the same thing. The entities that change the play
timer only appear once *Orders may change the play timer* is on (below), and
go again when it is turned off.

The play log's figures (time per game today, the last 7 days, the finished
days) are also on the broker (`playguard/<id>/activity`, `…/week`,
`…/activity/<date>`), for your own templates or the integration below.

## The PlayGuard integration (optional)

The [PlayGuard integration](https://github.com/JigSawFr/playguard-ha), a HACS
custom repository, replaces the console's own MQTT device with the same
entities and adds the game names, the week's play time, long-term statistics,
actions and repairs. It needs [HACS](https://hacs.xyz/) and Home Assistant
2026.10 or later.

[![Open your Home Assistant instance and open a repository inside the Home Assistant Community Store.](https://my.home-assistant.io/badges/hacs_repository.svg)](https://my.home-assistant.io/redirect/hacs_repository/?owner=JigSawFr&repository=playguard-ha&category=integration)
[![Open your Home Assistant instance and start setting up a new integration.](https://my.home-assistant.io/badges/config_flow_start.svg)](https://my.home-assistant.io/redirect/config_flow_start/?domain=playguard)

From the console: **Remote access › Home Assistant › Install the Home Assistant
integration** shows the same two links as QR codes, for a phone where Home
Assistant opens (My Home Assistant asks once for its address). The first opens
the repository in HACS: *Download*, then restart Home Assistant; the console
then shows up by itself in *Settings › Devices & services*. The second adds it
by hand when it does not, with the console id shown under it.

Once installed, the integration tells the console which version runs (retained
on `playguard/<id>/integration`, at each start of Home Assistant): the same
cell then reads *PlayGuard integration* with that version. PlayGuard checks
the integration's latest release with its own updates (*Check for updates
every day*, or *Check for updates* in About); a newer one is in the
notification bell and on that cell (*0.1.0 · 0.2.0 available*). *What's new*
shows what changed since the installed version and the QR code that opens the
integration in HACS: *Update*, then restart Home Assistant. Nothing goes
online for it before the integration is installed.

## Orders

**Orders received** sets what PlayGuard does with an order:

- **Ask on the console** (the default): the console shows the order and asks
  before carrying it out. Nobody at the console, nothing happens; the order
  waits.
- **Carry them out at once**: no question, and no PIN prompt for the order
  (the broker already checked who sent it).
- **Refuse them**: Home Assistant only watches.

**Orders may change the play timer** is off by default. Without it, the link
can watch, lock the console again and change the restrictions, but not the
limits, bedtime or extra time. With it, an order goes exactly like a change
made on the console: when the timer counts down, PlayGuard unlocks parental
controls for a moment with the stored PIN, writes, and locks again.

With *Security › Pause parental controls while I'm in PlayGuard* on, parental
controls are paused while you are in PlayGuard: Home Assistant still sees the
console as it is for the child. *Unlocked* turned on then keeps that unlock
once PlayGuard is left; *Turn parental controls back on* ends the pause, as on
the console.

Every order is in the change history (*History › Changes made with PlayGuard*, "Remote
link"). Some things are **never** possible through the link: deleting
parental controls, unlinking the phone app, and reading, setting or changing
the PIN. Read-only mode (a firmware PlayGuard does not know yet) refuses every
order.

## Examples

A button on a dashboard that gives 30 minutes more:

```yaml
type: button
name: +30 min
icon: mdi:timer-plus-outline
tap_action:
  action: perform-action
  perform_action: button.press
  target:
    entity_id: button.living_room_extra_time_30_min
```

The weekend limits on Friday evening:

```yaml
alias: Weekend limits
triggers:
  - trigger: time
    at: "18:00:00"
conditions:
  - condition: time
    weekday: [fri]
actions:
  - action: number.set_value
    target:
      entity_id: [number.living_room_limit_saturday, number.living_room_limit_sunday]
    data:
      value: 180
```

A notification when an order is refused:

```yaml
alias: PlayGuard order refused
triggers:
  - trigger: state
    entity_id: event.living_room_events
conditions:
  - condition: template
    value_template: "{{ trigger.to_state.attributes.event_type == 'command_rejected' }}"
actions:
  - action: notify.notify
    data:
      message: "PlayGuard refused {{ trigger.to_state.attributes.entity }}: {{ trigger.to_state.attributes.reason }}"
```

A notification 10 minutes before the end of today's limit, with a button
that adds a quarter of an hour (the agent sends `time_warning` while a game
is played; *Orders may change the play timer* on for the button):

```yaml
alias: PlayGuard 10 minutes left
triggers:
  - trigger: state
    entity_id: event.living_room_events
conditions:
  - condition: template
    value_template: "{{ trigger.to_state.attributes.event_type == 'time_warning' }}"
actions:
  - action: notify.mobile_app_my_phone
    data:
      message: "{{ trigger.to_state.attributes.minutes_left }} minutes of play left"
      data:
        actions:
          - action: PLAYGUARD_EXTRA_15
            title: +15 min
  - wait_for_trigger:
      - trigger: event
        event_type: mobile_app_notification_action
        event_data:
          action: PLAYGUARD_EXTRA_15
    timeout: "00:10:00"
    continue_on_timeout: false
  - action: button.press
    target:
      entity_id: button.living_room_extra_time_15_min
```

Entity ids depend on the console's name; check them on the device's page.

## Security

- Give PlayGuard **a broker user of its own**. With a plain Mosquitto, an
  access list can keep it to its topics:

  ```
  user playguard
  topic readwrite playguard/#
  topic readwrite homeassistant/device/#
  topic read homeassistant/status
  ```

- Anyone who can publish on `playguard/<id>/…/set` can send orders. Keep the
  broker on your local network, and leave *Orders may change the play timer*
  off unless you need it.
- The settings, password included, are in `sd:/config/playguard/sync.conf`,
  in plain text. That file is never in a diagnostic report or an online
  upload; the report's *Remote link* section only has the switches and the
  counters.
- **TLS**: port 8883 and *Encrypted connection*. The console checks the
  broker's certificate against its own certificate store; for a private
  certificate authority, put its PEM file on the SD card and set
  `ca_file=/config/playguard/sync/ca.pem` in `sync.conf`. TLS has not been
  tried on a console yet.

- **sysMMC and emuMMC**: both use the same `sync.conf` (set up once), but
  each has its own parental controls, so each is a device of its own in Home
  Assistant: the emuMMC's id differs from the console id by its first digit
  (*Remote access › Console id* shows the one in use) and its name ends in
  *(emuMMC)*. An order reaches only the system it was sent to, when that
  system next runs.

## When it does not connect

*Remote access › Status* says why, and *Log* shows the last lines:

| Status | What to check |
|---|---|
| *Incomplete: the broker's address is missing* | Fill in *Broker address*. |
| *Not connected: cannot find …* | The name does not resolve: use the IP address. |
| *Not connected: connect: Connection refused* / *no answer from the broker* | Wrong address or port, the broker is stopped, or a firewall is in the way. |
| *Not connected: the broker refused the connection: bad user name or password* | The user name or password. |
| *Not connected: the broker refused a subscription* | The user's access list (above). |
| *Not connected: the broker does not speak this MQTT version* | *MQTT version* is set to one the broker does not take: set it back to *Automatic*. |
| *Not connected: the broker does not keep retained messages* | The broker (5.0) says it does not keep them; the link needs them: turn them on in its settings. |

The console retries on its own: after 5 seconds, then twice as long each time,
up to 5 minutes. *Sync now* publishes everything again at once.

## Under the hood

The topics, documents, orders and their bounds, and every key of `sync.conf`
are in [`sync-protocol.md`](sync-protocol.md); the design and its reasons in
[`sync-design.md`](sync-design.md).

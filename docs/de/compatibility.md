<!-- translated from docs/compatibility.md, sha256 74e7e81f48cb -->
# Kompatibilität

Unterstützte Firmware, Atmosphère und Launcher, emuMMC und sysMMC, und wie PlayGuard sich mit einem Ersatz-Sysmodul vergleicht. Zurück zum [README](../../README.md).

## Unterstützte Versionen

**Auf einer Konsole getestet** — PlayGuard lief auf diesen:

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira und der Homebrew App Store

**Sollte funktionieren, ungetestet** — nichts in PlayGuard schließt diese aus, aber noch niemand hat sie gemeldet ([Berichte willkommen](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Jede Firmware braucht ein Atmosphère, das sie startet: das angegebene oder ein neueres.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Dieselben Befehle wie 23.0.1 in der Tabelle von [switchbrew](https://switchbrew.org/wiki/Parental_Control_services) (auf Englisch).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Alle Funktionen.

**Vor 21.0.0 alles außer dem Timer** — Einschränkungen, Geheimzahl, vorübergehendes Aufheben, Aktivität, Netzwerkuhr, Smart-Geräte-App und Verlauf funktionieren. Der Einstellungsblock des Timers (0x44 Bytes) hat diesen Aufbau erst seit 21.0.0, daher schaltet PlayGuard aus, was darin liegt: die Tageslimits, den Gute-Nacht-Alarm, die Zusatzzeit und *Alle Spiele sperren*.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x und älter** — Atmosphère 1.0.0 und älter

PlayGuard prüft, dass Atmosphère läuft, nicht welche Version: Was zählt, ist die Firmware, die es startet. *Konsole* zeigt beides.

## Gut zu wissen

### Launcher

Empfohlen ist das Starten über ein Spiel (Title Override). Die App sagt, ob sie als Anwendung (über einem Spiel oder über ihren eigenen Forwarder im HOME-Menü) oder als Applet (Album) läuft. Über einem Spiel zählt die Konsole die Zeit in PlayGuard als die dieses Spiels. In der Aktivität geht sie an den Benutzer, der beim Start gewählt wurde: Öffne es mit dem Benutzer eines Elternteils, nicht mit dem eines Kindes. Der Timer ist der der Konsole, für jeden Benutzer derselbe: Er zählt diese Zeit, egal wer PlayGuard geöffnet hat, außer der Timer ist aus oder die Geheimzahl hat den Jugendschutz pausiert ([Details](features.md#die-geheimzahl-pausiert-den-jugendschutz-solange-du-in-playguard-bist)).

### Nintendo Switch 2

Nicht unterstützt: Für sie gibt es kein Atmosphère.

### Neuere Firmware

Oberhalb der neuesten auf einer Konsole getesteten Firmware öffnet PlayGuard **schreibgeschützt** und prüft, ob eine neuere Version sie unterstützt. Wenn ja, bietet es ein Update an (in PlayGuard selbst, über sphaira oder den Homebrew App Store). Sonst wählst du: schreibgeschützt, schreibgeschützt mit den Entwicklerwerkzeugen (um die Firmware zu untersuchen) oder alle Funktionen auf eigene Gefahr. Die Wahl kann für diese Firmware und App-Version gemerkt werden; *Konsole › Kompatibilität* bringt den Bildschirm zurück.

### emuMMC und sysMMC

Jede hat ihren eigenen Jugendschutz, und PlayGuard sieht nur den des Systems, auf dem es läuft. Auf emuMMC sagen *Konsole* und *Erste Schritte* das; auf sysMMC ebenfalls, wenn eine emuMMC auf der SD-Karte eingerichtet ist. Stelle den Jugendschutz auf beiden ein oder blende das Bootmenü von hekate aus, damit ein Kind nicht das andere System wählen kann. PlayGuard hält Verlauf, Sicherungen und Zustand jedes Systems getrennt (die sysMMC und die emuMMC, in `nand/`); die Einstellungen und Profile sind gemeinsam.

### Kann die Konsole dadurch gesperrt werden?

PlayGuard kontaktiert nie Nintendo: Online erreicht es nur öffentliche Zeitserver, GitHub (Updates und die Berichte, die du sendest) und switchbrew (die neueste Firmware und ihre Versionshinweise). Gesperrt werden Konsolen, deren Custom Firmware oder emuMMC sich mit Nintendo verbindet ([switchbrew-FAQ](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), auf Englisch). Eine Sperre wegen Änderungen am Jugendschutz oder an der Uhr ist nicht dokumentiert; eine Garantie ist das nicht.

### Eine Konsole ohne Internet

Genau dafür ist PlayGuard da, und der Timer braucht trotzdem eine Netzwerkuhr, die gestellt wurde. Ohne Zeitserver, den es fragen kann, bietet *Konsole › Netzwerkuhr › Zeitabweichung messen* an, sie stattdessen auf die Konsolenuhr zu stellen: Prüfe zuerst die Konsolenuhr, mit *Uhr mithilfe des Internets synchronisieren* aus, und schalte diese Einstellung erst danach ein (siehe [Netzwerkuhr](features.md#kein-server-antwortet)).

### Benachrichtigungen auf dem Handy

Seit 22.0.0 kann eine Konsole, die noch mit der Smart-Geräte-App verknüpft ist, den Elternteil benachrichtigen, wenn die Geheimzahl an der Konsole eingegeben wird ([switchbrew](https://switchbrew.org/wiki/22.0.0), auf Englisch). Ob das Aufheben durch PlayGuard das auslöst, ist unbekannt.

## Warum PlayGuard

Kinder lieben Videospiele, und Videospiele bedeuten Bildschirme: Ein Limit gehört dazu, wenn man sich um sie kümmert. Auf einer unveränderten Switch hat sich die App von Nintendo darum gekümmert. Sobald die Konsole eine Custom Firmware nutzt, öffnet sich eine ganz neue Welt — und der Jugendschutz fällt weg, weil die App die Konsole nicht mehr erreicht. Zu wissen, wie lange sie gespielt haben, ein Limit festzulegen, das Spiel ohne Streit und ohne ein weiteres „noch fünf Minuten“ zu beenden, wurde schwierig.

Die wenigen Werkzeuge, die es gab, erfüllten den Zweck nicht: Aktivitätsberichte, die kaum gepflegt und wenig detailliert waren, ein Jugendschutz, der grob und unfertig war. Und ich wollte auch keinen selbstgebauten Ersatz und kein Sysmodul, das ständig im Hintergrund läuft. Die Idee war, **den Jugendschutz der Konsole so weit wie möglich wiederzuverwenden**, mit allem, was er schon kann — das Limit, die Geheimzahl, die Warnungen, die Unterbrechung — und seine Einstellungen zurück auf die Konsole zu bringen. Das ist PlayGuard.

Die Jugendschutzfunktionen der Konsole selbst zu steuern, öffnet auch die Tür zu viel mehr: die Spielzeit an einen Server zu Hause melden, Befehle von ihm annehmen, Home Assistant, Automatisierungen. Der erste Schritt ist da, eine optionale Verbindung zu Home Assistant über MQTT ([Anleitung](../home-assistant.md), auf Englisch); wohin es als Nächstes gehen könnte, steht im [Horizont der Roadmap](../../ROADMAP.md#horizon) (auf Englisch).

## PlayGuard oder ein Ersatz-Sysmodul?

PlayGuard steuert den in die Konsole eingebauten Jugendschutz (den Dienst `pctl`), offline. Der andere Ansatz ersetzt ihn durch ein Sysmodul, zum Beispiel [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (ein Sysmodul und ein Tesla- / Ultrahand-Overlay).

| | PlayGuard | Ein Ersatz-Sysmodul |
|---|---|---|
| **Spielzeitlimit** | Eines für die Konsole (der eigene Timer der Konsole) | Eines pro Benutzerkonto |
| **Geheimzahl, Warnungen, Unterbrechung** | Die der Konsole | Seine eigenen |
| **Im Hintergrund** | Nichts (das optionale Wiederherstellungs-Sysmodul wirkt nur beim Start, wenn eine Datei `RESCUE` es verlangt; der Agent der Verbindung zu Home Assistant, ebenfalls optional, läuft nur, wenn er installiert ist) | Ein Sysmodul, ab dem Start |
| **Nach einem Firmware-Update** | Der Jugendschutz der Konsole funktioniert weiter; PlayGuard öffnet schreibgeschützt, bis eine Version die Firmware unterstützt | Der Jugendschutz hängt davon ab, dass das Sysmodul weiter funktioniert |
| **Von der SD-Karte gelöscht** | Die Limits bleiben auf der Konsole | Der Jugendschutz verschwindet mit ihm |

**Brauchst du für jedes Kind ein anderes Limit?** Wähle ein Ersatz-Sysmodul — der Timer der Konsole hat ein Limit für die ganze Konsole. Sonst behält PlayGuard den Jugendschutz von Nintendo und bringt nur seine Einstellungen auf die Konsole.

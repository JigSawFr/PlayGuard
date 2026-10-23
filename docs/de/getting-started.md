<!-- translated from docs/getting-started.md, sha256 d7c8cb8b1c87 -->
# Einstieg

Von der SD-Karte zu einer Konsole, die für dein Kind eingerichtet ist, in etwa zehn Minuten, dann die alltäglichen Aufgaben. Zurück zum [README](../../README.md).

## Bevor du anfängst

- **Eine Konsole mit Atmosphère.** PlayGuard steuert den eigenen Jugendschutz der Konsole über einen eingeschränkten Dienst: Es braucht eine Custom Firmware. Nicht modifiziert? Dann kann es nicht helfen ([Fehlerbehebung](troubleshooting.md#konsole-nicht-modifiziert)).
- **Eine unterstützte Firmware.** 21.0.0 bis 23.0.1 für alle Funktionen; ältere: alles außer dem Timer; eine neuere öffnet schreibgeschützt, bis eine Version sie unterstützt ([Kompatibilität](compatibility.md)).
- **Ein Limit für die ganze Konsole.** Der Timer ist der der Konsole: Er ist für jeden Benutzer derselbe. Unterschiedliche Limits pro Kind brauchen ein anderes Werkzeug ([warum](compatibility.md#playguard-oder-ein-ersatz-sysmodul)).

Willst du dich erst einmal umsehen? [Probiere PlayGuard im Browser aus](https://playguard.link/simulator/?not_set_up=1), mit einer simulierten Konsole bei ihrem ersten Start.

## 1. Installieren

Entpacke `playguard.zip` aus der [neuesten Version](https://github.com/JigSawFr/PlayGuard/releases/latest) ins Hauptverzeichnis der SD-Karte ([Installation](install.md)). PlayGuard ist noch nicht im Homebrew App Store oder in sphaira: Sein Eintrag wartet noch auf die Freigabe.

> [!TIP]
> Entpacke auch `playguard-rescue.zip`, das optionale Wiederherstellungs-Sysmodul. Es tut nichts, bis du es brauchst, und es ist der einzige Weg zurück, wenn ein Limit von 0 Minuten alles sperrt und die Geheimzahl vergessen ist ([Ausgesperrt?](troubleshooting.md#ausgesperrt)).

## 2. Richtig öffnen

Öffne PlayGuard **über ein Spiel** (halte R gedrückt, während du ein Spiel startest, mit hbmenu, sphaira oder dem Homebrew App Store), mit **dem Benutzer eines Elternteils**:

- über einem Spiel läuft PlayGuard als Anwendung, mit seinem ganzen Arbeitsspeicher; aus dem Album läuft es als Applet, mit weniger;
- die Konsole zählt die Zeit in PlayGuard als die dieses Spiels und schreibt sie dem Benutzer zu, der beim Start gewählt wurde: dem eines Elternteils, nicht dem eines Kindes.

## 3. *Erste Schritte* folgen

Beim ersten Start erscheint die Anleitung *Erste Schritte* (später: *Optionen › Erste Schritte*). *Heute* zeigt dieselben Schritte als Karte, bis sie erledigt sind (*1 von 3*):

1. **Die Geheimzahl.** Die eigene Geheimzahl der Konsole, die das HOME-Menü abfragt, um ein Spiel über das Limit hinaus zu starten oder die Einstellungen zu ändern. Vermeide 0000, 1234 oder 2580: PlayGuard meldet eine Geheimzahl, die jeder zuerst ausprobieren würde.
2. **Ein Tageslimit.** Jeden Tag gleich, oder eines für die Wochentage und eines für das Wochenende. Ändere jeden einzelnen Tag später in *Spielzeit*.
3. **Die Netzwerkuhr.** Der Timer zählt mit ihr. Eine Konsole, die nie Nintendos Server erreicht, hält sie ungenau: siehe Schritt 5.

## 4. Die Smart-Geräte-App trennen

Noch mit der App „Nintendo Switch-Altersbeschränkungen“ verknüpft (oder eine gebrauchte Konsole, die mit der App von jemand anderem verknüpft ist)? *Sicherheit › Smart-Geräte-App trennen*. Sonst überschreibt die nächste Synchronisierung der App, was du in PlayGuard festgelegt hast. Es funktioniert offline.

## 5. Die Netzwerkuhr stellen

*Konsole › Netzwerkuhr › Zeitabweichung messen* fragt drei öffentliche Zeitserver, dann *Netzwerkuhr stellen*. Das Stellen der Uhr lässt die heutige Spielzeit neu beginnen, also mach es, bevor das Kind spielt.

Eine Konsole ohne Internet: Wenn kein Server antwortet, bietet PlayGuard an, die Netzwerkuhr stattdessen auf die Konsolenuhr zu stellen. Vergleiche die Konsolenuhr vorher mit einem Handy oder einer Uhr, mit *Systemeinstellungen › Konsole › Datum und Uhrzeit › Uhr mithilfe des Internets synchronisieren* aus ([Details](features.md#netzwerkuhr)).

## 6. Wählen, was passiert, wenn die Zeit um ist

*Spielzeit › Wenn die Zeit um ist*: Die Konsole zeigt „Zeit ist um“ an und **unterbricht dann das Spiel** oder **warnt nur**. Das Unterbrechen beendet das Spielen ohne Streit, aber nicht gespeicherter Fortschritt kann verloren gehen; das Warnen überlässt es dem Kind. Füge im selben Tab einen **Gute-Nacht-Alarm** hinzu, wenn du einen willst.

## 7. PlayGuard selbst schützen

*Sicherheit › PlayGuard mit der Geheimzahl schützen*:

- **Vor einer Änderung** (der Standard) — jeder kann schauen, nur der Elternteil ändert etwas;
- **Beim Öffnen von PlayGuard** — ein Sperrbildschirm beim Start;
- **Nie** — PlayGuard meldet das in der Glocke, solange eine Geheimzahl festgelegt ist.

## 8. Die Konsole übergeben

*Heute › Dem Kind zeigen* zeigt nur die verbleibende Zeit und wann das Spielen endet, groß genug, um quer durch den Raum lesbar zu sein. Ⓐ und die Geheimzahl bringen die Bildschirme der Eltern zurück.

> [!IMPORTANT]
> **Ein im HOME-Menü offen gelassenes Spiel zählt weiter.** Schließe es oder versetze die Konsole in den Standby-Modus ([was zählt](features.md#was-als-spielzeit-zählt)).

## Alltägliche Aufgaben

| Um… | Wo |
|---|---|
| Heute mehr Zeit zu geben | *Heute › Zusatzzeit* (+15 min, +30 min, +1 h). Sie wird am nächsten Tag von selbst zurückgenommen. |
| An einem anderen Tag mehr Zeit zu geben | *Heute › Zusatzzeit › Ein anderer Tag…* |
| Für heute aufzuhören | *Heute › Für heute beenden*: jetzt, oder in 5 Minuten, um zu speichern. |
| Das Limit eines Tages zu ändern | *Spielzeit*: Ⓐ auf dem Tag, ↑/↓, dann **+** zum Speichern. |
| Zwischen Schulzeit und Ferien zu wechseln | *Spielzeit › Profile*; *Geplantes Profil* für einen im Voraus gewählten Zeitraum. |
| Zu sehen, wer was gespielt hat | *Aktivität*: pro Spiel, pro Benutzerkonto, heute, 7 Tage, insgesamt; Export oder ein QR-Code. |
| Vorerst alle Spiele zu sperren | *Spielzeit › Alle Spiele sperren*. Ausgeschaltet kommen die vorherigen Limits zurück. |
| Eine Änderung rückgängig zu machen | **ZR** *Rückgängig* direkt danach, oder jederzeit *Verlauf › Änderungen durch PlayGuard*. |
| Eine Kopie der Einstellungen zu behalten | *Verlauf › Einstellungen sichern* (nie die Geheimzahl). |
| Zu wissen, was dich braucht | Die Glocke (Ⓨ): was zu tun ist in Bernstein, was zu wissen ist mit einem „i“. |

## Weiter

- Jeder Tab und jede Einstellung: [Funktionen](features.md).
- Etwas funktioniert nicht wie erwartet: [Fehlerbehebung](troubleshooting.md#häufige-probleme).
- Was die Konsole tut, und wie sicher wir uns dessen sind: [wie der Jugendschutz funktioniert](../parental-controls.md) (auf Englisch).

<!-- translated from docs/troubleshooting.md, sha256 498fbfed802c -->
# Fehlerbehebung

Wieder Zugang zu einer gesperrten Konsole bekommen, die häufigen Probleme, einen Fehler melden, und der Entwicklermodus. Zurück zum [README](../../README.md).

## Ausgesperrt?

PlayGuard sieht nur den Jugendschutz des Systems, auf dem es läuft: **emuMMC und sysMMC haben jeweils ihren eigenen** (eine auf der einen entfernte Geheimzahl ist auf der anderen noch da). Starte es auf jeder, die korrigiert werden muss.

### Gebrauchte Konsole

Du kennst die Geheimzahl, aber die Smart-Geräte-App des Vorbesitzers ist noch verknüpft (das Trennen schlägt fehl, oder ein Zurücksetzen auf Werkseinstellungen verlangt dessen Konto).

*Geheimzahl › Smart-Geräte-App trennen*, dann, wenn du gar keinen Jugendschutz mehr willst, *Alle Altersbeschränkungen löschen*. Beides funktioniert offline, auf emuMMC wie auf sysMMC.

### Geheimzahl vergessen

*Geheimzahl anzeigen* fragt nach der Geheimzahl selbst, kann hier also nicht helfen. Mit installiertem Wiederherstellungs-Sysmodul (siehe unten) legst du eine Datei `RESCUE` ab und startest neu: Der Wiederherstellungsbildschirm legt dann eine neue fest (die alte zeigt er nie). Ohne das Sysmodul fängt *Alle Altersbeschränkungen löschen* von vorn an (vorher wird eine Sicherung der Einstellungen gespeichert; sie enthält nie die Geheimzahl): Ist *PlayGuard mit der Geheimzahl schützen* an (der Standard, *Vor einer Änderung*), stecke die SD-Karte zuerst in einen Computer und setze `"pin_lock"` in `sd:/config/playguard/config.json` auf `"off"`.

### Der Timer sperrt alles (Limit von 0 Minuten) und die Geheimzahl ist vergessen

PlayGuard selbst kann dann nicht starten. Installiere das optionale Wiederherstellungs-Sysmodul (`playguard-rescue.zip`) **vorher**; bist du ausgesperrt, lege eine leere Datei `config/playguard/RESCUE` auf die SD-Karte und starte die Konsole — es hebt die Sperre der Konsole auf, damit PlayGuard öffnen kann. Siehe [`sysmodule/README.md`](../../sysmodule/README.md) (auf Englisch).

Das Sysmodul muss der Version von PlayGuard folgen: Entpacke bei jedem Update von PlayGuard die `playguard-rescue.zip` dieser Version erneut (oder *Aktualisieren* auf *Das Wiederherstellungs-Sysmodul ist veraltet* in der Benachrichtigungsglocke) und starte die Konsole dann neu. Ein älteres Sysmodul hinterlässt einen Bericht, den PlayGuard nicht prüfen kann: Der Wiederherstellungsbildschirm bietet dann nichts an, sagt, dass das Sysmodul älter ist, und ein erneutes `RESCUE` ergibt dasselbe, bis es aktualisiert ist.

### Homebrew-Menü gesperrt

*Konsole › Forwarder › Homebrew-Menü sperren* verhindert, dass das Album und ein mit R gestartetes Spiel das Homebrew-Menü öffnen. Wenn der Forwarder von PlayGuard und das geschützte Menü beide weg sind (in den Systemeinstellungen gelöscht, oder auf dem anderen System: Die Sperre liegt auf der SD-Karte und wird von sysMMC und emuMMC gelesen):

- **Von einem Computer aus:** Lösche `atmosphere/config/override_config.ini` auf der SD-Karte (oder lege `config/playguard/override_config.ini.backup` zurück, die Datei, wie sie war) und starte die Konsole dann neu.
- **Mit installiertem Rettungsmodul** (optional): Lege eine leere Datei `config/playguard/RESCUE` auf die SD-Karte und starte neu; das Modul hebt die Sperre beim Start auf, und PlayGuard sagt es bei seinem nächsten Start. Ein Update von Atmosphère seit dem Sperren hebt sie auf dieselbe Weise auf.

### Konsole nicht modifiziert

PlayGuard kann nicht helfen: Es braucht Atmosphère. Der offizielle Weg ist das Masterschlüssel-Verfahren des Nintendo-Supports.

## Häufige Probleme

### Die verbleibende Zeit sinkt nicht

*Heute* sagt *Die Konsole zählt die Spielzeit gerade nicht*, wenn sie sich etwa 90 s lang nicht bewegt hat. Die wahrscheinlichen Ursachen:

- **Die Netzwerkuhr wurde nie gestellt.** *Konsole › Netzwerkuhr › Zeitabweichung messen*, dann stelle sie ([Netzwerkuhr](features.md#netzwerkuhr)).
- **Die Uhr wurde zurückgestellt.** Nach einem Sprung zurück hält die Zählung an, bis die Uhr die alte Uhrzeit überschreitet; nichts zu tun.
- **Ein Limit wurde gerade geschrieben.** Starte ein Spiel für eine Minute, dann prüfe erneut.
- **Der Jugendschutz ist vorübergehend aufgehoben.** Die Spielzeit wird dann möglicherweise nicht gezählt: *Geheimzahl › Altersbeschränkungen wieder aktivieren*.

### Meine Limits haben sich von selbst geändert

- **Die Smart-Geräte-App ist noch verknüpft.** Ihre nächste Synchronisierung überschreibt, was PlayGuard festgelegt hat: *Geheimzahl › Smart-Geräte-App trennen*.
- **Ein anderes Werkzeug, oder ein Profil.** *Heute* zeigt eine bernsteinfarbene Karte für Limits, die von denen abweichen, die PlayGuard zuletzt gesehen hat, und *Optionen › Änderungsverlauf* listet jede außerhalb von PlayGuard bemerkte Änderung auf. Ⓐ auf einer stellt den vorherigen Wert wieder her.
- **Ein geplantes Profil**, das angewendet wurde, oder dessen ersetzte Limits nach Ende seines Zeitraums wiederhergestellt wurden (*Spielzeit › Profile › Geplantes Profil*).

### Das Kind hat über das Limit hinaus gespielt

- **Die Konsole warnt nur.** *Spielzeit › Wenn die Zeit um ist*: Wähle *Software unterbrechen*.
- **Der Jugendschutz blieb aufgehoben.** Solange er es ist, ist das heutige Limit ausgesetzt: Die Kapsel in der Kopfzeile sagt das in jedem Tab; *Geheimzahl › Altersbeschränkungen wieder aktivieren*. Ein Neustart der Konsole aktiviert ihn ebenfalls wieder.
- **Die Uhr wurde zurückgestellt.** Die Zählung hält an, bis die Uhr die alte Uhrzeit überschreitet ([was zählt](features.md#was-als-spielzeit-zählt)).
- **Das andere System.** emuMMC und sysMMC haben jeweils ihren eigenen Jugendschutz: Stelle ihn auf beiden ein oder blende das Bootmenü von hekate aus ([Kompatibilität](compatibility.md#emummc-und-sysmmc)).

### Heute steht wieder das ganze Limit zur Verfügung

Das Ändern der Uhr (der Netzwerkuhr oder der Konsolenuhr) lässt die heutige Spielzeit neu beginnen. Stelle die Uhr, bevor das Kind spielt, nicht danach.

### PlayGuard öffnet schreibgeschützt

Auf der Konsole läuft eine Firmware, die neuer ist als die neueste getestete: PlayGuard prüft, ob eine neuere Version sie unterstützt, und bietet ein Update an ([neuere Firmware](compatibility.md#neuere-firmware)). Ein durchgestrichener Stift rechts vom Titel zeigt, dass die App schreibgeschützt ist; ein Tippen darauf öffnet den Firmware-Bildschirm.

### PlayGuard fragt ständig nach der Geheimzahl, oder nie

*Geheimzahl › PlayGuard mit der Geheimzahl schützen*: *Vor einer Änderung* (der Standard) fragt einmal, vor der ersten Änderung, und die Geheimzahl gilt, bis PlayGuard geschlossen wird. *Geheimzahl nach 5 Minuten erneut abfragen* fragt danach erneut. *Geheimzahl anzeigen* fragt immer.

### Ein Spiel startet nicht, oder zeigt ein „!“ im HOME-Menü

*Konsole › Installierte Spiele prüfen* sagt, was bei jedem nicht stimmt und was zu tun ist; Spielstände bleiben immer erhalten ([installierte Spiele](features.md#installierte-spiele)).

### Die Glocke oder ein Tab zeigt ein bernsteinfarbenes Abzeichen

Etwas braucht dich: Ⓨ öffnet die Liste, und Ⓐ auf einem Eintrag führt die Lösung aus oder öffnet, wo sie gemacht wird ([Benachrichtigungen](features.md#benachrichtigungen--was-dich-braucht-aus-jedem-tab)).

## Einen Fehler melden

Ein Fehler zeigt den Code des Systems (etwa `0x0001088E`) mit dem, was über ihn bekannt ist: was zu tun ist bei den Codes, deren Bedeutung dokumentiert ist, sonst hilft ein Bericht.

1. *Info › Hilfe & Diagnose › Diagnosebericht exportieren* speichert eine Textdatei in `sd:/config/playguard/logs/reports/`: Firmware, Atmosphère-Version, wie lange der Start und die langsamen Bildschirme gedauert haben (der Änderungsverlauf, *Alle Spiele*, die Prüfung der installierten Spiele, die Tabs, die sich selbst aktualisieren), Uhren, Speicher, Status der Anonymisierung der Seriennummer und der Spiel-Patches, das rohe Ergebnis jeder Abfrage des Jugendschutzes, und die Protokolle: wie der letzte Lauf endete, der verbleibende Arbeitsspeicher und Platz auf der SD-Karte, die neuesten Absturzdateien und die letzten 80 protokollierten Zeilen. **Er enthält nie die Geheimzahl oder die Seriennummer.**
2. [Eröffne ein Issue](https://github.com/JigSawFr/PlayGuard/issues/new) (auf Englisch oder Französisch) und hänge ihn an.

### Einen Bericht online senden

Mit der Konsole online sendet *Info › Hilfe & Diagnose › Bericht online senden* (Ⓧ auf dem Diagnosebericht im Entwicklermodus) ihn an einen geheimen Gist in deinem GitHub-Konto, nachdem es genau gesagt hat, was gesendet wird. Ein früher in `logs/reports/` gespeicherter Bericht kann auf dieselbe Weise gesendet werden.

- **Was gesendet wird:** der Bericht und jede Datei als eigene Datei:
  - die neuesten Absturzberichte: die letzten 3 von PlayGuard in `logs/crashes/` und die letzten 2 von Atmosphère zu einem Homebrew der vergangenen Woche, in `sd:/atmosphere/crash_reports/` (der Absturz eines anderen Homebrews kann darunter sein: Die vor dem Senden gezeigte Liste sagt, welcher);
  - die Protokolle dieses Laufs und des vorherigen (`logs/sessions/`; bei einem langen: sein Anfang und seine letzten 64 KiB);
  - die Debug-Dateien: die Referenz des Spielzeit-Blocks, das Ende der Spielzeit-Aufzeichnung, solange *Spielzeit aufzeichnen* an ist, die letzten 20 Änderungen des Änderungsverlaufs (ohne Namen von Spielen, Benutzern oder Profilen, und nur mit ihrem Datum) und die Einstellungen von PlayGuard.
- **Anmelden:** Bist du abgemeldet, bietet PlayGuard zuerst die Anmeldung an; *GitHub-Konto*, direkt darunter, sobald du angemeldet bist, meldet dich ab; lehnt GitHub den Gist ab, bietet PlayGuard eine erneute Anmeldung an.
- **Danach:** Es zeigt den Link und zwei QR-Codes: den Bericht und das Formular für Fehlerberichte mit dem Link, deinen Versionen, wie PlayGuard gestartet wurde (über ein Spiel oder aus dem Album; füge das Menü hinzu) und dem System (sysMMC oder emuMMC) schon ausgefüllt. Jeder Link wird in `logs/uploads/` als Internetverknüpfung (`.url`) aufbewahrt, die ein Computer in seinem Browser öffnet.

> [!WARNING]
> Jeder mit dem Link kann den Bericht lesen, bis du den Gist auf GitHub löschst, und ein damit eröffneter Fehlerbericht ist öffentlich: Die Bestätigung sagt das.

### Was PlayGuard auf der SD-Karte speichert

Alles liegt in `sd:/config/playguard/logs/`, als einfacher Text, den ein Computer öffnet:

- `sessions/session_<date>.txt` — **eine Datei pro Lauf**: alles, was PlayGuard protokolliert hat (was früher nur über nxlink einen PC erreichte), jede Zeile mit ihrer Uhrzeit und ihrer Stufe (`E` Fehler, `W` Warnung, `I` Information), und eine letzte Zeile, die sagt, wie der Lauf endete: `=== end: clean exit ===` (normal beendet) oder `=== end: crash, see logs/crashes/… ===` (Absturz). Eine Datei wächst ab 1 MiB nicht weiter.
- `crashes/crash_<date>.txt` — **eine Datei pro Absturz**, deren erste Zeile sagt, was passiert ist:
  - eine **CPU-Ausnahme** (ein ungültiger Zeiger, eine undefinierte Anweisung, ein Stapelüberlauf): die Art des Fehlers und die Adresse, auf die zugegriffen wurde, der Backtrace und die Register. Adressen sind Offsets in `playguard.nro` (`playguard+0x1a2b3c`): Die Linker-Map des Builds (das Artefakt `-linker-map` seines CI-Laufs) liefert die Funktion. PlayGuard endet dann wie zuvor: der Fehlerbildschirm der Konsole und der eigene Bericht von Atmosphère in `sd:/atmosphere/crash_reports/`;
  - eine **nicht abgefangene Ausnahme**: ihr Typ und ihre Meldung, und der Backtrace;
  - ein **Lauf, der nicht endete**: beim nächsten Start bemerkt, wenn die Sitzungsdatei des letzten Laufs keine Endzeile hat (ein Absturz, den PlayGuard nicht abfangen konnte, ein Schließen über das HOME-Menü, während es hing, oder die ausgeschaltete Konsole), mit den letzten 60 Zeilen dieses Laufs.

  Die Absturzdateien, die erfasst werden, während PlayGuard läuft, enden mit den letzten 80 vor dem Absturz protokollierten Zeilen.
- `reports/`, `uploads/`, `play_timer/` — die gespeicherten Berichte, die Links der online gesendeten Berichte, die Spielzeit-Aufzeichnung.

*Info › Hilfe & Diagnose* legt fest, wie viele Berichte, Absturzdateien, Sitzungsprotokolle und Links aufbewahrt werden (jeder Ordner für sich; die neueste bleibt immer).

## Entwicklermodus (eine neue Firmware untersuchen)

Drücke siebenmal Ⓨ auf *Info › Version*; die Entwicklerwerkzeuge erscheinen am Ende von *Info › Hilfe & Diagnose*. Er fügt hinzu:

- einen Schalter **schreibgeschützt** — ist er an, kann die App nichts ändern: der sichere Weg, eine neue Firmware zu untersuchen (der Firmware-Bildschirm bietet ihn direkt an);
- den Diagnosebericht auf dem Bildschirm (Ⓨ speichert ihn, Ⓧ sendet ihn online) und eine Abkürzung, um ihn aus dem Tab Spielzeit zu exportieren;
- **Anderen Build installieren** an Ort und Stelle, um eine Korrektur zu testen, bevor sie veröffentlicht wird ([unten](#einen-anderen-build-installieren));
- **Spielzeit-Block vergleichen** (nur solange die Smart-Geräte-App verknüpft ist und synchronisieren kann — nicht mit einer Hosts-Datei, die Nintendo blockiert; auch in *Geheimzahl*), um Einstellungen zu entschlüsseln, die PlayGuard noch nicht anzeigt: Speichere den rohen Block als Referenz, ändere eine Einstellung in der Smart-Geräte-App, komm zurück — PlayGuard listet die Werte auf, die sich geändert haben (auf Wunsch in `logs/reports/` gespeichert). Unterschiedliche Schlafenszeiten pro Tag, in der Smart-Geräte-App eingestellt, könnten so bestätigt werden. Hänge diese Datei an ein Issue an oder sende sie mit dem Bericht (*Bericht online senden*).
- **Spielzeit aufzeichnen**: alle 30 s, solange PlayGuard offen ist, eine Zeile mit dem, was der Timer meldet (verbleibende Zeit, gespielte Zeit, der rohe Einstellungsblock…), in `logs/play_timer/log.csv`, einer Datei, die sich in einer Tabellenkalkulation öffnen lässt, dazu eine Zeile direkt nach jeder Änderung von PlayGuard, die sagt, welche (ihre letzte Spalte, `event`), und ein Vermerk, wenn sich die Uhr um mehr bewegt hat als die vergangene Zeit. Über Mitternacht offen gelassen, oder bis die Zeit um ist, zeigt es, was ein einzelner Bericht nicht kann: wann die gespielte Zeit zurückgesetzt wird, was die Konsole kurz vor dem Ende sagt. Der Schalter wird gemerkt; er zeichnet nur im Entwicklermodus auf. Was bisher bekannt ist, steht in [parental-controls.md](../parental-controls.md) (auf Englisch).
- **Cache leeren** und **Protokolldateien löschen**, jeweils nach einer Bestätigung, die sagt, wie viele Dateien und wie viel Platz: Der Cache (`cache/` und das `nand/<system>/cache/` jedes Systems: Spielnamen und -symbole, die letzten Zahlen der Aktivität, die Build-Liste) wird bei Bedarf erneut von der Konsole gelesen; die Protokolldateien sind die Absturzdateien, die Sitzungsprotokolle (außer dem dieses Laufs, der noch geschrieben wird), die gespeicherten Berichte und die Spielzeit-Aufzeichnung, während `uploads/` (die Löschlinks der online gesendeten Berichte) und die Referenz des Blocks bleiben. Einstellungen, Verlauf, Sicherungen, Profile und Exporte werden nie angerührt.
- **Migrationen erneut ausführen**: die Verschiebungen der früheren Anordnungen — der Datenordner neben der `.nro`, die Protokolle vor ihren Unterordnern, was alle Systeme gemeinsam hatten — jede auf einer Zeile zum An- oder Abhaken (anfangs alle angehakt; A schaltet eine um), dann *Die N gewählten ausführen* und eine Bestätigung, die sie mit dem auflistet, was jede verschiebt. Sie laufen sofort, egal was `migrations.json` festhält (sonst läuft jede nur einmal, beim ersten Start eines Builds, der sie braucht; die dort festgehaltene Stufe wird nie gesenkt); eine Datei, die schon an ihrem Platz ist, wird nie ersetzt. Ein Dialog sagt, wie viele Einträge verschoben wurden; jeder Schritt steht im Sitzungsprotokoll.
- **Bildstatistik**: eine Zeile pro Sekunde in `logs/frame_stats.txt`, die Zeit, die das Zeichnen des Bildschirms gebraucht hat, und die Bilder, die zu spät kamen, um auf der Konsole einen Bildschirm mit einem anderen zu vergleichen. **Hintergrundschein** schaltet das sanfte Licht hinter den Bildschirmen aus (der Modus in der Zeile endet dann auf `-noglow`), um zu messen, was es kostet.

### Einen anderen Build installieren

*Anderen Build installieren* setzt einen Build an die Stelle der eigenen `.nro` von PlayGuard, um eine Korrektur zu testen, bevor sie veröffentlicht wird.

**Was es auflistet**, jede Art in einem eigenen Tab (Ⓛ / Ⓡ, zu beiden Seiten der Tabs angezeigt, wechseln von einem zum nächsten; der Bildschirm öffnet beim Tab des installierten Builds, und jeder Tab zählt die Builds, die neuer sind als der installierte):

- eine der letzten 20 Versionen (Vorabversionen eingeschlossen, markiert);
- einer der letzten 20 Commits von `main`;
- der neueste Build eines offenen Pull Requests aus einem Branch dieses Repositorys (nie aus einem Fork: dessen Code wurde noch nicht geprüft). A auf dem Tab der Pull Requests, wenn er angezeigt wird, sortiert sie andersherum (neuester Build zuerst, oder nach Nummer).

**Jeder Build** zeigt seinen Titel (der Betreff eines Commits oder eines Pull Requests; die ganze Nachricht eines Commits ist seine Beschreibung) über Etiketten: die Art der Änderung (*Funktion*, *Fehlerbehebung*, *Doku*…) und ihr Bereich, *Inkompatibel*, der Pull Request, aus dem ein Commit stammt, wie seine Prüfungen endeten (*CI OK*, *CI fehlgeschlagen*, *CI läuft*), Vorabversion, *installiert* oder *neu*; rechts, wie alt er ist, und sein Commit. Ein Build öffnet zuerst seine Seite: Etiketten (seine Art, seine Prüfungen, installiert oder neu), eine Karte mit Fakten (erstellt, Größe, Commit, Prüfsumme, wann GitHub ihn löscht), **Installieren**, **Auf GitHub ansehen** (seine Seite als QR-Code für ein Handy), dann die Hinweise einer Version oder die Beschreibung eines Pull Requests, aus dem Markdown von GitHub dargestellt (Überschriften, Listen, Aufgabenkästchen, Code, Zitate und Hinweisboxen, Tabellen).

**Sicher zu lesen:** Alles, was von GitHub kommt, wird als einfacher Text angezeigt: Kein Link wird geöffnet, kein HTML oder Skript ausgeführt, und unsichtbare Zeichen, die einen Titel tarnen könnten (Rechts-nach-links-Umschaltungen, Zeichen ohne Breite, Steuerzeichen, ungültiges UTF-8), werden entfernt.

**Anmelden.** Die Versionen brauchen nichts; die anderen sind die Artefakte des Build-Workflows, die GitHub nur angemeldeten Benutzern gibt. **GitHub-Konto** meldet dich mit einem Code und einem QR-Code zum Scannen mit einem Handy an. Die einzigen angefragten Berechtigungen sind, die Dateien des Build-Workflows zu lesen (Actions) und Gists zu erstellen, für *Bericht online senden*; ansonsten kann das Token nur lesen, was öffentlich ist. Es wird in `github_token` aufbewahrt, nie mit einem Bericht gesendet, und *GitHub-Konto* meldet dich ab. Angemeldet nimmt Ⓨ (*Suchen*) die Nummer eines Pull Requests (offen oder geschlossen) oder einen Commit (7 bis 40 Hexadezimalziffern) und findet seinen Build, nur aus einem Branch dieses Repositorys.

**Installieren:** PlayGuard lädt den Build herunter, prüft ihn (Größe, die von GitHub erfasste SHA-256, den NRO-Header), setzt ihn an die Stelle seiner eigenen `.nro` und startet damit neu (vorher wird die Geheimzahl abgefragt, wann immer eine festgelegt ist).

- Die Liste wird 10 Minuten lang aufbewahrt und sofort angezeigt, mit ihrem Alter (Ⓧ ruft sie erneut ab); während sie abgerufen wird, zeigt ein Balken den Fortschritt.
- Ein abgebrochener Download (Netzwerk verloren) geht beim nächsten Installieren desselben Builds dort weiter, wo er aufgehört hat.
- Artefakte verfallen nach 90 Tagen.

**Zurückkehren.** Der ersetzte Build wird aufbewahrt (`playguard.nro.previous`): **Zurück zum vorherigen Build** in derselben Liste tauscht sie wieder, nach der Geheimzahl. Stürzt ein von hier (oder über *Aktualisieren mit › In PlayGuard*) installierter Build vor seinem ersten Bildschirm ab, bietet der nächste Start von PlayGuard auf einem einfachen Textbildschirm an, zum vorherigen Build zurückzukehren, bevor irgendetwas anderes geladen wird (A kehrt zurück, B startet ihn trotzdem), ohne die Geheimzahl, da der Bildschirm der Geheimzahl selbst das sein kann, was abstürzt. Dieselbe Liste kehrt jederzeit zu jeder Version zurück.

**Welcher Build läuft:** *Info › Version* zeigt im Entwicklermodus den Commit. Die Kopfzeile jedes Bildschirms zeigt die laufende Version neben dem Titel: die einer Version in einem grauen Etikett (`1.2.0`), die jedes anderen Builds in einem bernsteinfarbenen Etikett, gefolgt von seinem Commit, damit ein Test-Build nie für die Version gehalten wird.

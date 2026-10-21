<!-- translated from docs/features.md, sha256 daaab39fd40b -->
# Funktionen

Was jeder Tab macht, wie PlayGuard sicher schreibt, und was die Konsole als Spielzeit zählt. Neu bei PlayGuard? Beginne mit dem [Einstieg](getting-started.md). Zurück zum [README](../../README.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

Die Screenshots zeigen das dunkle Design, die Voreinstellung von PlayGuard (auf Englisch).

| Heute | Konsole |
|---|---|
| ![Heute](../../images/screenshots/dashboard.png) | ![Konsole](../../images/screenshots/console.png) |
| **Spielzeit** | **Spielzeit › einen Tag bearbeiten** |
| ![Spielzeit](../../images/screenshots/play_timer.png) | ![Einen Tag bearbeiten](../../images/screenshots/per_day.png) |
| **Aktivität** | **Ein Spiel** |
| ![Aktivität](../../images/screenshots/activity.png) | ![Ein Spiel](../../images/screenshots/activity_game.png) |
| **Einschränkungen** | **Konsole › Netzwerkuhr** |
| ![Einschränkungen](../../images/screenshots/restrictions.png) | ![Netzwerkuhr](../../images/screenshots/clock.png) |
| **Geheimzahl** | **Optionen** |
| ![Geheimzahl](../../images/screenshots/security.png) | ![Optionen](../../images/screenshots/preferences.png) |

## Steuerung

| Taste | Was sie tut |
|---|---|
| ↑/↓ | Bewegen |
| Ⓐ | Bestätigen |
| Ⓑ | Zurück oder abbrechen (zweimal in der Seitenleiste zum Beenden) |
| Ⓧ | Aktualisieren (in den Tabs, die die Konsole lesen) |
| Ⓨ | Benachrichtigungen, aus jedem Tab |
| **−** | Sprache und Design, aus jedem Tab (die Fußzeile lässt sie weg; die Glocke, die Sprache und das Design in der Kopfzeile zeigen sie) |
| **+** | Speichert die Limits pro Tag (und die in *Spielzeit* bearbeitete Woche) |
| **ZR** | *Rückgängig*, einige Sekunden lang, nachdem eine Änderung gespeichert wurde |

Die App ist in Tabs gegliedert, wie die Systemeinstellungen; jeder hat unten seinen Abschnitt.

- **Was fehlgeschlagen ist, steht in einem Dialog, was geklappt hat, in einer kurzen Meldung.** Ein Dialog sagt nur, was jetzt für diese Konsole gilt: dass das laufende Spiel unterbrochen würde, nur solange ein Spiel offen ist; dass die Smart-Geräte-App eine Änderung überschreiben könnte, nur solange sie verknüpft ist und synchronisieren kann.
- **Die Kopfzeile** zeigt in jedem Tab an, solange der Jugendschutz vorübergehend aufgehoben ist (ihre Kapsel), und einen durchgestrichenen Stift rechts vom Titel, wenn die App schreibgeschützt ist; ein Tippen auf eines von beiden führt dorthin, wo es rückgängig gemacht wird (*Geheimzahl › Altersbeschränkungen wieder aktivieren*, der Firmware-Bildschirm).
- **Im schreibgeschützten Modus** bleiben die Aktionen an ihrem Platz, ausgegraut, und sagen beim Drücken, warum.

## Das Aussehen: Cadran

Das Aussehen von PlayGuard heißt Cadran: eine schmale Leiste mit Symbolen für die Tabs, eine Palette in Marineblau oder kühlem Weiß (je nach Design), Karten statt Listen. Jeder Tab öffnet mit dem Wichtigsten (ein Ring, Kacheln, Karten, Chips), seine übrigen Einstellungen darunter.

- **Die Kapsel in der Kopfzeile** sagt, worum es im Tab geht (*Jugendschutz aktiv*, *Spielzeit aktiv*…); solange der Jugendschutz aufgehoben ist, sagt sie das in jedem Tab, und ein Tippen darauf öffnet *Geheimzahl › Altersbeschränkungen wieder aktivieren*. Nebentext ist nie kleiner als 15 px, damit er vom Sofa aus auf einem Fernseher lesbar ist.
- **Dialoge** sind Karten mit Schaltflächen als Kapseln, die Bestätigung in Petrol (in Korallenrot, wenn sie etwas entfernt oder beendet).
  - Die Auswahl öffnet sich als Karte über den Tastenhinweisen, die aktuelle Wahl auf hellem Petrol mit ihrem Häkchen. Der Ring in Petrol gehört überall allein dem Fokus: eine Kategorie, ein Benutzer, eine geltende Wahl behält die helle Füllung.
  - Ein langer Dialog wird an seinem unteren Rand ausgeblendet, solange noch etwas folgt, und seine Warnung (ein Spiel, das gleich unterbrochen wird) steht zuerst, in einer bernsteinfarbenen Karte.
  - Ein Fehlschlag nennt als Titel, was fehlgeschlagen ist, den Fehlercode klein unter dem Grund, und bietet *Bericht senden* an, wenn das hilft.
  - Meldungen erscheinen als Karten unter der Kopfzeile, mit einem Häkchen in Petrol für das, was erledigt wurde, und einem bernsteinfarbenen Zeichen für das, was abgelehnt wurde.
- **Die anderen Bildschirme** bekommen dasselbe Aussehen: Der Editor pro Tag eines Profils zeigt die Woche wie in *Spielzeit* (auf dieselbe Weise bearbeitet, die Schnellvorgaben als Kacheln); *Installierte Spiele prüfen* und *Alle Spiele* setzen ihre Zeilen in Karten; *Profile* und *Hilfe & Diagnose* geben jeder Zeile ihr Symbol.
- **Textgröße** (*Optionen*, oder **AA** in der Kopfzeile):
  - *Automatisch*, der Standard, macht jeden Text 25 % größer, solange die Konsole im TV-Modus ist, am Fernseher, und setzt ihn im Handheld-Modus zurück; *Größer* lässt ihn immer vergrößert, *Normal* nie.
  - Kopfzeile, Seitenleiste und Fußzeile behalten ihre Größe (sie haben keinen Platz für mehr), ebenso die Namen der Kategorien in *Einschränkungen*, die schon groß sind.
  - Die Änderung wirkt sofort, auf jedem Bildschirm, ohne Neustart.
  - Die 25 % stammen von einem 55"-Fernseher in 4K: Der kleinste Text (15 px) ist dann in Großbuchstaben 14,6 mm hoch, 20′ Sehwinkel aus 2,5 m, wo das Lesen angenehm ist (16′ in normaler Größe, das absolute Minimum).
- **Visuelle Effekte** (*Optionen*): *Sparsamer im Akkubetrieb*, der Standard, lässt den Hintergrundschein, das Pulsieren des Rings auf dem Kinderbildschirm in seinen letzten Minuten und das Gleiten des Bogens zu einem neuen Wert weg, solange kein Ladegerät angeschlossen ist (Handheld, ohne Kabel); *Immer vollständig* behält sie. Alle 5 Sekunden neu geprüft: Wird die Konsole angeschlossen, kommen sie zurück.
- **Rückgängig:** Nachdem eine Änderung gespeichert ist, bleibt **ZR** *Rückgängig* einige Sekunden in der Fußzeile: Es öffnet diese Änderung im Änderungsverlauf, um den vorherigen Wert wiederherzustellen. Ein leerer Änderungsverlauf sagt das in einer Karte.

## Benachrichtigungen — was dich braucht, aus jedem Tab

Eine Glocke in der Kopfzeile, links von der Version, sammelt, was den Elternteil braucht; Ⓨ öffnet die Liste aus jedem Tab (der Zähler an der Glocke zeigt *2*, wenn zwei Dinge zu tun sind).

Links von der Glocke die Sprache (ihr Kürzel, *FR*, *EN*…) und das Design (eine Sonne für hell, ein Mond für dunkel, ein halb gefüllter Kreis, wenn es der Konsole folgt), dann die Textgröße (**AA**, dessen großes A in Petrol, solange der Text vergrößert ist): Ein Tippen öffnet dieselbe Liste wie in *Optionen*, und **−** bietet sie aus jedem Tab an. Ein neues Design oder eine neue Sprache wirkt sofort, jeder Bildschirm wird auf demselben Tab neu aufgebaut, mit dem Fokus auf derselben Zeile, Chinesisch und seine eigene Schrift eingeschlossen.

### Zu erledigen

Jeweils mit einem bernsteinfarbenen Warnzeichen, in Bernstein auf der Glocke gezählt; bleibt, bis es behoben ist:

- noch keine Geheimzahl, oder eine Geheimzahl, die jeder zuerst ausprobieren würde (0000, 1234, 1212, 2580…: auf der Konsole geprüft, nie angezeigt);
- eine Geheimzahl auf der Konsole, während PlayGuard selbst ohne sie öffnet (*PlayGuard mit der Geheimzahl schützen* auf *Nie*);
- vorübergehend aufgehobener Jugendschutz (*Wieder aktivieren*);
- Zusatzzeit von einem früheren Tag, die noch zurückzunehmen ist;
- der Alarm „Zeit ist um“ ausgeschaltet;
- Änderungen außerhalb von PlayGuard;
- die Konsole, die keine Spielzeit zählt;
- eine ungenaue Netzwerkuhr.

### Zur Information

Jeweils mit einem „i“, ein Punkt auf der Glocke; kann als gelesen markiert werden:

- ein Limit, das an einem früheren Tag durch *Heute nicht mehr spielen* noch gesenkt ist (*Wiederherstellen*);
- eine neue installierte Version (*Neuigkeiten*; nach einem Update an Ort und Stelle der aufbewahrte vorherige Build), oder eine neuere Version von PlayGuard;
- ein online gesendeter Bericht (*Link anzeigen*), oder PlayGuard wurde beim letzten Mal unerwartet beendet (*Bericht senden*);
- die Spielzeit der Woche (alle 7 Tage, bis sie gelesen ist; mit dem Vergleich zur Vorwoche, pro Tag; *QR-Code zeigen* öffnet die Wochenübersicht in *Aktivität*) und ein Limit, das an zwei oder mehr der letzten 7 Tage überschritten wurde, während die Konsole nur warnt;
- eine Sicherung der Einstellungen, die fällig ist (noch keine, oder keine seit 30 Tagen);
- alle Spiele seit einer Woche gesperrt;
- ein neues Benutzerkonto;
- die verknüpfte Smart-Geräte-App;
- die Netzwerkuhr, die beim Start abweichend gefunden wurde.

### Konsole

Die Einträge, die die Konsole statt des Jugendschutzes betreffen, kommen zuletzt, in einer Zeile, die Ⓐ öffnet (gleich offen, wenn sonst nichts da ist). Die zu erledigenden werden auf dem Abzeichen des Tabs Konsole gezählt, nicht auf der Glocke:

- installierte Spiele, die nicht starten können (*Installierte Spiele prüfen*), Spiel-Patches, die einen Blick brauchen;
- die SD-Karte fast voll (unter 500 MB);
- eine von PlayGuards eigenen Dateien, die nicht beiseitegelegt oder auf die SD-Karte geschrieben werden konnte;
- das veraltete Wiederherstellungs-Sysmodul (nicht das, das mit dieser Version kommt, oder installiert, bevor es seine Version angab; *Aktualisieren* installiert das, das diese Version mitbringt), dann die Konsole, die dafür neu zu starten ist, bis sie es wurde;
- ein Build, der nicht fertig gestartet ist und von dem zurückgekehrt wurde (*Bericht senden*).

Die zur Information: ein Build, von dem zurückgekehrt wurde, eine mit dieser Version nicht getestete Firmware, ein neueres Atmosphère, hekate oder eine neuere Firmware (geprüft mit *Täglich nach Updates suchen*; der Eintrag der Firmware rät, zu warten, bis Atmosphère sie unterstützt), die nicht anonymisierte Seriennummer auf emuMMC, der eigene Jugendschutz des anderen Systems, eine von PlayGuards eigenen Dateien, die beschädigt gefunden und beiseitegelegt wurde (eine Meldung beim Start verweist darauf).

### Wie sich Einträge verhalten

- Jeder Abschnitt listet seine Einträge in einer festen Reihenfolge, egal welche Prüfung zuerst fertig war.
- **In der Seitenleiste** zählt ein bernsteinfarbenes Abzeichen neben dem Namen eines Tabs dessen zu erledigende Einträge, dort, wo sie behoben werden: der Tab Geheimzahl (keine Geheimzahl, eine schwache Geheimzahl, PlayGuard ohne seine Geheimzahl, vorübergehend aufgehoben), die Spielzeit (der Alarm aus, die Konsole zählt nicht), *Heute* (Änderungen außerhalb von PlayGuard, Zusatzzeit zum Zurücknehmen), die Konsole (ihre Einträge oben und die Netzwerkuhr). Ignorierte Einträge und die zur Information werden nicht gezählt.
- Im Tab steht dasselbe Abzeichen in der Zeile, in der jeder behoben wird (*Netzwerkuhr*, *Installierte Spiele prüfen*, *Speicher*, *Spiel-Patches*; *Geheimzahl festlegen oder ändern*, *PlayGuard mit der Geheimzahl schützen*, *Altersbeschränkungen wieder aktivieren*; *Erweitert* der Spielzeit für den Alarm), und der Tab öffnet beim ersten davon.
- **Ⓐ** auf einem Eintrag führt die Lösung aus (*Einschalten*, *Stellen*, *Aktualisieren*…) oder öffnet den Tab, in dem sie gemacht wird. Ein Eintrag verschwindet von selbst, sobald behoben ist, was er meldet.
- **Ⓧ** markiert einen Eintrag zur Information als gelesen; **Ⓨ** markiert alle. Ein als gelesen markierter Eintrag bleibt beim nächsten Start gelesen, bis sich sein Text ändert.
- **Ignorieren:** Einige zu erledigende Einträge können mit Ⓧ ignoriert werden, nach einer Bestätigung und der Geheimzahl: eine Geheimzahl, die jeder zuerst ausprobieren würde, PlayGuard, das ohne die Geheimzahl öffnet, der Alarm „Zeit ist um“ ausgeschaltet, die SD-Karte fast voll (nie die fehlende Geheimzahl, der aufgehobene Jugendschutz, die Uhr, die Spielzeit, die nicht gezählt wird, oder PlayGuards eigene Dateien). Ein ignorierter Eintrag wird nicht mehr gezählt und steht zuletzt unter *Ignoriert*, wo Ⓐ ihn wieder zählen lässt. Er kommt von selbst 30 Tage später zurück (außer *Ignorierte Warnungen nach 30 Tagen wieder zählen* ist aus), sobald das Gemeldete behoben ist und dann erneut auftritt, oder, bei der Geheimzahl, bei einer neuen Geheimzahl mit anderer Länge.
- *Optionen › Benachrichtigungen* schaltet drei Arten aus: neue Versionen von Atmosphère, hekate und Firmware; die Spielzeit der Woche; Erinnerungen (Sicherungen, das andere System, alle Spiele gesperrt).
- Der Zustand dahinter wird beim Start von PlayGuard gelesen, jede Minute auf dem Hauptbildschirm und alle 5 s, solange *Heute* angezeigt wird.

## Heute — der Tag auf einen Blick

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/)*

### Der Ring und die Linie

**Die verbleibende Zeit** als Ring: Ein Tag ohne Spielen zeigt *Kein Spielen*, nicht *0 min*; die heute gegebene Zusatzzeit erscheint als hellerer Abschnitt an seinem Ende, und der Ring gleitet zu jedem neuen Wert.

Darunter, was das Spielen zuerst beendet:

- vor dem Ende der Zeit oder dem Gute-Nacht-Alarm der Rest des Tages als Linie, das Jetzt an ihrem linken Ende, jedes davon eine Marke mit seiner Uhrzeit (*16:40 Zeit um*, *21:00 Schlafenszeit*), in Petrol bis zur ersten, in Bernstein in ihren letzten 15 Minuten;
- sonst in Worten: der Gute-Nacht-Alarm, sobald die Zeit des Tages aufgebraucht ist, wann wieder gespielt werden darf; solange der Jugendschutz aufgehoben ist, das heutige Limit ausgesetzt und warum.

### Die Aktionen

- **Zusatzzeit**, mit ihren Mengen nur einen Druck entfernt: Eine ausgewählte Menge verschiebt das Zeitende auf der Linie (*mit 15 min mehr*) oder sagt an der Marke des Gute-Nacht-Alarms, dass dieser trotzdem zuerst kommt; die erste hebt sich in Petrol ab, sobald 15 min oder weniger übrig sind, oder nichts mehr.
- **Für heute beenden**, **Dem Kind zeigen**.
- **Heutiges Limit…**: das heutige Limit direkt dort gewählt, nach einer Bestätigung geschrieben.
- **Das geltende Profil**, mit seinem Namen, sobald ein Profil gespeichert ist (*Profile…*, wenn keines passt): Ⓐ listet sie auf, das geltende abgehakt, um nach einer Bestätigung ein anderes anzuwenden (die Tage, die sich ändern), oder *Profile verwalten…*.
- Solange der Jugendschutz aufgehoben ist, kommt **Wieder aktivieren** zuerst, über die ganze Breite der Zeile, und *Zusatzzeit* tritt zurück.
- Ist an keinem Tag ein Limit gesetzt, öffnet **Limits der Woche festlegen** die Woche in *Spielzeit*.

### Die Karten und Kapseln

- Der **Gute-Nacht-Alarm** (wann wieder gespielt werden darf, oder wie lange es noch dauert, bis er klingelt, wenn die Konsole es nicht sagt; ausgeschaltet *Aus* in groß und Ⓐ *Einstellen*), der **Jugendschutz** und die **Netzwerkuhr**, als Karten.
- Der Zustand der Konsole als **Kapseln**:
  - eine sagt, ob die DNS-Umleitung von Atmosphère Nintendos Server blockiert, und Ⓐ darauf, welche und was das ändert: keine Synchronisierung für die Smart-Geräte-App, keine Netzwerkuhr, die sich selbst stellt;
  - eine andere, über einem Spiel geöffnet, dass die Zeit in PlayGuard mitgezählt wird, Ⓐ sagt, wie;
  - ein im HOME-Menü offen gelassenes Spiel erscheint als bernsteinfarbene Kapsel (*zählt weiter*, Ⓐ sagt, warum), wenn die Kapsel zu PlayGuard über einem Spiel es nicht schon sagt.
- Die Kapseln stellen an den Anfang, was den Elternteil braucht (Bernstein, mit einem Dreieck), dann, was aktiv ist (Petrol, mit einem Häkchen), dann den Rest (grau, mit einem Punkt): Die Form sagt es ebenso wie die Farbe.
- Unter den Kapseln die noch zurückzunehmende Zusatzzeit (Ⓐ nimmt sie zurück) und der Alarm „Zeit ist um“, solange er aus ist (Ⓐ schaltet ihn wieder ein).

### Erste Schritte

Solange die Geheimzahl und die Limits der Woche nicht festgelegt sind, listet eine Karte über den Karten die drei Schritte auf (die Geheimzahl, ein Tageslimit, die Netzwerkuhr), jeder in Petrol abgehakt, sobald er erledigt ist, oder in Bernstein umkreist, mit der Zahl der erledigten (*1 von 3*); Ⓐ auf einem erledigt ihn direkt dort.

### Zählt die Konsole?

Die Zeit von PlayGuard selbst zählt, während es als Anwendung läuft; mit einem Limit für heute sollte die verbleibende Zeit also sinken, solange es offen ist. Hat sie sich auf dem Bildschirm etwa 90 s lang nicht bewegt (Timer an, ein Limit für heute, nicht aufgehoben, nicht aus dem Album geöffnet), sagt eine bernsteinfarbene Zeile unter der gespielten Zeit *Die Konsole zählt die Spielzeit gerade nicht*, mit den wahrscheinlichen Ursachen:

- die Uhr wurde zurückgestellt (sie zählt später wieder);
- die Netzwerkuhr wurde nie gestellt (*Konsole › Netzwerkuhr*);
- ein Limit wurde gerade geschrieben (starte ein Spiel für eine Minute, dann prüfe).

Sie verschwindet, sobald sich die verbleibende Zeit wieder bewegt. Der Tab Spielzeit sagt es auch.

### Außerhalb von PlayGuard geändert

Die heutige Spielzeit, die am selben Tag neu beginnt (eine Änderung der Uhr bewirkt das), die Konsolenuhr um mehr als 5 Minuten verstellt, Limits, die von denen abweichen, die PlayGuard zuletzt gesehen hat (die Smart-Geräte-App, ein anderes Werkzeug): eine bernsteinfarbene Karte über dem Ring, bis sie geschlossen wird (Ⓐ) oder bis zum nächsten Tag, und ein Eintrag im Änderungsverlauf. PlayGuard läuft nicht im Hintergrund: Es sieht nur, was sich geändert hat, während es offen war, oder seit es zuletzt offen war, wenn es die Konsole erneut liest.

### Beim Limit

Die Konsole zeigt „Zeit ist um“ an und unterbricht dann das Spiel (ohne „Weiterspielen“) oder warnt nur: *Wenn die Zeit um ist* zeigt, was davon, und ändert es, sobald ein Limit oder eine Schlafenszeit festgelegt ist ([Details, auf Englisch](../parental-controls.md#when-the-time-runs-out)).

### Zusatzzeit und heute nicht mehr spielen

- **Zusatzzeit für heute**: standardmäßig +15 min, +30 min, +1 h.
- **Heute nicht mehr spielen**: Limit 0 nur für heute, *Jetzt* oder *In 5 Minuten*, wenn die gespielte Zeit bekannt ist (das Limit wird dann die gespielte Zeit + 5 min, Zeit zum Speichern). Bei der Sperre erscheint „Zeit ist um“ der Konsole, und nicht gespeicherter Fortschritt kann verloren gehen — die Bestätigung sagt das.
- **Am nächsten Tag** — beim Start, oder um Mitternacht, wenn die App offen ist — setzt PlayGuard das übliche Limit zurück:
  - nach Zusatzzeit von selbst, ohne Frage und ohne Geheimzahl (es senkt das Limit nur, wie das Wiederaktivieren des Jugendschutzes); es hebt die Sperre kurz auf, wenn der Timer zählt, und sperrt sofort wieder. Es wartet, solange es nicht schreiben kann (schreibgeschützter Modus, eine Firmware, die es nicht unterstützt, die Konsolensperre an) und, eine Woche später am selben Wochentag, solange das niedrigere Limit das laufende Spiel beenden würde (dann am nächsten Tag). Um an einem Tag mehr Zeit zu behalten, ändere das Limit dieses Tages;
  - nach *Heute nicht mehr spielen* fragt es, ob 0 bleiben oder das Limit zurückgesetzt werden soll (mit der Geheimzahl, wie jede Änderung, die ein Limit erhöht), oder setzt es wie jede Änderung zurück (*Spielzeit › Limit nach Spielstopp automatisch zurücksetzen*); ein Zurücksetzen auf „kein Limit“ wird immer erfragt.
- **Zusatzzeit an einem anderen Tag** (*Zusatzzeit › Ein anderer Tag…*, am Ende ihrer Liste): morgen oder einer der fünf Tage danach, jeder mit seinem Limit aufgeführt, dann die Menge. Das Limit dieses Tages wird sofort erhöht, nur für dieses Datum, und von selbst zurückgesetzt (es senkt das Limit nur), wenn PlayGuard nach Ablauf des Tages das nächste Mal geöffnet wird — nicht auf einer Konsole, auf der PlayGuard nicht geöffnet wird; dort behält es auch derselbe Wochentag danach. Immer nur eine: Die Liste bietet dann an, sie zu entfernen (ihr Limit sofort zurückgesetzt). Eine zwischenzeitliche Änderung des Limits dieses Tages (die Woche, ein Profil…) hat Vorrang: Es wird nichts zurückgesetzt.

### Der Kinderbildschirm

*Dem Kind zeigen* in *Heute*, um die Konsole zu übergeben:

- nur die verbleibende Zeit (solange der Jugendschutz aufgehoben ist, das heutige Limit, ausgesetzt), wann das Spielen endet (*Ende gegen 16:40*; kommt der Gute-Nacht-Alarm zuerst, zählt der Ring stattdessen bis zu ihm herunter, *45 min bis zum Schlafen*, wie in der letzten Stunde eines Tages ohne Limit), die Schlafenszeit und was passiert, wenn die Zeit um ist;
- so groß, dass er quer durch den Raum lesbar ist, am Fernseher; sein Text behält seine Größe, egal welche Textgröße eingestellt ist;
- in den letzten 5 Minuten (des Spielens oder vor der Schlafenszeit) wird das Ende bernsteinfarben und der Ring pulsiert langsam; die Schlafenszeit wird in ihren letzten 30 bernsteinfarben;
- Ⓐ und Ⓨ, die Tasten der Eltern, stehen klein oben rechts, sodass die Fußzeile nur die des Kindes zeigt;
- Ⓐ kehrt mit der Geheimzahl zu den Bildschirmen der Eltern zurück; Ⓨ gibt von dort aus Zusatzzeit, nach der Geheimzahl (die Mengen von *Zusatzzeit*, der Kinderbildschirm bleibt stehen); Ⓑ verlässt PlayGuard (und fragt zuerst, ob der Jugendschutz wieder aktiviert werden soll, wie beim Beenden).

Bei größerem Text passt *Heute* weiterhin auf einen Bildschirm: Die Zeile über der Überschrift nennt nur noch den Tag und sein Limit (*DONNERSTAG · 2 h*), die Schaltflächen und die Karten rücken etwas enger zusammen. Aktualisiert sich alle 5 s (Ⓧ aktualisiert sofort).

## Spielzeit — die Woche ist der Editor

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/?tab=play_timer)*

### Die Woche bearbeiten

Ⓐ auf einem Tag bearbeitet ihn:

| Taste | Bei der Bearbeitung |
|---|---|
| ↑/↓ | Um 15 min ändern, gehalten in Stundenschritten |
| ←/→ | Zum nächsten Tag gehen |
| Ⓛ | Auf Montag–Freitag kopieren |
| Ⓡ | Aufs Wochenende kopieren |
| Ⓨ | Kein Limit |
| Ⓧ | Eine genaue Dauer (*Dauer eingeben…*: ↑/↓ in 15-min-Schritten, gehalten in Stundenschritten, Ⓛ/Ⓡ in 5-min-Schritten, OK ausgewählt) |
| Ⓐ oder Ⓑ | Die Bearbeitung beenden |

- Per Touch wird ein angetippter Tag bearbeitet, dann ändert ein Tippen auf sein oberes oder unteres Ende ihn um 15 min.
- Die Änderungen werden gemeinsam gespeichert, mit **+** oder *Speichern*, wie im Editor pro Tag (die Bestätigung listet die geänderten Tage auf, vorher → nachher); *Verwerfen* setzt sie zurück.
- In jedem Tagesbalken die Zeit, die an diesem Wochentag zuletzt gespielt wurde (heute eingeschlossen, alle Benutzer), in Korallenrot, sobald das Limit erreicht wurde, sein Limit auch, und eine Legende unter der Woche sagt das (*Limit überschritten*).
- Ein geänderter, noch nicht gespeicherter Tag hat sein Limit in Bernstein und seinen Balken bernsteinfarben umrandet; unter der Woche die Zahlen des ausgewählten Tages (*Donnerstag: 1 h 20 von 2 h gespielt*).
- Über der Woche, in Bernstein, ein erreichtes Limit und was die Konsole dann tut, oder dass die Konsole keine Spielzeit zählt.
- `0` Minuten bedeutet *an diesem Tag kein Spielen*; *Spielzeitlimit entfernen* schaltet den Timer aus.

### Chips und Schnelleinstellungen

**Chips** über und unter der Woche öffnen ihre Auswahl sofort: die Profile (wie viele gespeichert sind), jeden Tag gleiches Limit, der Gute-Nacht-Alarm, *Wenn die Zeit um ist* und das Entfernen.

- **Jeden Tag gleiches Limit:** eine Schnellliste oder eine genaue Dauer (*Dauer eingeben…*: ↑/↓ in 15-min-Schritten, gehalten in Stundenschritten, Ⓛ/Ⓡ in 5-min-Schritten).
- **Wochentage und Wochenende ›:** der Editor, der nach Montag–Freitag und dann nach dem Wochenende fragt.
- **Zusatzzeit für heute** und **heute nicht mehr spielen**, wie in *Heute*. Ein Hinweis darunter sagt, was die Konsole beim Limit tut. Während der Jugendschutz vorübergehend aufgehoben ist, wird die Spielzeit *möglicherweise* nicht gezählt: Das ist noch offen ([Doku, auf Englisch](../parental-controls.md#still-open)).
- Jede Bestätigung listet die Tage auf, die sich ändern, vorher → nachher.

### Profile

- **Profile**, auf der SD-Karte gespeichert: eines anwenden (ein bereits geltendes Profil sagt das einfach), bearbeiten, umbenennen oder löschen; die aktuellen Limits speichern oder ein neues anlegen.
- Jeder Name, in jeder Schrift (*École*, *周末*, *Выходные* …) — ein Name ohne lateinische Buchstaben bekommt einen eigenen Dateinamen, und zwei Namen, die auf dieselbe Datei hinauslaufen würden, werden erkannt.
- **Geplantes Profil** (*Profile › Geplantes Profil*): ein Profil für einen Zeitraum — Ferien, eine Reise —, dessen erster und letzter Tag aus den nächsten 60 Tagen gewählt werden.
  - Wenn der Zeitraum beginnt, fragt PlayGuard, ob es angewendet werden soll (die Tage, die es ändert, unter der Frage dargestellt); wenn er vorbei ist, ob die Limits, die es ersetzt hat, wiederhergestellt oder behalten werden sollen.
  - Einmal am Tag gefragt, auf dem Hauptbildschirm, wenn nichts anderes im Vordergrund ist: PlayGuard handelt nur, wenn es geöffnet wird, also behält eine Konsole, auf der es nicht geöffnet wird, ihre Limits. Ein Zeitraum, der verstrichen ist, ohne dass das Profil je angewendet wurde, wird vergessen.
  - Immer nur eines; die Zeile ändert, entfernt oder beendet es.

### Gute-Nacht-Alarm

- Die Alarmzeit (16:00 bis 23:45, oder aus) und wann wieder gespielt werden darf (05:00 bis 09:00), jeden Tag gleich.
- Oder, über *Nach Wochentag…* am Ende der Liste des Alarms, für Montag bis Freitag, das Wochenende oder einen einzelnen Tag, während die anderen Tage ihre Zeit behalten (jede Gruppe von Tagen mit ihrer Uhrzeit aufgeführt). Wenn sich die Tage unterscheiden, zeigt der Tab *Je nach Tag verschieden*, und die Woche in *Spielzeit* zeigt den Alarm jedes Tages unter seinem Namen.
- Seine Stelle in den Einstellungen des Timers, aus den Einstellungen der Smart-Geräte-App erschlossen, ist auf einer Konsole (22.0.0) bestätigt; PlayGuard ändert ihn trotzdem nur, wenn die Konsole meldet, was PlayGuard dort liest, prüft die Antwort der Konsole nach der Änderung und setzt die vorherigen Einstellungen zurück, wenn sie abweicht.

### Einstellungen der Zusatzzeit und Erweitert

- **Zusatzzeit:** die angebotenen Mengen (+15/+30/+1 h, +10/+20/+30 min …) und das Zurücksetzen des üblichen Limits ohne Nachfrage nach *Heute nicht mehr spielen* (Zusatzzeit wird immer ohne Nachfrage zurückgenommen). Beides geschieht beim nächsten Öffnen von PlayGuard, ab dem nächsten Tag: Eine Konsole, auf der es nicht geöffnet wird, behält das Limit dieses Tages.
- **Erweitert ›**, ein eigener Bildschirm: der Schalter für die Debug-Aktionen und, solange er an ist, der Alarm „Zeit ist um“ an/aus und Countdown pausieren / fortsetzen. Derselbe Schalter erlaubt einer Sicherung oder dem Verlauf, den Alarm wiederherzustellen.

## Aktivität — wer was gespielt hat, und wie lange

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/?tab=activity)*

Zeit pro Spiel **heute**, in den **letzten 7 Tagen** und **insgesamt**, aus dem eigenen Aktivitätsprotokoll der Konsole — für alle Benutzer oder **ein Benutzerkonto** (die Benutzer als Chips oben, wenn die Konsole mehrere hat). PlayGuard startet mit dem zuletzt angezeigten Benutzer, oder mit dem unter *Standardkonto* festgelegten (alle Benutzer, oder einer).

### Oben

- Die Benutzer als Chips, der Zeitraum und der Export.
- Heute, die letzten 7 Tage und insgesamt als Zahlen. Unter den letzten 7 Tagen der Vergleich mit den 7 davor, pro Tag: ▲ *+20 min/Tag ggü. Vorwoche* in Bernstein, ▼ weniger in Petrol, *wie in der Vorwoche* bei höchstens 5 Minuten Unterschied.
- Die vier meistgespielten Spiele als Karten (Ⓐ öffnet eines); über die letzten 7 Tage sagt das Spiel, das am meisten mehr gespielt wurde als in der Woche davor, um wie viel, ab einer halben Stunde (▲ *+2 h*).
- Im Zeitraum nichts gespielt: Eine Karte sagt das, mit *Gesamt ansehen*, wenn es etwas zu sehen gibt.

### Diagramme und Zusammenfassung

- **Ein Diagramm der letzten sieben Tage** (heute rechts, eine Legende darunter), mit dem Limit dieser Woche für jeden Tag als Linie und der Zeit darüber in Bernstein, mit ! markiert; die Summen von heute, der Woche und insgesamt.
- **Eine Zusammenfassung** über den gewählten Zeitraum (heute, die letzten 7 Tage, die **letzten 30 Tage** oder insgesamt), für den angezeigten Benutzer: Durchschnitt pro Tag, das meistgespielte Spiel, die Spieltage und der intensivste (letzte 7 Tage), die durchschnittliche Sitzung (insgesamt).
- **Spielzeiten, letzte 7 Tage**: ein Feld für jede Stunde jedes Tages, Montag zuerst, je kräftiger, desto mehr wurde gespielt (alle Spiele, der angezeigte Benutzer); dann die Sitzungen (Spielen mit Pausen unter 5 Minuten ist eine Sitzung, auch über einen Spielwechsel hinweg) und ihre durchschnittliche Länge, die längste und ihr Tag, die zwei aufeinanderfolgenden Stunden, in denen am meisten gespielt wurde, und die zwischen 21:00 und 06:00 gespielte Zeit.

### Alle Spiele und die Seite eines Spiels

- **Alle Spiele**, auf einem eigenen Bildschirm: jedes im Zeitraum gespielte Spiel, das meistgespielte zuerst, als **Raster aus Covern** oder als **Liste** (Ⓨ wechselt, und die Wahl wird gemerkt), mit den Zahlen des ausgewählten Spiels immer oben: sein Symbol und Name, die Spielzeit im Zeitraum, insgesamt, Starts und zuletzt gespielt. Ⓧ wählt den Zeitraum (der Tab Aktivität folgt).
- Symbole werden rund um das ausgewählte Spiel gelesen (nicht im Applet-Modus, um Speicher zu sparen); ein grauer Controller steht da, bis eines ankommt, und für ein Spiel ohne Symbol.
- Eine große Bibliothek zeigt ihre ersten 50 Spiele, dann *Alle Spiele anzeigen* (höchstens 100 Spiele im Applet-Modus); der Export enthält immer alle. PlayGuard liest höchstens 256 Spiele (die dieser Woche zuerst): Darüber hinaus sagen der Tab Aktivität, Alle Spiele und der Export (nicht CSV), dass welche fehlen, und die Gesamtsumme lässt sie weg.
- **Ⓐ auf einem Spiel:** seine letzten sieben Tage als Balken, Starts, zuerst und zuletzt gespielt, Zeit pro Benutzerkonto. Gelöschte Spiele behalten ihre Gesamtwerte, auch eines, das gelöscht wurde, bevor PlayGuard installiert war, solange das Spielprotokoll der Konsole es noch nennt, und ihren Namen, sobald PlayGuard ihn gesehen hat (sonst *Gelöschtes Spiel* und seine Title-ID).

### Benutzerkonten

- Die Zeit eines Spiels, die keinem Benutzerkonto gehört (manche Spiele fragen nie nach einem, oder das Konto wurde inzwischen gelöscht), erscheint als *Ohne Nutzer*, ab einer Minute.
- Ein Benutzerkonto, das PlayGuard nie zuvor gesehen hat, wird 30 Tage lang im Tab in Bernstein markiert (*Neuer Nutzer seit …*, ein frisches Konto ist ein bekannter Weg, von vorn anzufangen) und im Änderungsverlauf festgehalten; die Konten, die da sind, wenn PlayGuard sie zum ersten Mal auflistet, werden nicht markiert.

### Export und Teilen

- **Auf die SD-Karte exportieren** (der Chip *Exportieren*) als CSV, JSON, XLSX (Excel) oder PDF, eine Spalte pro Tag, dann die letzten 7 Tage, die letzten 30 Tage und insgesamt.
- **Wochenübersicht als QR-Code:** Die Kamera eines Handys liest ihn als einige Zeilen Text, in der angezeigten Sprache — die letzten 7 Tage und ihr Durchschnitt, der Trend, jeder Tag, die drei meistgespielten Spiele, die Sitzungen, die längste davon und die zwischen 21:00 und 06:00 gespielte Zeit. Nichts geht online.
- Öffnet sofort mit den zuletzt gelesenen Zahlen (zwischen den Starts auf der SD-Karte aufbewahrt), im Hintergrund aktualisiert, wenn sie älter als eine Minute sind; Ⓧ liest sie sofort neu, mit einer Ladeanzeige.
- Zeiten sind ungefähr, wenn die Konsolenuhr geändert wurde.

## Einschränkungen — Kategorie, Altersfreigabe, Kommunikation

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/?tab=restrictions)*

- **Die Einschränkungskategorie:** Kleinkinder, Kinder, Jugendliche und Nutzerdefiniert nebeneinander als Kacheln, jede mit dem, was sie einschränkt (ein Vorhängeschloss für eingeschränkt, ein Häkchen für erlaubt, und die installierten Spiele, die sie sperrt; Ⓐ legt sie fest; ist *Nutzerdefiniert* aktiv, führt Ⓐ zu seinen Einstellungen darunter); *Keine* darunter. Was eine Kategorie einschränkt (Altersgrenze, Posten, Kommunikation), wird noch einmal gesagt, bevor sie festgelegt wird.
- **Bei Nutzerdefiniert:** Altersfreigabe, Posten in sozialen Netzwerken, Kommunikation mit anderen.
- **VR-Modus** und das **Alterseinstufungssystem** (PEGI, ESRB, USK, CERO …): Die Liste sagt, für welche Länder jedes einstuft (PEGI: der größte Teil Europas, ESRB: die USA, Kanada und Lateinamerika, USK: Deutschland, PEGI BBFC: das Vereinigte Königreich…).
- **Die Anzahl der Spiele, die kommunizieren dürfen** (die Liste selbst steht im Diagnosebericht, roh, bis ihr Aufbau bekannt ist).
- **Blockierte installierte Spiele:** wie viele der installierten Spiele die geltende Kategorie wegen ihrer Altersfreigabe blockiert (*3 von 15*), Ⓐ listet sie mit ihrer Freigabe auf (*PEGI 16*); die Kachel jeder Kategorie sagt es auch (*Blockierte Spiele · 3*).
  - Die Freigaben sind die jedes Spiels selbst (seine Steuerdaten, gelesen von der Prüfung der installierten Spiele beim Start oder über *Konsole › Installierte Spiele prüfen*), nach dem System, das die Konsole verwendet.
  - Ein Spiel, das dieses System nicht einstuft, wird nicht gezählt, ebenso wenig die in der Smart-Geräte-App festgelegten Ausnahmen.
  - Der Bildschirm eines Spiels in Aktivität zeigt seine Freigabe, und *auf der aktuellen Stufe blockiert*, wenn es das ist.

## Geheimzahl — die Geheimzahl, Aufheben, Spiele sperren, Smart-Geräte-App

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/?tab=security)*

- **Oben** drei Kacheln: die Geheimzahl, die Einschränkungen und das vorübergehende Aufheben, jede mit einem Häkchen in Petrol, wenn alles ist, wie es sein soll (eine Geheimzahl festgelegt, Einschränkungen aktiv, gesperrt), oder einem bernsteinfarbenen Warnzeichen, wenn etwas zu tun ist.
- **Geheimzahl festlegen oder ändern** (Geheimzahl-Bildschirm des Systems).
- **Geheimzahl anzeigen**: nach einer Warnung und der Geheimzahl selbst, jedes Mal abgefragt; im Verlauf festgehalten, dann wird eine neue Geheimzahl angeboten.
- **Vorübergehend aufheben**, **Altersbeschränkungen wieder aktivieren**, und ob PlayGuard sie direkt nach einer Änderung, die das Aufheben brauchte, von selbst wieder aktiviert (standardmäßig an).

### PlayGuard mit der Geheimzahl schützen

Die Geheimzahl in PlayGuard selbst:

| Einstellung | Wirkung |
|---|---|
| *Nie* | Keine Geheimzahl in PlayGuard (die Glocke meldet das, solange die Konsole eine hat) |
| *Vor einer Änderung* (Standard) | Jeder kann schauen, nur der Elternteil ändert etwas |
| *Beim Öffnen von PlayGuard* | Ein Sperrbildschirm beim Start |

- Einmal eingegeben, gilt die Geheimzahl, bis PlayGuard geschlossen wird; *Geheimzahl nach 5 Minuten erneut abfragen* (standardmäßig aus) fragt danach erneut und bringt den Sperrbildschirm von *Beim Öffnen von PlayGuard* zurück, wenn PlayGuard nach 5 min oder mehr außerhalb des Fokus zurückkehrt.
- Wird in der Dienstschicht geprüft, sodass keine Änderung sie umgeht; das Wiederaktivieren fragt nie danach. *Geheimzahl anzeigen* fragt jedes Mal danach, in jedem Modus und sogar direkt nach der Eingabe, und eine fehlende oder beschädigte `config.json` zählt als *Vor einer Änderung*.
- **Beenden**, während der Jugendschutz noch aufgehoben ist, fragt, ob er wieder aktiviert werden soll (*Wieder aktivieren und beenden* schließt auch das Spiel, über dem PlayGuard geöffnet wurde, zurück zum HOME-Menü statt zu hbmenu); wird PlayGuard stattdessen über das HOME-Menü geschlossen, aktiviert es ihn beim nächsten Start wieder, und ein Neustart der Konsole aktiviert ihn von selbst.

### Alle Spiele sperren

Ein Schalter, der das Limit jedes Tages auf 0 setzt, sodass zum Starten eines Spiels eine Geheimzahl nötig ist — eine leichte Sperre ohne Altersfreigaben oder Kommunikationsgrenzen.

- Sie sperrt das Starten von Spielen, nicht das HOME-Menü, und braucht eine Geheimzahl.
- Die vorherigen Limits kommen zurück, wenn sie ausgeschaltet wird.
- Solange sie an ist, werden *Zusatzzeit* und *heute nicht mehr spielen* abgelehnt, und auf anderem Weg gesetzte Limits (ein Profil, eine Sicherung, der Verlauf…) ersetzen sie.

### Smart-Geräte-App

- Ob die App „Nintendo Switch-Altersbeschränkungen“ verknüpft ist, ihre letzte Synchronisierung, und **trennen** (sonst überschreibt ihre nächste Synchronisierung die hier gesetzten Limits: Solange sie verknüpft ist, sagt das jede Änderung, die PlayGuard bestätigen lässt).
- Wenn die DNS-Umleitung von Atmosphère (`atmosphere/hosts`) Nintendos Jugendschutz-Server umleitet, kann sich die App nicht synchronisieren: PlayGuard zeigt dann hier *Sync blockiert (DNS)* und erwähnt die App nirgends sonst (kein Hinweis in den Dialogen, kein Eintrag in der Glocke, kein Schritt in den ersten Schritten, keine Kapsel in *Heute*, keine Hilfe beim Entschlüsseln, und das Trennen fragt nach, ohne zu sagen, was die App überschreiben würde).
- Solange sie verknüpft ist und synchronisieren kann, öffnet *Vor dem Trennen: zwei Einstellungen entschlüsseln* den Vergleich des Blocks (siehe [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), auf Englisch).

### Alle Altersbeschränkungen löschen

Zwei Bestätigungen, unumkehrbar; vorher wird eine Sicherung der Einstellungen gespeichert.

## Konsole — die Netzwerkuhr, die installierten Spiele, was auf der Konsole läuft

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/?tab=console)*

**Oben**, was auf der Konsole läuft, als Kacheln (die Firmware, Atmosphère, und hekate, wenn es auf der SD-Karte liegt; Ⓐ öffnet seine Versionshinweise), dann die Diagnose, dann die Zeilen der Konsole, die Kompatibilität zuerst (nur ein Häkchen in Petrol, wenn PlayGuard auf dieser Firmware geprüft wurde, sonst eine Warnung und was nicht stimmt).

### Netzwerkuhr

*Netzwerkuhr ›*, ein eigener Bildschirm — die Uhr, auf die sich der Timer verlässt. Eine Konsole, die nie Nintendos Server erreicht, hält diese Uhr ungenau, was den Timer verfälscht.

- **Konsolen- und Netzwerkuhr**, Zeitzone, Genauigkeit. Wenn die DNS-Umleitung von Atmosphère (`atmosphere/hosts`) Nintendos Zeitserver (`aauth-lp1.ndas.srv.nintendo.net`) umleitet, kann die Konsole ihre Netzwerkuhr nicht mehr selbst stellen: Dieser Bildschirm, der Eintrag in der Glocke und die Karte in *Heute* sagen das und verweisen auf die Messung unten.
- **Wähle einen öffentlichen NTP-Server** (≈ 50 eingebaut, nach Region, oder einen eigenen).
- **Messen** gegen 3 Server (Median; eine bernsteinfarbene Warnung, wenn sie sich nicht einig sind), dann **die Netzwerkuhr stellen** — eine Messung bleibt 2 Minuten lang nutzbar, mit einem Countdown. Die Bestätigung sagt es: Das Ändern der Uhr lässt die heutige Spielzeit neu beginnen, also steht das ganze Limit wieder zur Verfügung.
- **Eine Prüfung der Netzwerkuhr beim Start**: eine Meldung, wenn sie mehr als eine Minute abweicht; sie stellt die Uhr nie.

#### Kein Server antwortet

Auf einer Konsole ohne Internet erscheint *Netzwerkuhr auf die Konsolenuhr stellen* unter dem Ergebnis (und auf der Karte der Netzwerkuhr in *Heute*, nachdem ihre Messung fehlschlägt), hinter derselben Bestätigung.

- PlayGuard hat nichts anderes, womit es die Konsolenuhr vergleichen könnte: Die Bestätigung zeigt ihre Uhrzeit und bittet dich, sie zuerst mit einem Handy oder einer Uhr zu vergleichen, und eine Konsolenuhr vor 2026 (nie gestellt) wird abgelehnt.
- Mit *Uhr mithilfe des Internets synchronisieren* aus bleibt die Konsolenuhr, wo sie ist, und das spätere Einschalten der Einstellung lässt sie nicht mehr springen; ist es an, folgt die Konsolenuhr bereits der Netzwerkuhr, also sagt die Bestätigung, es auszuschalten und die Uhrzeit von Hand zu stellen, falls sie falsch ist.
- Es schreibt nur die Uhr, nicht den Jugendschutz, braucht also kein Aufheben (die Geheimzahl von *PlayGuard mit der Geheimzahl schützen* gilt trotzdem); wie jede Änderung der Uhr lässt es die heutige Spielzeit neu beginnen.
- Noch nicht auf einer Konsole ausprobiert.

### Installierte Spiele

*Installierte Spiele prüfen* findet die Spiele, die das HOME-Menü nicht starten oder darstellen kann — ein Symbol, das ewig lädt, ein „!“, ein gestrichelter Rahmen — mit dem, was nicht stimmt, und was zu tun ist, unter dem Symbol des Spiels, und der Anzahl der installierten Spiele, die ein Problem haben.

**Was es findet:** Name und Symbol unlesbar, das Spiel selbst fehlt hinter seinem Update, nichts mehr installiert, fehlende Dateien, von der Konsole gefundene beschädigte Daten, eine neuere Firmware, die das Spiel oder nur sein Update braucht, Schlüssel zu neu für die Firmware, eine fehlende Lizenz (Ticket), die Konsole verlangt ein entferntes Update, die SD-Karte nicht lesbar, ein Start, den die Konsole ablehnt.

**Die Liste hat einen Abschnitt pro Lösungsweg**, jeder mit seiner Anzahl und was zu tun ist, die Spiele mit demselben Problem zusammen (zwei Spiele mit demselben Namen werden an ihrer Version oder Title-ID unterschieden):

| Abschnitt | Was er enthält |
|---|---|
| **Hier reparierbar** | Was PlayGuard repariert |
| **Zu prüfen** | Daten, denen die Konsole misstraut: Die Prüfung sagt, ob das Spiel neu zu installieren ist |
| **Neu installieren** | Dateien weg oder unbrauchbar, oder Daten beschädigt gefunden: entfernen, dann neu installieren |
| **Firmware oder SD-Karte** | Das Spiel ist in Ordnung, die Konsole nicht |

- **Daten prüfen:** Was die Datenprüfung gefunden hat, bleibt beim Spiel, bis eine andere Version installiert wird: beschädigt, wandert es nach *Neu installieren* als *Beschädigt (geprüft)*, mit dem Datum und dem Code der Konsole; intakt, verlässt es die Liste. *Diese Spiele prüfen* prüft den ganzen Abschnitt *Zu prüfen*, ein Spiel nach dem anderen in einem Dialog (*Spiel 2 von 5*), und sagt dann, wie viele beschädigt, intakt oder nicht prüfbar waren, mit dem Entfernen der beschädigten gleich zur Hand.
- **Mehrere Spiele auf einmal:** Ⓨ wählt Spiele aus (ein Häkchen davor), und *Mehrere Spiele auf einmal*, über den Abschnitten, wählt einen ganzen Abschnitt; dann eine Aktion für alle: ihre Daten prüfen oder sie entfernen (ihre Liste zuerst gespeichert, eine Frage, eine Geheimzahl).
- **Unter *Neu installieren*** kann ihre Liste gespeichert werden (JSON und Text, im Export-Ordner: Name, Title-ID, Version, Problem), und alle auf einmal entfernt werden, nach einer Frage und einer Geheimzahl: Die Liste wird zuerst gespeichert, dann werden die Spiele entfernt, damit ein Installationsprogramm sie nicht mehr als installiert zählt.
- **Die Aktionen:** Wenn eine Aktion die Lösung ist, ist sie die erste Schaltfläche des Dialogs; die anderen folgen: die installierte Version wieder akzeptieren (wenn die Konsole ein entferntes Update verlangt), nur das Update oder einen Zusatzinhalt entfernen, die Daten auf Beschädigung prüfen (die eigene Prüfung der Konsole, mit ihrem Fortschritt und Abbrechen) oder das Spiel entfernen.
- Versionen werden so angezeigt, wie das Spiel sie zeigt (*Version 1.4.0*, *Update Nr. 4*); nach einer Reparatur sagt eine Meldung, ob das Spiel wieder funktioniert. Spielstände bleiben immer erhalten; jede Änderung steht im Änderungsverlauf.
- **Wann es prüft:** Die letzte Prüfung wird sofort angezeigt; dann werden nur die Spiele erneut geprüft, deren Eintrag sich geändert hat (jedes Spiel einmal am Tag, nach einem Firmware-Update oder mit Ⓧ *Erneut prüfen*), jedes angezeigt, sobald es geprüft ist.
- Mehrere defekte Spiele auf einmal deuten auf die SD-Karte oder auf ein Spiel, das aus dem anderen Speicher installiert wurde (sysMMC / emuMMC).
- *Ansicht*, über den Abschnitten, zeigt die Spiele als **Liste** oder als **Cover** (Symbol, Name und Problem; die Wahl wird gemerkt), wie *Alle Spiele*.

### Forwarder

*Forwarder ›*, ein eigener Bildschirm unter *Homebrew* — ein Homebrew im HOME-Menü, mit seinem Symbol und seinem Namen, von dort wie ein Spiel gestartet (ein NRO-Forwarder, wie sphaira sie erstellt).

- **Eine Karte oben** sagt, wie es um das Homebrew-Menü steht: gesperrt oder offen, seine Rückwege (der Forwarder von PlayGuard, die geschützten Menüs, die noch nie über das HOME-Menü gestarteten), was es noch ohne Geheimzahl öffnet und ob das Rettungsmodul installiert ist. Ⓐ darauf listet auf, was zu tun ist, jedes einen Druck entfernt (der nächste Schritt zur Sperre, einen Forwarder schützen oder löschen, der es ohne Geheimzahl öffnet, die Prüfung, das Notfallblatt).
- **Die Liste nach Gruppen:** der von PlayGuard, die mit der Geheimzahl geschützten, die ohne, die einer anderen App, die nicht mehr im HOME-Menü. Ein geschützter hat ein Vorhängeschloss auf seinem Symbol (ein Häkchen für ein Menü, das nachweislich über sein Tor funktioniert), ein hier ohne Geheimzahl erstellter ein offenes Vorhängeschloss in Bernstein.
- **Ⓨ** öffnet diesen Teil der Anleitung (ein QR-Code). Nach dem Löschen oder Vergessen eines Forwarders oder dem Ändern seiner Zeiten ohne Geheimzahl stellt **ZR** ihn einige Sekunden lang wieder her (mit derselben Programm-ID neu installiert, oder die Zeiten wie vorher).
- **PlayGuard** mit einem Druck: sein eigenes Symbol und sein Name, der Forwarder startet die `.nro`, von der aus PlayGuard geöffnet wurde.
- **Ein anderes Homebrew**: jede `.nro` aus dem Ordner `/switch` der SD-Karte (und einen Ordner tiefer), zuerst wird nach ihrem Namen gefragt (dem der `.nro`, zum Behalten oder Ändern), dann bestätigt mit dem, was das HOME-Menü zeigen wird: ihr Symbol, ihr Name und ihr Herausgeber. Ihr Symbol ist das der `.nro`, umgewandelt in das JPEG mit 256 × 256, das das HOME-Menü anzeigt, wenn es eine andere Größe hat oder ein PNG ist (ein Controller an seiner Stelle, wenn die `.nro` keines hat), wie es sphaira tut. Ein Pfad mit einem Leerzeichen wird dem Homebrew in Anführungszeichen übergeben.
- **Die Forwarder im HOME-Menü**, jeder mit seinem Symbol: die, die PlayGuard erstellt hat (Ⓐ: Details, umbenennen, den Herausgeber ändern, neu installieren, löschen), und die einer anderen App (Details, löschen). Einer, dessen `.nro` verschwunden ist, sagt das; einer, der in den Systemeinstellungen gelöscht wurde, bleibt aufgeführt, bis er neu installiert oder vergessen wird. Neu installieren behält seine Programm-ID, sodass der Eintrag im HOME-Menü ersetzt und nicht verdoppelt wird; ein Forwarder, den ein anderes Werkzeug für dieselbe `.nro` erstellt hat, wird ebenfalls ersetzt.
- **Mit der Geheimzahl geschützt** (Ⓐ › *Mit der Geheimzahl schützen*, für einen von PlayGuard erstellten außer seinem eigenen): Über das HOME-Menü gestartet, öffnet der Forwarder zuerst ein kleines Tor (`config/playguard/gate.nro`, ein paar hundert Kilobyte, die PlayGuard aus seinen eigenen Dateien schreibt), das sofort den Geheimzahl-Bildschirm der Konsole zeigt, dann das Homebrew, sobald die Geheimzahl stimmt; B dort führt zurück ins HOME-Menü. Um den Geheimzahl-Bildschirm herum das eigene des Tors: Symbol und Name des Homebrews, *Protected by PlayGuard* und wie es ausging (Start, in seinen freien Zeiten, Geheimzahl nicht eingegeben), in der Sprache der Konsole, auf dem Nachtblau von Cadran (`make gate-preview` zeichnet es auf einem Computer). Er behält seine Programm-ID und sein Symbol, die Zeile zeigt *Geheimzahl*, und der Änderungsverlauf hält es fest. Er braucht eine Geheimzahl auf der Konsole. *Nicht mehr nach der Geheimzahl fragen* fragt zuerst nach ihr, egal was *PlayGuard mit der Geheimzahl schützen* vorsieht. Er schützt nur das Symbol im HOME-Menü: Die `.nro` startet weiterhin über das Homebrew-Menü, außer es ist gesperrt (unten). Ein Forwarder, den ein früheres PlayGuard geschützt hat, startet PlayGuard selbst als sein Tor, was weiterhin funktioniert; *Alle neu installieren* stellt ihn auf das kleine Tor um.
  - **Zeiten ohne Geheimzahl** (Ⓐ auf einem geschützten): jeden Tag, Montag bis Freitag oder am Wochenende, von einer Uhrzeit bis zu einer anderen (auch über Mitternacht); die Zeile sagt, wann. Sie festzulegen fragt nach der Geheimzahl.
  - **Zeiten, in denen er nie öffnet** (Ⓐ auf einem geschützten), genauso festgelegt: Dann sagt das Tor *Nicht zu dieser Zeit* und die Zeiten und führt zurück ins HOME-Menü, selbst mit eingegebener Geheimzahl (Schlafenszeit für Homebrew). Sie gehen den Zeiten ohne Geheimzahl vor.
  - **Eine Pause nach abgelehnten Geheimzahlen** (*Pause nach abgelehnten Geheimzahlen*, unter *Homebrew-Menü*): fünf Ablehnungen hintereinander (B auf dem Geheimzahl-Bildschirm), und der Forwarder fragt 15 Minuten lang nicht mehr, wobei das Tor sagt, bis wann; 3, 5 oder 10 Versuche und 15 Minuten oder eine Stunde, oder nie. Die Glocke sagt, welcher Forwarder pausiert, seine Zeile ebenfalls; *Pause aufheben* (Ⓐ darauf, nach der Geheimzahl gefragt) lässt ihn sofort wieder fragen. Jede Pause und jede aufgehobene steht im Änderungsverlauf.
  - **Jeder Versuch steht im Änderungsverlauf**: mit der Geheimzahl geöffnet, in seinen Zeiten geöffnet oder abgelehnt (B auf dem Geheimzahl-Bildschirm), zu der Uhrzeit, zu der es geschah; die Versuche einer Art an einem Tag für einen Forwarder ergeben einen Eintrag, mit ihren Uhrzeiten. Das Tor notiert sie auf der SD-Karte; PlayGuard liest sie bei seinem nächsten Start. *Verlauf › Anzeigen › Homebrew und Forwarder* listet nur diese, die Forwarder und die Sperre auf.
  - **Der Bildschirm des Tors sagt mehr:** seine Zeiten ohne Geheimzahl, während er fragt, und die bisherigen Ablehnungen des Tages (in Bernstein), während er fragt und nach einer Ablehnung, unter *Bitte einen Elternteil, die Geheimzahl einzugeben*.
- **Sein Symbol aus dem Album** (Ⓐ › *Symbol ändern*): ein Screenshot aus dem Album (die neuesten 30, nach Datum), dessen mittleres Quadrat zum JPEG mit 256 × 256 des HOME-Menüs wird; oder wieder das der `.nro`. Neu installieren behält es.
- **Symbole und Sprachen:** Ein Symbol einer anderen Größe, ein PNG oder ein JPEG mit EXIF-Daten oder progressiver Kodierung (das HOME-Menü zeigt diese als „?“) wird zu einem einfachen JPEG mit 256 × 256. Die Steuerdaten des Forwarders führen nur amerikanisches Englisch auf, die Sprache seines einzigen Symbols: So erscheint das Symbol auf einer Konsole, die auf eine beliebige Sprache eingestellt ist.
- **Alle Forwarder mit der Geheimzahl schützen** und **Alle neu installieren** (Symbol und Version einer neuen `.nro`, das Tor, wie dieses PlayGuard es mitbringt, ein aus dem HOME-Menü gelöschter wieder da): mit je einem Druck.
- **Homebrew-Menü sperren** (*Homebrew-Menü*, zuletzt auf dem Bildschirm): Weder das Album noch ein Spiel, das bei gedrückter R-Taste gestartet wird, öffnet noch das Homebrew-Menü, sodass ein Homebrew nur noch über seinen Forwarder startet, der nach der Geheimzahl fragen kann.
  - **Schritt für Schritt:** Solange es nicht gesperrt werden kann, listet Ⓐ die Schritte auf, nummeriert, den nächsten in Bernstein und einen Druck entfernt (*Nächster Schritt*): eine Geheimzahl, die Signatur-Patches, der Forwarder von PlayGuard, ein mit der Geheimzahl geschütztes Homebrew-Menü (sphaira, in `/switch` gefunden, sonst hbmenu, in einem Zug geschützt erstellt), einmal über das HOME-Menü gestartet; die Liste kommt nach jedem Schritt zurück, der einen Forwarder erstellt.
  - Es braucht zwei Rückwege zu PlayGuard aus dem HOME-Menü: seinen eigenen Forwarder und einen mit der Geheimzahl geschützten Forwarder eines Homebrew-Menüs (sphaira oder hbmenu, nach dem Namen der `.nro`), der jedes Homebrew, einen Homebrew-Store eingeschlossen, hinter der Geheimzahl hält. Dazu eine Geheimzahl auf der Konsole und die Signatur-Patches, die Forwarder brauchen. Bis dahin sagt Ⓐ, was fehlt.
  - Der Rückweg über das Menü zählt, sobald er nachweislich funktioniert hat: einmal über das HOME-Menü gestartet, die Geheimzahl an seinem Tor eingegeben (das Protokoll des Tors sagt es, gelesen, wenn PlayGuard öffnet oder sperrt). Ein geschütztes, nie so gestartetes Menü wird als solches aufgeführt.
  - Sperren zeigt die Prüfungen als Liste (ein türkises Häkchen für das, was bereit ist, ein bernsteinfarbenes oder korallenrotes Dreieck für das, was es nicht ist) und verlangt, Ⓐ 2 Sekunden lang auf *Sperren* zu halten (oder einen Finger darauf), während sich ein Balken füllt; früher losgelassen, ändert sich nichts. Dann kommt das **Notfallblatt** (auch in `config/playguard/rescue.html` geschrieben, eine Seite zum Öffnen und Drucken auf einem Computer, in der angezeigten Sprache, mit dem QR-Code): wie man es von einem Computer aus rückgängig macht, oder mit dem Rettungsmodul, wenn es installiert ist, und ein QR-Code zur Hilfe ([unten](troubleshooting.md#homebrew-menü-gesperrt)); *Homebrew-Menü prüfen* hat es auch.
  - Auf emuMMC (oder einem sysMMC mit eingerichtetem emuMMC) warnt es, dass die Sperre auf der SD-Karte liegt, also beide Systeme sie lesen, während die Forwarder zum jeweiligen System gehören: Das andere System hat keinen dieser Rückwege.
  - Solange es gesperrt ist, wird das Löschen des eigenen Forwarders von PlayGuard oder des letzten geschützten Menüs (oder das Entfernen seiner Geheimzahl) abgelehnt: zuerst entsperren. Das Löschen eines von mehreren geschützten Menüs fragt zuerst nach, in Korallenrot, und nennt die, die bleiben.
  - Vor dem Sperren bietet es an, die von PlayGuard erstellten, noch nicht geschützten Forwarder in einem Zug zu schützen, und sagt, welche noch ohne Geheimzahl Homebrew starten würden (ein nicht geschütztes Menü oder eines, das eine andere App erstellt hat), welche auf die SD-Karte schreiben (ein Dateimanager, ein Installationsprogramm wie Cyberfoil oder Goldleaf, ein FTP-Server: Sie könnten die Datei der Sperre ändern) und wann uLaunch installiert ist (es startet Homebrew auf eigenem Weg, den die Sperre nicht erreicht).
  - Die Geheimzahl, die an einem Tag dreimal oder öfter am Tor eines Forwarders abgelehnt wurde, meldet die Glocke (welcher Forwarder, der Tag, wie oft).
  - Wenn das Schreiben einer Datei fehlschlägt, sagt der Fehler, warum, wenn er es erkennen kann: die SD-Karte voll, oder gar nichts darauf geschrieben (ihr Schreibschutzschalter, oder das Archiv-Bit, das *Tools* von hekate korrigiert).
  - *Homebrew-Menü prüfen* zeigt, was Atmosphère lesen wird (ob das Album und ein mit R gestartetes Spiel das Homebrew-Menü öffnen), die Rückwege, die Signatur-Patches, die Atmosphère-Version, unter der gesperrt wurde, und das Rettungsmodul; der Diagnosebericht enthält dasselbe.
  - Es sind ein paar Zeilen am Ende von `atmosphere/config/override_config.ini`, zwischen zwei Kommentaren von PlayGuard; der Rest der Datei bleibt, wie er ist, und wird so, wie er war, in `config/playguard/override_config.ini.backup` aufbewahrt. Atmosphère liest die Datei bei jedem Start eines Programms: kein Neustart. Entsperren entfernt die Zeilen und schreibt die eigenen Werte von Atmosphère für die Schlüssel zurück, die der Rest der Datei nicht festlegt (Atmosphère behält, was es gelesen hat, bis die Konsole neu startet).
  - Sperren und Entsperren fragen nach der Geheimzahl, egal was *PlayGuard mit der Geheimzahl schützen* vorsieht, und stehen im Änderungsverlauf. Solange es gesperrt ist, prüft PlayGuard bei jedem Start, dass beide Rückwege noch da sind; die Glocke sagt, wenn einer fehlt. Eine außerhalb von PlayGuard aus der Datei entfernte Sperre (ein Computer, ein Homebrew) meldet die Glocke und der Änderungsverlauf; PlayGuard setzt sie nie von selbst wieder, denn das Löschen der Datei ist der Rückweg von einem Computer aus. Das Löschen eines Forwarders (Systemeinstellungen › Datenverwaltung, wo keine Geheimzahl abgefragt wird) hebt die Sperre nie auf: Es nimmt nur einen Rückweg weg.
  - Von einem Computer aus: `atmosphere/config/override_config.ini` löschen (oder die Sicherung zurücklegen), dann die Konsole neu starten.
  - Mit installiertem Rettungsmodul (optional, [sysmodule/README.md](../../sysmodule/README.md), auf Englisch) hebt eine `RESCUE`-Anfrage sie ebenfalls beim Start auf, und ebenso ein Update von Atmosphère seit dem Sperren (seine Signatur-Patches können hinterherhinken, und ohne sie startet kein Forwarder): Der nächste Start von PlayGuard sagt es und bietet an, wieder zu sperren.
- **Ein außerhalb von PlayGuard gelöschter Forwarder** (in den Systemeinstellungen): einmal im Änderungsverlauf gemeldet, und in der Glocke, solange es andauert, mit *Neu installieren* oder *Vergessen* in seiner Zeile.
- **Signatur-Patches:** Die Konsole startet einen Forwarder nur mit Signatur-Patches für fs und loader (sys-patch oder sigpatch-Dateien): Ohne sie sagt der Bildschirm das über der Liste (*Konsole › Spiel-Patches* hat die Details). Die `.nro` bleibt, wo sie ist; das Löschen eines Forwarders berührt sie nie.
- **Spielzeit:** Von seinem Symbol aus gestartet, läuft ein Homebrew wie ein Spiel: Der Timer zählt es und unterbricht es wie jedes Spiel, sobald die Zeit um ist, und *Aktivität* führt es auf. PlayGuard bleibt über das Homebrew-Menü erreichbar.
- **In den Sicherungen der Einstellungen** (*Verlauf › Sicherungen*): was PlayGuard erstellt hat und ob das Homebrew-Menü gesperrt war. Eine Sicherung wiederherzustellen bietet nach dem Rest an, die Forwarder zurückzulegen, die nicht mehr im HOME-Menü sind (geschützt wie zuvor, mit ihren Zeiten und ihrem Symbol); eine Sperre wird nur gemeldet und ist von hier aus erneut zu setzen.
- Jede Änderung fragt nach der Geheimzahl, wenn *PlayGuard mit der Geheimzahl schützen* das vorsieht, wird im schreibgeschützten Modus abgelehnt und steht im Änderungsverlauf. Was PlayGuard erstellt hat, wird pro System (sysMMC / emuMMC) in `forwarders.json` gespeichert.

### Die Zeilen der Konsole

- **Firmware, Atmosphère und hekate** (wenn seine `bootloader/update.bin` auf der SD-Karte liegt), jedes mit der neueren Version, die die letzte Update-Suche gefunden hat, in Bernstein (*22.0.0 · 23.0.1 verfügbar*, und wie viele Versionen zurück, wenn es mehr als eine ist), und ein Warnsymbol in der Zeile der Firmware, wenn die neueste Firmware zuerst ein neueres Atmosphère oder hekate braucht oder von ihnen noch nicht unterstützt wird (die Versionshinweise der Firmware sagen, warum: Deren Versionshinweise sagen, welche Firmware sie unterstützen).
- **Versionshinweise:** Ⓐ auf jedem öffnet sie: ein Tab für die installierte Version, einer für jede neuere (höchstens fünf) und ältere in den übrigen Tabs (sechs insgesamt; Ⓛ / Ⓡ wechseln zwischen ihnen), geöffnet bei der nächsten, mit Etiketten (installiert oder neuer, die neueste unterstützte Firmware, Sicherheitskorrekturen, Jugendschutz geändert), den Hinweisen wie veröffentlicht (auf Englisch), *Online ansehen* (ein QR-Code) und wann sie abgerufen wurden. Die von Atmosphère und hekate kommen aus ihren GitHub-Releases, die der Firmware von switchbrew; sie werden auf der SD-Karte zum Offline-Lesen aufbewahrt und einmal am Tag neu abgerufen (Ⓧ sofort).
- **Kompatibilität**, dann **Speicher** (emuMMC oder sysMMC, mit einem Infosymbol, wenn das andere System seinen eigenen Jugendschutz hat: Ⓐ erklärt es).
- Ob Atmosphère **die Seriennummer anonymisiert** (teilweise verborgen bis Ⓐ; auf emuMMC, wenn nicht, ein Warnsymbol in der Zeile darüber, das Ⓐ erklärt).
- **Spiel-Patches** (sys-patch oder Sigpatch-Dateien, mit der Empfehlung von sys-patch, wenn nur Dateien verwendet werden: ein Infosymbol, oder ein Warnsymbol, wenn etwas einen Blick braucht, das Ⓐ erklärt).

## Optionen

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/?tab=preferences)*

- Was PlayGuard selbst einrichtet:
  - Sprache (jede Sprache der Konsole, oder die der Konsole), Design (standardmäßig dunkel, hell oder das der Konsole), Textgröße (automatisch / normal / größer): alle drei sofort.
  - Der Tab, mit dem gestartet wird.
  - **Das Intro** beim Start: der Ladebildschirm für 2 Sekunden (standardmäßig an; Ⓐ oder Ⓑ überspringt es; aus öffnet PlayGuard, sobald es bereit ist).
- Die Meldungen, die es zeigt:
  - Die Arten von Benachrichtigungen (siehe [oben](#wie-sich-einträge-verhalten)).
  - Eine **monatliche Erinnerung, PlayGuard zu unterstützen** (standardmäßig an, nie im ersten Monat und nicht direkt nach einem Update; *Nicht mehr anzeigen* auf der Erinnerung oder dieser Schalter schaltet sie endgültig aus, auch nach Updates).
- Eine Einstellung, die eine Funktion prägt, steht bei ihr: Zusatzzeit in *Spielzeit*, das Wiederaktivieren des Jugendschutzes in *Geheimzahl*, die Prüfung der Uhr in *Konsole › Netzwerkuhr*. Die Updates und *Hilfe & Diagnose* sind in *Info*.

### Änderungen und Sicherungen

Am Ende von *Optionen* (der Tab Verlauf früherer Versionen).

#### Änderungsverlauf

- Die neuesten 500 Änderungen von PlayGuard (Limits, Einschränkungskategorie, Geheimzahl, Aufheben, Trennen, die Uhr, Wiederherstellungen, Pausieren oder Fortsetzen des Countdowns …) und was es außerhalb von sich geändert gesehen hat, nach Tag gruppiert (*Heute*, *Gestern*, dann Wochentag und Datum), mit der Uhrzeit und woher; ein Limit, das für einen Tag geändert wurde, nennt diesen Tag.
- Eine Änderung, die die Konsole **abgelehnt** hat, steht auch da, in Bernstein, mit dem Fehler der Konsole und dem, was versucht wurde (nie wiederherstellbar: Es hat sich nichts geändert).
- *Anzeigen* behält eine Art (Spielzeit, Einschränkungen, Sicherheit und App, außerhalb von PlayGuard bemerkt, von der Konsole abgelehnt); *Angezeigtes exportieren* schreibt sie nach `exports/` als CSV, JSON, XLSX oder PDF.
- Ⓐ auf einer Änderung zeigt sie und **stellt** bei einem Wert **den vorherigen wieder her** — mit demselben Aufheben und derselben Geheimzahl wie jede Änderung, und sagt, ob er sich inzwischen geändert hat.
- Jede Änderung behält die PlayGuard-Version, die sie gemacht hat; eine Änderung, die von einer neueren Version von PlayGuard aufgezeichnet wurde, erscheint als *Andere Änderung*.

#### Sicherungen

- **Einstellungen sichern / wiederherstellen** auf der SD-Karte: Einschränkungskategorie, nutzerdefinierte Einstellungen, VR-Modus, Alterseinstufungssystem, Tageslimits, der Alarm „Zeit ist um“ (mit eingeschalteten erweiterten Aktionen) und der rohe Block des Timers zur Dokumentation — nie die Geheimzahl.
- Eine Wiederherstellung listet nur auf, was sich ändern würde (nichts zu ändern: Sie sagt es einfach, ohne Aufheben und ohne etwas zu schreiben), und sagt, wenn in der Sicherung ein Gute-Nacht-Alarm an war (er wird nicht zurückgeschrieben).
- Wähle, wie viele Sicherungen aufbewahrt werden.

## Info

*[Im Simulator ausprobieren ›](https://playguard.link/simulator/?tab=about)*

### Version und Updates

- Das installierte PlayGuard: Version (in Bernstein, wenn eine neuere erschienen ist; Ⓐ öffnet ihre **Versionshinweise**), Startmodus und Datenordner.
- **Nach Updates suchen** sofort; *Täglich nach Updates suchen* beim Start.
- *Aktualisieren mit*: in PlayGuard, das die Version herunterlädt, prüft, sich selbst ersetzt und neu startet, oder sphaira, Homebrew App Store oder von Hand (beim ersten Update gefragt, wenn nichts gewählt ist). Aus dem Homebrew App Store oder dem App Store von sphaira installiert, ist der Store dort der empfohlene Weg, damit er die Version im Blick behält; wenn sein Katalog die neue Version noch nicht hat (er veröffentlicht eine nach einer Prüfung, oft ein paar Tage später), sagt PlayGuard das und bietet stattdessen an, direkt zu aktualisieren.
- Solange eine neuere Version bekannt ist, öffnet PlayGuard mit **PlayGuard X.Y.Z ist verfügbar** (ihr Datum und ihre Hinweise): *Aktualisieren*, oder *Später* für 1 Tag, 1 Woche oder höchstens 30 Tage.
- Die Versionshinweise sind die von PlayGuard selbst, auf demselben Bildschirm wie die von Atmosphère und hekate in *Konsole*: aus seinen GitHub-Releases, ein Tab pro Version (die installierte, die neueren, die älteren), jede mit ihren Highlights als Karten und jeder Änderung, auf der SD-Karte zum Offline-Lesen aufbewahrt (die der installierten Version sind auch in der App, für den Fall, dass noch nichts abgerufen wurde).
- Nach einem Update öffnet PlayGuard einmal mit **Neu in X.Y.Z** (dieselben Highlights, dann die QR-Codes).

### Hilfe & Diagnose

*Hilfe & Diagnose ›*, ein eigener Bildschirm, unter *Aktualisieren mit*:

- **Erste Schritte** öffnet die Anleitung erneut (mit einem Schritt *Smart-Geräte-App trennen*, solange sie verknüpft ist, einem Schritt *Alarm „Zeit ist um“ wieder einschalten*, solange er aus ist, einem Schritt *PlayGuard schützen*, sobald eine Geheimzahl festgelegt ist — die Wahl *PlayGuard mit der Geheimzahl schützen* —, und einem Schalter, damit sie beim Start nicht mehr erscheint). Unter *Schließen* zeigt *PlayGuard unterstützen* die QR-Codes zur Unterstützung.
- **Diagnosebericht exportieren** oder **online senden** (siehe [Einen Fehler melden](troubleshooting.md#einen-fehler-melden)); wie viele gespeicherte Berichte, Absturzdateien, Sitzungsprotokolle und Links online gesendeter Berichte aufbewahrt werden, nach Anzahl, Alter oder Größe.

### Danksagungen

Die Danksagungen, wie man **PlayGuard unterstützt** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), als QR-Codes zum Scannen mit einem Handy) und ein kleines *Made in France* 🇫🇷.

## Sicher durch Design

**Das Spielzeitlimit sicher schreiben.** Die Konfiguration des Timers zu überschreiben, während er herunterzählt, bringt Atmosphère aus dem Gleichgewicht. Deshalb prüft PlayGuard zuerst den Zustand; ist der Timer aktiv, sagt die Bestätigung, dass der Jugendschutz vorübergehend aufgehoben wird — mit der gespeicherten Geheimzahl, **du musst sie dir nicht merken**. Ein Druck hebt dann auf, prüft, dass das System das Aufheben wirklich meldet, schreibt und **aktiviert sofort wieder** (oder bietet es an, wenn *Nach einer Änderung automatisch wieder aktivieren* aus ist). Die Dienstschicht prüft den Zustand direkt vor dem Schreiben erneut, sodass kein Bildschirm die Schutzmaßnahme umgehen kann.

> [!CAUTION]
> Ein Limit unter der heute schon gespielten Zeit unterbricht das Spiel, sobald der Jugendschutz wieder aktiviert ist. Die Bestätigung sagt das, wenn es passieren würde.

**Keine Abstürze mehr mit 22.5.** `pctl:a`, der privilegierte Jugendschutzdienst, nimmt **nur eine einzige Sitzung** an. Ältere Builds der ursprünglichen App hielten sie offen, sodass die Geheimzahl-Abfrage des HOME-Menüs (oder das Geheimzahl-Applet) sie nicht bekam und Atmosphère abstürzen konnte. PlayGuard öffnet die Sitzung für jede Aktion und gibt sie sofort wieder frei; regelmäßige Aktualisierungen pausieren, während die App im Hintergrund ist. *(Diagnose durch [den Fork von anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Nichts läuft im Hintergrund.** Das Limit, die Geheimzahl, die Warnungen und die Unterbrechung sind die der Konsole; PlayGuard ändert nur ihre Einstellungen.

## Was als Spielzeit zählt

Der Timer ist der der Konsole: PlayGuard liest ihn, es zählt selbst nichts. Was bekannt ist, und wie sicher ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), auf Englisch, hat die Messungen):

| Situation | Zählt? | Wie sicher |
|---|---|---|
| Ein laufendes Spiel | Ja | Gemessen (22.0.0) |
| **Ein im HOME-Menü offen gelassenes Spiel** (im Hintergrund unterbrochen) | **Ja**: Schließe es oder versetze die Konsole in den Standby-Modus | Von Eltern berichtet ([Arqade](https://gaming.stackexchange.com/questions/398954/)); passt zu den Messungen (die Konsole zählt, solange eine Anwendung offen ist) |
| Standby-Modus | Nein | Berichtet ([Arqade](https://gaming.stackexchange.com/questions/398954/)); von PlayGuard nicht gemessen |
| **PlayGuard über einem Spiel geöffnet** (als Anwendung) | **Ja**, als dieses Spiel | Gemessen (22.0.0): Öffne es mit dem Benutzer eines Elternteils, oder für einen kurzen Blick aus dem Album |
| PlayGuard aus dem Album geöffnet (Applet-Modus) | Nein | Abgeleitet: Es ist keine Anwendung, und Zeit außerhalb von Anwendungen schien nicht zu zählen |
| Während der Jugendschutz vorübergehend aufgehoben ist | Vielleicht nicht | Nicht gemessen: noch offen |
| Nachdem die Uhr geändert wurde | Der Tag beginnt neu; nach einem Sprung zurück hält die Zählung an, bis die Uhr die alte Uhrzeit überschreitet | Beobachtet (22.0.0); *Heute* sagt, wenn die Konsole nicht zählt |

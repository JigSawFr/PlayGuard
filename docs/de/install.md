<!-- translated from docs/install.md, sha256 8e4d0c633103 -->
# Installation

Wie du PlayGuard installierst, aktualisierst und entfernst, und was es auf die SD-Karte schreibt. Nach der Installation: [Einstieg](getting-started.md). Zurück zum [README](../../README.md).

## Wo du es bekommst

Wähle eine Möglichkeit:

- **Homebrew App Store** / **App Store von sphaira** (derselbe Katalog) — Suche nach *PlayGuard* und installiere es. Neue Versionen werden automatisch übernommen, nach einer Prüfung: oft ein paar Tage nach GitHub. PlayGuard weiß, dass es von dort installiert wurde, und empfiehlt, über den Store zu aktualisieren.
- **Manuell** — Lade `playguard.zip` aus der [neuesten Version](https://github.com/JigSawFr/PlayGuard/releases/latest) herunter und entpacke es ins **Hauptverzeichnis** der SD-Karte. Die App landet in `sd:/switch/playguard/`, ihre Dateien in `sd:/config/playguard/`.
- **sphaira › GitHub** — Das Zip der Version enthält den Eintrag bereits (`/config/sphaira/github/playguard.json`): Nach einer ersten Installation aktualisierst du in sphaira über *GitHub*.

Das optionale **Wiederherstellungs-Sysmodul** (`playguard-rescue.zip`) wird separat heruntergeladen: Entpacke es ebenfalls ins Hauptverzeichnis der SD-Karte und starte dann die Konsole neu. Es landet in `atmosphere/contents/4200000000505247/` und wirkt nur, wenn eine Datei `RESCUE` es verlangt — siehe [Ausgesperrt?](troubleshooting.md#ausgesperrt).

Öffne PlayGuard dann **über ein Spiel**, mit dem Benutzer eines Elternteils ([warum](compatibility.md#launcher)).

## Aktualisieren

- **In PlayGuard:** *Optionen › Aktualisieren mit* wählt, wie — in PlayGuard selbst (es lädt die Version herunter, prüft sie, ersetzt sich selbst und startet neu), sphaira, der Homebrew App Store oder von Hand. Mit *Täglich nach Updates suchen* an öffnet PlayGuard mit der neuen Version, wenn es eine gibt.
- **Das Wiederherstellungs-Sysmodul** folgt der Version von PlayGuard: Entpacke nach jedem Update die `playguard-rescue.zip` dieser Version erneut (oder *Aktualisieren* auf *Das Wiederherstellungs-Sysmodul ist veraltet* in der Glocke) und starte die Konsole dann neu.
- Einstellungen, Verlauf, Profile und Sicherungen bleiben erhalten: Sie liegen in `sd:/config/playguard/`, nicht neben der App.

## Entfernen

Lösche `sd:/switch/playguard/` (die App) und, um auch ihre Dateien zu entfernen, `sd:/config/playguard/`; mit dem Wiederherstellungs-Sysmodul auch `sd:/atmosphere/contents/4200000000505247/`. Die Limits, die Geheimzahl und die Einschränkungen gehören der Konsole selbst: Sie **bleiben auf der Konsole**. Um auch sie zu entfernen, verwende vorher *Sicherheit › Alle Altersbeschränkungen löschen*.

## Dateien auf der SD-Karte

Alle in `sd:/config/playguard/` (die App selbst bleibt in `sd:/switch/playguard/`: Wenn du sie entfernst oder neu installierst, bleiben die Dateien erhalten). Versionen, die sie neben der App ablegten, haben sie beim ersten Start dorthin verschoben.

- **`config.json`** — Einstellungen (Sprache, Design, NTP-Server, *PlayGuard mit der Geheimzahl schützen* …); jeder Schlüssel in [config.md](../config.md) (auf Englisch). Eine Datei, die nicht gelesen werden kann, wird als `config.json.bad` aufbewahrt (die vorherige als `.bad.1`), und die Einstellungen beginnen wieder bei ihren Standardwerten; PlayGuard meldet das einmal beim Start
- **`profiles/`** — Gespeicherte Profile für das Spielzeitlimit
- **`exports/`** — Exporte der Aktivität
- **`nand/<system>/`** — Was zu einem System gehört, jedes mit seinem eigenen Jugendschutz: `sysmmc/` für die sysMMC, `emummc/` für die emuMMC. Darin: `history.json` der Änderungsverlauf (die neuesten 500, auch Änderungen, die die Konsole abgelehnt hat; eine Datei, die nicht gelesen werden kann, wird als `history.json.bad` aufbewahrt, die vorherige als `.bad.1`, statt überschrieben zu werden, und PlayGuard meldet das einmal); `state.json` das, was *Alle Spiele sperren*, die Zusatzzeit, ein ausstehendes Wiederaktivieren und die Firmware-Wahl sich merken müssen ([config.md](../config.md#state-kept-between-launches), auf Englisch); `forwarders.json` die Forwarder, die PlayGuard erstellt hat; `watch.json` das, was PlayGuard zuletzt von der Konsole gesehen hat, um zu erkennen, was sich außerhalb geändert hat ([config.md](../config.md#watchjson), auf Englisch; sie zu löschen ist harmlos); `backups/` die Sicherungen der Einstellungen (enthalten nie die Geheimzahl); `cache/` die zuletzt gelesene Spielaktivität (alle Benutzer und jeder angesehene Benutzer), beim nächsten Start sofort angezeigt, während das Protokoll neu gelesen wird. Die Dateien einer Version vor diesen Ordnern gehen an das erste System, auf dem PlayGuard startet
- **`cache/`** — Die Spielsymbole (`icons/`, höchstens 16 MB, geleert, wenn sich die Sprache der Konsole ändert), gemeinsam für alle Systeme, damit Namen und Symbole nicht bei jedem Start erneut aus jedem Spiel gelesen werden; die Versionshinweise von Atmosphère, hekate, sys-patch und der Firmware (`sys_notes.json`, für *Konsole*) und die von PlayGuard selbst (für *Info*); im Entwicklermodus die Liste von *Anderen Build installieren* (`dev_builds.json`)
- **`github_token`** — Die GitHub-Anmeldung für *Bericht senden…* und, im Entwicklermodus, für *Anderen Build installieren*; wird beim Abmelden gelöscht (*Info › Hilfe & Diagnose › GitHub-Konto*, angezeigt, sobald du angemeldet bist)
- **`gate.nro`**, **`gate-<hash>.nro`**, **`gate_*.txt`** — Das Tor der geschützten Forwarder (*Konsole › Forwarder*): sein Programm, seine freien Zeiten, die Zeiten, in denen er nie öffnet, und die Pause nach Ablehnungen, seine Versuche, bis PlayGuard sie liest, und wie lange sein Startbildschirm dauert (`gate_wait.txt`, bei jedem Start gelesen)
- **`previous_build.json`**, **`boot_check.json`** — Nachdem ein Build an Ort und Stelle installiert wurde: der Name des ersetzten Builds (neben der `.nro` als `playguard.nro.previous` aufbewahrt) und die Prüfung beim nächsten Start (gelöscht, sobald ihr erster Bildschirm erscheint)
- **`rescue_report.txt`** — Vom Wiederherstellungs-Sysmodul hinterlassen, nachdem es eingegriffen hat, bis PlayGuard es beim Start anzeigt (einen Bericht, den es nicht lesen kann, entfernt es, und PlayGuard meldet das)
- **`rescue_nonce`** — Eine zufällige Aufgabe, die das Wiederherstellungs-Sysmodul in seinem Bericht beantwortet, damit PlayGuard einen Bericht, den es geschrieben hat, von einem von Hand geschriebenen unterscheiden kann ([sysmodule/README.md](../../sysmodule/README.md), auf Englisch); bei jedem Start eine neue
- **`logs/`** — `reports/` die gespeicherten Diagnoseberichte (enthalten nie die Geheimzahl oder die Seriennummer); `crashes/` eine Datei pro Absturz (was PlayGuard zum Anhalten gebracht hat); `uploads/` eine Internetverknüpfung (`.url`) pro online gesendetem Bericht; `play_timer/` die Spielzeit-Dateien der Entwicklerwerkzeuge. Wie viele Berichte, Absturzdateien und Links aufbewahrt werden: *Info › Hilfe & Diagnose* (nach Anzahl, Alter oder Größe; die neueste bleibt immer)

Mehr in [packaging/README.md](../../packaging/README.md) (auf Englisch).

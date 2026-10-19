<!-- translated from docs/install.md, sha256 f12b746bb6b9 -->
# Installatie

Hoe je PlayGuard installeert, en wat het op de SD-kaart schrijft. Terug naar de [README](../../README.md).

## Waar je het vindt

Kies er één:

- **Homebrew App Store** / **App Store van sphaira** (dezelfde catalogus) — Zoek naar *PlayGuard* zodra de vermelding is goedgekeurd. Nieuwe versies worden automatisch opgepikt.
- **sphaira › GitHub** — De zip van de release bevat de vermelding al (`/config/sphaira/github/playguard.json`): werk na een eerste installatie bij via *GitHub* in sphaira.
- **Handmatig** — Download `playguard.zip` van de [nieuwste release](https://github.com/JigSawFr/PlayGuard/releases/latest) en pak het uit in de **hoofdmap** van de SD-kaart. De app komt in `sd:/switch/playguard/`, zijn bestanden in `sd:/config/playguard/`.

De optionele **herstel-sysmodule** (`playguard-rescue.zip`) is een aparte download — zie [Buitengesloten?](troubleshooting.md#buitengesloten).

## Bestanden op de SD-kaart

Alles staat in `sd:/config/playguard/` (de app zelf blijft in `sd:/switch/playguard/`: als je die verwijdert of opnieuw installeert, blijven ze bewaard). Versies die ze naast de app bewaarden, hebben ze bij hun eerste start hierheen verplaatst.

- **`config.json`** — Voorkeuren (taal, thema, NTP-server, *PlayGuard beveiligen met de pincode* …); elke sleutel staat in [config.md](../config.md) (in het Engels). Een bestand dat niet te lezen is, wordt bewaard als `config.json.bad` (het vorige als `.bad.1`) en de instellingen beginnen opnieuw bij hun standaardwaarden; PlayGuard meldt dat één keer bij het opstarten
- **`profiles/`** — Opgeslagen profielen met speeltijdlimieten
- **`exports/`** — Exports van de activiteit
- **`nand/<system>/`** — Wat bij één systeem hoort, elk met zijn eigen ouderlijk toezicht: `sysmmc/` voor de sysMMC, `emummc/` voor de emuMMC. Daarin: `history.json` de wijzigingsgeschiedenis (de nieuwste 500, ook een wijziging die de console weigerde; een bestand dat niet te lezen is, wordt bewaard als `history.json.bad`, het vorige als `.bad.1`, in plaats van overschreven, en PlayGuard meldt dat één keer); `state.json` wat *Alle games blokkeren*, de extra tijd, een wachtende hervergrendeling en de firmwarekeuze moeten onthouden ([config.md](../config.md#state-kept-between-launches), in het Engels); `watch.json` wat PlayGuard het laatst van de console zag, om te zien wat erbuiten is gewijzigd ([config.md](../config.md#watchjson); verwijderen kan geen kwaad); `backups/` de back-ups van de instellingen (nooit met de pincode); `cache/` de laatst gelezen speelactiviteit (alle accounts, en elk bekeken account), die bij de volgende start meteen wordt getoond terwijl het log opnieuw wordt gelezen. De bestanden van een versie van vóór deze mappen gaan naar het eerste systeem waarop PlayGuard start
- **`cache/`** — De icoontjes van de games (`icons/`, hoogstens 16 MB, geleegd als de taal van de console verandert), gedeeld door alle systemen, zodat namen en icoontjes niet bij elke start opnieuw uit elke game worden gelezen; de release-opmerkingen van Atmosphère, hekate en de firmware (`sys_notes.json`, voor *Console*) en die van PlayGuard zelf (voor *Info*); in ontwikkelaarsmodus de lijst van *Andere build installeren* (`dev_builds.json`)
- **`github_token`** — De GitHub-aanmelding van *Rapport online versturen* en, in ontwikkelaarsmodus, van *Andere build installeren*; verwijderd door af te melden (*Info › Hulp en diagnose › GitHub-account*, zichtbaar zodra je bent aangemeld)
- **`previous_build.json`**, **`boot_check.json`** — Nadat een build ter plekke is geïnstalleerd: de naam van de vervangen build (bewaard naast de `.nro` als `playguard.nro.previous`), en de controle van de volgende start (verwijderd zodra het eerste scherm verschijnt)
- **`rescue_report.txt`** — Achtergelaten door de herstel-sysmodule nadat die iets heeft gedaan, tot PlayGuard het bij het opstarten toont (een rapport dat het niet kan lezen, wordt verwijderd, en PlayGuard meldt dat)
- **`rescue_nonce`** — Een willekeurige uitdaging die de herstel-sysmodule in zijn rapport beantwoordt, zodat PlayGuard een rapport dat het zelf schreef kan onderscheiden van een met de hand geschreven rapport ([sysmodule/README.md](../../sysmodule/README.md), in het Engels); bij elke start een nieuwe
- **`logs/`** — `reports/` de opgeslagen diagnoserapporten (nooit met de pincode of het serienummer); `crashes/` één bestand per crash (waardoor PlayGuard stopte); `uploads/` één internetsnelkoppeling (`.url`) per online verstuurd rapport; `play_timer/` de speeltijdtimerbestanden van de ontwikkelaarstools. Hoeveel rapporten, crashbestanden en links er bewaard blijven: *Info › Hulp en diagnose* (op aantal, leeftijd of grootte; het nieuwste blijft altijd)

Meer in [packaging/README.md](../../packaging/README.md) (in het Engels).

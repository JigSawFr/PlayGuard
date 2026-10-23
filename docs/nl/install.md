<!-- translated from docs/install.md, sha256 e4ba1fa1809a -->
# Installatie

Hoe je PlayGuard installeert, bijwerkt en verwijdert, en wat het op de SD-kaart schrijft. Eenmaal geïnstalleerd: [aan de slag](getting-started.md). Terug naar de [README](../../README.md).

## Waar je het vindt

> [!WARNING]
> PlayGuard staat **nog niet in de Homebrew App Store (en ook niet in die van sphaira)**: de vermelding wacht nog op goedkeuring. Installeer het voorlopig met de hand.

Kies er één:

- **Handmatig** (voorlopig aanbevolen) — Download `playguard.zip` van de [nieuwste release](https://github.com/JigSawFr/PlayGuard/releases/latest) en pak het uit in de **hoofdmap** van de SD-kaart. De app komt in `sd:/switch/playguard/`, zijn bestanden in `sd:/config/playguard/`.
- **sphaira › GitHub** — De zip van de release bevat de vermelding al (`/config/sphaira/github/playguard.json`): werk na een eerste installatie bij via *GitHub* in sphaira.
- **Homebrew App Store** / **App Store van sphaira** (dezelfde catalogus) — **Nog niet vermeld.** Zoek naar *PlayGuard* zodra de vermelding is goedgekeurd. Nieuwe versies worden dan automatisch opgepikt, na een controle: vaak een paar dagen na GitHub. PlayGuard weet dat het daarvandaan is geïnstalleerd en raadt aan om via de store bij te werken.

De optionele **herstel-sysmodule** (`playguard-rescue.zip`) is een aparte download: pak die ook uit in de hoofdmap van de SD-kaart en start de console daarna opnieuw op. Hij komt in `atmosphere/contents/4200000000505247/` en doet alleen iets als een `RESCUE`-bestand daarom vraagt — zie [Buitengesloten?](troubleshooting.md#buitengesloten).

Open PlayGuard daarna **boven een game**, met de gebruiker van een ouder ([waarom](compatibility.md#launchers)).

## Bijwerken

- **In PlayGuard:** *Voorkeuren › Bijwerken met* kiest hoe — in PlayGuard zelf (het downloadt de release, controleert hem, vervangt zichzelf en herstart), sphaira, de Homebrew App Store of met de hand. Met *Elke dag op updates controleren* aan opent PlayGuard op de nieuwe release als die er is.
- **De herstel-sysmodule** volgt de versie van PlayGuard: pak na elke update de `playguard-rescue.zip` van die release opnieuw uit (of *Bijwerken* bij *De herstel-sysmodule is verouderd* onder de bel), en start de console daarna opnieuw op.
- Instellingen, geschiedenis, profielen en back-ups blijven bewaard: ze staan in `sd:/config/playguard/`, niet naast de app.

## Verwijderen

Verwijder `sd:/switch/playguard/` (de app) en, om ook zijn bestanden te verwijderen, `sd:/config/playguard/`; met de herstel-sysmodule ook `sd:/atmosphere/contents/4200000000505247/`. De limieten, de pincode en de beperkingen zijn die van de console zelf: ze **blijven op de console**. Wil je ook die verwijderen, gebruik dan eerst *Beveiliging › Ouderlijk toezicht wissen*.

## Bestanden op de SD-kaart

Alles staat in `sd:/config/playguard/` (de app zelf blijft in `sd:/switch/playguard/`: als je die verwijdert of opnieuw installeert, blijven ze bewaard). Versies die ze naast de app bewaarden, hebben ze bij hun eerste start hierheen verplaatst.

- **`config.json`** — Voorkeuren (taal, thema, NTP-server, *PlayGuard beveiligen met de pincode* …); elke sleutel staat in [config.md](../config.md) (in het Engels). Een bestand dat niet te lezen is, wordt bewaard als `config.json.bad` (het vorige als `.bad.1`) en de instellingen beginnen opnieuw bij hun standaardwaarden; PlayGuard meldt dat één keer bij het opstarten
- **`profiles/`** — Opgeslagen profielen met speeltijdlimieten
- **`exports/`** — Exports van de activiteit
- **`nand/<system>/`** — Wat bij één systeem hoort, elk met zijn eigen ouderlijk toezicht: `sysmmc/` voor de sysMMC, `emummc/` voor de emuMMC. Daarin: `history.json` de wijzigingsgeschiedenis (de nieuwste 500, ook een wijziging die de console weigerde; een bestand dat niet te lezen is, wordt bewaard als `history.json.bad`, het vorige als `.bad.1`, in plaats van overschreven, en PlayGuard meldt dat één keer); `state.json` wat *Alle games blokkeren*, de extra tijd, een wachtende hervergrendeling en de firmwarekeuze moeten onthouden ([config.md](../config.md#state-kept-between-launches), in het Engels); `forwarders.json` de forwarders die PlayGuard heeft gemaakt; `watch.json` wat PlayGuard het laatst van de console zag, om te zien wat erbuiten is gewijzigd ([config.md](../config.md#watchjson); verwijderen kan geen kwaad); `backups/` de back-ups van de instellingen (nooit met de pincode); `cache/` de laatst gelezen speelactiviteit (alle accounts, en elk bekeken account), die bij de volgende start meteen wordt getoond terwijl het log opnieuw wordt gelezen. De bestanden van een versie van vóór deze mappen gaan naar het eerste systeem waarop PlayGuard start
- **`cache/`** — De icoontjes van de games (`icons/`, hoogstens 16 MB, geleegd als de taal van de console verandert), gedeeld door alle systemen, zodat namen en icoontjes niet bij elke start opnieuw uit elke game worden gelezen; de release-opmerkingen van Atmosphère, hekate, sys-patch en de firmware (`sys_notes.json`, voor *Console*) en die van PlayGuard zelf (voor *Info*); in ontwikkelaarsmodus de lijst van *Andere build installeren* (`dev_builds.json`)
- **`github_token`** — De GitHub-aanmelding van *Rapport versturen…* en, in ontwikkelaarsmodus, van *Andere build installeren*; verwijderd door af te melden (*Info › Hulp en diagnose › GitHub-account*, zichtbaar zodra je bent aangemeld)
- **`gate.nro`**, **`gate-<hash>.nro`**, **`gate_*.txt`** — De poort van de beveiligde forwarders (*Console › Forwarders*): zijn programma, zijn vrije uren, de uren waarin hij nooit opent en de pauze na weigeringen, zijn pogingen tot PlayGuard ze leest, en hoe lang zijn openingsscherm duurt (`gate_wait.txt`, bij elke start gelezen)
- **`previous_build.json`**, **`boot_check.json`** — Nadat een build ter plekke is geïnstalleerd: de naam van de vervangen build (bewaard naast de `.nro` als `playguard.nro.previous`), en de controle van de volgende start (verwijderd zodra het eerste scherm verschijnt)
- **`rescue_report.txt`** — Achtergelaten door de herstel-sysmodule nadat die iets heeft gedaan, tot PlayGuard het bij het opstarten toont (een rapport dat het niet kan lezen, wordt verwijderd, en PlayGuard meldt dat)
- **`rescue_nonce`** — Een willekeurige uitdaging die de herstel-sysmodule in zijn rapport beantwoordt, zodat PlayGuard een rapport dat het zelf schreef kan onderscheiden van een met de hand geschreven rapport ([sysmodule/README.md](../../sysmodule/README.md), in het Engels); bij elke start een nieuwe
- **`logs/`** — `reports/` de opgeslagen diagnoserapporten (nooit met de pincode of het serienummer); `crashes/` één bestand per crash (waardoor PlayGuard stopte); `uploads/` één internetsnelkoppeling (`.url`) per online verstuurd rapport; `play_timer/` de speeltijdtimerbestanden van de ontwikkelaarstools. Hoeveel rapporten, crashbestanden en links er bewaard blijven: *Info › Hulp en diagnose* (op aantal, leeftijd of grootte; het nieuwste blijft altijd)

Meer in [packaging/README.md](../../packaging/README.md) (in het Engels).

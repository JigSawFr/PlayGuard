<!-- translated from docs/compatibility.md, sha256 33251c660faf -->
# Compatibiliteit

Ondersteunde firmware, Atmosphère en launchers, emuMMC en sysMMC, en hoe PlayGuard zich verhoudt tot een vervangende sysmodule. Terug naar de [README](../../README.md).

## Ondersteunde versies

**Getest op een console** — PlayGuard is hierop gedraaid:

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira en de Homebrew App Store

**Zou moeten werken, niet getest** — niets in PlayGuard sluit deze uit, maar nog niemand heeft ze gemeld ([meldingen welkom](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Elke firmware heeft een Atmosphère nodig die hem opstart: de vermelde of een nieuwere.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Dezelfde opdrachten als 23.0.1 in de tabel van [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Alle functies.

**Vóór 21.0.0 alles behalve de speeltijdtimer** — beperkingen, pincode, tijdelijk ontgrendelen, activiteit, netwerkklok, smartphone-app en geschiedenis werken. Het instellingenblok van de speeltijdtimer (0x44 bytes) heeft deze opbouw pas sinds 21.0.0, dus zet PlayGuard uit wat erin staat: de daglimieten, het bedtijdalarm, extra tijd en het blokkeren van de console.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x en ouder** — Atmosphère 1.0.0 en ouder

PlayGuard controleert of Atmosphère draait, niet welke versie: wat telt, is de firmware die het opstart. *Console* toont beide.

## Goed om te weten

### Launchers

Starten boven een game (title override) wordt aanbevolen. De app zegt of hij als applicatie of als applet (album) draait. Boven een game telt de console de tijd in PlayGuard als tijd van die game. In de activiteit gaat die naar de gebruiker die bij het starten is gekozen: open het met de gebruiker van een ouder, niet van een kind. De speeltijdtimer is die van de console, dezelfde voor elke gebruiker: hij telt die tijd mee, wie PlayGuard ook opende (tenzij de timer uit staat).

### Nintendo Switch 2

Niet ondersteund: er bestaat geen Atmosphère voor.

### Nieuwere firmware

Boven de nieuwste firmware die op een console is getest, opent PlayGuard **alleen-lezen** en controleert het of een nieuwere release hem ondersteunt. Zo ja, dan stelt het voor om bij te werken (in PlayGuard zelf, via sphaira of de Homebrew App Store). Anders kies je: alleen-lezen, alleen-lezen met de ontwikkelaarstools (om de firmware te onderzoeken), of alle functies op eigen risico. De keuze kan worden onthouden voor die firmware en die versie van de app; *Console › Compatibiliteit* brengt het scherm terug.

### emuMMC en sysMMC

Elk heeft zijn eigen ouderlijk toezicht, en PlayGuard ziet alleen dat van het systeem waarop het draait. Op emuMMC melden *Console* en *Eerste stappen* dat; op sysMMC ook, als er een emuMMC op de SD-kaart is ingesteld. Stel het toezicht op beide in, of verberg het opstartmenu van hekate zodat een kind het andere systeem niet kan kiezen. PlayGuard houdt de geschiedenis, back-ups en toestand van elk systeem gescheiden (de sysMMC en de emuMMC, in `nand/`); de voorkeuren en profielen worden gedeeld.

### Kan de console hierdoor een ban krijgen?

PlayGuard neemt nooit contact op met Nintendo: online bereikt het alleen openbare tijdservers en GitHub (updates, en de rapporten die je verstuurt). Consoles krijgen een ban door custom firmware of een emuMMC die online gaat naar Nintendo ([FAQ van switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), in het Engels). Er is geen ban gedocumenteerd die samenhangt met wijzigingen aan ouderlijk toezicht of de klok; dat is geen garantie.

### Een console zonder internet

Daar is PlayGuard voor, en de speeltijdtimer heeft toch een netwerkklok nodig die is ingesteld. Zonder tijdserver om te raadplegen biedt *Console › Netwerkklok › Tijdsverschil meten* aan om hem in plaats daarvan op de consoleklok in te stellen: controleer eerst de consoleklok, met *Klok synchroniseren via internet* uit, en zet die instelling pas daarna aan (zie [netwerkklok](features.md#geen-enkele-server-antwoordt)).

### Meldingen op de telefoon

Sinds 22.0.0 kan een console die nog aan de app op de telefoon gekoppeld is, de ouder waarschuwen als de pincode op de console wordt ingevoerd ([switchbrew](https://switchbrew.org/wiki/22.0.0), in het Engels). Of de ontgrendelingen van PlayGuard dat ook doen, is niet bekend.

## Waarom PlayGuard

Kinderen houden van videogames, en videogames betekenen schermen: een limiet hoort bij goed voor ze zorgen. Op een ongewijzigde Switch regelde de app van Nintendo op de telefoon dat. Zodra de console custom firmware draait, gaat er een hele nieuwe wereld open — en verdwijnt het ouderlijk toezicht, omdat de app op de telefoon de console niet meer bereikt. Weten hoe lang ze speelden, een limiet instellen, de game laten stoppen zonder ruzie of nog een keer "nog vijf minuutjes": het werd lastig.

De paar hulpmiddelen die er waren, deden het niet goed: activiteitsoverzichten die nauwelijks werden onderhouden en weinig detail gaven, ouderlijk toezicht dat ruw en onaf was. En ik wilde ook geen zelfgemaakte vervanging, of een sysmodule die altijd op de achtergrond draait. Het idee was om **het ouderlijk toezicht van de console zelf zoveel mogelijk te hergebruiken**, met alles wat het al doet — de limiet, de pincode, de waarschuwingen, de onderbreking — en de instellingen ervan terug op de console te brengen. Dat is PlayGuard.

Door het toezicht van de console zelf aan te sturen, gaat ook de deur open naar veel meer: speeltijd doorgeven aan een server thuis, er opdrachten van ontvangen, Home Assistant, automatiseringen. Waar dat heen kan gaan, staat in [de horizon van de roadmap](../../ROADMAP.md#horizon) (in het Engels).

## PlayGuard of een vervangende sysmodule?

PlayGuard stuurt het ouderlijk toezicht aan dat in de console zit (de dienst `pctl`), offline. De andere aanpak vervangt het door een sysmodule, bijvoorbeeld [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (een sysmodule en een Tesla / Ultrahand-overlay).

| | PlayGuard | Een vervangende sysmodule |
|---|---|---|
| **Speeltijdlimiet** | Eén voor de console (de eigen timer van de console) | Eén per gebruikersaccount |
| **Pincode, waarschuwingen, onderbreking** | Die van de console | Zijn eigen |
| **Op de achtergrond** | Niets (de optionele herstel-sysmodule doet alleen iets bij het opstarten, als een `RESCUE`-bestand daarom vraagt) | Een sysmodule, vanaf het opstarten |
| **Na een firmware-update** | Het toezicht van de console blijft werken; PlayGuard opent alleen-lezen tot een versie de firmware ondersteunt | Het toezicht hangt ervan af of de sysmodule nog werkt |
| **Verwijderd van de SD-kaart** | De limieten blijven op de console | Het toezicht verdwijnt mee |

**Een andere limiet nodig voor elk kind?** Kies een vervangende sysmodule — de timer van de console heeft één limiet voor de hele console. Anders behoudt PlayGuard het toezicht van Nintendo en brengt het alleen de instellingen ervan naar de console.

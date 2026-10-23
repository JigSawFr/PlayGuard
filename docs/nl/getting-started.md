<!-- translated from docs/getting-started.md, sha256 150621760428 -->
# Aan de slag

Van de SD-kaart tot een console die voor je kind is ingesteld, in ongeveer tien minuten, en daarna de dagelijkse taken. Terug naar de [README](../../README.md).

## Voordat je begint

- **Een console met Atmosphère.** PlayGuard stuurt het eigen ouderlijk toezicht van de console aan via een afgeschermde dienst: het heeft custom firmware nodig. Niet gemod? Dan kan het niet helpen ([problemen oplossen](troubleshooting.md#console-niet-gemod)).
- **Een ondersteunde firmware.** 21.0.0 tot 23.0.1 voor alle functies; oudere: alles behalve de speeltijdtimer; een nieuwere opent alleen-lezen tot een release hem ondersteunt ([compatibiliteit](compatibility.md)).
- **Eén limiet voor de hele console.** De speeltijdtimer is die van de console: hij is dezelfde voor elke gebruiker. Voor verschillende limieten per kind heb je een ander hulpmiddel nodig ([waarom](compatibility.md#playguard-of-een-vervangende-sysmodule)).

Wil je eerst rondkijken? [Probeer PlayGuard in je browser](https://playguard.link/simulator/?not_set_up=1), met een gesimuleerde console bij zijn eerste start.

## 1. Installeer het

Pak `playguard.zip` van de [nieuwste release](https://github.com/JigSawFr/PlayGuard/releases/latest) uit in de hoofdmap van de SD-kaart ([installatie](install.md)). PlayGuard staat nog niet in de Homebrew App Store of sphaira: de vermelding wacht nog op goedkeuring.

> [!TIP]
> Pak ook `playguard-rescue.zip` uit, de optionele herstel-sysmodule. Die doet niets tot je hem nodig hebt, en is de enige weg terug als een limiet van 0 minuten alles blokkeert en de pincode vergeten is ([Buitengesloten?](troubleshooting.md#buitengesloten)).

## 2. Open het op de juiste manier

Open PlayGuard **boven een game** (houd R ingedrukt terwijl je een game start, met hbmenu, sphaira of de Homebrew App Store), met **de gebruiker van een ouder**:

- boven een game draait PlayGuard als applicatie, met al zijn geheugen; vanuit het album draait het als applet, met minder;
- de console telt de tijd in PlayGuard als tijd van die game, en geeft die aan de gebruiker die bij het starten is gekozen: die van een ouder, niet van een kind;
- zodra de pincode is ingevoerd, pauzeert PlayGuard ouderlijk toezicht zolang je erin bent, zodat die tijd niet van die van het kind afgaat; het gaat weer aan zodra je het verlaat.

## 3. Volg *Eerste stappen*

Bij de eerste start verschijnt de gids *Eerste stappen* (later: *Voorkeuren › Eerste stappen*). *Vandaag* toont dezelfde stappen als kaart tot ze gedaan zijn (*1 van 3*):

1. **De pincode.** De eigen pincode van de console, die het HOME-menu vraagt om na de limiet een game te starten of om de instellingen te wijzigen. Vermijd 0000, 1234 of 2580: PlayGuard markeert een pincode die iedereen als eerste zou proberen.
2. **Een daglimiet.** Elke dag dezelfde, of één voor doordeweeks en één voor het weekend. Wijzig later elke afzonderlijke dag in *Speeltijdtimer*.
3. **De netwerkklok.** De speeltijdtimer telt ermee. Een console die de servers van Nintendo nooit bereikt, houdt hem onnauwkeurig: zie stap 5.

## 4. Ontkoppel de app op de telefoon

Nog gekoppeld aan de app *Ouderlijk toezicht* van Nintendo (of een tweedehands console die aan die van iemand anders is gekoppeld)? *Beveiliging › Smartphone-app ontkoppelen*. Anders overschrijft de volgende synchronisatie van de app wat je in PlayGuard hebt ingesteld. Het werkt offline.

## 5. Stel de netwerkklok in

*Console › Netwerkklok › Tijdsverschil meten* raadpleegt drie openbare tijdservers, daarna *Netwerkklok instellen*. De klok instellen zet de speeltijd van vandaag terug op nul, dus doe het voordat het kind speelt.

Een console zonder internet: als geen enkele server antwoordt, biedt PlayGuard aan om de netwerkklok in plaats daarvan op de consoleklok in te stellen. Controleer eerst de consoleklok met een telefoon of een horloge, met *Systeeminstellingen › Systeem › Datum en tijd › Klok synchroniseren via internet* uit ([details](features.md#netwerkklok)).

## 6. Kies wat er gebeurt als de tijd om is

*Speeltijdtimer › Als de tijd om is*: de console toont "Tijd is om", en **onderbreekt dan de game** of **waarschuwt alleen**. Onderbreken stopt het spelen zonder ruzie, maar niet-opgeslagen voortgang kan verloren gaan; waarschuwen laat het aan het kind over. Voeg op hetzelfde tabblad een **bedtijdalarm** toe als je dat wilt.

## 7. Beveilig PlayGuard zelf

*Beveiliging › PlayGuard beveiligen met de pincode*:

- **Vóór een wijziging** (de standaard) — iedereen kan kijken, alleen de ouder wijzigt iets;
- **Om PlayGuard te openen** — een vergrendelscherm bij het opstarten;
- **Nooit** — PlayGuard meldt het bij de bel zolang er een pincode is ingesteld.

## 8. Geef de console door

*Vandaag › Aan het kind tonen* toont alleen de resterende tijd en wanneer het spelen eindigt, groot genoeg om vanaf de andere kant van de kamer te lezen. Ⓐ en de pincode brengen de schermen van de ouder terug.

> [!IMPORTANT]
> **Een game die open blijft in het HOME-menu, telt door.** Sluit hem, of zet de console in de slaapstand ([wat telt](features.md#wat-telt-als-speeltijd)).

## Dagelijkse taken

| Om… | Waar |
|---|---|
| Vandaag meer tijd te geven | *Vandaag › Extra tijd* (+15 min, +30 min, +1 u). De gewone limiet komt de volgende dag vanzelf terug. |
| Op een andere dag meer tijd te geven | *Vandaag › Extra tijd › Een andere dag…* |
| Te stoppen voor vandaag | *Vandaag › Stoppen voor vandaag*: nu, of over 5 minuten om op te slaan. |
| De limiet van één dag te wijzigen | *Speeltijdtimer*: Ⓐ op de dag, ↑/↓, dan **+** om op te slaan. |
| Te wisselen tussen schooltijd en vakantie | *Speeltijdtimer › Profielen*; *Gepland profiel* voor een vooraf gekozen periode. |
| Te zien wie wat speelde | *Activiteit*: per game, per account, vandaag, 7 dagen, altijd; exporteren. |
| Voorlopig alle games te blokkeren | *Speeltijdtimer › Alle games blokkeren*. Zet je het uit, dan komen de vorige limieten terug. |
| Een wijziging ongedaan te maken | **ZR** *Ongedaan maken* er direct na, of *Geschiedenis › Wijzigingen via PlayGuard* op elk moment. |
| Een kopie van de instellingen te bewaren | *Geschiedenis › Back-up van de instellingen maken* (nooit de pincode). |
| Te weten wat je aandacht vraagt | De bel (Ⓨ): wat te doen is in oranje, wat je moet weten met een "i". |

## Verder

- Elk tabblad en elke instelling: [functies](features.md).
- Iets werkt niet zoals verwacht: [problemen oplossen](troubleshooting.md#veelvoorkomende-problemen).
- Wat de console doet, en hoe zeker we daarvan zijn: [hoe ouderlijk toezicht werkt](../parental-controls.md) (in het Engels).

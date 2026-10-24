<!-- translated from docs/features.md, sha256 06fe889ee4f2 -->
# Functies

Wat elk tabblad doet, hoe PlayGuard veilig schrijft, en wat de console als speeltijd telt. Nieuw met PlayGuard? Begin met [aan de slag](getting-started.md). Terug naar de [README](../../README.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

De schermafbeeldingen tonen het donkere thema, de standaard van PlayGuard (in het Engels).

| Vandaag | Console |
|---|---|
| ![Vandaag](../../images/screenshots/dashboard.png) | ![Console](../../images/screenshots/console.png) |
| **Speeltijdtimer** | **Speeltijdtimer › een dag bewerken** |
| ![Speeltijdtimer](../../images/screenshots/play_timer.png) | ![Een dag bewerken](../../images/screenshots/per_day.png) |
| **Activiteit** | **Eén game** |
| ![Activiteit](../../images/screenshots/activity.png) | ![Eén game](../../images/screenshots/activity_game.png) |
| **Beperkingen** | **Console › Netwerkklok** |
| ![Beperkingen](../../images/screenshots/restrictions.png) | ![Netwerkklok](../../images/screenshots/clock.png) |
| **Beveiliging** | **Voorkeuren** |
| ![Beveiliging](../../images/screenshots/security.png) | ![Voorkeuren](../../images/screenshots/preferences.png) |

## Bediening

| Knop | Wat hij doet |
|---|---|
| ↑/↓ | Verplaatsen |
| Ⓐ | Bevestigen |
| Ⓑ | Terug of annuleren (twee keer in de zijbalk om af te sluiten) |
| Ⓧ | Vernieuwen (op de tabbladen die de console lezen) |
| Ⓨ | Meldingen, vanuit elk tabblad |
| **−** | Taal en thema, vanuit elk tabblad (in de voettekst, als eerste weggelaten wanneer de eigen knoppen van een scherm de ruimte nodig hebben) |
| **+** | Slaat de limieten per dag op (en de week die in *Speeltijdtimer* is bewerkt) |
| **ZR** | *Ongedaan maken*, een paar seconden nadat een wijziging is opgeslagen |

De app is in tabbladen ingedeeld, zoals de systeeminstellingen; elk tabblad heeft hieronder een eigen sectie.

- **Wat mislukt, staat in een dialoogvenster; wat lukt, in een korte melding.** Een dialoogvenster zegt alleen wat nu voor deze console geldt: dat de lopende game zou worden onderbroken, alleen zolang er een game open is; dat de smartphone-app een wijziging zou kunnen overschrijven, alleen zolang die gekoppeld is en kan synchroniseren.
- **De kopregel** geeft op elk tabblad aan wanneer ouderlijk toezicht tijdelijk ontgrendeld is (met de pil), en wanneer de app alleen-lezen is (de pil zegt dan *Alleen-lezen*; acties die zouden schrijven zijn grijs en worden geweigerd, met waar je het uitzet); erop tikken brengt je naar de plek waar je het ongedaan maakt (*Vandaag › Weer inschakelen*, het firmwarescherm of *Hulp en diagnose*). De voettekst toont de knoppen die ze vanuit elk tabblad openen: ⊖ voor de taal en het thema, Ⓨ voor de meldingen (met hun aantal).
- **In alleen-lezenmodus** blijven de acties op hun plaats, grijs, en zeggen ze waarom als je erop drukt.

## Het uiterlijk: Cadran

Het uiterlijk van PlayGuard heet Cadran: een smalle balk met iconen voor de tabbladen, een palet in marineblauw of koel wit (volgens het thema), kaarten in plaats van lijsten. Elk tabblad opent op wat het belangrijkst is (een ring, tegels, kaarten, chips), met de overige instellingen eronder.

- **De pil in de kopregel** zegt waar het tabblad over gaat (*Ouderlijk toezicht actief*, *Timer actief*…); zolang ouderlijk toezicht ontgrendeld is, zegt ze dat op elk tabblad, en erop tikken opent *Vandaag › Weer inschakelen*; zolang het voor jou gepauzeerd is (zie [hieronder](#de-pincode-pauzeert-ouderlijk-toezicht-zolang-je-in-playguard-bent)), zegt ze rustig *Ouderlijk toezicht gepauzeerd*, en erop tikken legt uit wat dat is. Secundaire tekst is nooit kleiner dan 15 px, zodat je hem vanaf de bank op een tv kunt lezen. De klok in de voettekst toont uren en minuten (*16:00*).
- **Dialoogvensters** zijn kaarten met hun knoppen als pillen, de bevestiging in blauwgroen (in koraalrood als ze iets verwijdert of stopt).
  - De keuzes openen als een kaart boven de knopaanwijzingen, de huidige keuze op zacht blauwgroen met een vinkje. De blauwgroene rand hoort overal alleen bij de focus: een niveau, een account, een geldende keuze houdt de zachte vulling.
  - Een lang dialoogvenster vervaagt onderaan zolang er meer volgt, en de waarschuwing erin (een game die op het punt staat te worden onderbroken) komt eerst, in een oranje kaart.
  - Een mislukking heeft als titel wat er mislukt is, de foutcode klein onder de reden, en biedt *Rapport versturen* aan als dat helpt.
  - Meldingen verschijnen als kaarten onder de kopregel, met een blauwgroen vinkje voor wat is gedaan en een oranje teken voor wat is geweigerd.
- **De andere schermen** krijgen dezelfde look: de editor per dag van een profiel toont de week zoals in *Speeltijdtimer* (op dezelfde manier bewerkt, de snelle waarden als tegels); *Geïnstalleerde games* en *Alle games* zetten hun regels in kaarten; *Profielen* en *Hulp en diagnose* geven elke regel een eigen icoon.
- **Tekstgrootte** (*Voorkeuren*, of **AA** in de kopregel):
  - *Automatisch*, de standaard, maakt alle tekst 25 % groter zolang de console in het dock staat, op de tv, en zet hem terug in handheldmodus; *Groter* houdt hem altijd groter, *Normaal* nooit.
  - De namen van de tabbladen in de zijbalk groeien mee, ook met 25 %; de titel, het label en de datum van de kopregel en de knopaanwijzingen en de klok van de voettekst groeien iets minder (15 tot 20 %), omdat hun regels al vol zijn. Een naam die in een taal te lang is voor zijn ruimte wordt iets kleiner getekend, nooit kleiner dan op normale grootte, in plaats van afgekapt. De namen van de niveaus in *Beperkingen* houden hun grootte, die al groot zijn.
  - Het verandert meteen, op elk scherm, zonder herstart.
  - De 25 % komen van een 55"-tv in 4K: de kleinste tekst (15 px) is dan 14,6 mm hoog in hoofdletters, 20′ gezichtshoek op 2,5 m, waar lezen comfortabel is (16′ in normale grootte, het absolute minimum).
- **Visuele effecten** (*Voorkeuren*): *Automatisch: volledig op de tv*, de standaard, houdt de achtergrondgloed en elke beweging van het uiterlijk zolang de console in het dock staat: de schermen en hun regels die op hun plek schuiven, de ring die zichzelf tekent en naar een nieuwe waarde glijdt, het kloppen van de ring op het kinderscherm in de laatste minuten, de kleine reacties op een druk op een knop. In de hand (handheld of in tafelmodus, opladend of niet) laat het ze allemaal weg, en de console terug in het dock zetten brengt ze terug. *Altijd volledig* houdt ze ook in de hand. *Minder beweging* laat elke beweging die alleen voor de show is weg, ook in het dock, en houdt de gloed: voor wie beweging vermoeiend vindt.
- **Ongedaan maken:** nadat een wijziging is opgeslagen, blijft **ZR** *Ongedaan maken* een paar seconden in de voettekst staan: het opent die wijziging in de wijzigingsgeschiedenis, om de vorige waarde terug te zetten. Een lege wijzigingsgeschiedenis zegt dat in een kaart.

## Meldingen — wat je aandacht vraagt, vanuit elk tabblad

Een bel in de kopregel, links van de versie, verzamelt wat de ouder moet doen; Ⓨ opent de lijst vanuit elk tabblad (de teller op de bel toont *2* als er twee dingen te doen zijn).

Links van de bel staan de taal (de code, *FR*, *EN*…) en het thema (een zon voor licht, een maan voor donker, een half gevulde cirkel als het de console volgt), en daarna de tekstgrootte (**AA**, met de grote A in blauwgroen zolang de tekst groter is): een tik opent dezelfde lijst als in *Voorkeuren*, en **−** biedt ze aan vanuit elk tabblad. Een nieuw thema of een nieuwe taal geldt meteen, en elk scherm wordt op hetzelfde tabblad opnieuw opgebouwd met de focus op dezelfde regel, Chinees en zijn eigen lettertype inbegrepen.

### Te doen

Elk met een oranje waarschuwingsteken, in oranje geteld op de bel; een melding blijft staan tot ze is opgelost:

- nog geen pincode, of een pincode die iedereen als eerste zou proberen (0000, 1234, 1212, 2580…: beoordeeld op de console, nooit getoond);
- een pincode op de console terwijl PlayGuard zelf zonder pincode opent (*PlayGuard beveiligen met de pincode* op *Nooit*);
- ouderlijk toezicht tijdelijk ontgrendeld (*Weer inschakelen*);
- extra tijd van een eerdere dag die nog moet worden teruggezet;
- het alarm "Tijd is om" uit;
- wijzigingen buiten PlayGuard;
- een console die geen speeltijd telt;
- een onnauwkeurige netwerkklok.

### Ter informatie

Elk met een "i", een stip op de bel; kan als gelezen worden gemarkeerd:

- een limiet die op een eerdere dag met *Vandaag niet meer spelen* is verlaagd en nog laag staat (*Terugzetten*);
- een nieuwe versie geïnstalleerd (*Wat is er nieuw*; na een update ter plekke blijft de vorige build bewaard), of een nieuwere versie van PlayGuard;
- een online verstuurd rapport (*Link tonen*), of PlayGuard de vorige keer onverwacht gesloten (*Rapport versturen*);
- de speeltijd van de week (elke 7 dagen tot je het leest; met de vergelijking met de week ervoor, per dag), en een limiet die op twee of meer van de afgelopen 7 dagen is overschreden terwijl de console alleen waarschuwt;
- een back-up van de instellingen die nog moet worden gemaakt (nog geen, of geen in 30 dagen);
- alle games al een week geblokkeerd;
- een nieuw gebruikersaccount;
- de smartphone-app gekoppeld;
- de netwerkklok die bij het opstarten niet klopte.

### Console

De meldingen over de console in plaats van over het ouderlijk toezicht komen als laatste, op één regel die Ⓐ opent (meteen open als er verder niets is). Wat te doen is, wordt geteld op de badge van het tabblad Console, niet op de bel:

- geïnstalleerde games die niet kunnen starten (*Geïnstalleerde games controleren*), gamepatches die moeten worden nagekeken;
- een bijna volle SD-kaart (minder dan 500 MB);
- een bestand van PlayGuard zelf dat niet opzij kon worden gezet of niet naar de SD-kaart kon worden geschreven;
- een verouderde herstel-sysmodule (niet de module die met deze versie meekomt, of geïnstalleerd voordat hij zijn versie vermeldde; *Bijwerken* plaatst de module die deze versie meebrengt), daarna de console die daarvoor opnieuw moet opstarten, tot dat is gebeurd;
- een build die niet helemaal opstartte en waarvan is teruggegaan (*Rapport versturen*).

Ter informatie: een build waarvan is teruggegaan, een firmware die niet met deze versie is getest, een nieuwere versie van Atmosphère, hekate of de firmware (gecontroleerd met *Elke dag op updates controleren*; de melding over de firmware zegt dat je moet wachten tot Atmosphère hem ondersteunt), een nieuwere release van de Home Assistant-integratie (zodra die heeft gemeld welke versie Home Assistant draait: *Wat is er nieuw* toont wat er sindsdien is veranderd en de QR-code die haar in HACS opent), het serienummer dat op emuMMC niet is afgeschermd, het eigen ouderlijk toezicht van het andere systeem, een bestand van PlayGuard zelf dat beschadigd werd gevonden en opzij is gezet (een korte melding bij het opstarten verwijst ernaar).

### Hoe meldingen zich gedragen

- Elke sectie toont haar meldingen in een vaste volgorde, welke controle ook als eerste klaar was.
- **In de zijbalk** telt een oranje badge naast de naam van een tabblad de meldingen die te doen zijn, op de plek waar ze worden opgelost: het tabblad Beveiliging (geen pincode, een zwakke pincode, PlayGuard zonder pincode, tijdelijk ontgrendeld), de Speeltijdtimer (het alarm uit, de console die niet telt), *Vandaag* (wijzigingen buiten PlayGuard, extra tijd om terug te zetten), de Console (de meldingen hierboven, en de netwerkklok). Genegeerde meldingen en meldingen ter informatie worden niet geteld.
- In het tabblad staat dezelfde badge op de regel waar elke melding wordt opgelost (*Netwerkklok*, *Games*, *Opslag*, *Gamepatches*; *Pincode instellen of wijzigen*, *PlayGuard beveiligen met de pincode*; *Geavanceerd* van de Speeltijdtimer voor het alarm), en het tabblad opent op de eerste daarvan. Een regel zonder eigen waarde zegt ervoor, in oranje, waarvoor de badge is (*Makkelijk te raden*, *Niet nauwkeurig*).
- **Ⓐ** op een melding voert de oplossing uit (*Inschakelen*, *Instellen*, *Bijwerken*…) of opent het tabblad waar je het oplost. Een melding verdwijnt vanzelf zodra wat ze meldt is opgelost.
- **Ⓧ** markeert een melding ter informatie als gelezen; **Ⓨ** markeert ze allemaal. Een melding die als gelezen is gemarkeerd, blijft gelezen bij de volgende start, tot de tekst ervan verandert.
- **Negeren:** een paar meldingen die te doen zijn, kun je met Ⓧ negeren, na een bevestiging en de pincode: een pincode die iedereen als eerste zou proberen, PlayGuard dat zonder pincode opent, het alarm "Tijd is om" uit, een bijna volle SD-kaart (nooit: geen pincode, ontgrendeld toezicht, de klok, de speeltijdtimer die niet telt of de bestanden van PlayGuard zelf). Een genegeerde melding telt niet meer mee en staat als laatste onder *Genegeerd*, waar Ⓐ hem weer laat meetellen. Hij komt vanzelf terug na 30 dagen (tenzij *Genegeerde waarschuwingen na 30 dagen weer meetellen* uit staat), zodra wat hij meldt is opgelost en daarna opnieuw gebeurt, of, voor de pincode, bij een nieuwe pincode met een andere lengte.
- *Voorkeuren › Meldingen* zet drie soorten uit: nieuwe versies van Atmosphère, hekate en firmware; de speeltijd van de week; herinneringen (back-ups, tweede systeem, games op slot).
- De toestand erachter wordt gelezen als PlayGuard start, elke minuut op het hoofdscherm, en elke 5 s zolang *Vandaag* wordt getoond.

## Vandaag — de dag in één oogopslag

*[Probeer het in de simulator ›](https://playguard.link/simulator/)*

### De ring en de lijn

**De resterende tijd** als ring: een dag zonder spelen toont *Niet spelen*, niet *0 min*; de extra tijd die vandaag is gegeven, verschijnt als lichter segment aan het einde, en de ring schuift naar elke nieuwe waarde.

Daaronder wat het spelen het eerst stopt:

- vóór het einde van de tijd of het bedtijdalarm de rest van de dag als lijn, met het huidige moment links, elk ervan een markering met het tijdstip (*16:40 tijd om*, *21:00 bedtijd*), blauwgroen tot de eerste, oranje in de laatste 15 minuten;
- anders in woorden: het bedtijdalarm zodra de tijd van de dag op is, wanneer er weer gespeeld mag worden; zolang ouderlijk toezicht ontgrendeld is, de limiet van vandaag gepauzeerd en waarom.

### De acties

- **Extra tijd**, de hoeveelheden op één rij (+15 min, +30 min, +1 u), elk op één druk afstand, dan **…** voor elke andere (een duur, een andere dag): een geselecteerde hoeveelheid schuift het einde van de tijd op de lijn op (*met 15 min extra*), of zegt bij de markering van het bedtijdalarm dat dat toch eerst komt; de eerste springt eruit in blauwgroen zodra er 15 min of minder over is, of niets meer.
- **Stoppen voor vandaag**, **Aan het kind tonen**.
- **Limiet vandaag…**: de limiet van vandaag ter plekke gekozen, geschreven na één bevestiging.
- **Het geldende profiel**, met zijn naam, zodra er een profiel is opgeslagen (*Profielen…* als er geen overeenkomt): Ⓐ toont ze, het geldende aangevinkt, om na één bevestiging een ander toe te passen (de dagen die veranderen), of *Profielen beheren…*.
- Zolang ouderlijk toezicht ontgrendeld is, komt **Weer inschakelen** eerst, over de hele breedte van de rij, en gaat *Extra tijd* opzij.
- Staat er op geen enkele dag een limiet, dan opent **Limieten van de week instellen** de week in *Speeltijdtimer*.

### De kaarten en pillen

- Het **bedtijdalarm** (wanneer er weer gespeeld mag worden, of over hoelang het afgaat als de console het niet zegt; uit, *Uit* in het groot en Ⓐ *Instellen*), **ouderlijk toezicht** en de **netwerkklok**, als kaarten.
- De toestand van de console als **pillen**:
  - één zegt of de DNS-omleiding van Atmosphère de servers van Nintendo blokkeert, en Ⓐ erop welke en wat dat verandert: geen synchronisatie voor de smartphone-app, geen netwerkklok die zichzelf instelt;
  - een andere, geopend als applicatie, dat de tijd in PlayGuard meetelt: als die game wanneer het boven een game is geopend (*Vanuit een game geopend*), onder de naam van zijn icoon wanneer het via een forwarder in het HOME-menu is gestart (*Via zijn HOME-pictogram geopend*), met Ⓐ hoe;
  - een game die open blijft in het HOME-menu, verschijnt als oranje pil (*telt door*, met Ⓐ waarom), als de pil over de eigen tijd van PlayGuard het niet al zegt;
  - geen van beide zolang ouderlijk toezicht voor jou gepauzeerd is: de tijd die je hier doorbrengt, is dan niet die van het kind.
- De pillen zetten eerst wat de ouder nodig heeft (oranje, met een driehoek), dan wat aan staat (blauwgroen, met een vinkje), dan de rest (grijs, met een stip): de vorm zegt het net zo goed als de kleur.
- Wat in orde is en wat alleen wordt gemeld, valt samen in één pil na de pillen die de ouder nodig hebben (*✓ 3 controles in orde*); Ⓐ erop toont ze, zolang het tabblad open is.
- Onder de pillen de extra tijd die nog moet worden teruggezet (Ⓐ zet hem terug) en het alarm "Tijd is om" zolang het uit staat (Ⓐ zet het weer aan).

### Eerste stappen

Zolang de pincode en de limieten van de week niet zijn ingesteld, toont een kaart boven de kaarten de drie stappen (de pincode, een daglimiet, de netwerkklok), elk blauwgroen aangevinkt zodra hij gedaan is of oranje omcirkeld, met hoeveel er gedaan zijn (*1 van 3*); Ⓐ op een stap voert die ter plekke uit.

### Telt de console?

De tijd van PlayGuard zelf telt mee zolang het als applicatie draait, dus met een limiet vandaag moet de resterende tijd dalen zolang het open is. Is die ongeveer 90 s op het scherm niet veranderd (timer aan, een limiet vandaag, niet ontgrendeld of voor jou gepauzeerd, niet geopend vanuit het album), dan meldt een oranje regel onder de gespeelde tijd *De console telt op dit moment geen speeltijd*, met de waarschijnlijke oorzaken:

- de klok is teruggezet (later telt hij weer);
- de netwerkklok is nooit ingesteld (*Console › Netwerkklok*);
- er is net een limiet geschreven (start een game voor een minuut en kijk dan opnieuw).

De regel verdwijnt zodra de resterende tijd weer verandert. De Speeltijdtimer meldt het ook.

### Buiten PlayGuard gewijzigd

De speeltijd van vandaag die op dezelfde dag opnieuw begint (een klokwijziging doet dat), de klok van de console die meer dan 5 minuten is verzet, limieten die afwijken van wat PlayGuard het laatst zag (de app op de telefoon, een ander hulpmiddel): een oranje kaart boven de ring tot je die verbergt (Ⓐ) of tot de volgende dag, en een vermelding in de wijzigingsgeschiedenis. PlayGuard draait niet op de achtergrond: het ziet alleen wat er veranderde terwijl het open was, of sinds het de vorige keer open was, wanneer het de console opnieuw leest.

### Bij de limiet

De console toont "Tijd is om" en onderbreekt daarna de game (geen "doorgaan") of waarschuwt alleen: *Als de tijd om is* toont welke van de twee en laat je dat wijzigen, zodra er een limiet of een bedtijd is ingesteld ([details, in het Engels](../parental-controls.md#when-the-time-runs-out)).

### Extra tijd en vandaag niet meer spelen

- **Extra tijd vandaag**: standaard +15 min, +30 min, +1 u.
- **Vandaag niet meer spelen**: limiet 0 voor alleen vandaag, *Nu* of *Over 5 minuten* als de gespeelde tijd bekend is (de limiet wordt dan de gespeelde tijd + 5 min, tijd om op te slaan). Bij de vergrendeling verschijnt "Tijd is om" van de console, en niet-opgeslagen voortgang kan verloren gaan — de bevestiging zegt dat.
- **De volgende dag** — bij het opstarten, of om middernacht als de app open is — zet PlayGuard de gewone limiet terug:
  - na extra tijd vanzelf, zonder vraag en zonder pincode (het verlaagt de limiet alleen, net als ouderlijk toezicht weer inschakelen), en het ontgrendelt even als de timer telt om meteen weer te vergrendelen. Het wacht zolang het niet kan schrijven (alleen-lezenmodus, een firmware die het niet ondersteunt, *Alle games blokkeren* aan) en, een week later op dezelfde dag, zolang de lagere limiet de lopende game zou stoppen (dan de dag erna). Wil je op een dag meer tijd houden, wijzig dan de limiet van die dag;
  - na *Vandaag niet meer spelen* vraagt het of het 0 moet houden of de limiet moet terugzetten (met de pincode, zoals elke wijziging die een limiet verhoogt), of zet het de limiet terug zoals elke wijziging (*Speeltijdtimer › Limiet vanzelf terug na "Vandaag niet meer spelen"*); het terugzetten van "geen limiet" wordt altijd gevraagd.
- **Extra tijd op een andere dag** (*Extra tijd › Een andere dag…*, aan het einde van de lijst): morgen of een van de vijf dagen daarna, elk met zijn limiet erbij, dan de hoeveelheid. De limiet van die dag wordt nu verhoogd, alleen voor die datum, en vanzelf teruggezet (het verlaagt de limiet alleen) de volgende keer dat PlayGuard wordt geopend nadat de dag voorbij is — niet op een console waarop PlayGuard niet wordt geopend; daar houdt ook dezelfde dag van de week erna hem. Eén tegelijk: de lijst biedt dan aan om hem te verwijderen (de limiet meteen terug). Een wijziging van de limiet van die dag in de tussentijd (de week, een profiel…) gaat voor: er wordt niets teruggezet.

### Het kinderscherm

*Aan het kind tonen* op *Vandaag*, om de console over te dragen:

- alleen de resterende tijd (zolang ouderlijk toezicht ontgrendeld is, de limiet van vandaag, gepauzeerd), wanneer het spelen eindigt (*Eindigt rond 16:40*; als het bedtijdalarm eerst komt, telt de ring in plaats daarvan daarnaar af, *45 min tot bedtijd*, zoals in het laatste uur van een dag zonder limiet), de bedtijd en wat er gebeurt als de tijd om is;
- groot genoeg om vanaf de andere kant van de kamer op de tv te lezen; de tekst houdt zijn grootte, welke tekstgrootte ook is ingesteld (alleen de titel en de voettekst volgen die, zoals op elk scherm);
- in de laatste 5 minuten (van het spelen of vóór de bedtijd) wordt het einde oranje en pulseert de ring langzaam; de bedtijd wordt oranje in de laatste 30;
- Ⓐ en Ⓨ, de knoppen van de ouder, staan klein rechtsboven, zodat de voettekst alleen die van het kind toont;
- Ⓐ gaat met de pincode terug naar de schermen van de ouder; Ⓨ geeft van daaruit extra tijd, na de pincode (de hoeveelheden van *Extra tijd*, terwijl het kinderscherm blijft staan); Ⓑ sluit PlayGuard af (en vraagt eerst of ouderlijk toezicht weer moet worden ingeschakeld, zoals bij afsluiten).

Met grotere tekst past *Vandaag* nog steeds op één scherm: de regel boven de kop noemt alleen nog de dag en zijn limiet (*DONDERDAG · 2 u*), de knoppen en de kaarten staan iets dichter op elkaar. Het wordt elke 5 s vernieuwd (Ⓧ vernieuwt nu).

## Speeltijdtimer — de week is de editor

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=play_timer)*

### De week bewerken

Ⓐ op een dag bewerkt hem:

| Knop | Tijdens het bewerken |
|---|---|
| ↑/↓ | Met 15 min wijzigen, per uur zodra je ze ingedrukt houdt |
| ←/→ | Naar de volgende dag |
| Ⓛ | Naar maandag–vrijdag kopiëren |
| Ⓡ | Naar het weekend kopiëren |
| Ⓨ | Geen limiet |
| Ⓧ | Een exacte duur (*Een duur invoeren…*: ↑/↓ per 15 min, per uur zodra je ze ingedrukt houdt, Ⓛ/Ⓡ per 5 min, met OK geselecteerd) |
| Ⓐ of Ⓑ | Het bewerken beëindigen (Ⓐ zegt welke dag: *Klaar met maandag*; er is nog niets opgeslagen) |

- Met aanraken wordt een aangetikte dag bewerkt, waarna een tik op de boven- of onderkant hem met 15 min wijzigt.
- De wijzigingen worden samen opgeslagen, met **+** of *Opslaan*, zoals in de editor per dag (de bevestiging toont de gewijzigde dagen, voor → na); *Verwerpen* draait ze terug.
- In de balk van elke dag de tijd die de laatste keer op die dag van de week is gespeeld (vandaag inbegrepen, alle accounts). Boven de limiet is de balk vol en de bovenkant koraalrood, voor hetzelfde deel als de tijd boven de limiet van de gespeelde tijd is; een legenda onder de week zegt dat (*limiet overschreden*), en ook de limiet staat in koraalrood. De dagen na vandaag tonen het spelen van vorige week, vager (geselecteerd: *Vorige vrijdag: 2 u 10 gespeeld van 2 u*).
- Een gewijzigde dag die nog niet is opgeslagen heeft zijn limiet in oranje en een oranje rand om zijn balk; onder de week de cijfers van de geselecteerde dag (*donderdag: 1 u 20 gespeeld van 2 u*).
- Boven de week, in oranje, een bereikte limiet en wat de console dan doet, of een console die geen speeltijd telt.
- `0` minuten betekent *die dag niet spelen*; *Speeltijdlimiet verwijderen* zet de timer uit.

### Chips en snelle instellingen

**Chips** boven en onder de week openen meteen hun keuzes: de profielen (hoeveel er zijn opgeslagen), elke dag dezelfde limiet, het bedtijdalarm en *Als de tijd om is*. *Limiet verwijderen* is de laatste regel van het tabblad, apart (de geschiedenis maakt het ongedaan).

- **Elke dag dezelfde limiet:** een snelle lijst of een exacte duur (*Een duur invoeren…*: ↑/↓ per 15 min, per uur zodra je ze ingedrukt houdt, Ⓛ/Ⓡ per 5 min).
- **Doordeweeks en weekend ›:** de editor, die eerst om maandag–vrijdag en daarna om het weekend vraagt.
- **Extra tijd of vandaag niet meer spelen ›** opent *Vandaag*, waar ze allebei staan. Een opmerking eronder zegt wat de console bij de limiet doet. Zolang ouderlijk toezicht tijdelijk ontgrendeld is, wordt speeltijd *mogelijk* niet geteld: dat is nog een open vraag ([docs, in het Engels](../parental-controls.md#still-open)).
- Elke bevestiging toont de dagen die veranderen, voor → na.

### Profielen

- **Profielen** opgeslagen op de SD-kaart: een profiel toepassen (een profiel dat al geldt, zegt dat gewoon), bewerken, hernoemen of verwijderen; de huidige limieten opslaan of een nieuw profiel maken.
- Elke naam, in elk schrift (*École*, *周末*, *Выходные* …) — een naam zonder Latijnse letters krijgt een eigen bestandsnaam, en twee namen die hetzelfde bestand zouden opleveren, worden opgemerkt.
- **Gepland profiel** (*Profielen › Gepland profiel*): *Met Home Assistant*. PlayGuard draait alleen terwijl het open is, dus kan het niet zelf op de juiste dag van profiel wisselen: Home Assistant doet dat wel, vanuit een agenda of de schoolvakanties, zodra *Console › Toegang op afstand* de console ermee verbindt (een automatisering die zijn keuzelijst *Profiel* instelt; [de Home Assistant-handleiding](../home-assistant.md), in het Engels). Een profiel dat met een eerdere versie is gepland, wordt nog steeds aangeboden als zijn periode begint, en de regel wijzigt, verwijdert of beëindigt het nog steeds.

### Alle games blokkeren

*Alle games blokkeren*, onder de instellingen van de week: één schakelaar die de limiet van elke dag op 0 zet, zodat er een pincode nodig is om een game te starten — een lichte blokkade zonder leeftijdsclassificatie of communicatiebeperkingen.

- Het blokkeert het starten van games, niet het HOME-menu, en vereist een pincode.
- De vorige limieten komen terug als je het uitzet.
- Zolang het aan staat, worden *extra tijd* en *vandaag niet meer spelen* geweigerd, en vervangen limieten die op een andere manier worden ingesteld (een profiel, een back-up, de geschiedenis…) het.

### Bedtijdalarm

- Het tijdstip van het alarm (16:00 tot 23:45, of *Uit*) en wanneer er weer gespeeld mag worden (05:00 tot 09:00), elke dag hetzelfde, ingesteld met de stick: omhoog en omlaag een kwartier, L en R een uur.
- Of, met Ⓨ *Per dag* in het dialoogvenster van het alarm, voor maandag tot vrijdag, het weekend of één dag, terwijl de andere dagen hun eigen tijd houden (elke groep dagen met het tijdstip erbij). Als de dagen verschillen, toont het tabblad *Verschilt per dag*, en de week in *Speeltijdtimer* toont het alarm van elke dag onder de naam ervan.
- De plek ervan in de instellingen van de speeltijdtimer, afgeleid uit de instellingen van de smartphone-app, is op een console geverifieerd (22.0.0); PlayGuard wijzigt het toch pas als de console meldt wat PlayGuard daar leest, controleert na de wijziging het antwoord van de console en zet de vorige instellingen terug als dat afwijkt.

### Instellingen voor extra tijd en Geavanceerd

- **Extra tijd:** de aangeboden hoeveelheden (+15/+30/+1 u, +10/+20/+30 min …) en de gewone limiet zonder vragen terugzetten na *Vandaag niet meer spelen* (extra tijd wordt altijd zonder vragen teruggezet). Beide gebeuren de volgende keer dat PlayGuard wordt geopend, vanaf de volgende dag: een console waarop het niet wordt geopend, houdt de limiet van die dag.
- **Geavanceerd ›**, een eigen scherm: de schakelaar voor de acties van de debugklasse en, zolang die aan staat, het alarm "Tijd is om" aan/uit en het aftellen pauzeren / hervatten. Met dezelfde schakelaar kan een back-up of de geschiedenis het alarm terugzetten.

## Activiteit — wie wat speelde, en hoe lang

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=activity)*

Tijd per game **vandaag**, in de **afgelopen 7 dagen** en **in totaal**, uit het activiteitenlog van de console zelf — voor alle accounts of voor **één gebruikersaccount** (de accounts als chips bovenaan, als de console er meerdere heeft). PlayGuard start op het laatst getoonde account, of op het account dat bij *Account dat eerst wordt getoond* is gekozen (alle accounts, of één; onderaan het tabblad, bij *Alle games*).

### Bovenaan

- De accounts als chips, de periode en *Exporteren*.
- Vandaag, de afgelopen 7 dagen en altijd als cijfers. Onder de afgelopen 7 dagen de vergelijking met de 7 ervoor, per dag: ▲ *+20 min/dag t.o.v. vorige week* in oranje, ▼ minder in blauwgroen, *zoals vorige week* bij hoogstens 5 minuten verschil.
- De vier meest gespeelde games als kaarten (Ⓐ opent er een); over de afgelopen 7 dagen zegt de game die het meest meer is gespeeld dan de week ervoor met hoeveel, vanaf een half uur (▲ *+2 u t.o.v. vorige week*).
- Niets gespeeld in de periode: een kaart zegt dat, met *Totaal bekijken* als er iets te zien is.

### Grafieken en samenvatting

- **Een grafiek van de afgelopen zeven dagen** (vandaag rechts, een legenda eronder), met de limiet van deze week voor elke dag als lijn en de tijd daarboven in oranje, gemarkeerd met !; de totalen van vandaag, van de week en van altijd.
- **Een samenvatting** over de gekozen periode (vandaag, de afgelopen 7 dagen, de **afgelopen 30 dagen** of altijd), voor het getoonde account: gemiddelde per dag, de meest gespeelde game, de gespeelde dagen en de drukste (afgelopen 7 dagen), de gemiddelde sessie (altijd).
- **Speeltijden, afgelopen 7 dagen**: een vakje voor elk uur van elke dag, maandag eerst, hoe feller hoe meer er is gespeeld (alle games, het getoonde account); dan de sessies (spelen met pauzes korter dan 5 minuten is één sessie, ook als er van game wordt gewisseld) en hun gemiddelde lengte, de langste en de dag ervan, de twee drukste uren na elkaar, en de tijd gespeeld tussen 21:00 en 06:00.

### Alle games en de pagina van een game

- **Alle games**, op een eigen scherm: elke game die in de periode is gespeeld, de meest gespeelde eerst, als **raster van covers** of als **lijst** (Ⓨ wisselt, en de keuze wordt onthouden), met de cijfers van de geselecteerde game altijd bovenaan: het icoontje en de naam, de speeltijd in de periode, in totaal, het aantal keer gestart en de laatste keer gespeeld. Ⓧ kiest de periode (het tabblad Activiteit volgt).
- Icoontjes worden rond de geselecteerde game gelezen (niet in appletmodus, om geheugen te sparen); een grijze controller staat er tot er een binnenkomt, en bij een game zonder icoontje.
- Een grote bibliotheek toont de eerste 50 games, dan *Alle games tonen* (hoogstens 100 games in appletmodus); de export bevat ze altijd allemaal. PlayGuard leest hoogstens 256 games (die van deze week eerst): daarboven melden het tabblad Activiteit, Alle games en de export (niet CSV) dat er enkele ontbreken, en het totaal van altijd laat ze weg.
- **Ⓐ op een game:** de afgelopen zeven dagen als staven, het aantal keer gestart, de eerste en laatste keer gespeeld, de tijd per gebruikersaccount. Verwijderde games houden hun totale cijfers, zelfs een game die werd verwijderd voordat PlayGuard was geïnstalleerd, zolang het speellog van de console hem nog noemt, en hun naam zodra PlayGuard die heeft gezien (anders *Verwijderde game* en de titel-ID).

### Accounts

- Speeltijd van een game die bij geen enkel gebruikersaccount hoort (sommige games vragen er nooit om, of het account is sindsdien verwijderd), verschijnt als *Zonder account*, vanaf een minuut.
- Een gebruikersaccount dat PlayGuard nog nooit zag, wordt 30 dagen in oranje gemarkeerd op het tabblad (*Nieuw account sinds …*: een nieuw account is een bekende manier om opnieuw te beginnen) en vastgelegd in de wijzigingsgeschiedenis; de accounts die er al waren toen PlayGuard ze voor het eerst opsomde, worden niet gemarkeerd.

### Exporteren

- **Exporteren naar de SD-kaart** (*Exporteren*) als CSV, JSON, XLSX (Excel) of PDF, één kolom per dag, en daarna de afgelopen 7 dagen, de afgelopen 30 dagen en altijd.
- Opent meteen met de laatst gelezen cijfers (bewaard op de SD-kaart tussen twee starts), op de achtergrond vernieuwd als ze ouder zijn dan een minuut; Ⓧ leest ze nu opnieuw, met een laadindicator.
- De tijden zijn bij benadering als de klok van de console is gewijzigd.

## Beperkingen — niveau, leeftijdsclassificatie, communicatie

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=restrictions)*

- **Het beperkingsniveau:** Jong kind, Kind, Tiener en Aangepast naast elkaar als tegels, elk met wat het beperkt (een koraalrood hangslot voor beperkt, anders een groen vinkje, en de geïnstalleerde games die het blokkeert; Ⓐ stelt het in; geldt *Aangepast* al, dan gaat Ⓐ naar de instellingen eronder); *Geen* eronder. Het geldende niveau zegt *✓ Huidig*; de blauwgroene rand staat alleen waar de focus is. Wat een niveau beperkt (leeftijdsgrens, berichten plaatsen, communicatie), wordt nog eens gemeld voordat het wordt ingesteld; *Geen* vraagt het in koraalrood, omdat het elke beperking opheft.
- **De instellingen van het niveau:** leeftijdsclassificatie, berichten plaatsen op sociale media, communicatie met anderen. Bij een vooraf ingesteld niveau zijn het diens eigen instellingen: er één wijzigen maakt het niveau *Aangepast*, vanuit de instellingen van dat niveau met die wijziging, na een vraag (met de instellingen op een rij).
- **Onder *Leeftijdsclassificatie*** de **classificatie-instantie** (PEGI, ESRB, USK, CERO …): de lijst zegt voor welke landen elke instantie classificeert (PEGI: het grootste deel van Europa, ESRB: de Verenigde Staten, Canada en Latijns-Amerika, USK: Duitsland, PEGI BBFC: het Verenigd Koninkrijk…).
- **Geblokkeerde geïnstalleerde games** (ook onder *Leeftijdsclassificatie*): hoeveel van de geïnstalleerde games het geldende niveau blokkeert op hun leeftijdsclassificatie (*3 van 15*), waarbij Ⓐ ze toont met hun classificatie (*PEGI 16*); de tegel van elk niveau zegt het ook (*Geblokkeerde games · 3*).
  - De classificaties zijn die van elke game zelf (de controlegegevens van de game, gelezen door de controle van de geïnstalleerde games bij het opstarten of via *Console › Games*), volgens de instantie die de console gebruikt.
  - Een game die deze instantie niet classificeert, telt niet mee, en ook de uitzonderingen die in de smartphone-app zijn ingesteld niet.
  - Het scherm van een game in Activiteit toont de classificatie, en *geblokkeerd op het huidige niveau* als dat zo is.
- **Onder *Overig*** de VR-modus. De games die mogen communiceren staan ruw in het diagnoserapport, tot hun lijst wordt begrepen.

## Beveiliging — de pincode, ontgrendelen, het homebrewmenu, smartphone-app

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=security)*

- **Bovenaan** vier tegels: de pincode, de beperkingen, het tijdelijk ontgrendelen en het homebrewmenu, elk met een blauwgroen vinkje als het is zoals het hoort (een pincode ingesteld, beperkingen aan, vergrendeld) of een oranje waarschuwingsteken als er iets moet gebeuren; het homebrewmenu zegt *Open* of *Vergrendeld* (een vinkje zodra het vergrendeld is: open is prima zonder forwarders).
- **Pincode instellen of wijzigen** (pincodescherm van het systeem).
- **Pincode tonen**: na een waarschuwing en de pincode zelf, die elke keer wordt gevraagd; vastgelegd in de geschiedenis, daarna wordt een nieuwe pincode voorgesteld.
- **Tijdelijk ontgrendelen**, en of PlayGuard het vanzelf weer inschakelt meteen na een wijziging waarvoor het ontgrendeld moest worden (standaard aan).

### PlayGuard beveiligen met de pincode

De pincode in PlayGuard zelf:

| Instelling | Effect |
|---|---|
| *Nooit* | Geen pincode in PlayGuard (de bel meldt het zolang de console er een heeft) |
| *Vóór een wijziging* (standaard) | Iedereen kan kijken, alleen de ouder wijzigt iets |
| *Om PlayGuard te openen* | Een vergrendelscherm bij het opstarten |

- Eenmaal ingevoerd, geldt de pincode tot PlayGuard wordt gesloten; *Na 5 minuten opnieuw om de pincode vragen* (standaard uit) vraagt er daarna opnieuw om, en brengt het vergrendelscherm van *Om PlayGuard te openen* terug als PlayGuard na 5 min of langer op de achtergrond weer op de voorgrond komt.
- Gecontroleerd in de servicelaag, dus geen enkele wijziging slaat het over; opnieuw vergrendelen vraagt er nooit om. *Pincode tonen* vraagt er elke keer om, in elke modus en zelfs vlak nadat hij is ingevoerd, en een ontbrekend of beschadigd `config.json` geldt als *Vóór een wijziging*.
#### De pincode pauzeert ouderlijk toezicht zolang je in PlayGuard bent

- Alleen met *Beveiliging › Ouderlijk toezicht pauzeren zolang ik in PlayGuard ben* aan (standaard uit: dan laat het invoeren van de pincode ouderlijk toezicht zoals het is, en telt de tijd in PlayGuard als speeltijd). Aanzetten vraagt om de pincode.
- Zodra de pincode is ingevoerd (het vergrendelscherm van *Om PlayGuard te openen*, de eerste wijziging met *Vóór een wijziging*, de weg terug vanaf het kinderscherm…), ontgrendelt PlayGuard ouderlijk toezicht tijdelijk, zodat de tijd die je erin doorbrengt niet van de speeltijd van het kind afgaat; *Vanuit een game geopend* verdwijnt uit Vandaag.
- De pil in de kopregel zegt *Ouderlijk toezicht gepauzeerd*, in blauwgroen; erop tikken legt uit wat dat is, met *Nu weer aanzetten* (daarna niet opnieuw gepauzeerd tot de pincode weer wordt ingevoerd). Vandaag toont de dag zoals die voor het kind is, en de tegel Beveiliging zegt *Ouderlijk toezicht gepauzeerd*, zonder waarschuwing.
- Het gaat vanzelf weer aan zodra PlayGuard wordt verlaten (HOME-menu, slaapstand) of gesloten, ook via het HOME-menu (het afsluiten wacht erop), en zolang het kinderscherm of het vergrendelscherm wordt getoond. Terug in PlayGuard, terwijl de pincode nog geldt, pauzeert het weer.
- Niet vanuit het album (de tijd telt daar niet, en er kan een game open blijven op de achtergrond), niet in alleen-lezen-modus, en nooit bovenop een ontgrendeling met *Tijdelijk ontgrendelen*, die blijft zoals ze is.
- Vastgelegd zoals elke ontgrendeling: stopt PlayGuard zonder opnieuw te vergrendelen, dan vergrendelt het bij de volgende start opnieuw, en een herstart van de console doet het vanzelf. Het steunt erop dat de tijd niet telt zolang er ontgrendeld is, afgeleid maar nog niet gemeten ([hieronder](#wat-telt-als-speeltijd)).
- **Homebrewmenu vergrendelen ›** opent *Console › Forwarders*, waar het na zijn controles wordt vergrendeld (zie [Forwarders](#forwarders)).
- **Afsluiten** terwijl ouderlijk toezicht nog ontgrendeld is, vraagt of het weer moet worden vergrendeld (*Weer inschakelen en afsluiten* sluit ook de game waarboven PlayGuard werd geopend, terug naar het HOME-menu in plaats van hbmenu); wordt PlayGuard in plaats daarvan via het HOME-menu gesloten, dan vergrendelt het bij de volgende start opnieuw, en een herstart van de console vergrendelt vanzelf. De eigen pauze van PlayGuard vraagt niets: die eindigt wanneer PlayGuard sluit.

### Smartphone-app

- Of de app Nintendo Switch Ouderlijk toezicht gekoppeld is, de laatste synchronisatie, en **ontkoppelen** (anders overschrijft de volgende synchronisatie de limieten die hier zijn ingesteld: zolang de app gekoppeld is, vermeldt elke wijziging die PlayGuard laat bevestigen dat).
- Als de DNS-omleiding van Atmosphère (`atmosphere/hosts`) de server voor ouderlijk toezicht van Nintendo ergens anders heen stuurt, kan de app niet synchroniseren: PlayGuard toont dan hier *synchronisatie geblokkeerd (DNS)* en noemt de app nergens anders (geen opmerking in de dialoogvensters, geen melding bij de bel, geen stap in de eerste stappen, geen pil op *Vandaag*, geen hulp bij het ontcijferen, en bij het ontkoppelen wordt niet gezegd wat de app zou overschrijven).
- Zolang de app gekoppeld is en kan synchroniseren, opent *Vóór het ontkoppelen: twee instellingen ontcijferen* de vergelijking van het blok (zie [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), in het Engels).

### Ouderlijk toezicht wissen

Twee bevestigingen, onomkeerbaar; eerst wordt een back-up van de instellingen opgeslagen.

### Buitengesloten?

Als laatste op het tabblad: de optionele herstelmodule, voor een vergeten pincode met een limiet van 0 minuten. Ⓐ opent hem in *Console › Optionele modules*, dat hem installeert.

## Geschiedenis — wat PlayGuard veranderde, en de back-ups

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=history)*

**Bovenaan** als tegels hoeveel wijzigingen er zijn vastgelegd en de dag van de laatste back-up: een blauwgroen vinkje, of een oranje waarschuwing als er geen is of al 30 dagen geen.

### Wijzigingsgeschiedenis

- De nieuwste 500 wijzigingen die PlayGuard heeft gedaan (limieten, beperkingsniveau, pincode, ontgrendelingen, ontkoppelen, de klok, terugzettingen, het aftellen pauzeren of hervatten …) en wat het buiten zichzelf zag veranderen, per dag gegroepeerd (*Vandaag*, *Gisteren*, daarna de dag en de datum), met het tijdstip en de bron; een limiet die voor één dag is gewijzigd, noemt die dag.
- Een wijziging die de console **weigerde**, staat er ook, in oranje, met de foutmelding van de console en wat er werd geprobeerd (nooit terugzetten: er is niets veranderd).
- *Tonen* houdt één soort over (speeltijd, beperkingen, beveiliging en app, gezien buiten PlayGuard, geweigerd door de console); *Getoonde exporteren* schrijft dat naar `exports/` als CSV, JSON, XLSX of PDF.
- Ⓐ op een wijziging toont haar en, voor een waarde, **zet de vorige terug** — met hetzelfde ontgrendelen en dezelfde pincode als elke wijziging, en met de melding als de waarde sindsdien is veranderd.
- Elke wijziging bewaart de PlayGuard-versie die haar deed (een ontwikkelbuild: als zodanig vermeld, met zijn commit en, voor de build van een pull request, het nummer daarvan); een wijziging die door een nieuwere versie van PlayGuard is vastgelegd, verschijnt als *Andere wijziging*.

### Back-ups

- **Back-up maken / terugzetten van de instellingen** op de SD-kaart: beperkingsniveau, aangepaste instellingen, VR-modus, classificatie-instantie, daglimieten, het alarm "Tijd is om" (met de geavanceerde acties aan), en ter registratie het ruwe blok van de speeltijdtimer — nooit de pincode.
- Een terugzetting toont alleen wat er zou veranderen (niets te veranderen: dan zegt het dat gewoon, zonder ontgrendelen en zonder iets te schrijven), en meldt wanneer de back-up een bedtijdalarm aan had (dat wordt niet teruggeschreven).
- Kies hoeveel back-ups er bewaard blijven.

## Console — de netwerkklok, de geïnstalleerde games, wat de console draait

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=console)*

**Bovenaan** wat de console draait, als tegels, onder *Systeem* (de firmware, Atmosphère, hekate als het op de SD-kaart staat en sys-patch als het is geïnstalleerd; Ⓐ opent de release-opmerkingen), dan de hulpmiddelen als tegels, onder *Hulpmiddelen* (*Netwerkklok*, *Games* en *Forwarders*; de eerste twee tonen in hun hoek wat aandacht nodig heeft; dan *Toegang op afstand* en *Optionele modules*), dan de regels van de console, compatibiliteit eerst (alleen een blauwgroen vinkje als PlayGuard op deze firmware is gecontroleerd, anders een waarschuwing en wat er mis is).

### Netwerkklok

*Netwerkklok ›*, een eigen scherm — de klok waarop de speeltijdtimer vertrouwt. Een console die de servers van Nintendo nooit bereikt, houdt deze klok onnauwkeurig, en dat verstoort de speeltijdtimer.

- **Consoleklok en netwerkklok**, tijdzone, nauwkeurigheid. Als de DNS-omleiding van Atmosphère (`atmosphere/hosts`) de tijdserver van Nintendo (`aauth-lp1.ndas.srv.nintendo.net`) ergens anders heen stuurt, kan de console zijn netwerkklok niet meer zelf instellen: dit scherm, de melding bij de bel en de kaart op *Vandaag* zeggen dat en verwijzen naar de meting hieronder.
- **Kies een openbare NTP-server** (≈ 50 ingebouwd, per regio, of je eigen server).
- **Meten** tegen 3 servers (mediaan; een oranje waarschuwing als ze het oneens zijn): een dialoogvenster toont het resultaat — hoeveel de netwerkklok voor- of achterloopt, groot, de tijd van de servers naast die van de netwerkklok, het antwoord van elke server — en stelt de netwerkklok van daaruit in. Een meting blijft 2 minuten bruikbaar, met een aftelling onder *Netwerkklok instellen*. De bevestiging zegt het: de klok wijzigen zet de speeltijd van vandaag terug op nul, dus de hele limiet is weer beschikbaar.
- **Een controle van de netwerkklok bij het opstarten**: een korte melding als hij meer dan een minuut afwijkt; hij stelt de klok nooit in.

#### Geen enkele server antwoordt

Op een console zonder internet wordt *Consoleklok gebruiken* aangeboden in het dialoogvenster van het resultaat, en *Netwerkklok instellen op de consoleklok* onder de meting (en op de kaart van de netwerkklok op *Vandaag*, nadat de meting daar mislukt), achter dezelfde bevestiging.

- PlayGuard heeft niets anders om de consoleklok mee te vergelijken: de bevestiging toont de tijd ervan en vraagt je die eerst met een telefoon of een horloge te controleren, en een consoleklok van vóór 2026 (nooit ingesteld) wordt geweigerd.
- Met *Klok synchroniseren via internet* uit blijft de consoleklok waar hij is, en als je die instelling daarna aanzet, springt hij niet meer; staat ze aan, dan volgt de consoleklok de netwerkklok al, dus zegt de bevestiging dat je haar moet uitzetten en de tijd met de hand moet instellen als die verkeerd is.
- Alleen de klok wordt geschreven, niet het ouderlijk toezicht, dus er hoeft niets te worden ontgrendeld (de pincode van *PlayGuard beveiligen met de pincode* geldt nog wel); zoals elke klokwijziging zet het de speeltijd van vandaag terug op nul.
- Nog niet op een console geprobeerd.

### Geïnstalleerde games

*Games* (een tegel onder *Console › Hulpmiddelen*) vindt de games die het HOME-menu niet kan starten of tonen — een icoon dat blijft laden, een "!", een gestippeld kader — met wat er mis is en wat je moet doen, onder het icoon van de game, en hoeveel geïnstalleerde games een probleem hebben.

**Wat het vindt:** naam en icoon onleesbaar, de game zelf ontbreekt achter zijn update, niets meer geïnstalleerd, ontbrekende bestanden, beschadigde data die de console vond, een nieuwere firmware die de game of alleen zijn update nodig heeft, sleutels die te nieuw zijn voor de firmware, een ontbrekende licentie (ticket), de console die om een verwijderde update vraagt, een onleesbare SD-kaart, een start die de console weigert.

**De lijst heeft een sectie per oplossing**, elk met het aantal en wat je moet doen, de games met hetzelfde probleem bij elkaar (twee games met dezelfde naam worden onderscheiden door hun versie of titel-ID):

| Sectie | Wat erin staat |
|---|---|
| **Hier te repareren** | Wat PlayGuard herstelt |
| **Te controleren** | Data die de console verdenkt: de controle zegt of je de game opnieuw moet installeren |
| **Opnieuw installeren** | Bestanden weg of onbruikbaar, of data beschadigd gevonden: verwijderen en opnieuw installeren |
| **Firmware of SD-kaart** | De game is in orde, de console niet |

- **Data controleren:** wat de datacontrole vond, blijft bij de game tot er een andere versie wordt geïnstalleerd: beschadigd, dan gaat hij naar *Opnieuw installeren* als *Beschadigd (gecontroleerd)*, met de datum en de code van de console; intact, dan verdwijnt hij uit de lijst. *Deze spellen controleren* controleert de hele sectie *Te controleren*, de ene game na de andere in één dialoogvenster, met hun covers erboven (*spel 2 van 5*), en zegt dan hoeveel er beschadigd, intact of niet te controleren zijn, met het verwijderen van de beschadigde binnen handbereik.
- **Meerdere games tegelijk:** Ⓨ selecteert games (een vinkje ervoor); vanaf de eerste geselecteerde zegt een regel boven de secties hoeveel het er zijn, en A daarop selecteert ook een hele sectie, daarna één actie voor allemaal: hun data controleren, of ze verwijderen (eerst wordt hun lijst opgeslagen, één vraag, één pincode).
- **Meerdere verwijderen:** de covers zweven weg zodra elke game verdwijnt (een koraalrood "!" op een die blijft), de balk een segment per game (tot 20); aan het eind hoeveel er zijn verwijderd en hoeveel ruimte dat vrijmaakte, en het dialoogvenster sluit vanzelf. Blijven er over: hun covers koraalrood omrand, en *Opnieuw* voor die games.
- **Onder *Opnieuw installeren*** kun je hun lijst opslaan (JSON en tekst, in de exportmap: naam, titel-ID, versie, probleem), en ze allemaal tegelijk verwijderen na één vraag en één pincode: eerst wordt de lijst opgeslagen, daarna worden de games verwijderd zodat een installatieprogramma ze niet meer als geïnstalleerd telt.
- **De acties:** als één actie de oplossing is, is die de eerste knop van het dialoogvenster; de andere volgen: de geïnstalleerde versie weer accepteren (als de console om een verwijderde update vraagt), alleen de update of extra content verwijderen, de data op beschadiging controleren (de eigen controle van de console, met de voortgang en Annuleren), of de game verwijderen.
- Versies worden getoond zoals de game ze toont (*versie 1.4.0*, *update nr. 4*); na een reparatie zegt een korte melding of de game weer werkt. Opgeslagen spelgegevens blijven altijd staan; elke wijziging staat in de wijzigingsgeschiedenis.
- **Wanneer het controleert:** de laatste controle verschijnt meteen; daarna worden alleen de games waarvan de vermelding is veranderd opnieuw gecontroleerd (alle games één keer per dag, na een firmware-update, of met Ⓧ *Opnieuw controleren*), elk getoond zodra hij is gecontroleerd. Ⓧ *Opnieuw controleren* opent een dialoogvenster: de covers van de games die worden doorgebladerd, een balk en het aantal; *Annuleren* stopt na de game die bezig is, wat is gecontroleerd blijft. Staat er nog geen game in de lijst, dan trekken de covers tijdens de controle door de kaart voorbij.
- Meerdere kapotte games tegelijk wijzen op de SD-kaart of op een game die vanaf de andere opslag (sysMMC / emuMMC) is geïnstalleerd.
- Ⓡ toont de games als **lijst** of als **covers** (icoon, naam en probleem; de keuze wordt onthouden), zoals *Alle games* dat doet.

### Forwarders

*Forwarders ›*, een eigen scherm, een tegel onder *Hulpmiddelen* (ook *Beveiliging › Homebrewmenu vergrendelen*) — een homebrew in het HOME-menu, met zijn icoon en zijn naam, van daaruit gestart als een game (een NRO-forwarder, zoals sphaira ze maakt).

- **Een kaart bovenaan** zegt hoe het homebrew-menu ervoor staat: vergrendeld of open, zijn wegen terug (de forwarder van PlayGuard, de beveiligde menu's, die nog nooit vanuit het HOME-menu zijn gestart), wat het nog zonder de pincode opent, en of de herstel-sysmodule is geïnstalleerd. Ⓐ erop toont wat eraan te doen is, elk met één druk (de volgende stap naar een vergrendeling, een forwarder die het zonder de pincode opent beveiligen of verwijderen, de controle, het herstelblad).
- **De lijst per groep:** die van PlayGuard zelf, de andere die het heeft gemaakt (beveiligd of niet, in één kaart: het icoon zegt welke), die een andere app heeft gemaakt, die niet meer in het HOME-menu staan. Een beveiligde heeft een hangslot op zijn icoon (een vinkje voor een menu dat werkend door zijn poort is gezien), een die hier zonder de pincode is gemaakt een open hangslot in oranje.
- **Ⓨ** opent dit deel van de handleiding (een QR-code). Na het verwijderen of vergeten van een forwarder, of het wijzigen van zijn uren zonder de pincode, *Vraag om de pincode* of *Dicht zodra de speeltijd van vandaag op is*, kan **ZR** hem nog een paar seconden terugzetten (opnieuw geïnstalleerd met hetzelfde programma-ID, of de instelling zoals ze was).
- **PlayGuard** met één druk: zijn eigen icoon en naam, waarbij de forwarder de `.nro` start waarvandaan PlayGuard is geopend.
- **Andere homebrew**: elke `.nro` in de map `/switch` van de SD-kaart (en één map dieper), waarbij eerst om de naam wordt gevraagd (die van de `.nro` zelf, om te houden of te wijzigen), en daarna bevestigd met wat het HOME-menu zal tonen: het icoon, de naam en de uitgever. Het icoon is dat van de `.nro`, omgezet naar de JPEG van 256 × 256 die het HOME-menu toont als het een andere grootte heeft of een PNG is (een controller in de plaats ervan als de `.nro` er geen heeft), zoals sphaira dat doet. Een pad met een spatie wordt tussen aanhalingstekens aan de homebrew doorgegeven.
- **De forwarders in het HOME-menu**, elk met zijn icoon: die PlayGuard heeft gemaakt (Ⓐ: details, naam wijzigen, uitgever wijzigen, opnieuw installeren, verwijderen) en die een andere app heeft gemaakt (details, verwijderen). Ⓐ opent een blad in secties (*Deze forwarder*, *Bij het starten*, *Beveiliging*, *Installeren en verwijderen*) onder zijn icoon en zijn staat, elke regel met zijn icoon en wat er nu is ingesteld (zijn uren; een schakelaar voor *Vraag om de pincode*, *Dicht zodra de speeltijd van vandaag op is* en elke startoptie), *Verwijderen* in rood; B of een aanraking erbuiten sluit het. *Details* toont zijn regels op dezelfde manier, en daarna zijn uitgever, versie, `.nro` en programma-ID; Ⓐ op een regel (of de uitgever) wijzigt die, zoals in het blad. Onder de beveiligde zegt een regel wat elk icoon dat daar staat betekent. Een forwarder waarvan de `.nro` weg is, meldt dat; een die in de systeeminstellingen is verwijderd, blijft in de lijst staan tot hij opnieuw wordt geïnstalleerd of vergeten. Opnieuw installeren houdt zijn programma-ID, dus het item in het HOME-menu wordt vervangen, niet verdubbeld; een forwarder die een ander hulpmiddel voor dezelfde `.nro` heeft gemaakt, wordt ook vervangen.
- **Beveiligd met de pincode** (Ⓐ › *Beveiligen met de pincode*, voor een forwarder die PlayGuard heeft gemaakt, behalve zijn eigen): gestart vanuit het HOME-menu opent de forwarder eerst een kleine poort (`config/playguard/gate.nro`, een paar honderd kilobyte die PlayGuard uit zijn eigen bestanden schrijft), die meteen het pincodescherm van de console toont, en daarna de homebrew zodra de pincode klopt; B gaat daar terug naar het HOME-menu. Rond het pincodescherm dat van de poort zelf: het icoon en de naam van de homebrew, *Protected by PlayGuard*, en hoe het afliep (starten, binnen zijn vrije uren, pincode niet ingevoerd), opgebouwd als het laadscherm van PlayGuard, in zijn taal en thema (*Voorkeuren*) (`make gate-preview` tekent het op een computer). Hij houdt zijn programma-ID en zijn icoon, en de wijzigingsgeschiedenis vermeldt het. Zijn regel toont zijn regels als iconen, één voor elke ingeschakelde: een sleutel voor de pincode, een open slot voor uren zonder pincode, een vierkantje (stop) voor uren waarop hij nooit opent, een timer voor dicht zodra de speeltijd van vandaag op is; *Details* (Ⓐ) zegt ze in woorden. Er moet een pincode op de console staan. *Niet meer om de pincode vragen* vraagt er eerst om, wat *PlayGuard beveiligen met de pincode* ook zegt, en dat geldt ook voor hem verwijderen en hem opnieuw maken zonder de pincode (*Een forwarder toevoegen* voor dezelfde `.nro`). Het beveiligt alleen het icoon in het HOME-menu: de `.nro` start nog steeds vanuit het homebrew-menu, tenzij dat vergrendeld is (hieronder). Een forwarder die door een eerdere PlayGuard is beveiligd, start PlayGuard zelf als zijn poort, wat nog steeds werkt; *Alles opnieuw installeren* zet hem over op de kleine poort.
  - **Uren zonder de pincode** (Ⓐ op een beveiligde): elke dag, maandag tot en met vrijdag of het weekend, van het ene uur tot het andere (ook over middernacht heen), of *Maandag tot vrijdag, en het weekend apart* (een tijdvak voor elk); *Details* zegt wanneer. Ze instellen vraagt om de pincode. Ze volgen de consoleklok, die de systeeminstellingen kunnen veranderen: zodra die meer dan 5 minuten van de netwerkklok afwijkt, vraagt de poort er toch om de pincode (*Geen vrije uren: de klok van de console wijkt af van de netwerkklok*), en ook wanneer de netwerkklok niet gelezen kan worden om dat te controleren (*Geen vrije uren: netwerkklok niet leesbaar*); *Console › Netwerkklok* zet beide goed, ook op een console zonder internet.
  - **Uren waarin hij nooit opent** (Ⓐ op een beveiligde), op dezelfde manier ingesteld: de poort zegt dan *Niet op dit moment* en de uren, en gaat terug naar het HOME-menu, ook als de pincode is ingevoerd (bedtijd voor homebrew). Ze gaan voor de uren zonder de pincode. Wijkt de consoleklok meer dan 5 minuten af van de netwerkklok, dan blijft hij dicht zodra een van beide klokken erin valt, en een pauze na weigeringen telt op de netwerkklok en op de doorlopende klok van de console (die geen instelling verzet), zodat de tijd verzetten ze niet omzeilt.
  - **Gesloten zodra de speeltijd van vandaag op is** (Ⓐ op een beveiligde): als de speeltijdtimer van de console vandaag geen tijd meer heeft, zegt de poort *De speeltijd van vandaag is op* en gaat terug naar het HOME-menu, met of zonder pincode. Uit tenzij ingesteld.
  - **De regels staan in de forwarder, niet op de SD-kaart:** de uren, de daglimiet en de pauze na weigeringen gaan in de eigen gegevens van de forwarder (in de opslag van de console), zodat een computer geen vrije uren kan toevoegen door een bestand te bewerken; er één wijzigen maakt de forwarder opnieuw, in een paar seconden. De forwarder bewaart ook de SHA-256 van de poort (`config/playguard/gate-<hash>.nro`): een poort die op de SD-kaart is gewijzigd, wordt niet gestart (fout 2347-0100, [problemen oplossen](troubleshooting.md#een-beveiligde-forwarder-toont-fout-2347-0100)). Een forwarder die door een eerdere PlayGuard is beveiligd, leest zijn regels van de kaart: zijn uren zonder de pincode worden dan niet gerespecteerd, en de bel biedt *Opnieuw maken* aan.
  - **Een pauze na geweigerde pincodes** (*Pauze na geweigerde pincodes*, onder *Homebrewmenu*): vijf weigeringen achter elkaar (B op het pincodescherm) en de forwarder vraagt er 15 minuten lang niet meer om, terwijl de poort zegt tot wanneer; 3, 5 of 10 pogingen en 15 minuten of een uur, of nooit. De bel zegt welke forwarder gepauzeerd is, zijn regel ook (een pauze-icoon, en tot wanneer); *Pauze opheffen* (Ⓐ erop, de pincode wordt gevraagd) laat hem meteen weer vragen. Elke pauze en elke opgeheven pauze staat in de wijzigingsgeschiedenis.
  - **Elke poging staat in de wijzigingsgeschiedenis**: geopend met de pincode, geopend binnen zijn uren, of geweigerd (B op het pincodescherm), op het tijdstip waarop het gebeurde; de pogingen van één soort op één dag voor één forwarder vormen één item, met hun tijdstippen. De poort noteert ze op de SD-kaart; PlayGuard leest ze bij zijn volgende start. *Geschiedenis › Tonen › Homebrew en forwarders* toont alleen deze, de forwarders en de vergrendeling.
  - **Pogingen deze week** (in *Geschiedenis* en onder *Homebrewmenu*): voor de laatste zeven dagen de openingen van elke forwarder, zijn weigeringen en op welke dagen, en de keren dat hij niet opende; vier weken bewaard. Als de weigeringen van deze week meer dan het dubbele zijn van die van de week ervoor (minstens vijf), meldt de bel dat.
  - **Het scherm van de poort zegt meer:** zijn uren zonder de pincode terwijl het erom vraagt, en de weigeringen van die dag tot dan toe (in oranje) terwijl het erom vraagt en na een weigering, onder *Vraag een ouder om de pincode in te voeren*. Na *Protected by PlayGuard* een icoon voor elke regel die hij meedraagt, zoals zijn regel in PlayGuard: een open hangslot (uren zonder de pincode), een vierkant (uren waarin hij nooit opent), een timer (de speeltijd van vandaag). Na een weigering probeert Ⓐ het opnieuw en gaat Ⓑ terug naar het HOME-menu (net als 15 seconden wachten), totdat de pauze na weigeringen begint. Het zegt ook tot wanneer hij zonder de pincode opent, wanneer een pauze afloopt (*over 12 min*), de resterende speeltijd van vandaag (met *Dicht zodra de speeltijd van vandaag op is*) en de foutcode als het pincodescherm niet kan openen. De rand van het icoon krijgt de kleur van hoe het afliep. Mocht de poort ooit blijven hangen op weg naar de homebrew, dan stopt hij na 10 seconden, terug naar het HOME-menu, in plaats van de console op zijn scherm te laten staan.
  - **Het openingsscherm** (*Openingsscherm*, onder *Homebrewmenu*): gestart vanuit het HOME-menu opent de poort de homebrew eerst zoals PlayGuard zichzelf opent: gecentreerd op het nachtblauw met de wijzerplaat van Cadran rechts, het icoon van de homebrew boven een zachte blauwgroene gloed, zijn naam, *Protected by PlayGuard*, een balk die volloopt en onderaan het beeldmerk van PlayGuard met zijn draaiende ring. Daarna het pincodescherm van de console, en de homebrew meteen zodra de pincode klopt (binnen zijn vrije uren, direct na de balk). Het duurt 2 seconden, tenzij ingesteld op 1, 3 of 5 seconden, of *Uit* (meteen het pincodescherm); Ⓐ of Ⓑ slaat de rest over, en opnieuw proberen na een weigering toont het niet nog eens. Het wordt bij elke start van de SD-kaart gelezen (`config/playguard/gate_wait.txt`), dus het wijzigen maakt de forwarders niet opnieuw.
- **Zijn icoon uit het album** (Ⓐ › *Pictogram wijzigen*): de nieuwste 30 schermafbeeldingen van het album in een raster, elk als het icoon dat ze zou geven, eerst dat van de `.nro` en het gebruikte aangevinkt; ernaast het icoon dat het HOME-menu nu toont en het icoon dat het zal tonen, de game waarin de schermafbeelding is gemaakt en wanneer (*Vandaag*, *Gisteren*, tot op de seconde als er twee in dezelfde minuut zijn gemaakt). L en R verschuiven het vierkant van het icoon over de schermafbeelding (vijf plaatsen, van de linkerrand tot de rechterrand), Y neemt weer dat van de `.nro`, A het geselecteerde: het wordt de JPEG van 256 × 256 voor het HOME-menu. Opnieuw installeren houdt het, met zijn kader.
- **Iconen en talen:** een icoon van een andere grootte, een PNG, of een JPEG met EXIF-gegevens of progressieve codering (het HOME-menu toont die als een "?") wordt een gewone JPEG van 256 × 256. De controlegegevens van de forwarder vermelden alleen Amerikaans Engels, de taal van zijn enige icoon: zo verschijnt het icoon op een console die op elke taal is ingesteld.
- **Bij het starten** (Ⓐ op een forwarder, zoals de forwarderopties van sphaira): *Bij het starten een gebruiker kiezen* (standaard uit) vraagt bij het starten wie er speelt, zoals een game dat doet, zodat zijn speeltijd in *Activiteit* die van die gebruiker is (het eigen profiel van een ouder laat de statistieken van het kind zoals ze zijn); de speeltijdlimiet van de console telt hem toch mee, wie er ook speelt, zoals hij elke game meetelt. *Schermafbeeldingen* en *Video-opname* (standaard aan) laten de opnameknop ze maken. Elke schakelaar vraagt om de pincode, maakt de forwarder opnieuw (hetzelfde programma-ID) en staat in de wijzigingsgeschiedenis; *Details* vermeldt ze.
- **Alle forwarders beveiligen met de pincode** en **Alles opnieuw installeren** (het icoon en de versie van een nieuwe `.nro`, de poort zoals deze PlayGuard hem meebrengt, een uit het HOME-menu verwijderde forwarder teruggezet): elk met één druk.
- **Homebrewmenu vergrendelen** (*Homebrewmenu*, als laatste op het scherm): noch het album noch een game die wordt gestart terwijl R ingedrukt wordt gehouden, opent nog het homebrew-menu, dus een homebrew start alleen nog vanaf zijn forwarder, die om de pincode kan vragen.
  - **Stap voor stap:** zolang het niet kan worden vergrendeld, toont Ⓐ de stappen, genummerd, de volgende in oranje en met één druk bereikbaar (*Volgende stap*): een pincode, de signaturepatches, de forwarder van PlayGuard, een homebrew-menu beveiligd met de pincode (sphaira als het in `/switch` staat, anders hbmenu, in één keer beveiligd gemaakt), één keer gestart vanuit het HOME-menu; de lijst komt terug na elke stap die een forwarder maakt.
  - Het vraagt twee wegen terug naar PlayGuard vanuit het HOME-menu: zijn eigen forwarder, en een forwarder van een homebrew-menu (sphaira of hbmenu, naar de naam van de `.nro`) beveiligd met de pincode, die elke homebrew, ook een homebrewwinkel, achter een pincode houdt. En een pincode op de console, en de signaturepatches die forwarders nodig hebben. Tot dan zegt Ⓐ wat er ontbreekt.
  - De weg terug via het menu telt zodra hij werkend is gezien: één keer gestart vanuit het HOME-menu, met de pincode ingevoerd bij zijn poort (het logboek van de poort zegt dat, gelezen als PlayGuard opent of vergrendelt). Een menu dat beveiligd is maar nooit zo is gestart, wordt als zodanig vermeld.
  - Vergrendelen toont de controles als lijst (een blauwgroen vinkje voor wat klaar is, een oranje of koraalrode driehoek voor wat dat niet is), en vraagt om Ⓐ 2 seconden ingedrukt te houden op *Vergrendelen* (of er een vinger op te houden), terwijl een balk zich vult; laat je eerder los, dan verandert er niets. Daarna komt het **herstelblad** (ook geschreven naar `config/playguard/rescue.html`, een pagina om op een computer te openen en af te drukken, in de taal van de interface, met de QR-code): hoe je het ongedaan maakt vanaf een computer, of met de herstel-sysmodule als die is geïnstalleerd, en een QR-code naar de hulp ([hieronder](troubleshooting.md#homebrewmenu-vergrendeld)); *Homebrewmenu controleren* heeft het ook.
  - Op emuMMC (of een sysMMC waarop een emuMMC is ingesteld) waarschuwt het dat de vergrendeling op de SD-kaart staat, zodat beide systemen haar lezen, terwijl de forwarders per systeem zijn: het andere systeem heeft geen van deze wegen terug.
  - Zolang het vergrendeld is, wordt het verwijderen van de eigen forwarder van PlayGuard, of van het laatste beveiligde menu (of het weghalen van zijn pincode), geweigerd: ontgrendel eerst. Het verwijderen van een van meerdere beveiligde menu's vraagt eerst om bevestiging, in koraalrood, met de namen van de menu's die overblijven.
  - Vóór het vergrendelen biedt het aan om de forwarders die PlayGuard heeft gemaakt en die nog niet beveiligd zijn, in één keer te beveiligen, en zegt het welke nog homebrew zouden starten zonder de pincode (een menu dat niet beveiligd is, of een dat een andere app heeft gemaakt), welke naar de SD-kaart schrijven (een bestandsbeheerder, een installer zoals Cyberfoil of Goldleaf, een FTP-server: die zouden het bestand van de vergrendeling kunnen wijzigen), en wanneer uLaunch is geïnstalleerd (dat start homebrew op zijn eigen manier, waar de vergrendeling niet bij komt).
  - Een pincode die op één dag drie keer of vaker is geweigerd bij de poort van een forwarder, wordt in de bel gemeld (welke forwarder, de dag, hoe vaak).
  - Als het schrijven van een bestand mislukt, zegt de foutmelding waarom wanneer dat te achterhalen is: de SD-kaart is vol, of er kan helemaal niets op worden geschreven (de schrijfbeveiligingsschakelaar, of de archiefbit die *Tools* van hekate herstelt).
  - *Homebrewmenu controleren* toont wat Atmosphère zal lezen (of het album en een game die met R is gestart het homebrew-menu openen), de wegen terug, de signaturepatches, de versie van Atmosphère waarop het is vergrendeld en de herstel-sysmodule; het diagnoserapport bevat hetzelfde. Niet vergrendeld, zegt het dat eerst en biedt het *Vergrendelen* aan, met de stappen die nog te doen zijn (het herstelblad blijft); bij een verouderde herstel-sysmodule de PlayGuard waarmee die kwam en hoe je hem vervangt (*Module bijwerken* als deze build hem meebrengt). Op emuMMC, of op sysMMC met een ingestelde emuMMC, zegt het dat het andere systeem de vergrendeling ook leest, zonder de forwarders die hier zijn geïnstalleerd.
  - Het zijn een paar regels aan het einde van `atmosphere/config/override_config.ini`, tussen twee opmerkingen van PlayGuard; de rest van het bestand blijft zoals het is, en wordt zoals het was bewaard in `config/playguard/override_config.ini.backup`. Atmosphère leest het bestand bij elke start van een programma: geen herstart nodig. Ontgrendelen haalt de regels weg en schrijft de eigen waarden van Atmosphère terug voor de sleutels die de rest van het bestand niet instelt (Atmosphère houdt wat het heeft gelezen tot de console opnieuw opstart).
  - Vergrendelen en ontgrendelen vragen om de pincode, wat *PlayGuard beveiligen met de pincode* ook zegt, en staan in de wijzigingsgeschiedenis. Zolang het vergrendeld is, controleert PlayGuard bij elke start of beide wegen terug er nog zijn; de bel meldt het als er een verdwenen is. Een vergrendeling die buiten PlayGuard uit het bestand is gehaald (een computer, een homebrew), wordt gemeld in de bel en de wijzigingsgeschiedenis; PlayGuard zet haar nooit vanzelf terug, want het bestand verwijderen is de weg terug vanaf een computer. Een forwarder verwijderen (Systeeminstellingen › Gegevensbeheer, waar geen pincode wordt gevraagd) heft de vergrendeling nooit op: het neemt alleen een weg terug weg.
  - Vanaf een computer: verwijder `atmosphere/config/override_config.ini` (of zet de back-up terug) en start de console daarna opnieuw op.
  - Met de herstel-sysmodule geïnstalleerd (optioneel, [sysmodule/README.md](../../sysmodule/README.md), in het Engels) heft een `RESCUE`-verzoek haar ook op bij het opstarten, en dat doet ook een update van Atmosphère sinds de vergrendeling (de signaturepatches kunnen achterlopen, en zonder die patches start geen enkele forwarder): de volgende start van PlayGuard meldt het, en biedt aan om opnieuw te vergrendelen.
- **Een forwarder die buiten PlayGuard is verwijderd** (in de systeeminstellingen): één keer gemeld in de wijzigingsgeschiedenis, en in de bel zolang dat zo blijft, met *Opnieuw installeren* of *Vergeten* in zijn regel.
- **Signaturepatches:** de console start een forwarder alleen met signaturepatches voor fs en loader (sys-patch of sigpatch-bestanden), met elk patroon toegepast dat sys-patch voor deze firmware heeft. Als ze ontbreken, of als sys-patch er een heeft overgeslagen (een oudere sys-patch op een nieuwere firmware), meldt het scherm dat boven de lijst, met wat er niet gepatcht is, en toevoegen, die van PlayGuard zelf maken of ze allemaal opnieuw installeren zegt eerst dat de forwarder nog niet zou starten: toch toevoegen (hij start zodra de patches er zijn), of annuleren. Het homebrewmenu vergrendelen wacht ook op ze (*Console › Gamepatches* geeft de details). sigpatch-bestanden die naast sys-patch zijn achtergebleven, worden gemeld, en *Console › Gamepatches* zet ze opzij (verplaatst, nooit verwijderd). Elke forwarder houdt de loader waarmee hij is gebouwd: voor die van een oudere PlayGuard wordt aangeboden ze opnieuw te maken (de bel, *Console › Forwarders*). De `.nro` blijft waar hij is; een forwarder verwijderen raakt hem nooit aan.
- **Speeltijd:** gestart vanaf zijn icoon draait een homebrew als een game: de speeltijdtimer telt hem mee en schort hem op zoals elke game zodra de tijd om is, en *Activiteit* toont hem. PlayGuard blijft bereikbaar via het homebrew-menu.
- **In de back-ups van de instellingen** (*Geschiedenis › Back-ups*): wat PlayGuard heeft gemaakt en of het homebrew-menu vergrendeld was. Een back-up terugzetten biedt, na de rest, aan om de forwarders terug te zetten die niet meer in het HOME-menu staan (beveiligd zoals ze waren, met hun uren en icoon); een vergrendeling wordt alleen gemeld, en moet hier opnieuw worden ingesteld.
- Elke wijziging vraagt om de pincode als *PlayGuard beveiligen met de pincode* dat doet, wordt in de alleen-lezenmodus geweigerd en staat in de wijzigingsgeschiedenis. Wat PlayGuard heeft gemaakt, wordt per systeem (sysMMC / emuMMC) bewaard, in `forwarders.json`.

### Toegang op afstand

*Toegang op afstand ›*, een tegel onder *Hulpmiddelen*: een optionele verbinding met een MQTT-broker en Home Assistant op je lokale netwerk, standaard uit ([de Home Assistant-handleiding](../home-assistant.md), in het Engels). De broker en zijn MQTT-versie, de naam van de console, wat er met opdrachten gebeurt (vragen op de console, ze uitvoeren, ze weigeren), of ze de speeltijdtimer mogen wijzigen (standaard uit), de discovery van Home Assistant, de eigen gegevens van de console (model, versies, opslag, batterij, netwerk, games; het serienummer alleen met *Het serienummer publiceren*), QR-codes die de PlayGuard-integratie vanuit HACS installeren en de console toevoegen (eenmaal geïnstalleerd: de versie ervan, en de nieuwere release), de live status, *Nu synchroniseren* en het logboek.

### Optionele modules

*Optionele modules ›*, een tegel onder *Hulpmiddelen*: de herstelmodule en de agent voor de verbinding op afstand installeren, bijwerken, starten bij het opstarten of verwijderen, elk op een eigen tabblad, vanuit de kopieën die PlayGuard meebrengt (geen computer nodig). Elke wijziging vraagt om de pincode als *PlayGuard beveiligen met de pincode* dat doet.

### De regels van de console

- **Firmware, Atmosphère, hekate** (als zijn `bootloader/update.bin` op de SD-kaart staat) **en sys-patch** (als het is geïnstalleerd; zijn versie uit zijn `config/sys-patch/log.ini`), elk een tegel met de geïnstalleerde versie, dan de nieuwere versie die de laatste updatecontrole vond in een oranje pil (*23.0.1 beschikbaar*, de tegel oranje omrand, en hoeveel versies achter eronder als het er meer dan één zijn), of *Up-to-date*; een waarschuwing in de hoek van de firmwaretegel als de nieuwste firmware eerst een nieuwere Atmosphère of hekate nodig heeft, of door die nog niet wordt ondersteund (de release-opmerkingen van de firmware zeggen waarom: hun release-opmerkingen zeggen welke firmware ze ondersteunen).
- **Release-opmerkingen:** Ⓐ op elk ervan opent ze: een tabblad voor de geïnstalleerde versie, één voor elke nieuwere (hoogstens vijf) en oudere in de overige tabbladen (zes in totaal; Ⓛ / Ⓡ wisselen ertussen), geopend op de volgende versie, met labels (een vinkje voor de geïnstalleerde versie, een glinstering voor een nieuwere, de nieuwste firmware die wordt ondersteund, beveiligingsfixes, ouderlijk toezicht gewijzigd), de opmerkingen zoals gepubliceerd (in het Engels), *Online bekijken* (een QR-code) en wanneer ze zijn opgehaald. Die van Atmosphère, hekate en sys-patch komen uit hun GitHub-releases (die van sys-patch uit `impeeza/sys-patch`), die van de firmware van switchbrew; ze worden op de SD-kaart bewaard om offline te lezen en één keer per dag opnieuw opgehaald (Ⓧ nu).
- **Compatibiliteit**, dan **opslag** (emuMMC of sysMMC, met een info-icoon als het andere systeem een eigen ouderlijk toezicht heeft: Ⓐ legt het uit).
- Of Atmosphère **het serienummer afschermt** (*Ja (XAW00000000000)*, het nummer dat het systeem dan ziet; op emuMMC, als het niet is afgeschermd, een waarschuwingsicoon, dat Ⓐ uitlegt), dan het **echte serienummer** van de console, met het midden gemaskeerd: als het is afgeschermd, gelezen uit de PRODINFO-back-up die Atmosphère in `atmosphere/automatic_backups/` bewaart (die van deze console, op zijn apparaat-ID, als de SD-kaart er meerdere bevat). Ⓐ op het info-icoon toont het helemaal en waar het vandaan komt.
- **Gamepatches** (sys-patch of sigpatch-bestanden, met sys-patch als aanbeveling als er alleen bestanden worden gebruikt: een info-icoon, of een waarschuwingsicoon als er iets moet worden nagekeken, dat Ⓐ uitlegt).

## Voorkeuren

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=preferences)*

- Wat PlayGuard zelf instelt:
  - Eerst als tegels: taal (elke taal van de console, of die van de console zelf), thema (standaard donker, licht of dat van de console), tekstgrootte (automatisch / normaal / groter) en het tabblad om mee te starten.
  - **De intro** bij het opstarten: het laadscherm (zijn icoon met de draaiende ring, de balk, de wijzerplaat van Cadran rechts) gedurende 2 seconden (standaard aan; Ⓐ of Ⓑ slaat het over; uit, dan opent PlayGuard zodra het klaar is).
- De berichten die het toont:
  - De soorten meldingen (zie [hierboven](#hoe-meldingen-zich-gedragen)).
  - Een **maandelijkse herinnering om PlayGuard te steunen** (standaard aan, nooit in de eerste maand en niet meteen na een update; *Niet meer tonen* op de herinnering of deze schakelaar zet haar voorgoed uit, ook na updates).
- Hoe het bijwerkt: *Elke dag op updates controleren* bij het opstarten, en *Bijwerken met* (zie [Info](#versie-en-updates)).
- **Eerste stappen** opnieuw (ook in *Info › Hulp en diagnose*).
- Een instelling die bij één functie hoort, staat bij die functie: extra tijd in *Speeltijdtimer*, ouderlijk toezicht weer inschakelen in *Beveiliging*, de klokcontrole in *Console › Netwerkklok*. De wijzigingsgeschiedenis en de back-ups hebben hun eigen tabblad, [Geschiedenis](#geschiedenis--wat-playguard-veranderde-en-de-back-ups); nu op updates controleren en *Hulp en diagnose* staan in *Info*.

## Info

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=about)*

### Versie en updates

- Bovenaan zijn kaart: het logo, de naam, de versie.
- De geïnstalleerde PlayGuard: versie (in oranje als er een nieuwere is; Ⓐ opent de **release-opmerkingen**).
- Nu **op updates controleren**; *Elke dag op updates controleren* bij het opstarten staat in *Voorkeuren*.
- *Bijwerken met* (in *Voorkeuren*): in PlayGuard, dat de release downloadt, controleert, zichzelf vervangt en herstart, of sphaira, Homebrew App Store of met de hand (gevraagd bij de eerste update als er nog niets is gekozen). Overdragen aan een store vraagt om de pincode zoals een wijziging (*PlayGuard beveiligen met de pincode › Vóór een wijziging*), en altijd zolang het homebrew-menu vergrendeld is: een store start elke homebrew. Geïnstalleerd vanuit de Homebrew App Store of de App Store van sphaira is de store daar de aanbevolen weg, zodat hij de versie bijhoudt; als zijn catalogus de nieuwe release nog niet heeft (hij publiceert er een na een controle, vaak een paar dagen later), zegt PlayGuard dat en biedt het aan om in plaats daarvan ter plekke bij te werken.
- Zolang er een nieuwere release bekend is, opent PlayGuard op **PlayGuard X.Y.Z is beschikbaar** (de datum en de opmerkingen): *Bijwerken*, of *Later* voor 1 dag, 1 week of hoogstens 30 dagen.
- De release-opmerkingen zijn die van PlayGuard zelf, op hetzelfde scherm als die van Atmosphère en hekate in *Console*: uit de GitHub-releases, een tabblad per versie (de geïnstalleerde, de nieuwere, de oudere), elk met de hoogtepunten als kaarten en alle wijzigingen, bewaard op de SD-kaart om offline te lezen (die van de geïnstalleerde versie zitten ook in de app, voor als er nog niets is opgehaald).
- Na een update opent PlayGuard één keer op **Nieuw in X.Y.Z** (dezelfde hoogtepunten, daarna de QR-codes).

### Hulp en diagnose

*Hulp en diagnose ›*, een eigen scherm, onder *Op updates controleren*:

- **Bovenaan**, onder *Aan de slag*, twee tegels: *Eerste stappen* en *Rapport versturen…*; daarna *Diagnose*.
- **Eerste stappen** opent de gids opnieuw (met een stap *De smartphone-app ontkoppelen* zolang die gekoppeld is, een stap *Alarm "Tijd is om" weer inschakelen* zolang het uit staat, een stap *PlayGuard beveiligen* zodra er een pincode is — de keuze *PlayGuard beveiligen met de pincode* —, en een schakelaar zodat de gids niet meer bij het opstarten verschijnt). Onder *Sluiten* toont *PlayGuard steunen* de QR-codes voor een bijdrage.
- Hoe PlayGuard is gestart (een applicatie boven een game of vanuit zijn HOME-pictogram, of een applet) en zijn gegevensmap.
- **Een diagnoserapport exporteren**, of **online versturen** (*Rapport versturen…*; zie [Een bug melden](troubleshooting.md#een-bug-melden)); hoeveel opgeslagen rapporten, crashbestanden, sessielogs en links van online verstuurde rapporten er bewaard blijven, op aantal, leeftijd of grootte.

### Met dank aan

*Dankwoord en licentie ›*: wie het heeft gemaakt, waarop het voortbouwt, de licentie en de broncode als QR-code, in één dialoogvenster. Daarna hoe je **PlayGuard steunt** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), getoond als QR-codes om met een telefoon te scannen), en een klein *Made in France* 🇫🇷.

## Veilig ontworpen

**De speeltijdlimiet veilig schrijven.** De configuratie van de timer overschrijven terwijl die aftelt, maakt Atmosphère instabiel. Daarom controleert PlayGuard eerst de toestand; als de timer actief is, zegt de bevestiging dat ouderlijk toezicht tijdelijk wordt ontgrendeld — met de opgeslagen pincode, **je hoeft hem niet te onthouden**. Eén druk ontgrendelt dan, controleert of het systeem de ontgrendeling echt meldt, schrijft, en **vergrendelt meteen weer** (of stelt dat voor, als *Ouderlijk toezicht na een wijziging weer inschakelen* uit staat). De servicelaag controleert de toestand opnieuw vlak voor het schrijven, dus geen enkel scherm kan de beveiliging overslaan.

> [!CAUTION]
> Een limiet onder de tijd die vandaag al is gespeeld, onderbreekt de game zodra ouderlijk toezicht weer is vergrendeld. De bevestiging zegt het als dat zou gebeuren.

**Geen crashes meer op 22.5.** `pctl:a`, de bevoorrechte dienst voor ouderlijk toezicht, accepteert **één enkele sessie**. Oudere builds van de oorspronkelijke app hielden die open, waardoor de pincodevraag van het HOME-menu (of de pincode-applet) haar niet kon krijgen en Atmosphère kon crashen. PlayGuard opent de sessie voor elke actie en geeft haar meteen vrij; periodieke vernieuwingen pauzeren zolang de app op de achtergrond staat. *(Diagnose door [de fork van anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Niets draait op de achtergrond.** De limiet, de pincode, de waarschuwingen en de onderbreking zijn die van de console zelf; PlayGuard wijzigt alleen hun instellingen. De optionele verbinding op afstand werkt terwijl PlayGuard open is, of altijd zodra de optionele agent ervan is geïnstalleerd, en voert een opdracht precies zo uit als een wijziging op de console (hetzelfde ontgrendelen en opnieuw vergrendelen, dezelfde alleen-lezenmodus en geschiedenis); ze kan nooit het ouderlijk toezicht wissen, de smartphone-app ontkoppelen of aan de pincode komen.

## Wat telt als speeltijd

De speeltijdtimer is die van de console: PlayGuard leest hem, het telt zelf niets. Wat bekend is, en hoe zeker ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), in het Engels, bevat de metingen):

| Situatie | Telt het? | Hoe zeker |
|---|---|---|
| Een game die draait | Ja | Gemeten (22.0.0) |
| **Een game die open blijft in het HOME-menu** (onderbroken op de achtergrond) | **Ja**: sluit hem, of zet de console in de slaapstand | Gemeld door ouders ([Arqade](https://gaming.stackexchange.com/questions/398954/)); past bij de metingen (de console telt zolang er een applicatie open is) |
| Slaapstand | Nee | Gemeld ([Arqade](https://gaming.stackexchange.com/questions/398954/)); niet gemeten door PlayGuard |
| **PlayGuard geopend boven een game** (als applicatie) | **Ja**, als die game | Gemeten (22.0.0): open het met de gebruiker van een ouder, of vanuit het album om snel te kijken |
| **PlayGuard gestart via een forwarder** (zijn icoon in het HOME-menu, ook een applicatie) | **Ja**, onder de naam van het icoon | Afgeleid: een applicatie zoals elke andere; niet gemeten door PlayGuard |
| PlayGuard geopend vanuit het album (appletmodus) | Nee | Afgeleid: het is geen applicatie, en tijd buiten applicaties leek niet te tellen |
| Zolang ouderlijk toezicht tijdelijk ontgrendeld is (ook de pauze van PlayGuard zolang je erin bent) | Misschien niet | Niet gemeten: nog een open vraag |
| Nadat de klok is gewijzigd | De dag begint opnieuw; na een sprong terug stopt de telling tot de klok het oude tijdstip voorbij is | Waargenomen (22.0.0); *Vandaag* meldt wanneer de console niet telt |

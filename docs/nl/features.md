<!-- translated from docs/features.md, sha256 86931c8e4205 -->
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
| **Pincode** | **Voorkeuren** |
| ![Pincode](../../images/screenshots/security.png) | ![Voorkeuren](../../images/screenshots/preferences.png) |

## Bediening

| Knop | Wat hij doet |
|---|---|
| ↑/↓ | Verplaatsen |
| Ⓐ | Bevestigen |
| Ⓑ | Terug of annuleren (twee keer in de zijbalk om af te sluiten) |
| Ⓧ | Vernieuwen (op de tabbladen die de console lezen) |
| Ⓨ | Meldingen, vanuit elk tabblad |
| **−** | Taal en thema, vanuit elk tabblad (de voettekst laat ze weg; de bel, de taal en het thema in de kopregel tonen ze) |
| **+** | Slaat de limieten per dag op (en de week die in *Speeltijdtimer* is bewerkt) |
| **ZR** | *Ongedaan maken*, een paar seconden nadat een wijziging is opgeslagen |

De app is in tabbladen ingedeeld, zoals de systeeminstellingen; elk tabblad heeft hieronder een eigen sectie.

- **Wat mislukt, staat in een dialoogvenster; wat lukt, in een korte melding.** Een dialoogvenster zegt alleen wat nu voor deze console geldt: dat de lopende game zou worden onderbroken, alleen zolang er een game open is; dat de smartphone-app een wijziging zou kunnen overschrijven, alleen zolang die gekoppeld is en kan synchroniseren.
- **De kopregel** geeft op elk tabblad aan wanneer ouderlijk toezicht tijdelijk ontgrendeld is (met de pil), en toont een doorgestreept potlood rechts van de titel wanneer de app alleen-lezen is; op een van beide tikken brengt je naar de plek waar je het ongedaan maakt (*Pincode › Ouderlijk toezicht weer inschakelen*, het firmwarescherm).
- **In alleen-lezenmodus** blijven de acties op hun plaats, grijs, en zeggen ze waarom als je erop drukt.

## Het uiterlijk: Cadran

Het uiterlijk van PlayGuard heet Cadran: een smalle balk met iconen voor de tabbladen, een palet in marineblauw of koel wit (volgens het thema), kaarten in plaats van lijsten. Elk tabblad opent op wat het belangrijkst is (een ring, tegels, kaarten, chips), met de overige instellingen eronder.

- **De pil in de kopregel** zegt waar het tabblad over gaat (*Ouderlijk toezicht actief*, *Timer actief*…); zolang ouderlijk toezicht ontgrendeld is, zegt ze dat op elk tabblad, en erop tikken opent *Pincode › Ouderlijk toezicht weer inschakelen*. Secundaire tekst is nooit kleiner dan 15 px, zodat je hem vanaf de bank op een tv kunt lezen.
- **Dialoogvensters** zijn kaarten met hun knoppen als pillen, de bevestiging in blauwgroen (in koraalrood als ze iets verwijdert of stopt).
  - De keuzes openen als een kaart boven de knopaanwijzingen, de huidige keuze op zacht blauwgroen met een vinkje. De blauwgroene rand hoort overal alleen bij de focus: een niveau, een account, een geldende keuze houdt de zachte vulling.
  - Een lang dialoogvenster vervaagt onderaan zolang er meer volgt, en de waarschuwing erin (een game die op het punt staat te worden onderbroken) komt eerst, in een oranje kaart.
  - Een mislukking heeft als titel wat er mislukt is, de foutcode klein onder de reden, en biedt *Rapport versturen* aan als dat helpt.
  - Meldingen verschijnen als kaarten onder de kopregel, met een blauwgroen vinkje voor wat is gedaan en een oranje teken voor wat is geweigerd.
- **De andere schermen** krijgen dezelfde look: de editor per dag van een profiel toont de week zoals in *Speeltijdtimer* (op dezelfde manier bewerkt, de snelle waarden als tegels); *Geïnstalleerde games controleren* en *Alle games* zetten hun regels in kaarten; *Profielen* en *Hulp en diagnose* geven elke regel een eigen icoon.
- **Tekstgrootte** (*Voorkeuren*, of **AA** in de kopregel):
  - *Automatisch*, de standaard, maakt alle tekst 25 % groter zolang de console in het dock staat, op de tv, en zet hem terug in handheldmodus; *Groter* houdt hem altijd groter, *Normaal* nooit.
  - De kopregel, de zijbalk en de voettekst houden hun grootte (ze hebben geen ruimte voor meer), net als de namen van de niveaus in *Beperkingen*, die al groot zijn.
  - Het verandert meteen, op elk scherm, zonder herstart.
  - De 25 % komen van een 55"-tv in 4K: de kleinste tekst (15 px) is dan 14,6 mm hoog in hoofdletters, 20′ gezichtshoek op 2,5 m, waar lezen comfortabel is (16′ in normale grootte, het absolute minimum).
- **Visuele effecten** (*Voorkeuren*): *Lichter op de accu*, de standaard, laat de achtergrondgloed weg, het kloppen van de ring op het kinderscherm in de laatste minuten en het glijden van de boog naar een nieuwe waarde, zolang er geen lader is aangesloten (handheld, zonder kabel); *Altijd volledig* houdt ze. Elke 5 seconden opnieuw gelezen, dus de console aansluiten brengt ze terug.
- **Ongedaan maken:** nadat een wijziging is opgeslagen, blijft **ZR** *Ongedaan maken* een paar seconden in de voettekst staan: het opent die wijziging in de wijzigingsgeschiedenis, om de vorige waarde terug te zetten. Een lege wijzigingsgeschiedenis zegt dat in een kaart.

## Meldingen — wat je aandacht vraagt, vanuit elk tabblad

Een bel in de kopregel, links van de versie, verzamelt wat de ouder moet doen; Ⓨ opent de lijst vanuit elk tabblad (de teller op de bel toont *2* als er twee dingen te doen zijn).

Links van de bel staan de taal (de code, *FR*, *EN*…) en het thema (een zon voor licht, een maan voor donker, een half gevulde cirkel als het de console volgt), en daarna de tekstgrootte (**AA**, met de grote A in blauwgroen zolang de tekst groter is): een tik opent dezelfde lijst als in *Voorkeuren*, en **−** biedt ze aan vanuit elk tabblad. Een nieuwe taal of een nieuw thema geldt zodra PlayGuard opnieuw is gestart; de tekstgrootte meteen.

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

Ter informatie: een build waarvan is teruggegaan, een firmware die niet met deze versie is getest, een nieuwere versie van Atmosphère, hekate of de firmware (gecontroleerd met *Elke dag op updates controleren*; de melding over de firmware zegt dat je moet wachten tot Atmosphère hem ondersteunt), het serienummer dat op emuMMC niet is afgeschermd, het eigen ouderlijk toezicht van het andere systeem, een bestand van PlayGuard zelf dat beschadigd werd gevonden en opzij is gezet (een korte melding bij het opstarten verwijst ernaar).

### Hoe meldingen zich gedragen

- Elke sectie toont haar meldingen in een vaste volgorde, welke controle ook als eerste klaar was.
- **In de zijbalk** telt een oranje badge naast de naam van een tabblad de meldingen die te doen zijn, op de plek waar ze worden opgelost: het tabblad Pincode (geen pincode, een zwakke pincode, PlayGuard zonder pincode, tijdelijk ontgrendeld), de Speeltijdtimer (het alarm uit, de console die niet telt), *Vandaag* (wijzigingen buiten PlayGuard, extra tijd om terug te zetten), de Console (de meldingen hierboven, en de netwerkklok). Genegeerde meldingen en meldingen ter informatie worden niet geteld.
- In het tabblad staat dezelfde badge op de regel waar elke melding wordt opgelost (*Netwerkklok*, *Geïnstalleerde games controleren*, *Opslag*, *Gamepatches*; *Pincode instellen of wijzigen*, *PlayGuard beveiligen met de pincode*, *Ouderlijk toezicht weer inschakelen*; *Geavanceerd* van de Speeltijdtimer voor het alarm), en het tabblad opent op de eerste daarvan.
- **Ⓐ** op een melding voert de oplossing uit (*Inschakelen*, *Instellen*, *Bijwerken*…) of opent het tabblad waar je het oplost. Een melding verdwijnt vanzelf zodra wat ze meldt is opgelost.
- **Ⓧ** markeert een melding ter informatie als gelezen; **Ⓨ** markeert ze allemaal. Een melding die als gelezen is gemarkeerd, blijft gelezen bij de volgende start, tot de tekst ervan verandert.
- **Negeren:** een paar meldingen die te doen zijn, kun je met Ⓧ negeren, na een bevestiging en de pincode: een pincode die iedereen als eerste zou proberen, PlayGuard dat zonder pincode opent, het alarm "Tijd is om" uit, een bijna volle SD-kaart (nooit: geen pincode, ontgrendeld toezicht, de klok, de speeltijdtimer die niet telt of de bestanden van PlayGuard zelf). Een genegeerde melding telt niet meer mee en staat als laatste onder *Genegeerd*, waar Ⓐ hem weer laat meetellen. Hij komt vanzelf terug na 30 dagen (tenzij *Genegeerde waarschuwingen na 30 dagen weer meetellen* uit staat), zodra wat hij meldt is opgelost en daarna opnieuw gebeurt, of, voor de pincode, bij een nieuwe pincode met een andere lengte.
- *Voorkeuren › Meldingen* zet drie soorten uit: nieuwe versies van Atmosphère, hekate en firmware; de speeltijd van de week; herinneringen (back-ups, het andere systeem, alle games geblokkeerd).
- De toestand erachter wordt gelezen als PlayGuard start, elke minuut op het hoofdscherm, en elke 5 s zolang *Vandaag* wordt getoond.

## Vandaag — de dag in één oogopslag

*[Probeer het in de simulator ›](https://playguard.link/simulator/)*

### De ring en de lijn

**De resterende tijd** als ring: een dag zonder spelen toont *Niet spelen*, niet *0 min*; de extra tijd die vandaag is gegeven, verschijnt als lichter segment aan het einde, en de ring schuift naar elke nieuwe waarde.

Daaronder wat het spelen het eerst stopt:

- vóór het einde van de tijd of het bedtijdalarm de rest van de dag als lijn, met het huidige moment links, elk ervan een markering met het tijdstip (*16:40 tijd om*, *21:00 bedtijd*), blauwgroen tot de eerste, oranje in de laatste 15 minuten;
- anders in woorden: het bedtijdalarm zodra de tijd van de dag op is, wanneer er weer gespeeld mag worden; zolang ouderlijk toezicht ontgrendeld is, de limiet van vandaag gepauzeerd en waarom.

### De acties

- **Extra tijd**, met de hoeveelheden op één druk afstand: een geselecteerde hoeveelheid schuift het einde van de tijd op de lijn op (*met 15 min extra*), of zegt bij de markering van het bedtijdalarm dat dat toch eerst komt; de eerste springt eruit in blauwgroen zodra er 15 min of minder over is, of niets meer.
- **Stoppen voor vandaag**, **Aan het kind tonen**.
- **Limiet vandaag…**: de limiet van vandaag ter plekke gekozen, geschreven na één bevestiging.
- **Het geldende profiel**, met zijn naam, zodra er een profiel is opgeslagen (*Profielen…* als er geen overeenkomt): Ⓐ toont ze, het geldende aangevinkt, om na één bevestiging een ander toe te passen (de dagen die veranderen), of *Profielen beheren…*.
- Zolang ouderlijk toezicht ontgrendeld is, komt **Weer inschakelen** eerst, over de hele breedte van de rij, en gaat *Extra tijd* opzij.
- Staat er op geen enkele dag een limiet, dan opent **Limieten van de week instellen** de week in *Speeltijdtimer*.

### De kaarten en pillen

- Het **bedtijdalarm** (wanneer er weer gespeeld mag worden, of over hoelang het afgaat als de console het niet zegt; uit, *Uit* in het groot en Ⓐ *Instellen*), **ouderlijk toezicht** en de **netwerkklok**, als kaarten.
- De toestand van de console als **pillen**:
  - één zegt of de DNS-omleiding van Atmosphère de servers van Nintendo blokkeert, en Ⓐ erop welke en wat dat verandert: geen synchronisatie voor de smartphone-app, geen netwerkklok die zichzelf instelt;
  - een andere, geopend boven een game, dat de tijd in PlayGuard meetelt, met Ⓐ hoe;
  - een game die open blijft in het HOME-menu, verschijnt als oranje pil (*telt door*, met Ⓐ waarom), als de pil over PlayGuard geopend boven een game het niet al zegt.
- De pillen zetten eerst wat de ouder nodig heeft (oranje, met een driehoek), dan wat aan staat (blauwgroen, met een vinkje), dan de rest (grijs, met een stip): de vorm zegt het net zo goed als de kleur.
- Onder de pillen de extra tijd die nog moet worden teruggezet (Ⓐ zet hem terug) en het alarm "Tijd is om" zolang het uit staat (Ⓐ zet het weer aan).

### Eerste stappen

Zolang de pincode en de limieten van de week niet zijn ingesteld, toont een kaart boven de kaarten de drie stappen (de pincode, een daglimiet, de netwerkklok), elk blauwgroen aangevinkt zodra hij gedaan is of oranje omcirkeld, met hoeveel er gedaan zijn (*1 van 3*); Ⓐ op een stap voert die ter plekke uit.

### Telt de console?

De tijd van PlayGuard zelf telt mee zolang het als applicatie draait, dus met een limiet vandaag moet de resterende tijd dalen zolang het open is. Is die ongeveer 90 s op het scherm niet veranderd (timer aan, een limiet vandaag, niet ontgrendeld, niet geopend vanuit het album), dan meldt een oranje regel onder de gespeelde tijd *De console telt op dit moment geen speeltijd*, met de waarschijnlijke oorzaken:

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
  - na extra tijd vanzelf, zonder vraag en zonder pincode (het verlaagt de limiet alleen, net als ouderlijk toezicht weer inschakelen), en het ontgrendelt even als de timer telt om meteen weer te vergrendelen. Het wacht zolang het niet kan schrijven (alleen-lezenmodus, een firmware die het niet ondersteunt, alle games geblokkeerd) en, een week later op dezelfde dag, zolang de lagere limiet de lopende game zou stoppen (dan de dag erna). Wil je op een dag meer tijd houden, wijzig dan de limiet van die dag;
  - na *Vandaag niet meer spelen* vraagt het of het 0 moet houden of de limiet moet terugzetten (met de pincode, zoals elke wijziging die een limiet verhoogt), of zet het de limiet terug zoals elke wijziging (*Speeltijdtimer › Limiet vanzelf terug na "Vandaag niet meer spelen"*); het terugzetten van "geen limiet" wordt altijd gevraagd.
- **Extra tijd op een andere dag** (*Extra tijd › Een andere dag…*, aan het einde van de lijst): morgen of een van de vijf dagen daarna, elk met zijn limiet erbij, dan de hoeveelheid. De limiet van die dag wordt nu verhoogd, alleen voor die datum, en vanzelf teruggezet (het verlaagt de limiet alleen) de volgende keer dat PlayGuard wordt geopend nadat de dag voorbij is — niet op een console waarop PlayGuard niet wordt geopend; daar houdt ook dezelfde dag van de week erna hem. Eén tegelijk: de lijst biedt dan aan om hem te verwijderen (de limiet meteen terug). Een wijziging van de limiet van die dag in de tussentijd (de week, een profiel…) gaat voor: er wordt niets teruggezet.

### Het kinderscherm

*Aan het kind tonen* op *Vandaag*, om de console over te dragen:

- alleen de resterende tijd (zolang ouderlijk toezicht ontgrendeld is, de limiet van vandaag, gepauzeerd), wanneer het spelen eindigt (*Eindigt rond 16:40*; niet als het bedtijdalarm eerst komt), de bedtijd en wat er gebeurt als de tijd om is;
- groot genoeg om vanaf de andere kant van de kamer op de tv te lezen; de tekst houdt zijn grootte, welke tekstgrootte ook is ingesteld;
- in de laatste 5 minuten wordt het einde oranje en pulseert de ring langzaam; de bedtijd wordt oranje in de laatste 30;
- Ⓐ en Ⓨ, de knoppen van de ouder, staan klein rechtsboven, zodat de voettekst alleen die van het kind toont;
- Ⓐ gaat met de pincode terug naar de schermen van de ouder; Ⓨ geeft van daaruit extra tijd, na de pincode (de hoeveelheden van *Extra tijd*, terwijl het kinderscherm blijft staan); Ⓑ sluit PlayGuard af (en vraagt eerst of ouderlijk toezicht weer moet worden ingeschakeld, zoals bij afsluiten).

Met grotere tekst past *Vandaag* nog steeds op één scherm: de regel boven de kop verdwijnt, de knoppen en de kaarten staan iets dichter op elkaar. Het wordt elke 5 s vernieuwd (Ⓧ vernieuwt nu).

## Speeltijdtimer — de week is de editor

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=play_timer)*

### De week bewerken

Ⓐ op een dag bewerkt hem:

| Knop | Tijdens het bewerken |
|---|---|
| ↑/↓ | Met 15 min wijzigen |
| ←/→ | Naar de volgende dag |
| Ⓛ | Naar maandag–vrijdag kopiëren |
| Ⓡ | Naar het weekend kopiëren |
| Ⓨ | Geen limiet |
| Ⓧ | Een exacte duur (*Een duur invoeren…*: ↑/↓ per 15 min, Ⓛ/Ⓡ per 5 min, met OK geselecteerd) |
| Ⓐ of Ⓑ | Het bewerken beëindigen |

- Met aanraken wordt een aangetikte dag bewerkt, waarna een tik op de boven- of onderkant hem met 15 min wijzigt.
- De wijzigingen worden samen opgeslagen, met **+** of *Opslaan*, zoals in de editor per dag (de bevestiging toont de gewijzigde dagen, voor → na); *Verwerpen* draait ze terug.
- In de balk van elke dag de tijd die de laatste keer op die dag van de week is gespeeld (vandaag inbegrepen, alle accounts), in koraalrood zodra de limiet werd bereikt, de limiet zelf ook, en een legenda onder de week zegt dat (*limiet overschreden*).
- Een gewijzigde dag die nog niet is opgeslagen heeft zijn limiet in oranje en een oranje rand om zijn balk; onder de week de cijfers van de geselecteerde dag (*donderdag: 1 u 20 gespeeld van 2 u*).
- Boven de week, in oranje, een bereikte limiet en wat de console dan doet, of een console die geen speeltijd telt.
- `0` minuten betekent *die dag niet spelen*; *Speeltijdlimiet verwijderen* zet de timer uit.

### Chips en snelle instellingen

**Chips** boven en onder de week openen meteen hun keuzes: de profielen (hoeveel er zijn opgeslagen), elke dag dezelfde limiet, het bedtijdalarm, *Als de tijd om is* en het verwijderen.

- **Elke dag dezelfde limiet:** een snelle lijst of een exacte duur (*Een duur invoeren…*: ↑/↓ per 15 min, Ⓛ/Ⓡ per 5 min).
- **Doordeweeks en weekend ›:** de editor, die eerst om maandag–vrijdag en daarna om het weekend vraagt.
- **Extra tijd vandaag** en **vandaag niet meer spelen**, zoals op *Vandaag*. Een opmerking eronder zegt wat de console bij de limiet doet. Zolang ouderlijk toezicht tijdelijk ontgrendeld is, wordt speeltijd *mogelijk* niet geteld: dat is nog een open vraag ([docs, in het Engels](../parental-controls.md#still-open)).
- Elke bevestiging toont de dagen die veranderen, voor → na.

### Profielen

- **Profielen** opgeslagen op de SD-kaart: een profiel toepassen (een profiel dat al geldt, zegt dat gewoon), bewerken, hernoemen of verwijderen; de huidige limieten opslaan of een nieuw profiel maken.
- Elke naam, in elk schrift (*École*, *周末*, *Выходные* …) — een naam zonder Latijnse letters krijgt een eigen bestandsnaam, en twee namen die hetzelfde bestand zouden opleveren, worden opgemerkt.
- **Gepland profiel** (*Profielen › Gepland profiel*): een profiel voor een periode — vakantie, een reis —, waarvan de eerste en de laatste dag worden gekozen uit de komende 60 dagen.
  - Als de periode begint, vraagt PlayGuard of het profiel moet worden toegepast (de dagen die het verandert, getekend onder de vraag); als ze voorbij is, of de limieten die het verving moeten worden teruggezet of behouden.
  - Eén keer per dag gevraagd, op het hoofdscherm, als er niets anders voor staat: PlayGuard handelt alleen als het wordt geopend, dus een console waarop het niet wordt geopend, houdt de limieten die hij heeft. Een periode die voorbijging zonder dat het profiel ooit is toegepast, wordt vergeten.
  - Eén tegelijk; de regel wijzigt, verwijdert of beëindigt het.

### Bedtijdalarm

- Het tijdstip van het alarm (16:00 tot 23:45, of uit) en wanneer er weer gespeeld mag worden (05:00 tot 09:00), elke dag hetzelfde.
- Of, via *Per dag van de week…* aan het einde van de lijst van het alarm, voor maandag tot vrijdag, het weekend of één dag, terwijl de andere dagen hun eigen tijd houden (elke groep dagen met het tijdstip erbij). Als de dagen verschillen, toont het tabblad *Verschilt per dag*, en de week in *Speeltijdtimer* toont het alarm van elke dag onder de naam ervan.
- De plek ervan in de instellingen van de speeltijdtimer, afgeleid uit de instellingen van de smartphone-app, is op een console geverifieerd (22.0.0); PlayGuard wijzigt het toch pas als de console meldt wat PlayGuard daar leest, controleert na de wijziging het antwoord van de console en zet de vorige instellingen terug als dat afwijkt.

### Instellingen voor extra tijd en Geavanceerd

- **Extra tijd:** de aangeboden hoeveelheden (+15/+30/+1 u, +10/+20/+30 min …) en de gewone limiet zonder vragen terugzetten na *Vandaag niet meer spelen* (extra tijd wordt altijd zonder vragen teruggezet). Beide gebeuren de volgende keer dat PlayGuard wordt geopend, vanaf de volgende dag: een console waarop het niet wordt geopend, houdt de limiet van die dag.
- **Geavanceerd ›**, een eigen scherm: de schakelaar voor de acties van de debugklasse en, zolang die aan staat, het alarm "Tijd is om" aan/uit en het aftellen pauzeren / hervatten. Met dezelfde schakelaar kan een back-up of de geschiedenis het alarm terugzetten.

## Activiteit — wie wat speelde, en hoe lang

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=activity)*

Tijd per game **vandaag**, in de **afgelopen 7 dagen** en **in totaal**, uit het activiteitenlog van de console zelf — voor alle accounts of voor **één gebruikersaccount** (de accounts als chips bovenaan, als de console er meerdere heeft). PlayGuard start op het laatst getoonde account, of op het account dat bij *Standaardaccount* is gekozen (alle accounts, of één).

### Bovenaan

- De accounts als chips, de periode en de export.
- Vandaag, de afgelopen 7 dagen en altijd als cijfers. Onder de afgelopen 7 dagen de vergelijking met de 7 ervoor, per dag: ▲ *+20 min/dag t.o.v. vorige week* in oranje, ▼ minder in blauwgroen, *zoals vorige week* bij hoogstens 5 minuten verschil.
- De vier meest gespeelde games als kaarten (Ⓐ opent er een); over de afgelopen 7 dagen zegt de game die het meest meer is gespeeld dan de week ervoor met hoeveel, vanaf een half uur (▲ *+2 u*).
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

### Exporteren en delen

- **Exporteren naar de SD-kaart** (de chip *Exporteren*) als CSV, JSON, XLSX (Excel) of PDF, één kolom per dag, en daarna de afgelopen 7 dagen, de afgelopen 30 dagen en altijd.
- **Weekoverzicht als QR-code:** de camera van een telefoon leest hem als een paar regels tekst, in de getoonde taal — de afgelopen 7 dagen en hun gemiddelde, de trend, elke dag, de drie meest gespeelde games, de sessies, de langste en de tijd gespeeld tussen 21:00 en 06:00. Er gaat niets online.
- Opent meteen met de laatst gelezen cijfers (bewaard op de SD-kaart tussen twee starts), op de achtergrond vernieuwd als ze ouder zijn dan een minuut; Ⓧ leest ze nu opnieuw, met een laadindicator.
- De tijden zijn bij benadering als de klok van de console is gewijzigd.

## Beperkingen — niveau, leeftijdsclassificatie, communicatie

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=restrictions)*

- **Het beperkingsniveau:** Jong kind, Kind, Tiener en Aangepast naast elkaar als tegels, elk met wat het beperkt (een hangslot voor beperkt, een vinkje voor toegestaan, en de geïnstalleerde games die het blokkeert; Ⓐ stelt het in; geldt *Aangepast* al, dan gaat Ⓐ naar de instellingen eronder); *Geen* eronder. Wat een niveau beperkt (leeftijdsgrens, berichten plaatsen, communicatie), wordt nog eens gemeld voordat het wordt ingesteld.
- **Bij Aangepast:** leeftijdsclassificatie, berichten plaatsen op sociale media, communicatie met anderen.
- **VR-modus** en de **classificatie-instantie** (PEGI, ESRB, USK, CERO …): de lijst zegt voor welke landen elke instantie classificeert (PEGI: het grootste deel van Europa, ESRB: de Verenigde Staten, Canada en Latijns-Amerika, USK: Duitsland, PEGI BBFC: het Verenigd Koninkrijk…).
- **Het aantal games dat mag communiceren** (de lijst zelf staat ruw in het diagnoserapport, tot de opbouw ervan bekend is).
- **Geblokkeerde geïnstalleerde games:** hoeveel van de geïnstalleerde games het geldende niveau blokkeert op hun leeftijdsclassificatie (*3 van 15*), waarbij Ⓐ ze toont met hun classificatie (*PEGI 16*); de tegel van elk niveau zegt het ook (*Geblokkeerde games · 3*).
  - De classificaties zijn die van elke game zelf (de controlegegevens van de game, gelezen door de controle van de geïnstalleerde games bij het opstarten of via *Console › Geïnstalleerde games controleren*), volgens de instantie die de console gebruikt.
  - Een game die deze instantie niet classificeert, telt niet mee, en ook de uitzonderingen die in de smartphone-app zijn ingesteld niet.
  - Het scherm van een game in Activiteit toont de classificatie, en *geblokkeerd op het huidige niveau* als dat zo is.

## Pincode — de pincode, ontgrendelen, games blokkeren, smartphone-app

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=security)*

- **Bovenaan** drie tegels: de pincode, de beperkingen en het tijdelijk ontgrendelen, elk met een blauwgroen vinkje als het is zoals het hoort (een pincode ingesteld, beperkingen aan, vergrendeld) of een oranje waarschuwingsteken als er iets moet gebeuren.
- **Pincode instellen of wijzigen** (pincodescherm van het systeem).
- **Pincode tonen**: na een waarschuwing en de pincode zelf, die elke keer wordt gevraagd; vastgelegd in de geschiedenis, daarna wordt een nieuwe pincode voorgesteld.
- **Tijdelijk ontgrendelen**, **ouderlijk toezicht weer inschakelen**, en of PlayGuard het vanzelf weer inschakelt meteen na een wijziging waarvoor het ontgrendeld moest worden (standaard aan).

### PlayGuard beveiligen met de pincode

De pincode in PlayGuard zelf:

| Instelling | Effect |
|---|---|
| *Nooit* | Geen pincode in PlayGuard (de bel meldt het zolang de console er een heeft) |
| *Vóór een wijziging* (standaard) | Iedereen kan kijken, alleen de ouder wijzigt iets |
| *Om PlayGuard te openen* | Een vergrendelscherm bij het opstarten |

- Eenmaal ingevoerd, geldt de pincode tot PlayGuard wordt gesloten; *Na 5 minuten opnieuw om de pincode vragen* (standaard uit) vraagt er daarna opnieuw om, en brengt het vergrendelscherm van *Om PlayGuard te openen* terug als PlayGuard na 5 min of langer op de achtergrond weer op de voorgrond komt.
- Gecontroleerd in de servicelaag, dus geen enkele wijziging slaat het over; opnieuw vergrendelen vraagt er nooit om. *Pincode tonen* vraagt er elke keer om, in elke modus en zelfs vlak nadat hij is ingevoerd, en een ontbrekend of beschadigd `config.json` geldt als *Vóór een wijziging*.
- **Afsluiten** terwijl ouderlijk toezicht nog ontgrendeld is, vraagt of het weer moet worden vergrendeld (*Weer inschakelen en afsluiten* sluit ook de game waarboven PlayGuard werd geopend, terug naar het HOME-menu in plaats van hbmenu); wordt PlayGuard in plaats daarvan via het HOME-menu gesloten, dan vergrendelt het bij de volgende start opnieuw, en een herstart van de console vergrendelt vanzelf.

### Alle games blokkeren

Eén schakelaar die de limiet van elke dag op 0 zet, zodat er een pincode nodig is om een game te starten — een lichte blokkade zonder leeftijdsclassificatie of communicatiebeperkingen.

- Het blokkeert het starten van games, niet het HOME-menu, en vereist een pincode.
- De vorige limieten komen terug als je het uitzet.
- Zolang het aan staat, worden *extra tijd* en *vandaag niet meer spelen* geweigerd, en vervangen limieten die op een andere manier worden ingesteld (een profiel, een back-up, de geschiedenis…) het.

### Smartphone-app

- Of de app Nintendo Switch Ouderlijk toezicht gekoppeld is, de laatste synchronisatie, en **ontkoppelen** (anders overschrijft de volgende synchronisatie de limieten die hier zijn ingesteld: zolang de app gekoppeld is, vermeldt elke wijziging die PlayGuard laat bevestigen dat).
- Als de DNS-omleiding van Atmosphère (`atmosphere/hosts`) de server voor ouderlijk toezicht van Nintendo ergens anders heen stuurt, kan de app niet synchroniseren: PlayGuard toont dan hier *synchronisatie geblokkeerd (DNS)* en noemt de app nergens anders (geen opmerking in de dialoogvensters, geen melding bij de bel, geen stap in de eerste stappen, geen pil op *Vandaag*, geen hulp bij het ontcijferen, en bij het ontkoppelen wordt niet gezegd wat de app zou overschrijven).
- Zolang de app gekoppeld is en kan synchroniseren, opent *Vóór het ontkoppelen: twee instellingen ontcijferen* de vergelijking van het blok (zie [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), in het Engels).

### Ouderlijk toezicht wissen

Twee bevestigingen, onomkeerbaar; eerst wordt een back-up van de instellingen opgeslagen.

## Console — de netwerkklok, de geïnstalleerde games, wat de console draait

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=console)*

**Bovenaan** wat de console draait, als tegels (de firmware, Atmosphère, en hekate als het op de SD-kaart staat; Ⓐ opent de release-opmerkingen), dan de diagnose, dan de regels van de console, compatibiliteit eerst (alleen een blauwgroen vinkje als PlayGuard op deze firmware is gecontroleerd, anders een waarschuwing en wat er mis is).

### Netwerkklok

*Netwerkklok ›*, een eigen scherm — de klok waarop de speeltijdtimer vertrouwt. Een console die de servers van Nintendo nooit bereikt, houdt deze klok onnauwkeurig, en dat verstoort de speeltijdtimer.

- **Consoleklok en netwerkklok**, tijdzone, nauwkeurigheid. Als de DNS-omleiding van Atmosphère (`atmosphere/hosts`) de tijdserver van Nintendo (`aauth-lp1.ndas.srv.nintendo.net`) ergens anders heen stuurt, kan de console zijn netwerkklok niet meer zelf instellen: dit scherm, de melding bij de bel en de kaart op *Vandaag* zeggen dat en verwijzen naar de meting hieronder.
- **Kies een openbare NTP-server** (≈ 50 ingebouwd, per regio, of je eigen server).
- **Meten** tegen 3 servers (mediaan; een oranje waarschuwing als ze het oneens zijn), dan **de netwerkklok instellen** — een meting blijft 2 minuten bruikbaar, met een aftelling. De bevestiging zegt het: de klok wijzigen zet de speeltijd van vandaag terug op nul, dus de hele limiet is weer beschikbaar.
- **Een controle van de netwerkklok bij het opstarten**: een korte melding als hij meer dan een minuut afwijkt; hij stelt de klok nooit in.

#### Geen enkele server antwoordt

Op een console zonder internet verschijnt *Netwerkklok instellen op de consoleklok* onder het resultaat (en op de kaart van de netwerkklok op *Vandaag*, nadat de meting daar mislukt), achter dezelfde bevestiging.

- PlayGuard heeft niets anders om de consoleklok mee te vergelijken: de bevestiging toont de tijd ervan en vraagt je die eerst met een telefoon of een horloge te controleren, en een consoleklok van vóór 2026 (nooit ingesteld) wordt geweigerd.
- Met *Klok synchroniseren via internet* uit blijft de consoleklok waar hij is, en als je die instelling daarna aanzet, springt hij niet meer; staat ze aan, dan volgt de consoleklok de netwerkklok al, dus zegt de bevestiging dat je haar moet uitzetten en de tijd met de hand moet instellen als die verkeerd is.
- Alleen de klok wordt geschreven, niet het ouderlijk toezicht, dus er hoeft niets te worden ontgrendeld (de pincode van *PlayGuard beveiligen met de pincode* geldt nog wel); zoals elke klokwijziging zet het de speeltijd van vandaag terug op nul.
- Nog niet op een console geprobeerd.

### Geïnstalleerde games

*Geïnstalleerde games controleren* vindt de games die het HOME-menu niet kan starten of tonen — een icoon dat blijft laden, een "!", een gestippeld kader — met wat er mis is en wat je moet doen, onder het icoon van de game, en hoeveel geïnstalleerde games een probleem hebben.

**Wat het vindt:** naam en icoon onleesbaar, de game zelf ontbreekt achter zijn update, niets meer geïnstalleerd, ontbrekende bestanden, beschadigde data die de console vond, een nieuwere firmware die de game of alleen zijn update nodig heeft, sleutels die te nieuw zijn voor de firmware, een ontbrekende licentie (ticket), de console die om een verwijderde update vraagt, een onleesbare SD-kaart, een start die de console weigert.

**De lijst heeft een sectie per oplossing**, elk met het aantal en wat je moet doen, de games met hetzelfde probleem bij elkaar (twee games met dezelfde naam worden onderscheiden door hun versie of titel-ID):

| Sectie | Wat erin staat |
|---|---|
| **Hier te repareren** | Wat PlayGuard herstelt |
| **Te controleren** | Data die de console verdenkt: de controle zegt of je de game opnieuw moet installeren |
| **Opnieuw installeren** | Bestanden weg of onbruikbaar, of data beschadigd gevonden: verwijderen en opnieuw installeren |
| **Firmware of SD-kaart** | De game is in orde, de console niet |

- **Data controleren:** wat de datacontrole vond, blijft bij de game tot er een andere versie wordt geïnstalleerd: beschadigd, dan gaat hij naar *Opnieuw installeren* als *Beschadigd (gecontroleerd)*, met de datum en de code van de console; intact, dan verdwijnt hij uit de lijst. *Deze spellen controleren* controleert de hele sectie *Te controleren*, de ene game na de andere in één dialoogvenster (*spel 2 van 5*), en zegt dan hoeveel er beschadigd, intact of niet te controleren zijn, met het verwijderen van de beschadigde binnen handbereik.
- **Meerdere games tegelijk:** Ⓨ selecteert games (een vinkje ervoor), en *Meerdere spellen tegelijk*, boven de secties, selecteert een hele sectie; daarna één actie voor allemaal: hun data controleren, of ze verwijderen (eerst wordt hun lijst opgeslagen, één vraag, één pincode).
- **Onder *Opnieuw installeren*** kun je hun lijst opslaan (JSON en tekst, in de exportmap: naam, titel-ID, versie, probleem), en ze allemaal tegelijk verwijderen na één vraag en één pincode: eerst wordt de lijst opgeslagen, daarna worden de games verwijderd zodat een installatieprogramma ze niet meer als geïnstalleerd telt.
- **De acties:** als één actie de oplossing is, is die de eerste knop van het dialoogvenster; de andere volgen: de geïnstalleerde versie weer accepteren (als de console om een verwijderde update vraagt), alleen de update of extra content verwijderen, de data op beschadiging controleren (de eigen controle van de console, met de voortgang en Annuleren), of de game verwijderen.
- Versies worden getoond zoals de game ze toont (*versie 1.4.0*, *update nr. 4*); na een reparatie zegt een korte melding of de game weer werkt. Opgeslagen spelgegevens blijven altijd staan; elke wijziging staat in de wijzigingsgeschiedenis.
- **Wanneer het controleert:** de laatste controle verschijnt meteen; daarna worden alleen de games waarvan de vermelding is veranderd opnieuw gecontroleerd (alle games één keer per dag, na een firmware-update, of met Ⓧ *Opnieuw controleren*), elk getoond zodra hij is gecontroleerd.
- Meerdere kapotte games tegelijk wijzen op de SD-kaart of op een game die vanaf de andere opslag (sysMMC / emuMMC) is geïnstalleerd.
- *Weergave*, boven de secties, toont de games als **lijst** of als **covers** (icoon, naam en probleem; de keuze wordt onthouden), zoals *Alle games* dat doet.

### Forwarders

*Forwarders ›*, een eigen scherm onder *Homebrew* — een homebrew in het HOME-menu, met zijn icoon en zijn naam, van daaruit gestart als een game (een NRO-forwarder, zoals sphaira ze maakt).

- **PlayGuard** met één druk: zijn eigen icoon en naam, waarbij de forwarder de `.nro` start waarvandaan PlayGuard is geopend.
- **Andere homebrew**: elke `.nro` in de map `/switch` van de SD-kaart (en één map dieper), waarbij eerst om de naam wordt gevraagd (die van de `.nro` zelf, om te houden of te wijzigen). Het icoon is dat van de `.nro`, omgezet naar de JPEG van 256 × 256 die het HOME-menu toont als het een andere grootte heeft of een PNG is (een controller in de plaats ervan als de `.nro` er geen heeft), zoals sphaira dat doet. Een pad met een spatie wordt tussen aanhalingstekens aan de homebrew doorgegeven.
- **De forwarders in het HOME-menu**, elk met zijn icoon: die PlayGuard heeft gemaakt (Ⓐ: details, naam wijzigen, uitgever wijzigen, opnieuw installeren, verwijderen) en die een andere app heeft gemaakt (details, verwijderen). Een forwarder waarvan de `.nro` weg is, meldt dat; een die in de systeeminstellingen is verwijderd, blijft in de lijst staan tot hij opnieuw wordt geïnstalleerd of vergeten. Opnieuw installeren houdt zijn programma-ID, dus het item in het HOME-menu wordt vervangen, niet verdubbeld; een forwarder die een ander hulpmiddel voor dezelfde `.nro` heeft gemaakt, wordt ook vervangen.
- **Signaturepatches:** de console start een forwarder alleen met signaturepatches voor fs en loader (sys-patch of sigpatch-bestanden): zonder die patches meldt het scherm dat boven de lijst (*Console › Gamepatches* geeft de details). De `.nro` blijft waar hij is; een forwarder verwijderen raakt hem nooit aan.
- **Speeltijd:** gestart vanaf zijn icoon draait een homebrew als een game: de speeltijdtimer telt hem mee en schort hem op zoals elke game zodra de tijd om is, en *Activiteit* toont hem. PlayGuard blijft bereikbaar via het homebrew-menu.
- Elke wijziging vraagt om de pincode als *PlayGuard beveiligen met de pincode* dat doet, wordt in de alleen-lezenmodus geweigerd en staat in de wijzigingsgeschiedenis. Wat PlayGuard heeft gemaakt, wordt per systeem (sysMMC / emuMMC) bewaard, in `forwarders.json`.

### De regels van de console

- **Firmware, Atmosphère en hekate** (als zijn `bootloader/update.bin` op de SD-kaart staat), elk met de nieuwere versie die de laatste updatecontrole vond, in oranje (*22.0.0 · 23.0.1 beschikbaar*, en hoeveel versies achter als het er meer dan één zijn), en een waarschuwingsicoon op de regel van de firmware als de nieuwste firmware eerst een nieuwere Atmosphère of hekate nodig heeft, of door die nog niet wordt ondersteund (de release-opmerkingen van de firmware zeggen waarom: hun release-opmerkingen zeggen welke firmware ze ondersteunen).
- **Release-opmerkingen:** Ⓐ op elk ervan opent ze: een tabblad voor de geïnstalleerde versie, één voor elke nieuwere (hoogstens vijf) en oudere in de overige tabbladen (zes in totaal; Ⓛ / Ⓡ wisselen ertussen), geopend op de volgende versie, met labels (geïnstalleerd of nieuwer, de nieuwste firmware die wordt ondersteund, beveiligingsfixes, ouderlijk toezicht gewijzigd), de opmerkingen zoals gepubliceerd (in het Engels), *Online bekijken* (een QR-code) en wanneer ze zijn opgehaald. Die van Atmosphère en hekate komen uit hun GitHub-releases, die van de firmware van switchbrew; ze worden op de SD-kaart bewaard om offline te lezen en één keer per dag opnieuw opgehaald (Ⓧ nu).
- **Compatibiliteit**, dan **opslag** (emuMMC of sysMMC, met een info-icoon als het andere systeem een eigen ouderlijk toezicht heeft: Ⓐ legt het uit).
- Of Atmosphère **het serienummer afschermt** (deels verborgen tot Ⓐ; op emuMMC, als het niet is afgeschermd, een waarschuwingsicoon op de regel erboven, dat Ⓐ uitlegt).
- **Gamepatches** (sys-patch of sigpatch-bestanden, met sys-patch als aanbeveling als er alleen bestanden worden gebruikt: een info-icoon, of een waarschuwingsicoon als er iets moet worden nagekeken, dat Ⓐ uitlegt).

## Geschiedenis — wijzigingen en back-ups

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=history)*

### Wijzigingsgeschiedenis

- De nieuwste 500 wijzigingen die PlayGuard heeft gedaan (limieten, beperkingsniveau, pincode, ontgrendelingen, ontkoppelen, de klok, terugzettingen, het aftellen pauzeren of hervatten …) en wat het buiten zichzelf zag veranderen, per dag gegroepeerd (*Vandaag*, *Gisteren*, daarna de dag en de datum), met het tijdstip en de bron; een limiet die voor één dag is gewijzigd, noemt die dag.
- Een wijziging die de console **weigerde**, staat er ook, in oranje, met de foutmelding van de console en wat er werd geprobeerd (nooit terugzetten: er is niets veranderd).
- *Tonen* houdt één soort over (speeltijd, beperkingen, beveiliging en app, gezien buiten PlayGuard, geweigerd door de console); *Getoonde exporteren* schrijft dat naar `exports/` als CSV, JSON, XLSX of PDF.
- Ⓐ op een wijziging toont haar en, voor een waarde, **zet de vorige terug** — met hetzelfde ontgrendelen en dezelfde pincode als elke wijziging, en met de melding als de waarde sindsdien is veranderd.
- Elke wijziging bewaart de PlayGuard-versie die haar deed; een wijziging die door een nieuwere versie van PlayGuard is vastgelegd, verschijnt als *Andere wijziging*.

### Back-ups

- **Back-up maken / terugzetten van de instellingen** op de SD-kaart: beperkingsniveau, aangepaste instellingen, VR-modus, classificatie-instantie, daglimieten, het alarm "Tijd is om" (met de geavanceerde acties aan), en ter registratie het ruwe blok van de speeltijdtimer — nooit de pincode.
- Een terugzetting toont alleen wat er zou veranderen (niets te veranderen: dan zegt het dat gewoon, zonder ontgrendelen en zonder iets te schrijven), en meldt wanneer de back-up een bedtijdalarm aan had (dat wordt niet teruggeschreven).
- Kies hoeveel back-ups er bewaard blijven.

## Voorkeuren

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=preferences)*

- Wat PlayGuard zelf instelt:
  - Taal (elke taal van de console, of die van de console zelf), thema (standaard donker, licht of dat van de console), tekstgrootte (automatisch / normaal / groter, meteen); de andere met het aanbod om opnieuw te starten.
  - Het tabblad om mee te starten.
  - **De intro** bij het opstarten: het laadscherm gedurende 2 seconden (standaard aan; Ⓐ of Ⓑ slaat het over; uit, dan opent PlayGuard zodra het klaar is).
- De berichten die het toont:
  - De soorten meldingen (zie [hierboven](#hoe-meldingen-zich-gedragen)).
  - Een **maandelijkse herinnering om PlayGuard te steunen** (standaard aan, nooit in de eerste maand en niet meteen na een update; *Niet meer tonen* op de herinnering of deze schakelaar zet haar voorgoed uit, ook na updates).
- Een instelling die bij één functie hoort, staat bij die functie: extra tijd in *Speeltijdtimer*, ouderlijk toezicht weer inschakelen in *Pincode*, de klokcontrole in *Console › Netwerkklok*. De updates en *Hulp en diagnose* staan in *Info*.

## Info

*[Probeer het in de simulator ›](https://playguard.link/simulator/?tab=about)*

### Versie en updates

- De geïnstalleerde PlayGuard: versie (in oranje als er een nieuwere is; Ⓐ opent de **release-opmerkingen**), startmodus en gegevensmap.
- Nu **op updates controleren**; *Elke dag op updates controleren* bij het opstarten.
- *Bijwerken met*: in PlayGuard, dat de release downloadt, controleert, zichzelf vervangt en herstart, of sphaira, Homebrew App Store of met de hand (gevraagd bij de eerste update als er nog niets is gekozen).
- Zolang er een nieuwere release bekend is, opent PlayGuard op **PlayGuard X.Y.Z is beschikbaar** (de datum en de opmerkingen): *Bijwerken*, of *Later* voor 1 dag, 1 week of hoogstens 30 dagen.
- De release-opmerkingen zijn die van PlayGuard zelf, op hetzelfde scherm als die van Atmosphère en hekate in *Console*: uit de GitHub-releases, een tabblad per versie (de geïnstalleerde, de nieuwere, de oudere), elk met de hoogtepunten als kaarten en alle wijzigingen, bewaard op de SD-kaart om offline te lezen (die van de geïnstalleerde versie zitten ook in de app, voor als er nog niets is opgehaald).
- Na een update opent PlayGuard één keer op **Nieuw in X.Y.Z** (dezelfde hoogtepunten, daarna de QR-codes).

### Hulp en diagnose

*Hulp en diagnose ›*, een eigen scherm, onder *Bijwerken met*:

- **Eerste stappen** opent de gids opnieuw (met een stap *De smartphone-app ontkoppelen* zolang die gekoppeld is, een stap *Alarm "Tijd is om" weer inschakelen* zolang het uit staat, een stap *PlayGuard beveiligen* zodra er een pincode is — de keuze *PlayGuard beveiligen met de pincode* —, en een schakelaar zodat de gids niet meer bij het opstarten verschijnt). Onder *Sluiten* toont *PlayGuard steunen* de QR-codes voor een bijdrage.
- **Een diagnoserapport exporteren**, of **online versturen** (zie [Een bug melden](troubleshooting.md#een-bug-melden)); hoeveel opgeslagen rapporten, crashbestanden, sessielogs en links van online verstuurde rapporten er bewaard blijven, op aantal, leeftijd of grootte.

### Met dank aan

De dankbetuigingen, hoe je **PlayGuard steunt** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), getoond als QR-codes om met een telefoon te scannen), en een klein *Made in France* 🇫🇷.

## Veilig ontworpen

**De speeltijdlimiet veilig schrijven.** De configuratie van de timer overschrijven terwijl die aftelt, maakt Atmosphère instabiel. Daarom controleert PlayGuard eerst de toestand; als de timer actief is, zegt de bevestiging dat ouderlijk toezicht tijdelijk wordt ontgrendeld — met de opgeslagen pincode, **je hoeft hem niet te onthouden**. Eén druk ontgrendelt dan, controleert of het systeem de ontgrendeling echt meldt, schrijft, en **vergrendelt meteen weer** (of stelt dat voor, als *Ouderlijk toezicht na een wijziging weer inschakelen* uit staat). De servicelaag controleert de toestand opnieuw vlak voor het schrijven, dus geen enkel scherm kan de beveiliging overslaan.

> [!CAUTION]
> Een limiet onder de tijd die vandaag al is gespeeld, onderbreekt de game zodra ouderlijk toezicht weer is vergrendeld. De bevestiging zegt het als dat zou gebeuren.

**Geen crashes meer op 22.5.** `pctl:a`, de bevoorrechte dienst voor ouderlijk toezicht, accepteert **één enkele sessie**. Oudere builds van de oorspronkelijke app hielden die open, waardoor de pincodevraag van het HOME-menu (of de pincode-applet) haar niet kon krijgen en Atmosphère kon crashen. PlayGuard opent de sessie voor elke actie en geeft haar meteen vrij; periodieke vernieuwingen pauzeren zolang de app op de achtergrond staat. *(Diagnose door [de fork van anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Niets draait op de achtergrond.** De limiet, de pincode, de waarschuwingen en de onderbreking zijn die van de console zelf; PlayGuard wijzigt alleen hun instellingen.

## Wat telt als speeltijd

De speeltijdtimer is die van de console: PlayGuard leest hem, het telt zelf niets. Wat bekend is, en hoe zeker ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), in het Engels, bevat de metingen):

| Situatie | Telt het? | Hoe zeker |
|---|---|---|
| Een game die draait | Ja | Gemeten (22.0.0) |
| **Een game die open blijft in het HOME-menu** (onderbroken op de achtergrond) | **Ja**: sluit hem, of zet de console in de slaapstand | Gemeld door ouders ([Arqade](https://gaming.stackexchange.com/questions/398954/)); past bij de metingen (de console telt zolang er een applicatie open is) |
| Slaapstand | Nee | Gemeld ([Arqade](https://gaming.stackexchange.com/questions/398954/)); niet gemeten door PlayGuard |
| **PlayGuard geopend boven een game** (als applicatie) | **Ja**, als die game | Gemeten (22.0.0): open het met de gebruiker van een ouder, of vanuit het album om snel te kijken |
| PlayGuard geopend vanuit het album (appletmodus) | Nee | Afgeleid: het is geen applicatie, en tijd buiten applicaties leek niet te tellen |
| Zolang ouderlijk toezicht tijdelijk ontgrendeld is | Misschien niet | Niet gemeten: nog een open vraag |
| Nadat de klok is gewijzigd | De dag begint opnieuw; na een sprong terug stopt de telling tot de klok het oude tijdstip voorbij is | Waargenomen (22.0.0); *Vandaag* meldt wanneer de console niet telt |

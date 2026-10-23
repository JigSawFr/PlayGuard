<!-- translated from docs/features.md, sha256 584095baaee8 -->
# Funzioni

Cosa fa ogni scheda, come PlayGuard scrive senza rischi, e cosa la console conta come tempo di gioco. Nuovo su PlayGuard? Comincia da [Per iniziare](getting-started.md). Torna al [README](../../README.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

Le schermate mostrano il tema scuro, quello predefinito di PlayGuard (in inglese).

| Oggi | Console |
|---|---|
| ![Oggi](../../images/screenshots/dashboard.png) | ![Console](../../images/screenshots/console.png) |
| **Tempo di gioco** | **Tempo di gioco › modifica di un giorno** |
| ![Tempo di gioco](../../images/screenshots/play_timer.png) | ![Modifica di un giorno](../../images/screenshots/per_day.png) |
| **Attività** | **Un gioco** |
| ![Attività](../../images/screenshots/activity.png) | ![Un gioco](../../images/screenshots/activity_game.png) |
| **Limitazioni** | **Console › Orologio di rete** |
| ![Limitazioni](../../images/screenshots/restrictions.png) | ![Orologio di rete](../../images/screenshots/clock.png) |
| **Sicurezza** | **Preferenze** |
| ![Sicurezza](../../images/screenshots/security.png) | ![Preferenze](../../images/screenshots/preferences.png) |

## Comandi

| Tasto | Cosa fa |
|---|---|
| ↑/↓ | Sposta |
| Ⓐ | Conferma |
| Ⓑ | Indietro o annulla (due volte sulla barra laterale per uscire) |
| Ⓧ | Aggiorna (sulle schede che leggono la console) |
| Ⓨ | Notifiche, da qualsiasi scheda |
| **−** | Lingua e tema, da qualsiasi scheda (nel piè di pagina, i primi a essere tolti quando i tasti propri di una schermata hanno bisogno di spazio) |
| **+** | Salva i limiti per giorno (e la settimana modificata in *Tempo di gioco*) |
| **ZR** | *Annulla*, per qualche secondo dopo che una modifica è stata salvata |

L'app è organizzata in schede, come le Impostazioni di sistema; ognuna ha la sua sezione qui sotto.

- **Ciò che non è riuscito viene detto in una finestra di dialogo, ciò che è riuscito in un avviso a comparsa.** Una finestra di dialogo dice solo ciò che vale per questa console adesso: che il gioco in corso verrebbe sospeso, solo mentre un gioco è aperto; che l'app Filtro famiglia potrebbe sovrascrivere una modifica, solo mentre è collegata e può sincronizzarsi.
- **L'intestazione** indica, su ogni scheda, quando il filtro famiglia è sbloccato temporaneamente (la sua pillola), e quando l'app è in sola lettura (la pillola dice allora *Sola lettura*, e le azioni che scriverebbero sono in grigio e rifiutate indicando dove disattivarla); toccarla porta dove si annulla (*Oggi › Riattiva*, la schermata del firmware o *Aiuto e diagnostica*). Il piè di pagina indica i tasti che li aprono da qualsiasi scheda: ⊖ per la lingua e il tema, Ⓨ per le notifiche (con il loro numero).
- **In sola lettura**, le azioni restano al loro posto, in grigio, e dicono perché quando le premi.

## L'aspetto: Cadran

L'aspetto di PlayGuard si chiama Cadran: una barra di icone per le schede, una tavolozza blu notte o bianco freddo (a seconda del tema), riquadri al posto degli elenchi. Ogni scheda si apre su ciò che conta di più (un anello, tessere, riquadri, chip), con le altre impostazioni sotto.

- **La pillola dell'intestazione** dice di cosa tratta la scheda (*Filtro famiglia attivo*, *Timer attivo*…); mentre il filtro famiglia è sbloccato lo dice su ogni scheda, e toccarla apre *Oggi › Riattiva*; mentre è in pausa per te (vedi [sotto](#il-pin-mette-in-pausa-il-filtro-famiglia-mentre-sei-in-playguard)) dice *Filtro famiglia in pausa*, senza allarmi, e toccarla spiega cos'è. Il testo secondario non scende mai sotto i 15 px, per poterlo leggere dal divano su una TV. L'orologio a piè di pagina mostra ore e minuti (*16:00*).
- **Le finestre di dialogo** sono riquadri con i pulsanti a pillola, la conferma in verde acqua (in corallo quando rimuove o ferma qualcosa).
  - Le scelte si aprono in un riquadro sopra i suggerimenti dei tasti, quella attuale su un verde acqua tenue con la sua spunta. Il contorno verde acqua è solo del focus, ovunque: un livello, un utente, una scelta in vigore mantiene lo sfondo tenue.
  - Una finestra di dialogo lunga sfuma in fondo finché c'è altro sotto, e il suo avviso (un gioco sul punto di essere sospeso) viene per primo, in un riquadro ambra.
  - Un errore ha come titolo ciò che non è riuscito, il codice di errore in piccolo sotto il motivo, e propone *Invia un rapporto* quando serve.
  - I messaggi compaiono come riquadri sotto l'intestazione, con una spunta verde acqua per ciò che è stato fatto e un segno ambra per ciò che è stato rifiutato.
- **Le altre schermate** prendono lo stesso aspetto: l'editor per giorno di un profilo mostra la settimana come in *Tempo di gioco* (modificata allo stesso modo, i riempimenti rapidi come tessere); *Giochi installati* e *Tutti i giochi* mettono le loro righe in riquadri; *Profili* e *Aiuto e diagnostica* danno a ogni riga la sua icona.
- **Dimensione del testo** (*Preferenze*, o **AA** nell'intestazione):
  - *Automatica*, l'impostazione predefinita, ingrandisce tutti i testi del 25 % quando la console è nella base, sulla TV, e li riporta com'erano in modalità portatile; *Più grande* li tiene ingranditi, *Normale* mai.
  - Anche i nomi delle schede della barra laterale crescono, sempre del 25 %; il titolo, l'etichetta e la data dell'intestazione e i suggerimenti dei tasti e l'orologio del piè di pagina crescono un po' meno (dal 15 al 20 %), perché le loro righe sono già piene. Un nome troppo lungo per il suo spazio in una lingua viene disegnato un po' più piccolo, mai meno che nella dimensione normale, invece di essere tagliato. I nomi dei livelli di Limitazioni mantengono la loro dimensione, già grande.
  - Il cambio è immediato, su ogni schermata, senza riavviare.
  - Il 25 % viene da una TV da 55" in 4K: il testo più piccolo (15 px) è allora alto 14,6 mm in maiuscolo, 20′ d'arco da 2,5 m, dove la lettura è comoda (16′ in dimensione normale, il minimo indispensabile).
- **Effetti visivi** (*Preferenze*): *Sempre completi*, l'impostazione predefinita, mantiene l'alone di sfondo e ogni movimento dell'interfaccia a batteria come nella base: le schermate e le loro righe che salgono al loro posto, l'anello che si disegna e scorre verso un nuovo valore, il battito dell'anello della schermata del bambino nei suoi ultimi minuti, le piccole risposte a una pressione. *Più leggeri a batteria* li toglie tutti finché non è collegato un caricatore (in modalità portatile, senza alimentazione; riletto ogni 5 secondi, quindi collegare la console all'alimentazione li fa tornare). *Meno movimento* toglie ogni movimento decorativo, anche nella base, e mantiene l'alone: per chi si stanca con il movimento.
- **Annulla:** dopo che una modifica è stata salvata, **ZR** *Annulla* resta nel piè di pagina per qualche secondo: apre quella modifica nella cronologia delle modifiche, per rimettere il valore precedente. Una cronologia delle modifiche vuota lo dice in un riquadro.

## Notifiche — ciò che richiede la tua attenzione, da qualsiasi scheda

Una campanella nell'intestazione, a sinistra della versione, raccoglie ciò che richiede l'intervento del genitore; Ⓨ apre l'elenco da qualsiasi scheda (il contatore della campanella indica *2* quando ci sono due cose da fare).

A sinistra della campanella, la lingua (il suo codice, *FR*, *EN*…) e il tema (un sole per il chiaro, una luna per lo scuro, un cerchio pieno a metà quando segue la console), poi la dimensione del testo (**AA**, con la A grande in verde acqua quando il testo è ingrandito): un tocco apre lo stesso elenco di *Preferenze*, e **−** li offre da qualsiasi scheda. Un nuovo tema o una nuova lingua ha effetto subito, con ogni schermata rifatta sulla stessa scheda e il focus sulla stessa riga, cinese e il suo carattere compresi.

### Da fare

Ognuno con un segnale di avviso ambra, contato in ambra sulla campanella; resta finché non è risolto:

- ancora nessun PIN, o un PIN che chiunque proverebbe per primo (0000, 1234, 1212, 2580…: valutato sulla console, mai mostrato);
- un PIN sulla console mentre PlayGuard stesso si apre senza (*Proteggi PlayGuard con il PIN* su *Mai*);
- il filtro famiglia sbloccato temporaneamente (*Riattiva*);
- del tempo extra di un giorno precedente ancora da togliere;
- l'avviso "Tempo scaduto" disattivato;
- modifiche fatte fuori da PlayGuard;
- la console che non conta il tempo di gioco;
- un orologio di rete non preciso.

### Per informazione

Ognuno con una "i", un punto sulla campanella; si può segnare come letta:

- un limite ancora abbassato da *Basta giocare per oggi* in un giorno precedente (*Ripristina*);
- una nuova versione installata (*Novità*; dopo un aggiornamento sul posto, la build precedente conservata), o una versione più recente di PlayGuard;
- un rapporto inviato online (*Mostra il link*), o PlayGuard chiuso in modo imprevisto l'ultima volta (*Invia un rapporto*);
- il tempo di gioco della settimana (ogni 7 giorni finché non viene letto; con il confronto, al giorno, con la settimana precedente), e un limite superato in due o più degli ultimi 7 giorni mentre la console si limita ad avvisare;
- un backup delle impostazioni da fare (nessuno, o nessuno da 30 giorni);
- tutti i giochi bloccati da una settimana;
- un nuovo utente;
- l'app Filtro famiglia collegata;
- l'orologio di rete trovato sfasato all'avvio.

### Console

Le voci che riguardano la console più che il filtro famiglia vengono per ultime, su una riga che Ⓐ apre (aperta subito quando non c'è nient'altro). Quelle da fare sono contate sul badge della scheda Console, non sulla campanella:

- giochi installati che non si avviano (*Controlla i giochi installati*), patch dei giochi da controllare;
- la scheda SD quasi piena (meno di 500 MB);
- un file di PlayGuard che non è stato possibile mettere da parte o scrivere sulla scheda SD;
- il sysmodule di ripristino non aggiornato (non quello fornito con questa versione, o installato prima che indicasse la sua versione; *Aggiorna* installa quello che contiene questa versione), poi la console da riavviare perché entri in funzione, finché non è stata riavviata;
- una build che non ha finito di avviarsi e da cui si è tornati indietro (*Invia un rapporto*).

Quelle per informazione: un ritorno alla build precedente, un firmware non testato con questa versione, una versione più recente di Atmosphère, hekate o del firmware (verificata con *Cerca aggiornamenti ogni giorno*; la voce del firmware dice di aspettare finché Atmosphère non lo supporta), il numero di serie non oscurato su emuMMC, il filtro famiglia proprio dell'altro sistema, un file di PlayGuard trovato danneggiato e messo da parte (un avviso a comparsa all'avvio lo indica).

### Come si comportano le voci

- Ogni sezione elenca le sue voci in un ordine fisso, qualunque sia il controllo finito per primo.
- **Nella barra laterale**, un badge ambra accanto al nome di una scheda conta le sue voci da fare, là dove si risolvono: la scheda Sicurezza (nessun PIN, un PIN debole, PlayGuard senza il suo PIN, sbloccato temporaneamente), Tempo di gioco (l'avviso disattivato, la console che non conta), Oggi (modifiche fatte fuori da PlayGuard, tempo extra da togliere), la Console (le sue voci qui sopra, e l'orologio di rete). Le voci ignorate e quelle per informazione non vengono contate.
- Nella scheda, lo stesso badge è sulla riga dove ognuna si risolve (*Orologio di rete*, *Giochi installati*, *Memoria*, *Patch dei giochi*; *Imposta o cambia il PIN*, *Proteggi PlayGuard con il PIN*; *Avanzate* di Tempo di gioco per l'avviso), e la scheda si apre sulla prima di esse. Una riga senza un valore proprio dice prima, in ambra, a cosa serve il badge (*Facile da indovinare*, *Non preciso*).
- **Ⓐ** su una voce applica la soluzione (*Attivalo*, *Regolalo*, *Aggiorna*…) o apre la scheda dove si applica. Una voce sparisce da sola una volta risolto ciò che segnala.
- **Ⓧ** segna come letta una voce per informazione; **Ⓨ** le segna tutte. Una voce segnata come letta resta letta all'avvio successivo, finché il suo testo non cambia.
- **Ignorare:** alcune voci da fare si possono ignorare con Ⓧ, dopo una conferma e il PIN: un PIN che chiunque proverebbe per primo, PlayGuard che si apre senza il PIN, l'avviso "Tempo scaduto" disattivato, la scheda SD quasi piena (mai l'assenza di PIN, il filtro famiglia sbloccato, l'orologio, il tempo di gioco non contato o i file di PlayGuard). Una voce ignorata non viene più contata, e compare per ultima sotto *Ignorati*, dove Ⓐ la fa contare di nuovo. Torna da sola 30 giorni dopo (a meno che *Conta di nuovo gli avvisi ignorati dopo 30 giorni* non sia disattivato), una volta risolto ciò che segnala e poi ripresentatosi, oppure, per il PIN, con un nuovo PIN di un'altra lunghezza.
- *Preferenze › Notifiche* disattiva tre tipi: le versioni di Atmosphère, hekate e firmware; il tempo di gioco della settimana; i promemoria (backup, il secondo sistema della console, i giochi bloccati).
- Lo stato che c'è dietro viene letto all'avvio di PlayGuard, ogni minuto sulla schermata principale, e ogni 5 s mentre è mostrata la scheda *Oggi*.

## Oggi — la giornata a colpo d'occhio

*[Provalo nel simulatore ›](https://playguard.link/simulator/)*

### L'anello e la linea

**Il tempo rimanente** come anello: un giorno senza gioco indica *Niente gioco*, non *0 min*; il tempo extra dato oggi appare come un segmento più chiaro alla sua estremità, e l'anello scorre verso ogni nuovo valore.

Sotto, ciò che fermerà il gioco per primo:

- prima dello scadere del tempo o di Sogni d'oro, il resto della giornata come una linea, con il momento attuale alla sua sinistra, e ognuno dei due come un segno con la sua ora (*16:40 tempo scaduto*, *21:00 ora di dormire*), in verde acqua fino al primo, in ambra nei suoi ultimi 15 minuti;
- altrimenti a parole: Sogni d'oro una volta esaurito il tempo del giorno, quando si può giocare di nuovo; mentre il filtro famiglia è sbloccato, il limite di oggi in pausa e il perché.

### Le azioni

- **Tempo extra**, le sue quantità su una riga (+15 min, +30 min, +1 h), ognuna a portata di una sola pressione, poi **…** per qualsiasi altra (una durata, un altro giorno): una quantità selezionata sposta in avanti sulla linea lo scadere del tempo (*con 15 min in più*), o dice sul segno di Sogni d'oro che questo arriva comunque prima; la prima risalta in verde acqua quando restano 15 min o meno, o niente.
- **Basta per oggi**, **Mostra al bambino**.
- **Limite di oggi…**: il limite di oggi scelto sul momento, scritto dopo una sola conferma.
- **Il profilo in vigore**, con il suo nome, una volta salvato un profilo (*Profili…* quando nessuno corrisponde): Ⓐ li elenca, quello in vigore spuntato, per metterne in vigore un altro dopo una sola conferma (i giorni che cambiano), o *Gestisci i profili…*.
- Mentre il filtro famiglia è sbloccato, **Riattiva** viene per primo, su tutta la larghezza della riga, e *Tempo extra* si fa da parte.
- Senza limiti in nessun giorno, **Impostare i limiti della settimana** apre la settimana in *Tempo di gioco*.

### I riquadri e le pillole

- **Sogni d'oro** (quando si può giocare di nuovo, o tra quanto suona quando la console non lo dice; se è disattivato, *Disattivata* in grande e Ⓐ *Impostarla*), **il filtro famiglia** e **l'orologio di rete**, come riquadri.
- Lo stato della console come **pillole**:
  - una dice se il reindirizzamento DNS di Atmosphère blocca i server di Nintendo, e Ⓐ su di essa quali e cosa cambia: nessuna sincronizzazione per l'app Filtro famiglia, nessun orologio di rete che si regola da solo;
  - un'altra, quando PlayGuard è aperto come applicazione, che il tempo passato in PlayGuard viene contato: come quel gioco quando è aperto sopra un gioco (*Aperto da un gioco*), con il nome della sua icona quando è avviato da un forwarder del menu HOME (*Aperto dalla sua icona HOME*), con Ⓐ che spiega come;
  - un gioco lasciato aperto nel menu HOME compare come pillola ambra (*continua a contare*, con Ⓐ che spiega perché), quando la pillola sul tempo di PlayGuard stesso non lo dice già;
  - nessuna delle due mentre il filtro famiglia è in pausa per te: il tempo passato qui allora non è quello del bambino.
- Le pillole mettono prima ciò che richiede il genitore (ambra, con un triangolo), poi ciò che è attivo (verde acqua, con una spunta), poi il resto (grigio, con un punto): la forma lo dice quanto il colore.
- Ciò che va bene e ciò che è solo detto si raccolgono in una sola pillola dopo quelle che richiedono il genitore (*✓ 3 controlli a posto*); Ⓐ su di essa li mostra, finché la scheda resta aperta.
- Sotto le pillole, il tempo extra ancora da togliere (Ⓐ lo toglie) e l'avviso "Tempo scaduto" finché è disattivato (Ⓐ lo riattiva).

### Primi passi

Finché il PIN e i limiti della settimana non sono impostati, un riquadro sopra gli altri riquadri elenca i tre passaggi (il PIN, un limite giornaliero, l'orologio di rete), ognuno spuntato in verde acqua una volta fatto o cerchiato in ambra, con quanti sono fatti (*1 di 3*); Ⓐ su uno lo fa lì.

### La console sta contando?

Il tempo di PlayGuard stesso viene contato mentre gira come applicazione, quindi con un limite oggi il tempo rimanente deve scendere mentre è aperto. Quando non si è mosso per circa 90 s sullo schermo (timer attivo, un limite oggi, né sbloccato né in pausa per te, non aperto dall'album), una riga ambra sotto il tempo giocato indica *La console non sta conteggiando il tempo di gioco in questo momento*, con le cause probabili:

- l'orologio è stato portato indietro (riprende a contare più tardi);
- l'orologio di rete non è mai stato impostato (*Console › Orologio di rete*);
- un limite è appena stato scritto (avvia un gioco per un minuto, poi controlla).

Sparisce appena il tempo rimanente si muove di nuovo. Lo dice anche Tempo di gioco.

### Modificato fuori da PlayGuard

Il tempo di gioco di oggi ripartito da zero nello stesso giorno (lo fa un cambio d'orologio), l'orologio della console spostato di più di 5 minuti, limiti diversi da quelli che PlayGuard ha visto per ultimi (l'app Filtro famiglia, un altro strumento): un riquadro ambra sopra l'anello finché non viene chiuso (Ⓐ) o fino al giorno dopo, e una voce nella cronologia delle modifiche. PlayGuard non gira in background: vede solo ciò che è cambiato mentre era aperto, o dalla sua ultima apertura, quando rilegge la console.

### Al limite

La console mostra "Tempo scaduto", poi sospende il gioco (senza "continua") o si limita ad avvisare: *Quando il tempo è scaduto* indica quale e permette di cambiarlo, una volta impostato un limite o Sogni d'oro ([dettagli, in inglese](../parental-controls.md#when-the-time-runs-out)).

### Tempo extra e basta giocare per oggi

- **Tempo extra oggi**: +15 min, +30 min, +1 h per impostazione predefinita.
- **Basta giocare per oggi**: limite 0 solo per oggi, *Adesso* o *Tra 5 minuti* quando il tempo giocato è noto (il limite diventa allora il tempo giocato + 5 min, il tempo di salvare). Al blocco compare il "Tempo scaduto" della console, e i progressi non salvati possono andare persi — la conferma lo dice.
- **Il giorno dopo** — all'avvio, o a mezzanotte se l'app è aperta — PlayGuard rimette il limite abituale:
  - dopo il tempo extra, da solo, senza domande né PIN (abbassa solo il limite, come fa la riattivazione del filtro famiglia), sbloccando per un attimo se il timer sta contando e bloccando di nuovo subito. Aspetta finché non può scrivere (modalità sola lettura, un firmware che non supporta, *Blocca tutti i giochi* attivo) e, una settimana dopo nello stesso giorno, finché il limite più basso fermerebbe il gioco in corso (allora il giorno dopo). Per tenere più tempo in un giorno, cambia il limite di quel giorno;
  - dopo *Basta giocare per oggi*, chiede se tenere 0 o rimettere il limite (con il PIN, come ogni modifica che alza un limite), oppure lo rimette come ogni modifica (*Tempo di gioco › Dopo "Basta giocare per oggi", ripristina il limite*); rimettere "nessun limite" viene sempre chiesto.
- **Tempo extra in un altro giorno** (*Tempo extra › Un altro giorno…*, alla fine del suo elenco): domani o uno dei cinque giorni successivi, ognuno elencato con il suo limite, poi la quantità. Il limite di quel giorno viene alzato subito, solo per quella data, e rimesso da solo (abbassa solo il limite) la prossima volta che PlayGuard viene aperto dopo la fine di quel giorno — non su una console su cui PlayGuard non viene aperto, dove lo mantiene anche lo stesso giorno della settimana successiva. Uno alla volta: l'elenco propone allora di toglierlo (il suo limite torna subito com'era). Una modifica del limite di quel giorno nel frattempo (la settimana, un profilo…) prevale: non viene rimesso nulla.

### La schermata del bambino

*Mostra al bambino* in *Oggi*, per consegnare la console:

- solo il tempo rimanente (mentre il filtro famiglia è sbloccato, il limite di oggi, in pausa), quando finisce il gioco (*Finisce verso le 16:40*; quando Sogni d'oro arriva prima, l'anello fa invece il conto alla rovescia fino ad esso, *45 min prima di dormire*, come nell'ultima ora di un giorno senza limite), l'ora di andare a letto e cosa succede quando il tempo è scaduto;
- pensata per essere letta dall'altra parte della stanza, sulla TV; il suo testo mantiene la sua dimensione qualunque sia la dimensione del testo (solo il titolo e il piè di pagina la seguono, come in ogni schermata);
- negli ultimi 5 minuti (di gioco o prima dell'ora di andare a letto), la fine diventa ambra e l'anello pulsa lentamente; l'ora di andare a letto diventa ambra negli ultimi 30;
- Ⓐ e Ⓨ, i tasti del genitore, sono indicati in piccolo in alto a destra, così il piè di pagina mostra solo quello del bambino;
- Ⓐ torna alle schermate del genitore con il PIN; Ⓨ dà tempo extra da lì, dopo il PIN (le quantità di *Tempo extra*, restando sulla schermata del bambino); Ⓑ esce da PlayGuard (chiedendo prima di riattivare il filtro famiglia, come quando si esce).

Con il testo più grande, Oggi sta ancora in una sola schermata: la riga sopra il titolo indica solo il giorno e il suo limite (*GIOVEDÌ · 2 h*), i pulsanti e i riquadri si stringono un po'. Si aggiorna ogni 5 s (Ⓧ aggiorna subito).

## Tempo di gioco — la settimana è l'editor

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=play_timer)*

### Modificare la settimana

Ⓐ su un giorno lo modifica:

| Tasto | Nella modifica |
|---|---|
| ↑/↓ | Lo cambia di 15 min, di un'ora se tenuti premuti |
| ←/→ | Passa al giorno successivo |
| Ⓛ | Lo copia da lunedì a venerdì |
| Ⓡ | Lo copia sul weekend |
| Ⓨ | Nessun limite |
| Ⓧ | Una durata esatta (*Inserisci una durata…*: ↑/↓ di 15 min, di un'ora se tenuti premuti, Ⓛ/Ⓡ di 5 min, con OK selezionato) |
| Ⓐ o Ⓑ | Termina la modifica (Ⓐ dice quale giorno: *Fatto per lunedì*; non è ancora salvato nulla) |

- Al tocco, un giorno toccato passa in modifica, poi un tocco sulla sua parte alta o bassa lo cambia di 15 min.
- Le modifiche si salvano insieme, con **+** o *Salva*, come nell'editor per giorno (la conferma elenca i giorni cambiati, prima → dopo); *Annulla* le rimette com'erano.
- Dentro la barra di ogni giorno, il tempo giocato nella sua ultima occorrenza (oggi compreso, tutti gli utenti). Oltre il limite, la barra è piena e la sua cima in corallo, per la stessa parte che il tempo oltre il limite occupa nel tempo giocato; una legenda sotto la settimana lo dice (*limite superato*), e anche il limite è in corallo. I giorni dopo oggi mostrano il gioco della settimana scorsa, più tenui (selezionato: *Venerdì precedente: 2 h 10 giocato su 2 h*).
- Un giorno cambiato e non ancora salvato ha il limite in ambra e la barra contornata d'ambra; sotto la settimana, i dati del giorno selezionato (*Giovedì: 1 h 20 giocato su 2 h*).
- Sopra la settimana, in ambra, un limite raggiunto e ciò che fa allora la console, o la console che non conta il tempo di gioco.
- `0` minuti significa *niente gioco quel giorno*; *Rimuovi il limite di tempo di gioco* disattiva il timer.

### Chip e impostazioni rapide

**Chip** sopra e sotto la settimana aprono subito le loro scelte: i profili (quanti ne sono salvati), lo stesso limite ogni giorno, Sogni d'oro e *Quando il tempo è scaduto*. *Rimuovi il limite* è l'ultima riga della scheda, a parte (la cronologia lo annulla).

- **Stesso limite ogni giorno:** un elenco rapido o una durata esatta (*Inserisci una durata…*: ↑/↓ di 15 min, di un'ora se tenuti premuti, Ⓛ/Ⓡ di 5 min).
- **Settimana e weekend ›:** l'editor, che chiede il limite da lunedì a venerdì e poi quello del weekend.
- **Tempo extra o basta giocare oggi ›** apre *Oggi*, dove si trovano entrambi. Una nota sotto dice cosa fa la console al limite. Mentre il filtro famiglia è sbloccato temporaneamente, il tempo di gioco *potrebbe* non essere contato: la questione è ancora aperta ([documentazione, in inglese](../parental-controls.md#still-open)).
- Ogni conferma elenca i giorni che cambiano, prima → dopo.

### Profili

- **Profili** salvati sulla scheda SD: applica (un profilo già in vigore lo dice e basta), modifica, rinomina o elimina un profilo; salva i limiti attuali o creane uno nuovo.
- Qualsiasi nome, in qualsiasi scrittura (*École*, *周末*, *Выходные* …) — un nome senza lettere latine riceve un nome di file tutto suo, e due nomi che porterebbero allo stesso file vengono individuati.
- **Profilo programmato** (*Profili › Profilo programmato*): un profilo per un periodo — vacanze, un viaggio — con il primo e l'ultimo giorno scelti tra i prossimi 60 giorni.
  - Quando il periodo inizia, PlayGuard chiede di metterlo in vigore (i giorni che cambia sono mostrati sotto la domanda); una volta finito, di rimettere i limiti che ha sostituito, o di tenerli.
  - Lo chiede una volta al giorno, sulla schermata principale, quando non c'è nient'altro davanti: PlayGuard agisce solo quando viene aperto, quindi una console su cui non viene aperto mantiene i limiti che ha. Un periodo trascorso senza che il profilo sia mai stato messo in vigore viene dimenticato.
  - Uno alla volta; la riga lo cambia, lo toglie o lo termina.

### Blocca tutti i giochi

*Blocca tutti i giochi*, sotto le impostazioni della settimana: un interruttore che porta a 0 il limite di ogni giorno, quindi serve un PIN per avviare un gioco — un blocco leggero senza classificazioni per età né limiti di comunicazione.

- Blocca l'avvio dei giochi, non il menu HOME, e richiede un PIN.
- I limiti precedenti tornano quando lo si disattiva.
- Mentre è attivo, *tempo extra* e *basta giocare per oggi* vengono rifiutati, e i limiti impostati in un altro modo (un profilo, un backup, la cronologia…) lo sostituiscono.

### Sogni d'oro

- L'ora dell'avviso (dalle 16:00 alle 23:45, o *Disattivato*) e l'ora in cui si può giocare di nuovo (dalle 05:00 alle 09:00), la stessa ogni giorno, impostate con la levetta: su e giù di un quarto d'ora, L e R di un'ora.
- Oppure, con Ⓨ *Per giorno* nella finestra dell'avviso, da lunedì a venerdì, il weekend o un solo giorno, mentre gli altri giorni mantengono la loro (ogni gruppo di giorni elencato con la sua ora). Quando i giorni differiscono, la scheda indica *Varia a seconda del giorno*, e la settimana in *Tempo di gioco* mostra l'avviso di ogni giorno sotto il suo nome.
- La sua posizione nelle impostazioni del timer, ricavata dalle impostazioni dell'app Filtro famiglia, è verificata su una console (22.0.0); PlayGuard la modifica comunque solo quando la console riporta ciò che PlayGuard vi legge, controlla la risposta della console dopo la modifica e rimette le impostazioni precedenti se è diversa.

### Impostazioni del tempo extra e Avanzate

- **Tempo extra:** le quantità proposte (+15/+30/+1 h, +10/+20/+30 min …) e il ripristino del limite abituale senza chiedere dopo *Basta giocare per oggi* (il tempo extra viene sempre tolto senza chiedere). L'uno e l'altro avvengono la prossima volta che PlayGuard viene aperto, dal giorno dopo in poi: una console su cui non viene aperto mantiene il limite di quel giorno.
- **Avanzate ›**, una schermata a parte: l'interruttore per le azioni di classe debug e, mentre è attivo, l'avviso "Tempo scaduto" attivato/disattivato e la pausa / ripresa del conto alla rovescia. Lo stesso interruttore permette a un backup o alla cronologia di rimettere l'avviso.

## Attività — chi ha giocato a cosa, e per quanto

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=activity)*

Tempo per gioco **oggi**, negli **ultimi 7 giorni** e **in totale**, dal registro attività della console stessa — per tutti gli utenti o per **un solo utente** (gli utenti come chip in alto, quando la console ne ha più di uno). PlayGuard si apre sull'utente mostrato per ultimo, o su quello scelto in *Account mostrato per primo* (tutti gli utenti, o uno solo; in fondo alla scheda, con *Tutti i giochi*).

### In alto

- Gli utenti come chip, il periodo e *Esporta*.
- Oggi, gli ultimi 7 giorni e da sempre come cifre. Sotto gli ultimi 7 giorni, il confronto con i 7 precedenti, al giorno: ▲ *+20 min/giorno vs settimana prima* in ambra, ▼ in meno in verde acqua, *come la settimana prima* entro 5 minuti.
- I quattro giochi più giocati come riquadri (Ⓐ ne apre uno); sugli ultimi 7 giorni, quello giocato più della settimana prima dice di quanto, a partire da mezz'ora (▲ *+2 h vs settimana prima*).
- Niente giocato nel periodo: un riquadro lo dice, con *Vedi da sempre* quando c'è qualcosa da vedere.

### Grafici e riepilogo

- **Un grafico degli ultimi sette giorni** (oggi a destra, una legenda sotto), con il limite di questa settimana per ogni giorno come linea e il tempo oltre il limite in ambra, segnato !; i totali di oggi, della settimana e di sempre.
- **Un riepilogo** sul periodo scelto (oggi, gli ultimi 7 giorni, gli **ultimi 30 giorni** o da sempre), per l'utente mostrato: media al giorno, il gioco più giocato, i giorni giocati e quello più intenso (ultimi 7 giorni), la sessione media (da sempre).
- **Orari di gioco, ultimi 7 giorni**: un quadratino per ogni ora di ogni giorno, lunedì per primo, tanto più intenso quanto più si è giocato (tutti i giochi, l'utente mostrato); poi le sessioni (giocare con pause inferiori a 5 minuti è una sola sessione, cambio di gioco compreso) e la loro durata media, la più lunga e il suo giorno, le due ore consecutive più intense, e il tempo giocato tra le 21:00 e le 06:00.

### Tutti i giochi e la pagina di un gioco

- **Tutti i giochi**, in una schermata a parte: ogni gioco giocato nel periodo, i più giocati per primi, come **griglia di copertine** o come **elenco** (Ⓨ cambia, e la scelta viene ricordata), con i dati del gioco selezionato sempre in alto: icona e nome, il tempo di gioco del periodo, il totale di sempre, gli avvii e l'ultima partita. Ⓧ sceglie il periodo (la scheda Attività segue).
- Le icone vengono lette intorno al gioco selezionato (non in modalità applet, per risparmiare memoria); un controller grigio le sostituisce finché non arrivano, e per un gioco che non ne ha.
- Una libreria grande mostra i primi 50 giochi, poi *Mostra tutti i giochi* (al massimo 100 giochi in modalità applet); l'esportazione li contiene sempre tutti. PlayGuard legge al massimo 256 giochi (prima quelli di questa settimana): oltre, la scheda Attività, Tutti i giochi e l'esportazione (non il CSV) dicono che ne mancano alcuni, e il totale di sempre li esclude.
- **Ⓐ su un gioco:** i suoi ultimi sette giorni a barre, gli avvii, la prima e l'ultima partita, il tempo per utente. I giochi eliminati conservano i dati di sempre, anche uno eliminato prima dell'installazione di PlayGuard purché il registro di gioco della console lo nomini ancora, e il loro nome una volta che PlayGuard l'ha visto (altrimenti *Gioco eliminato* e il suo ID titolo).

### Utenti

- Il tempo di un gioco che nessun utente detiene (alcuni giochi non ne chiedono mai uno, o l'utente è stato eliminato nel frattempo) compare come *Senza utente*, a partire da un minuto.
- Un utente che PlayGuard non aveva mai visto viene segnalato in ambra nella scheda per 30 giorni (*Nuovo utente dal …*: un utente nuovo è un modo noto per ricominciare da zero) e registrato nella cronologia delle modifiche; gli utenti presenti quando PlayGuard li elenca per la prima volta non vengono segnalati.

### Esportazione e condivisione

- **Esportazione sulla scheda SD** (*Esporta*) in CSV, JSON, XLSX (Excel) o PDF, una colonna per giorno, poi gli ultimi 7 giorni, gli ultimi 30 giorni e da sempre.
- Si apre subito sugli ultimi dati letti (conservati sulla scheda SD tra un avvio e l'altro), aggiornati in background quando hanno più di un minuto; Ⓧ li rilegge subito, con un indicatore di caricamento.
- I tempi sono approssimativi se l'orologio della console è stato cambiato.

## Limitazioni — livello, classificazione per età, comunicazione

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=restrictions)*

- **Il livello limitazioni:** Bambino piccolo, Bambino, Adolescente e Personalizzato affiancati come tessere, ognuno con ciò che limita (un lucchetto per ciò che è limitato, *Consentito* scritto per il resto, e i giochi installati che blocca; Ⓐ lo imposta; con *Personalizzato* in vigore, Ⓐ porta alle sue impostazioni qui sotto); *Nessuno* sotto di esse. Il livello in vigore dice *✓ Attuale*; il contorno verde acqua è solo dove si trova il focus. Ciò che un livello limita (limite di età, pubblicazione, comunicazione) viene ripetuto prima di impostarlo; *Nessuno* lo chiede in corallo, perché toglie ogni limitazione.
- **Le impostazioni del livello:** classificazione per età, pubblicazione sui social media, comunicazione con altri utenti. Con un livello predefinito sono le sue: cambiarne una rende il livello *Personalizzato*, dalle impostazioni del livello predefinito con quella modifica, dopo una domanda (con le impostazioni elencate).
- **In *Classificazione*,** il **sistema di classificazione** (PEGI, ESRB, USK, CERO …): l'elenco indica i paesi per cui classifica ognuno (PEGI: gran parte dell'Europa, ESRB: Stati Uniti, Canada e America Latina, USK: Germania, PEGI BBFC: Regno Unito…).
- **Giochi installati bloccati** (anche in *Classificazione*): quanti dei giochi installati il livello in vigore blocca in base alla loro classificazione per età (*3 su 15*), con Ⓐ che li elenca con la loro classificazione (*PEGI 16*); lo dice anche la tessera di ogni livello (*Giochi bloccati · 3*).
  - Le classificazioni sono quelle di ogni gioco (i suoi dati di controllo, letti dal controllo dei giochi installati all'avvio o da *Console › Giochi installati*), secondo il sistema usato dalla console.
  - Un gioco che quel sistema non classifica non viene contato, né le eccezioni impostate nell'app Filtro famiglia.
  - La schermata di un gioco in Attività mostra la sua classificazione, e *bloccato al livello attuale* quando lo è.
- **In *Altro*,** la modalità VR. I giochi autorizzati a comunicare sono nel rapporto di diagnostica, grezzi, finché il loro elenco non è compreso.

## Sicurezza — il PIN, lo sblocco, il menu homebrew, l'app Filtro famiglia

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=security)*

- **In alto**, quattro tessere: il PIN, le limitazioni, lo sblocco temporaneo e il menu homebrew, ognuna con una spunta verde acqua quando è come deve essere (un PIN impostato, limitazioni attive, bloccato) o un segnale di avviso ambra quando serve intervenire; il menu homebrew dice *Aperto* o *Bloccato* (una spunta una volta bloccato: aperto va bene senza forwarder).
- **Imposta o cambia il PIN** (schermata PIN del sistema).
- **Mostra il PIN**: dopo un avviso e il PIN stesso, chiesto ogni volta; registrato nella cronologia, poi viene proposto un nuovo PIN.
- **Sblocca temporaneamente**, e se PlayGuard lo riattiva da solo subito dopo una modifica che ha richiesto lo sblocco (attivo per impostazione predefinita).

### Proteggi PlayGuard con il PIN

Il PIN in PlayGuard stesso:

| Impostazione | Effetto |
|---|---|
| *Mai* | Nessun PIN in PlayGuard (la campanella lo segnala finché la console ne ha uno) |
| *Prima di una modifica* (predefinita) | Tutti possono consultare, solo il genitore modifica |
| *Per aprire PlayGuard* | Una schermata di blocco all'avvio |

- Una volta inserito, il PIN vale fino alla chiusura di PlayGuard; *Richiedi di nuovo il PIN dopo 5 minuti* (disattivato per impostazione predefinita) lo richiede trascorso quel tempo, e riporta la schermata di blocco di *Per aprire PlayGuard* quando PlayGuard torna in primo piano dopo 5 min o più.
- Controllato nel livello dei servizi, quindi nessuna modifica lo salta; bloccare di nuovo non lo chiede mai. *Mostra il PIN* lo chiede ogni volta, in ogni modalità e anche subito dopo averlo inserito, e un `config.json` mancante o danneggiato vale come *Prima di una modifica*.
#### Il PIN mette in pausa il filtro famiglia mentre sei in PlayGuard

- Una volta inserito il PIN (la schermata di blocco di *Per aprire PlayGuard*, la prima modifica con *Prima di una modifica*, il ritorno dalla schermata del bambino…), PlayGuard sblocca temporaneamente il filtro famiglia, così il tempo passato al suo interno non viene tolto dal tempo di gioco del bambino; *Aperto da un gioco* sparisce da Oggi.
- La pillola dell'intestazione dice *Filtro famiglia in pausa*, in verde acqua; toccarla spiega cos'è, con *Riattiva ora* (poi non torna in pausa finché il PIN non viene inserito di nuovo). Oggi mostra la giornata com'è per il bambino, e la tessera di Sicurezza dice *Filtro famiglia in pausa*, senza avvisi.
- Si riattiva da solo appena si esce da PlayGuard (menu HOME, standby) o lo si chiude, anche dal menu HOME (l'uscita lo aspetta), e mentre è aperta la schermata del bambino o la schermata di blocco. Tornati in PlayGuard con il PIN ancora valido, torna in pausa.
- Non dall'album (lì il tempo non conta, e un gioco può restare aperto dietro), non in modalità sola lettura, e mai sopra uno sblocco fatto con *Sblocca temporaneamente*, che resta com'è.
- Registrato come qualsiasi sblocco: se PlayGuard si fermasse senza bloccarlo di nuovo, il suo avvio successivo lo blocca di nuovo, e un riavvio della console lo fa da solo. Si basa sul fatto che il tempo non conta durante uno sblocco, dedotto ma non ancora misurato ([sotto](#cosa-conta-come-tempo-di-gioco)).
- **Blocca il menu homebrew ›** apre *Console › Forwarder*, dove viene bloccato dopo i suoi controlli (vedi [Forwarder](#forwarder)).
- **Uscire** mentre il filtro famiglia è ancora sbloccato chiede se bloccarlo di nuovo (*Riattiva ed esci* chiude anche il gioco sopra cui PlayGuard è stato aperto, tornando al menu HOME anziché a hbmenu); se invece viene chiuso dal menu HOME, PlayGuard lo blocca di nuovo al prossimo avvio, e un riavvio della console lo blocca da solo. La pausa di PlayGuard stesso non chiede nulla: finisce con la chiusura di PlayGuard.

### App Filtro famiglia

- Se l'app Filtro famiglia per Nintendo Switch è collegata, la sua ultima sincronizzazione, e **scollega** (altrimenti la sua prossima sincronizzazione sovrascrive i limiti impostati qui: finché è collegata, ogni modifica che PlayGuard chiede di confermare lo ricorda).
- Quando il reindirizzamento DNS di Atmosphère (`atmosphere/hosts`) manda altrove il server del filtro famiglia di Nintendo, l'app non può sincronizzarsi: PlayGuard allora indica qui *sincronizzazione bloccata (DNS)* e non nomina l'app da nessun'altra parte (nessuna nota nelle finestre di dialogo, nessuna voce nella campanella, nessun passaggio nei primi passi, nessuna pillola in *Oggi*, nessun aiuto per decifrare, e lo scollegamento chiede conferma senza dire cosa sovrascriverebbe l'app).
- Finché è collegata e può sincronizzarsi, *Prima di scollegare: decifra due impostazioni* apre il confronto del blocco (vedi [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), in inglese).

### Elimina il filtro famiglia

Due conferme, irreversibile; prima viene salvato un backup delle impostazioni.

## Cronologia — cosa ha cambiato PlayGuard, e i backup

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=history)*

**In alto**, come tessere, quante modifiche sono registrate e il giorno dell'ultimo backup: una spunta verde acqua, o un segnale di avviso ambra quando non ce n'è nessuno o nessuno da 30 giorni.

### Cronologia delle modifiche

- Le 500 modifiche più recenti fatte da PlayGuard (limiti, livello limitazioni, PIN, sblocchi, scollegamento, l'orologio, ripristini, pausa o ripresa del conto alla rovescia …) e ciò che ha visto cambiare fuori da lui, raggruppate per giorno (*Oggi*, *Ieri*, poi il giorno e la data), con l'ora e da dove; un limite cambiato per un solo giorno nomina quel giorno.
- Anche una modifica **rifiutata** dalla console è elencata, in ambra, con l'errore della console e ciò che si è tentato (mai rimessa: non è cambiato nulla).
- *Mostra* tiene un solo tipo (tempo di gioco, restrizioni, sicurezza e app, visto fuori da PlayGuard, rifiutato dalla console); *Esporta ciò che è mostrato* lo scrive in `exports/` in CSV, JSON, XLSX o PDF.
- Ⓐ su una modifica la mostra e, per un valore, **rimette quello precedente** — con lo stesso sblocco e lo stesso PIN di ogni modifica, segnalando se nel frattempo è cambiato.
- Ogni modifica conserva la versione di PlayGuard che l'ha fatta (una build di sviluppo: lo dice, con il suo commit e, per la build di una pull request, il suo numero); una modifica registrata da una versione più recente di PlayGuard compare come *Altra modifica*.

### Backup

- **Backup / ripristino delle impostazioni** sulla scheda SD: livello limitazioni, impostazioni personalizzate, modalità VR, sistema di classificazione, limiti giornalieri, l'avviso "Tempo scaduto" (con le azioni avanzate attive), e il blocco grezzo del timer per memoria — mai il PIN.
- Un ripristino elenca solo ciò che cambierebbe (niente da cambiare: lo dice e basta, senza sblocco e senza scrivere nulla), e segnala quando il backup aveva Sogni d'oro attivo (non viene riscritto).
- Scegli quanti backup conservare.

## Console — l'orologio di rete, i giochi installati, cosa esegue la console

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=console)*

**In alto**, ciò che esegue la console come tessere, sotto *Sistema* (il firmware, Atmosphère, hekate quando è sulla scheda SD e sys-patch quando è installato; Ⓐ apre le sue note di rilascio), poi gli strumenti come tessere, sotto *Strumenti* (*Orologio di rete*, *Giochi installati* e *Forwarder*; i primi due mostrano ciò che richiede un'occhiata nel loro angolo), poi le righe della console, prima la compatibilità (solo una spunta verde acqua quando PlayGuard è stato verificato su questo firmware, altrimenti un avviso e cosa non va).

### Orologio di rete

*Orologio di rete ›*, una schermata a parte — l'orologio su cui si basa il tempo di gioco. Una console che non raggiunge mai i server di Nintendo mantiene questo orologio impreciso, il che falsa il tempo di gioco.

- **Orologio della console e orologio di rete**, fuso orario, precisione. Quando il reindirizzamento DNS di Atmosphère (`atmosphere/hosts`) manda altrove il server dell'ora di Nintendo (`aauth-lp1.ndas.srv.nintendo.net`), la console non può più regolare da sola il suo orologio di rete: questa schermata, la voce della campanella e il riquadro di *Oggi* lo dicono e rimandano alla misurazione qui sotto.
- **Scegli un server NTP pubblico** (≈ 50 integrati, per regione, o il tuo).
- **Misura** su 3 server (mediana; un avviso ambra quando non concordano): una finestra di dialogo mostra il risultato — di quanto l'orologio di rete è avanti o indietro, in grande, l'ora dei server accanto a quella dell'orologio di rete, la risposta di ogni server — e da lì imposta l'orologio di rete. Una misurazione resta valida per 2 minuti, con un conto alla rovescia sotto *Imposta l'orologio di rete*. La conferma lo dice: cambiare l'orologio azzera il tempo di gioco di oggi, quindi l'intero limite torna disponibile.
- **Un controllo dell'orologio di rete all'avvio**: un avviso a comparsa quando è sfasato di più di un minuto; non imposta mai l'orologio.

#### Nessun server risponde

Su una console senza Internet, *Usa l'orologio della console* è proposto nella finestra di dialogo del risultato, *Imposta l'orologio di rete sull'orologio della console* sotto la misurazione (e sul riquadro dell'orologio di rete di *Oggi*, quando la sua misurazione non riesce), dietro la stessa conferma.

- PlayGuard non ha nient'altro con cui confrontare l'orologio della console: la conferma mostra la sua ora e ti chiede di controllarla prima con un telefono o un orologio, e un orologio della console precedente al 2026 (mai impostato) viene rifiutato.
- Con *Sincronizza l'orologio tramite Internet* disattivato, l'orologio della console resta dov'è, e attivare l'impostazione in seguito non lo fa più saltare; se è attivato, l'orologio della console segue già l'orologio di rete, quindi la conferma dice di disattivarlo e di impostare l'ora a mano se è sbagliata.
- Scrive solo l'orologio, non il filtro famiglia, quindi non serve alcuno sblocco (il PIN di *Proteggi PlayGuard con il PIN* si applica comunque); come ogni cambio d'orologio, azzera il tempo di gioco di oggi.
- Non ancora provato su una console.

### Giochi installati

*Giochi installati* (una tessera sotto *Console › Strumenti*) trova i giochi che il menu HOME non riesce ad avviare o a mostrare — un'icona che continua a caricare, un "!", una cornice tratteggiata — con ciò che non va e cosa fare, sotto l'icona del gioco, e il numero di giochi installati che hanno un problema.

**Cosa trova:** nome e icona illeggibili, il gioco stesso mancante dietro il suo aggiornamento, più niente di installato, file mancanti, dati danneggiati rilevati dalla console, un firmware più recente richiesto dal gioco o solo dal suo aggiornamento, chiavi troppo recenti per il firmware, una licenza (ticket) mancante, la console che chiede un aggiornamento che è stato rimosso, la scheda SD illeggibile, un avvio rifiutato dalla console.

**L'elenco ha una sezione per ogni via d'uscita**, ognuna con il suo numero e cosa fare, i giochi con lo stesso problema raggruppati (due giochi con lo stesso nome si distinguono per versione o ID titolo):

| Sezione | Cosa contiene |
|---|---|
| **Riparabili qui** | Ciò che PlayGuard ripara |
| **Da verificare** | Dati di cui la console dubita: la verifica dice se reinstallare il gioco |
| **Da reinstallare** | File spariti o inutilizzabili, o dati trovati danneggiati: rimuovi, poi reinstalla |
| **Firmware o scheda SD** | Il gioco è a posto, la console no |

- **Verificare i dati:** ciò che la verifica dei dati ha trovato resta associato al gioco finché non viene installata un'altra versione: danneggiato, passa in *Da reinstallare* come *Danneggiato (verificato)*, con la data e il codice della console; integro, esce dall'elenco. *Verifica questi giochi* verifica tutta la sezione *Da verificare*, un gioco dopo l'altro in un'unica finestra, con le loro copertine sopra (*gioco 2 di 5*), poi dice quanti sono danneggiati, integri o non è stato possibile verificare, con la rimozione di quelli danneggiati a portata di mano.
- **Più giochi insieme:** Ⓨ seleziona dei giochi (un segno di spunta davanti); dal primo selezionato, una riga sopra le sezioni dice quanti sono, e A su di essa seleziona anche un'intera sezione, poi un'unica azione per tutti: verificarne i dati, o rimuoverli (il loro elenco salvato prima, una sola domanda, un solo PIN).
- **Rimuoverne diversi:** le copertine volano via man mano che ogni gioco se ne va (un "!" corallo su quello che resta), la barra, un segmento per gioco (fino a 20); alla fine, quanti ne sono stati rimossi e quanto spazio è stato liberato, e la finestra si chiude da sola. Se ne restano: le loro copertine cerchiate di corallo, e *Riprova* per loro.
- **Sotto *Da reinstallare*,** il loro elenco si può salvare (JSON e testo, nella cartella delle esportazioni: nome, ID titolo, versione, problema), e si possono rimuovere tutti insieme dopo una sola domanda e un solo PIN: prima viene salvato l'elenco, poi i giochi vengono rimossi perché un installer non li conti più come installati.
- **Le azioni:** quando un'azione è la soluzione, è il primo pulsante della finestra; seguono le altre: accettare di nuovo la versione installata (quando la console chiede un aggiornamento rimosso), rimuovere solo l'aggiornamento o un contenuto aggiuntivo, verificare se i dati sono danneggiati (la verifica della console stessa, con il suo avanzamento e Annulla), o rimuovere il gioco.
- Le versioni sono mostrate come le mostra il gioco (*versione 1.4.0*, *aggiornamento n. 4*); dopo una riparazione, un avviso a comparsa dice se il gioco funziona di nuovo. I dati di salvataggio restano sempre; ogni modifica è nella cronologia delle modifiche.
- **Quando controlla:** l'ultimo controllo compare subito; poi vengono ricontrollati solo i giochi la cui voce è cambiata (tutti i giochi una volta al giorno, dopo un aggiornamento del firmware, o con Ⓧ *Controlla di nuovo*), ognuno mostrato appena controllato. Ⓧ *Controlla di nuovo* apre una finestra: le copertine dei giochi sfogliate, una barra e il conteggio; *Annulla* si ferma dopo il gioco in corso, ciò che è stato controllato resta. Se nessun gioco è ancora elencato, le copertine scorrono nel riquadro durante il controllo.
- Più giochi guasti insieme indicano la scheda SD o un gioco installato dall'altra memoria (sysMMC / emuMMC).
- Ⓡ mostra i giochi come **elenco** o come **copertine** (icona, nome e problema; la scelta viene ricordata), come fa *Tutti i giochi*.

### Forwarder

*Forwarder ›*, una schermata a sé, una tessera sotto *Strumenti* (anche *Sicurezza › Blocca il menu homebrew*): un homebrew nel menu HOME, con la sua icona e il suo nome, avviato da lì come un gioco (un forwarder di NRO, come li crea sphaira).

- **Un riquadro in alto** dice a che punto è il menu homebrew: bloccato o aperto, le sue vie di ritorno (il forwarder di PlayGuard, i menu protetti, quelli mai avviati finora dal menu HOME), ciò che lo apre ancora senza il PIN, e se il sysmodule di ripristino è installato. Ⓐ su di esso elenca cosa fare, ogni azione a una pressione di distanza (il passo successivo verso il blocco, proteggere o eliminare un forwarder che lo apre senza il PIN, il controllo, la scheda di soccorso).
- **L'elenco per gruppi:** quello di PlayGuard, gli altri che ha creato (protetti o no, in un'unica scheda: l'icona dice quale), quelli creati da un'altra app, quelli non più nel menu HOME. Uno protetto ha un lucchetto sulla sua icona (una spunta per un menu visto funzionare attraverso il suo cancello), uno creato qui senza il PIN un lucchetto aperto in ambra.
- **Ⓨ** apre questa parte della guida (un codice QR). Dopo aver eliminato o dimenticato un forwarder, o cambiato i suoi orari senza il PIN, *Chiedi il PIN* o *Chiuso esaurito il tempo di gioco di oggi*, **ZR** lo rimette per qualche secondo (reinstallato con lo stesso ID programma, o l'impostazione com'era).
- **PlayGuard** con una sola pressione: la sua icona e il suo nome, con il forwarder che avvia il `.nro` da cui è stato aperto PlayGuard.
- **Un altro homebrew**: qualsiasi `.nro` della cartella `/switch` della scheda SD (e di una cartella più sotto), con il suo nome chiesto prima (quello del `.nro`, da tenere o cambiare), poi confermato con ciò che mostrerà il menu HOME: la sua icona, il suo nome e il suo editore. La sua icona è quella del `.nro`, convertita nel JPEG 256 × 256 che mostra il menu HOME quando ha un'altra dimensione o è un PNG (un controller al suo posto quando il `.nro` non ne ha), come fa sphaira. Un percorso con uno spazio viene passato all'homebrew tra virgolette.
- **I forwarder del menu HOME**, ognuno con la sua icona: quelli creati da PlayGuard (Ⓐ: dettagli, rinomina, cambia l'editore, reinstalla, elimina) e quelli di un'altra app (dettagli, elimina). Ⓐ apre una scheda per sezioni (*Questo forwarder*, *All'avvio*, *Protezione*, *Installa ed elimina*) sotto la sua icona e il suo stato, ogni riga con la sua icona e ciò che è impostato ora (i suoi orari; un interruttore per *Chiedi il PIN*, *Chiuso esaurito il tempo di gioco di oggi* e ogni opzione di avvio), *Elimina* in rosso; B o un tocco fuori la chiude. *Dettagli* elenca le sue regole allo stesso modo, poi il suo editore, la sua versione, il suo `.nro` e il suo ID programma; Ⓐ su una regola (o sull'editore) la cambia, come nella scheda. Sotto quelli protetti, una riga dice cosa significa ogni icona mostrata lì. Uno il cui `.nro` non c'è più lo dice; uno eliminato nelle Impostazioni di sistema resta in elenco finché non viene reinstallato o dimenticato. Reinstallare mantiene il suo ID programma, così la voce del menu HOME viene sostituita, non raddoppiata; anche un forwarder creato da un altro strumento per lo stesso `.nro` viene sostituito.
- **Protetto con il PIN** (Ⓐ › *Proteggi con il PIN*, per uno creato da PlayGuard diverso dal suo): avviato dal menu HOME, il forwarder apre prima un piccolo cancello (`config/playguard/gate.nro`, qualche centinaio di kilobyte che PlayGuard scrive dai propri file), che mostra subito la schermata del PIN della console, poi l'homebrew una volta inserito il PIN giusto; B lì riporta al menu HOME. Attorno alla schermata del PIN, quella del cancello: l'icona e il nome dell'homebrew, *Protected by PlayGuard*, e com'è andata (avvio, nei suoi orari liberi, PIN non inserito), nella lingua della console, sul blu notte di Cadran (`make gate-preview` la disegna su un computer). Mantiene il suo ID programma e la sua icona, e la cronologia delle modifiche lo registra. La sua riga mostra le sue regole come icone, una per ciascuna attiva: una chiave per il PIN, un lucchetto aperto per gli orari senza, un quadrato (stop) per gli orari in cui non si apre mai, un timer per chiuso esaurito il tempo di gioco di oggi; *Dettagli* (Ⓐ) le dice a parole. Serve un PIN sulla console. *Non chiedere più il PIN* lo chiede prima, qualunque cosa preveda *Proteggi PlayGuard con il PIN*, e così anche eliminarlo e ricrearlo senza PIN (*Aggiungi un forwarder* per lo stesso `.nro`). Protegge solo l'icona del menu HOME: il `.nro` si avvia ancora dal menu homebrew, a meno che questo non sia bloccato (sotto). Un forwarder protetto da un PlayGuard precedente avvia PlayGuard stesso come cancello, il che funziona ancora; *Reinstalla tutti* lo passa al piccolo cancello.
  - **Orari senza PIN** (Ⓐ su uno protetto): ogni giorno, dal lunedì al venerdì o nel fine settimana, da un'ora a un'altra (anche oltre la mezzanotte), oppure *Dal lunedì al venerdì, e il fine settimana a parte* (un intervallo per ciascuno); *Dettagli* dice quando. Impostarli chiede il PIN. Seguono l'orologio della console, che le Impostazioni di sistema possono cambiare: appena si discosta di più di 5 minuti dall'orologio di rete, il cancello chiede comunque il PIN in quegli orari (*Niente orari liberi: l'orologio della console non coincide con quello di rete*), e così pure quando l'orologio di rete non può essere letto per verificarlo (*Niente orari liberi: ora di rete non disponibile*); *Console › Orologio di rete* li rimette entrambi a posto, anche su una console senza Internet.
  - **Orari in cui non si apre mai** (Ⓐ su uno protetto), impostati allo stesso modo: il cancello dice allora *Non a quest'ora* e gli orari, e torna al menu HOME, anche con il PIN inserito (l'ora di andare a letto per gli homebrew). Vengono prima degli orari senza PIN. Con l'orologio della console a più di 5 minuti da quello di rete, resta chiuso quando uno dei due orologi è in quegli orari, e una pausa dopo i rifiuti si conta con l'orologio di rete e con l'orologio continuo della console (che nessuna impostazione sposta), così cambiare l'ora non li aggira.
  - **Chiuso quando il tempo di gioco di oggi è esaurito** (Ⓐ su uno protetto): quando il timer della console non ha più tempo per oggi, il cancello dice *Il tempo di gioco di oggi è esaurito* e torna al menu HOME, con o senza PIN. Disattivato finché non lo si imposta.
  - **Le regole sono nel forwarder, non sulla scheda SD:** gli orari, il limite del giorno e la pausa dopo i rifiuti vanno nei dati del forwarder stesso (nella memoria della console), così un computer non può aggiungere orari liberi modificando un file; cambiarne uno rifà il forwarder, in pochi secondi. Il forwarder conserva anche lo SHA-256 del cancello (`config/playguard/gate-<hash>.nro`): un cancello modificato sulla scheda SD non viene avviato (errore 2347-0100, [risoluzione dei problemi](troubleshooting.md#un-forwarder-protetto-mostra-lerrore-2347-0100)). Uno protetto da un PlayGuard precedente legge le sue regole dalla scheda: i suoi orari senza PIN allora non vengono rispettati, e la campanella propone *Rifai*.
  - **Una pausa dopo PIN rifiutati** (*Pausa dopo PIN rifiutati*, sotto *Menu homebrew*): cinque rifiuti di fila (B sulla schermata del PIN) e il forwarder non lo chiede più per 15 minuti, con il cancello che dice fino a quando; 3, 5 o 10 tentativi e 15 minuti o un'ora, oppure mai. La campanella dice quale forwarder è in pausa, e anche la sua riga (un'icona di pausa, e fino a quando); *Togli la pausa* (Ⓐ su di esso, con il PIN chiesto) lo fa richiedere subito. Ogni pausa e ogni pausa tolta compare nella cronologia delle modifiche.
  - **Ogni tentativo è nella cronologia delle modifiche**: aperto con il PIN, aperto nei suoi orari, o rifiutato (B sulla schermata del PIN), all'ora in cui è successo; i tentativi di uno stesso tipo, in uno stesso giorno, per uno stesso forwarder formano una sola voce, con i loro orari. Il cancello li annota sulla scheda SD; PlayGuard li legge al suo avvio successivo. *Cronologia › Mostra › Homebrew e forwarder* elenca solo questi, i forwarder e il blocco.
  - **Tentativi della settimana** (in *Cronologia* e sotto *Menu homebrew*): per gli ultimi sette giorni, le aperture di ogni forwarder, i suoi rifiuti e in quali giorni, e le volte in cui non si è aperto; conservati per quattro settimane. Quando i rifiuti di questa settimana sono più del doppio di quelli della settimana prima (almeno cinque), la campanella lo segnala.
  - **La schermata del cancello dice di più:** i suoi orari senza PIN mentre lo chiede, e i rifiuti del giorno finora (in ambra) mentre lo chiede e una volta rifiutato, sotto *Chiedi a un genitore di inserire il PIN*. Dopo *Protected by PlayGuard*, un'icona per ogni regola che porta, come la sua riga in PlayGuard: un lucchetto aperto (orari senza PIN), un quadrato (orari in cui non si apre mai), un timer (il tempo di gioco di oggi). Una volta rifiutato, Ⓐ riprova e Ⓑ torna al menu HOME (così come attendere 15 secondi), finché non inizia la pausa dopo i rifiuti. Dice anche fino a quando si apre senza PIN, quando finisce una pausa (*tra 12 min*), il tempo di gioco di oggi rimasto (con *Chiuso esaurito il tempo di gioco di oggi*), e il codice di errore quando la schermata del PIN non si apre. La cornice dell'icona prende il colore di com'è andata. Se il cancello dovesse mai fermarsi sulla strada verso l'homebrew, si chiude dopo 10 secondi, tornando al menu HOME, invece di lasciare la console sulla sua schermata.
  - **La schermata di apertura** (*Schermata di apertura*, sotto *Menu homebrew*): avviato dal menu HOME, il cancello apre prima l'homebrew come PlayGuard apre se stesso: centrato sul blu notte con il quadrante di Cadran a destra, l'icona dell'homebrew su un leggero alone verde acqua, il suo nome, *Protected by PlayGuard*, una barra che si riempie e, in basso, il marchio di PlayGuard con il suo anello che gira. Poi la schermata del PIN della console, e l'homebrew subito una volta inserito il PIN giusto (nei suoi orari liberi, subito dopo la barra). Dura 2 secondi, a meno che non sia impostata su 1, 3 o 5 secondi, oppure su *Disattivata* (subito la schermata del PIN); Ⓐ o Ⓑ saltano il resto, e riprovare dopo un rifiuto non la mostra di nuovo. Viene letta dalla scheda SD a ogni avvio (`config/playguard/gate_wait.txt`), quindi cambiarla non rifà i forwarder.
- **La sua icona dall'album** (Ⓐ › *Cambia l'icona*): i 30 screenshot più recenti dell'album in una griglia, ognuno come l'icona che darebbe, prima quella del `.nro` e spuntata quella in uso; accanto, l'icona che il menu HOME mostra ora e quella che mostrerà, il gioco in cui è stato scattato lo screenshot e quando (*Oggi*, *Ieri*, al secondo quando due sono stati scattati nello stesso minuto). L e R spostano il quadrato dell'icona lungo lo screenshot (cinque posizioni, dal bordo sinistro a quello destro), Y riprende quella del `.nro`, A quella selezionata: diventa il JPEG 256 × 256 del menu HOME. Reinstallarlo la mantiene, inquadratura compresa.
- **Icone e lingue:** un'icona di un'altra dimensione, un PNG, o un JPEG con dati EXIF o codifica progressiva (il menu HOME li mostra come un "?") diventa un semplice JPEG 256 × 256. I dati di controllo del forwarder dichiarano solo l'inglese americano, la lingua della sua unica icona: così l'icona compare su una console impostata in qualsiasi lingua.
- **All'avvio** (Ⓐ su un forwarder, come le opzioni dei forwarder di sphaira): *Scegli un utente all'avvio* (disattivato per impostazione predefinita) chiede quale utente gioca quando si avvia, come fa un gioco, così il suo tempo di gioco va a quell'utente in *Attività* (il profilo di un genitore lascia le statistiche del bambino come sono); il limite di tempo di gioco della console lo conta comunque, chiunque giochi, come conta ogni gioco. *Screenshot* e *Registrazione video* (attivi per impostazione predefinita) permettono al tasto Cattura di farli. Ogni interruttore chiede il PIN e ricrea il forwarder (lo stesso ID programma), ed è nella cronologia delle modifiche; *Dettagli* li elenca.
- **Proteggi tutti i forwarder con il PIN** e **Reinstalla tutti** (l'icona e la versione di un nuovo `.nro`, il cancello come lo porta questo PlayGuard, uno eliminato dal menu HOME rimesso): con una sola pressione ciascuno.
- **Blocca il menu homebrew** (*Menu homebrew*, in fondo alla schermata): né l'album né un gioco avviato tenendo premuto R aprono più il menu homebrew, così un homebrew si avvia solo dal suo forwarder, che può chiedere il PIN.
  - **Passo dopo passo:** finché non si può bloccare, Ⓐ elenca i passi, numerati, il successivo in ambra e a una pressione di distanza (*Passo successivo*): un PIN, le patch di firma, il forwarder di PlayGuard, un menu homebrew protetto con il PIN (sphaira trovato in `/switch`, altrimenti hbmenu, creato già protetto in un colpo solo), avviato una volta dal menu HOME; l'elenco ritorna dopo ogni passo che crea un forwarder.
  - Servono due vie di ritorno a PlayGuard dal menu HOME: il suo forwarder, e un forwarder di un menu homebrew (sphaira o hbmenu, dal nome del `.nro`) protetto con il PIN, che tiene ogni homebrew, uno store di homebrew compreso, dietro al PIN. E un PIN sulla console, e le patch di firma di cui i forwarder hanno bisogno. Fino ad allora, Ⓐ dice cosa manca.
  - La via di ritorno del menu conta una volta vista funzionare: avviato una volta dal menu HOME, il PIN inserito al suo cancello (lo dice il registro del cancello, letto quando PlayGuard si apre o blocca). Un menu protetto e mai avviato in quel modo viene indicato come tale.
  - Il blocco mostra i controlli come elenco (una spunta verde acqua per ciò che è pronto, un triangolo ambra o corallo per ciò che non lo è), e chiede di tenere premuto Ⓐ 2 secondi su *Blocca* (o un dito tenuto sopra), con una barra che intanto si riempie; lasciato prima, non cambia nulla. Poi arriva la **scheda di soccorso** (scritta anche in `config/playguard/rescue.html`, una pagina da aprire e stampare su un computer, nella lingua dell'interfaccia, con il codice QR): come annullarlo da un computer, o con il sysmodule di ripristino quando è installato, e un codice QR verso l'aiuto ([sotto](troubleshooting.md#menu-homebrew-bloccato)); anche *Controlla il menu homebrew* la propone.
  - Su emuMMC (o su un sysMMC con un emuMMC configurato), avverte che il blocco è sulla scheda SD, quindi lo leggono entrambi i sistemi, mentre i forwarder sono di ciascun sistema: l'altro sistema non ha nessuna di queste vie di ritorno.
  - Finché è bloccato, eliminare il forwarder di PlayGuard, o l'ultimo menu protetto (o togliergli il PIN), viene rifiutato: sblocca prima. Eliminare uno di più menu protetti chiede conferma, in corallo, nominando quelli che restano.
  - Prima di bloccare, propone di proteggere in un colpo solo i forwarder creati da PlayGuard non ancora protetti, e dice quali avvierebbero ancora homebrew senza il PIN (un menu non protetto, o uno creato da un'altra app), quali scrivono sulla scheda SD (un gestore di file, un installer come Cyberfoil o Goldleaf, un server FTP: potrebbero modificare il file del blocco), e quando è installato uLaunch (avvia gli homebrew a modo suo, dove il blocco non arriva).
  - Il PIN rifiutato tre volte o più in uno stesso giorno al cancello di un forwarder viene segnalato nella campanella (quale forwarder, il giorno, quante volte).
  - Quando la scrittura di un file non riesce, l'errore dice perché quando può saperlo: la scheda SD piena, o niente scritto su di essa (il suo interruttore di blocco, o il bit di archivio che corregge *Tools* di hekate).
  - *Controlla il menu homebrew* mostra ciò che leggerà Atmosphère (se l'album e un gioco avviato con R aprono il menu homebrew), le vie di ritorno, le patch di firma, la versione di Atmosphère su cui è stato bloccato e il sysmodule di ripristino; il rapporto di diagnostica riporta lo stesso. Se non è bloccato, lo dice per primo e propone *Blocca*, con i passaggi ancora da fare (la scheda di soccorso resta); un sysmodule di ripristino non aggiornato, il PlayGuard con cui è arrivato e come sostituirlo (*Aggiorna il modulo* quando questa build lo include). Su emuMMC, o su sysMMC con un emuMMC configurato, dice che anche l'altro sistema legge il blocco, senza i forwarder installati qui.
  - Sono alcune righe alla fine di `atmosphere/config/override_config.ini`, tra due commenti di PlayGuard; il resto del file resta com'è, e viene conservato com'era in `config/playguard/override_config.ini.backup`. Atmosphère legge il file a ogni avvio di un programma: nessun riavvio. Sbloccare toglie le righe, e riscrive i valori di Atmosphère per le chiavi che il resto del file non imposta (Atmosphère tiene ciò che ha letto fino al riavvio della console).
  - Bloccare e sbloccare chiedono il PIN qualunque cosa preveda *Proteggi PlayGuard con il PIN*, e compaiono nella cronologia delle modifiche. Finché è bloccato, PlayGuard controlla a ogni avvio che entrambe le vie di ritorno ci siano ancora; la campanella dice quando una manca. Un blocco tolto dal file fuori da PlayGuard (un computer, un homebrew) viene segnalato nella campanella e nella cronologia delle modifiche; PlayGuard non lo rimette mai da sé, perché eliminare il file è la via di ritorno da un computer. Eliminare un forwarder (Impostazioni di sistema › Gestione dati, dove non viene chiesto alcun PIN) non toglie mai il blocco: toglie solo una via di ritorno.
  - Da un computer: elimina `atmosphere/config/override_config.ini` (o rimetti il backup), poi riavvia la console.
  - Con il sysmodule di ripristino installato (facoltativo, [sysmodule/README.md](../../sysmodule/README.md), in inglese), anche una richiesta `RESCUE` lo toglie all'avvio, e così un aggiornamento di Atmosphère dopo il blocco (le sue patch di firma possono essere in ritardo, e senza di esse nessun forwarder si avvia): il successivo avvio di PlayGuard lo segnala, e propone di bloccarlo di nuovo.
- **Un forwarder eliminato fuori da PlayGuard** (nelle Impostazioni di sistema): segnalato una volta nella cronologia delle modifiche, e nella campanella finché dura, con *Reinstalla* o *Dimentica* sulla sua riga.
- **Patch di firma:** la console avvia un forwarder solo con le patch di firma di fs e del loader (sys-patch, o file sigpatch), con applicato ogni pattern che sys-patch ha per questo firmware. Quando mancano, o sys-patch ne ha tralasciato uno (un sys-patch più vecchio su un firmware più recente), la schermata lo dice sopra l'elenco, nominando ciò che non è patchato, e aggiungere un forwarder, creare quello di PlayGuard o reinstallarli tutti dice prima che il forwarder non si avvierebbe ancora: aggiungerlo comunque (si avvia una volta che le patch sono a posto), o annullare. Anche il blocco del menu homebrew le attende (*Console › Patch dei giochi* dà i dettagli). I file sigpatch lasciati accanto a sys-patch vengono segnalati, e *Console › Patch dei giochi* li mette da parte (spostati, mai eliminati). Ogni forwarder mantiene il loader con cui è stato creato: a quelli creati da un PlayGuard più vecchio viene proposto di essere ricreati (la campanella, *Console › Forwarder*). Il `.nro` resta dov'è; eliminare un forwarder non lo tocca mai.
- **Tempo di gioco:** avviato dalla sua icona, un homebrew funziona come un gioco: il timer lo conta e lo sospende come qualsiasi gioco quando il tempo è scaduto, e *Attività* lo elenca. PlayGuard resta a portata di mano dal menu homebrew.
- **Nei backup delle impostazioni** (*Cronologia › Backup*): ciò che PlayGuard ha creato e se il menu homebrew era bloccato. Ripristinare un backup propone, dopo il resto, di rimettere i forwarder che non sono più nel menu HOME (protetti come prima, con i loro orari e la loro icona); un blocco viene solo segnalato, da rifare da qui.
- Ogni modifica chiede il PIN quando lo prevede *Proteggi PlayGuard con il PIN*, viene rifiutata in modalità sola lettura e compare nella cronologia delle modifiche. Ciò che PlayGuard ha creato viene conservato per sistema (sysMMC / emuMMC), in `forwarders.json`.

### Le righe della console

- **Firmware, Atmosphère, hekate** (quando il suo `bootloader/update.bin` è sulla scheda SD) **e sys-patch** (quando è installato; la sua versione dal suo `config/sys-patch/log.ini`), ognuno una tessera con la versione installata, poi la versione più recente trovata dall'ultima ricerca di aggiornamenti in una pillola ambra (*23.0.1 disponibile*, la tessera bordata d'ambra, e di quante versioni si è indietro sotto di essa quando sono più di una), oppure *Aggiornato*; un avviso nell'angolo della tessera del firmware quando il firmware più recente richiede prima un Atmosphère o un hekate più recente, o non è ancora supportato da loro (le note di rilascio del firmware dicono perché: le loro note di rilascio dicono quali firmware supportano).
- **Note di rilascio:** Ⓐ su ognuno le apre: una scheda per la versione installata, una per ogni versione più recente (al massimo cinque) e versioni più vecchie nelle schede rimaste (sei in tutto; Ⓛ / Ⓡ per passare dall'una all'altra), aperte sulla successiva, con etichette (una spunta per la versione installata, una scintilla per una più recente, il firmware più recente che supporta, correzioni di sicurezza, filtro famiglia modificato), le note come pubblicate (in inglese), *Vedi online* (un codice QR) e quando sono state recuperate. Quelle di Atmosphère, hekate e sys-patch vengono dalle loro release su GitHub (quelle di sys-patch da `impeeza/sys-patch`), quelle del firmware da switchbrew; sono conservate sulla scheda SD per la lettura offline, recuperate di nuovo una volta al giorno (Ⓧ subito).
- **Compatibilità**, poi **memoria** (emuMMC o sysMMC, con un'icona informativa quando l'altro sistema ha il proprio filtro famiglia: Ⓐ spiega).
- Se Atmosphère **oscura il numero di serie** (*Sì (XAW00000000000)*, il numero che il sistema vede allora; su emuMMC, quando non lo fa, un'icona di avviso, che Ⓐ spiega), poi il **numero di serie reale** della console, con la parte centrale mascherata: quando è oscurato, letto dal backup di PRODINFO che Atmosphère conserva in `atmosphere/automatic_backups/` (quello di questa console, in base al suo ID dispositivo, quando la scheda SD ne contiene diversi). Ⓐ sulla sua icona informativa lo mostra per intero e dice da dove viene.
- **Patch dei giochi** (sys-patch o file sigpatch, con sys-patch consigliato quando si usano solo file: un'icona informativa, o di avviso quando qualcosa va controllato, che Ⓐ spiega).

## Preferenze

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=preferences)*

- Ciò che configura PlayGuard stesso:
  - Prima, come tessere: lingua (tutte le lingue della console, o quella della console), tema (scuro per impostazione predefinita, chiaro, o quello della console), dimensione del testo (automatica / normale / più grande) e la scheda da cui partire.
  - **L'intro** all'avvio: la schermata di caricamento (la sua icona con l'anello che gira, la barra, il quadrante di Cadran a destra) per 2 secondi (attiva per impostazione predefinita; Ⓐ o Ⓑ la salta; disattivata, PlayGuard si apre appena è pronto).
- I messaggi che mostra:
  - I tipi di notifiche (vedi [sopra](#come-si-comportano-le-voci)).
  - Un **promemoria mensile per sostenere PlayGuard** (attivo per impostazione predefinita, mai nel primo mese né subito dopo un aggiornamento; *Non mostrare più* sul promemoria o questo interruttore lo disattiva per sempre, aggiornamenti compresi).
- Come si aggiorna: *Cerca aggiornamenti ogni giorno* all'avvio, e *Aggiorna con* (vedi [Informazioni](#versione-e-aggiornamenti)).
- **Primi passi** di nuovo (anche in *Informazioni › Aiuto e diagnostica*).
- Un'impostazione che riguarda una sola funzione sta accanto a essa: il tempo extra in *Tempo di gioco*, la riattivazione del filtro famiglia in *Sicurezza*, il controllo dell'orologio in *Console › Orologio di rete*. La cronologia delle modifiche e i backup hanno la loro scheda, [Cronologia](#cronologia--cosa-ha-cambiato-playguard-e-i-backup); la ricerca degli aggiornamenti adesso e *Aiuto e diagnostica* sono in *Informazioni*.

## Informazioni

*[Provalo nel simulatore ›](https://playguard.link/simulator/?tab=about)*

### Versione e aggiornamenti

- In alto, il suo riquadro: il logo, il nome, la versione.
- Il PlayGuard installato: versione (in ambra quando ne è uscita una più recente; Ⓐ apre le sue **note di rilascio**).
- **Cerca aggiornamenti** ora; *Cerca aggiornamenti ogni giorno* all'avvio è in *Preferenze*.
- *Aggiorna con* (in *Preferenze*): in PlayGuard, che scarica la versione, la verifica, si sostituisce e si riavvia, oppure sphaira, Homebrew App Store o manualmente (chiesto al primo aggiornamento se non è stato scelto nulla). Passare la mano a uno store chiede il PIN come una modifica (*Proteggi PlayGuard con il PIN › Prima di una modifica*), e sempre mentre il menu homebrew è bloccato: uno store avvia qualsiasi homebrew. Installato dall'Homebrew App Store o dall'App Store di sphaira, lì è consigliato lo store, così tiene traccia della versione; quando il suo catalogo non ha ancora la nuova versione (ne pubblica una dopo una verifica, spesso qualche giorno più tardi), PlayGuard lo dice e propone invece di aggiornarsi da sé.
- Finché è nota una versione più recente, PlayGuard si apre su **PlayGuard X.Y.Z è disponibile** (la sua data e le sue note): *Aggiorna*, o *Più tardi* per 1 giorno, 1 settimana o al massimo 30 giorni.
- Le note di rilascio sono quelle di PlayGuard, sulla stessa schermata di quelle di Atmosphère e hekate in *Console*: dalle sue release su GitHub, una scheda per versione (quella installata, le più recenti, le più vecchie), ognuna con i suoi punti salienti come schede e tutte le modifiche, conservate sulla scheda SD per la lettura offline (quelle della versione installata sono anche nell'app, per quando non è stato ancora recuperato nulla).
- Dopo un aggiornamento, PlayGuard si apre una volta su **Novità della X.Y.Z** (gli stessi punti salienti, poi i codici QR).

### Aiuto e diagnostica

*Aiuto e diagnostica ›*, una schermata a parte, sotto *Cerca aggiornamenti*:

- **In alto**, sotto *Per iniziare*, due tessere: *Primi passi* e *Invia un rapporto…*; poi *Diagnostica*.
- **Primi passi** riapre la guida (con un passaggio *Scollega l'app Filtro famiglia* finché è collegata, un passaggio *Riattiva l'avviso "Tempo scaduto"* finché è disattivato, un passaggio *Proteggi PlayGuard* una volta impostato un PIN — la scelta *Proteggi PlayGuard con il PIN* —, e un interruttore per non farla più comparire all'avvio). Sotto *Chiudi*, *Sostieni PlayGuard* mostra i codici QR per il sostegno.
- Come è stato avviato PlayGuard (un'applicazione sopra un gioco o dalla sua icona HOME, o un applet) e la sua cartella dei dati.
- **Esporta un rapporto di diagnostica**, o **invialo online** (*Invia un rapporto…*; vedi [Segnalare un bug](troubleshooting.md#segnalare-un-bug)); quanti rapporti salvati, file di crash, registri di sessione e link dei rapporti inviati online vengono conservati, per numero, età o dimensione.

### Crediti

*Crediti e licenza ›*: chi l'ha fatto, su cosa si basa, la sua licenza e il suo codice sorgente come codice QR, in un'unica finestra. Poi come **sostenere PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), mostrati come codici QR da scansionare con un telefono), e un piccolo *Made in France* 🇫🇷.

## Sicurezza fin dalla progettazione

**Scrivere il limite di tempo di gioco in sicurezza.** Sovrascrivere la configurazione del timer mentre conta alla rovescia destabilizza Atmosphère. Quindi PlayGuard controlla prima lo stato; se il timer è attivo, la conferma dice che il filtro famiglia verrà sbloccato temporaneamente — con il PIN memorizzato, **non serve ricordarlo**. Una sola pressione allora sblocca, controlla che il sistema confermi davvero lo sblocco, scrive, e **blocca di nuovo subito** (o lo propone, se *Riattiva il filtro famiglia dopo una modifica* è disattivato). Il livello dei servizi ricontrolla lo stato subito prima di scrivere, quindi nessuna schermata può saltare la protezione.

> [!CAUTION]
> Un limite inferiore al tempo già giocato oggi sospende il gioco appena il filtro famiglia viene bloccato di nuovo. La conferma lo dice quando succederebbe.

**Niente più crash con la 22.5.** `pctl:a`, il servizio privilegiato del filtro famiglia, accetta **una sola sessione**. Le vecchie versioni dell'app originale la tenevano aperta, così la richiesta del PIN del menu HOME (o l'applet del PIN) non poteva ottenerla e Atmosphère poteva andare in crash. PlayGuard apre la sessione per ogni azione e la rilascia subito; gli aggiornamenti periodici si fermano mentre l'app è in background. *(Diagnosi del [fork di anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Niente gira in background.** Il limite, il PIN, gli avvisi e la sospensione sono quelli della console; PlayGuard cambia solo le loro impostazioni.

## Cosa conta come tempo di gioco

Il timer è quello della console: PlayGuard lo legge, non conta nulla da sé. Cosa si sa, e con quale certezza ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), in inglese, riporta le misurazioni):

| Situazione | Conta? | Con quale certezza |
|---|---|---|
| Un gioco in esecuzione | Sì | Misurato (22.0.0) |
| **Un gioco lasciato aperto nel menu HOME** (sospeso in background) | **Sì**: chiudilo, o metti la console in modalità standby | Riferito da genitori ([Arqade](https://gaming.stackexchange.com/questions/398954/)); coerente con le misurazioni (la console conta mentre un'applicazione è aperta) |
| Modalità standby | No | Riferito ([Arqade](https://gaming.stackexchange.com/questions/398954/)); non misurato da PlayGuard |
| **PlayGuard aperto sopra un gioco** (come applicazione) | **Sì**, come quel gioco | Misurato (22.0.0): aprilo con l'utente di un genitore, o dall'album per una rapida occhiata |
| **PlayGuard avviato da un forwarder** (la sua icona nel menu HOME, anch'essa un'applicazione) | **Sì**, con il nome dell'icona | Dedotto: un'applicazione come le altre; non misurato da PlayGuard |
| PlayGuard aperto dall'album (modalità applet) | No | Dedotto: non è un'applicazione, e il tempo fuori dalle applicazioni non sembrava essere contato |
| Mentre il filtro famiglia è sbloccato temporaneamente (anche la pausa di PlayGuard mentre sei al suo interno) | Forse no | Non misurato: questione ancora aperta |
| Dopo un cambio d'orologio | La giornata riparte da zero; dopo un salto indietro il conteggio si ferma finché l'orologio non supera l'ora precedente | Osservato (22.0.0); *Oggi* dice quando la console non sta contando |

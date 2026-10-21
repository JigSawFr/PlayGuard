<!-- translated from docs/troubleshooting.md, sha256 5b0b1b94198f -->
# Risoluzione dei problemi

Riprendere il controllo di una console bloccata, i problemi comuni, segnalare un bug, e la modalità sviluppatore. Torna al [README](../../README.md).

## Console bloccata?

PlayGuard vede solo il filtro famiglia del sistema su cui gira: **emuMMC e sysMMC hanno ognuno il proprio** (un PIN rimosso su uno è ancora presente sull'altro). Avvialo su ognuno di quelli da sistemare.

### Console usata

Conosci il PIN, ma l'app Filtro famiglia del proprietario precedente è ancora collegata (lo scollegamento non riesce, o un ripristino alle impostazioni di fabbrica chiede il suo account).

*PIN › Scollega l'app Filtro famiglia*, poi, se non vuoi nessun filtro famiglia, *Elimina il filtro famiglia*. Entrambi funzionano offline, su emuMMC come su sysMMC.

### PIN dimenticato

*Mostra il PIN* chiede il PIN stesso, quindi qui non può aiutare. Con il sysmodule di ripristino installato (vedi sotto), metti un file `RESCUE` e riavvia: la schermata di recupero ne imposta allora uno nuovo (non mostra mai quello vecchio). Senza di esso, *Elimina il filtro famiglia* permette di ricominciare da zero (prima viene salvato un backup delle impostazioni; non contiene mai il PIN): con *Proteggi PlayGuard con il PIN* attivo (l'impostazione predefinita, *Prima di una modifica*), inserisci prima la scheda SD in un computer e imposta `"pin_lock"` su `"off"` in `sd:/config/playguard/config.json`.

### Il tempo di gioco blocca tutto (limite di 0 minuti) e il PIN è dimenticato

In questo caso PlayGuard stesso non può avviarsi. Installa il sysmodule di ripristino facoltativo (`playguard-rescue.zip`) **in anticipo**; una volta bloccato, metti un file vuoto `config/playguard/RESCUE` sulla scheda SD e avvia la console — sblocca la console perché PlayGuard possa aprirsi. Vedi [`sysmodule/README.md`](../../sysmodule/README.md) (in inglese).

Il modulo deve seguire la versione di PlayGuard: a ogni aggiornamento di PlayGuard, estrai di nuovo il `playguard-rescue.zip` di quella versione (oppure *Aggiorna* sulla voce *Il sysmodule di ripristino non è aggiornato* della campanella delle notifiche), poi riavvia la console. Un modulo più vecchio lascia un rapporto che PlayGuard non può verificare: la schermata di recupero allora non propone nulla, dice che il modulo è più vecchio, e rifare `RESCUE` dà lo stesso risultato finché non viene aggiornato.

### Menu homebrew bloccato

*Console › Forwarder › Blocca il menu homebrew* impedisce all'album e a un gioco avviato con R di aprire il menu homebrew. Se il forwarder di PlayGuard e il menu protetto sono spariti entrambi (eliminati nelle Impostazioni di sistema, o sull'altro sistema: il blocco è sulla scheda SD, letto sia dal sysMMC sia dall'emuMMC):

- **Da un computer:** elimina `atmosphere/config/override_config.ini` sulla scheda SD (o rimetti `config/playguard/override_config.ini.backup`, il file com'era), poi riavvia la console.
- **Con il sysmodule di ripristino installato** (facoltativo): metti un file vuoto `config/playguard/RESCUE` sulla scheda SD e riavvia; il modulo toglie il blocco all'avvio, e PlayGuard lo segnala al suo avvio successivo. Un aggiornamento di Atmosphère dopo il blocco lo toglie allo stesso modo.

### Console non modificata

PlayGuard non può aiutare: richiede Atmosphère. La procedura della master key dell'assistenza Nintendo è la via ufficiale.

## Problemi comuni

### Il tempo rimanente non scende

*Oggi* indica *La console non sta conteggiando il tempo di gioco in questo momento* quando non si è mosso per circa 90 s. Le cause probabili:

- **L'orologio di rete non è mai stato impostato.** *Console › Orologio di rete › Misura la differenza oraria*, poi impostalo ([orologio di rete](features.md#orologio-di-rete)).
- **L'orologio è stato portato indietro.** Dopo un salto indietro, il conteggio si ferma finché l'orologio non supera l'ora precedente; non c'è niente da fare.
- **Un limite è appena stato scritto.** Avvia un gioco per un minuto, poi controlla di nuovo.
- **Il filtro famiglia è sbloccato temporaneamente.** Il tempo di gioco potrebbe allora non essere contato: *PIN › Riattiva il filtro famiglia*.

### I miei limiti sono cambiati da soli

- **L'app Filtro famiglia è ancora collegata.** La sua prossima sincronizzazione sovrascrive ciò che PlayGuard ha impostato: *PIN › Scollega l'app Filtro famiglia*.
- **Un altro strumento, o un profilo.** *Oggi* mostra un riquadro ambra per i limiti diversi da quelli che PlayGuard ha visto per ultimi, e *Preferenze › Cronologia delle modifiche* elenca ogni modifica vista fuori da PlayGuard. Ⓐ su una di esse rimette il valore precedente.
- **Un profilo programmato** messo in vigore, o rimesso com'era una volta finito il suo periodo (*Tempo di gioco › Profili › Profilo programmato*).

### Il bambino ha giocato oltre il limite

- **La console si limita ad avvisare.** *Tempo di gioco › Quando il tempo è scaduto*: scegli *Sospendi il software*.
- **Il filtro famiglia è rimasto sbloccato.** Finché lo è, il limite di oggi è in pausa: la pillola dell'intestazione lo dice su ogni scheda; *PIN › Riattiva il filtro famiglia*. Anche un riavvio della console lo blocca.
- **L'orologio è stato portato indietro.** Il conteggio si ferma finché l'orologio non supera l'ora precedente ([cosa conta](features.md#cosa-conta-come-tempo-di-gioco)).
- **L'altro sistema.** emuMMC e sysMMC hanno ognuno il proprio filtro famiglia: impostalo su entrambi, o nascondi il menu di avvio di hekate ([compatibilità](compatibility.md#emummc-e-sysmmc)).

### L'intero limite è di nuovo disponibile oggi

Cambiare l'orologio (l'orologio di rete o l'orologio della console) azzera il tempo di gioco di oggi. Imposta l'orologio prima che il bambino giochi, non dopo.

### PlayGuard si apre in sola lettura

La console usa un firmware più recente del più recente testato: PlayGuard controlla se una versione più recente lo supporta e propone di aggiornare ([firmware più recente](compatibility.md#firmware-più-recente)). Una matita barrata a destra del titolo indica che l'app è in sola lettura; toccarla apre la schermata del firmware.

### PlayGuard chiede il PIN di continuo, o mai

*PIN › Proteggi PlayGuard con il PIN*: *Prima di una modifica* (l'impostazione predefinita) lo chiede una volta, prima della prima modifica, e il PIN vale fino alla chiusura di PlayGuard. *Richiedi di nuovo il PIN dopo 5 minuti* lo richiede trascorso quel tempo. *Mostra il PIN* lo chiede sempre.

### Un gioco non si avvia, o mostra un "!" nel menu HOME

*Console › Controlla i giochi installati* dice cosa non va in ognuno e cosa fare; i dati di salvataggio restano sempre ([giochi installati](features.md#giochi-installati)).

### Un forwarder protetto mostra l'errore 2347-0100

Il suo cancello (`config/playguard/gate-….nro`) non è più il file con cui è stato creato il forwarder: è stato modificato o sostituito sulla scheda SD, quindi il forwarder non lo avvia. Apri PlayGuard dal suo forwarder: riscrive il cancello all'avvio; *Console › Forwarder › Reinstalla tutti* rifà ogni forwarder con esso.

### La campanella o una scheda mostra un badge ambra

Qualcosa richiede la tua attenzione: Ⓨ apre l'elenco, e Ⓐ su una voce applica la soluzione o apre dove si applica ([notifiche](features.md#notifiche--ciò-che-richiede-la-tua-attenzione-da-qualsiasi-scheda)).

## Segnalare un bug

Un errore mostra il codice del sistema (per esempio `0x0001088E`) con ciò che se ne sa: cosa fare per i codici il cui significato è documentato, altrimenti un rapporto aiuta.

1. *Informazioni › Aiuto e diagnostica › Esporta un rapporto di diagnostica* salva un file di testo in `sd:/config/playguard/logs/reports/`: firmware, versione di Atmosphère, quanto è durato l'avvio e quanto le schermate lente (la cronologia delle modifiche, *Tutti i giochi*, il controllo dei giochi installati, le schede che si aggiornano da sole), orologi, memoria, stato dell'oscuramento del numero di serie e delle patch dei giochi, il risultato grezzo di ogni richiesta al filtro famiglia, e i registri: come si è conclusa l'ultima sessione, la memoria e lo spazio libero sulla scheda SD, i file di crash più recenti e le ultime 80 righe registrate. **Non contiene mai il PIN né il numero di serie.**
2. [Apri una issue](https://github.com/JigSawFr/PlayGuard/issues/new) e allegalo.

### Inviare un rapporto online

Con la console online, *Informazioni › Aiuto e diagnostica › Invia un rapporto online* (Ⓧ sul rapporto di diagnostica in modalità sviluppatore) lo invia in un gist segreto del tuo account GitHub, dopo aver detto esattamente cosa viene inviato. Un rapporto salvato in precedenza in `logs/reports/` si può inviare allo stesso modo.

- **Cosa viene inviato:** il rapporto e ogni file come file a sé:
  - i rapporti di crash più recenti: gli ultimi 3 di PlayGuard in `logs/crashes/`, e gli ultimi 2 di Atmosphère relativi a un homebrew della settimana passata, in `sd:/atmosphere/crash_reports/` (tra questi può esserci il crash di un altro homebrew: l'elenco mostrato prima dell'invio dice quali);
  - i registri di questa sessione e di quella precedente (`logs/sessions/`; se uno è lungo: il suo inizio e i suoi ultimi 64 KiB);
  - i file di debug: il riferimento del blocco del timer, la fine della registrazione del tempo di gioco mentre *Registra il tempo di gioco* è attivo, le ultime 20 modifiche della cronologia (senza nomi di giochi, utenti o profili, e solo con la data), e le impostazioni di PlayGuard.
- **Accesso:** se non hai effettuato l'accesso, PlayGuard propone prima di accedere; *Account GitHub*, subito sotto una volta effettuato l'accesso, permette di uscire; quando GitHub rifiuta il gist, PlayGuard propone di accedere di nuovo.
- **Dopo:** mostra il link e due codici QR: il rapporto, e il modulo di segnalazione bug con il link, le tue versioni, come è stato avviato PlayGuard (sopra un gioco o dall'Album; aggiungi il menu) e il sistema (sysMMC o emuMMC) già compilati. Ogni link viene conservato in `logs/uploads/` come collegamento Internet (`.url`) che un computer apre nel suo browser.

> [!WARNING]
> Chiunque abbia il link può leggere il rapporto, finché non elimini il gist su GitHub, e una segnalazione di bug aperta con esso è pubblica: la conferma lo dice.

### Cosa conserva PlayGuard sulla scheda SD

Tutto è in `sd:/config/playguard/logs/`, testo semplice che un computer apre:

- `sessions/session_<date>.txt` — **un file per sessione**: tutto ciò che PlayGuard ha registrato (ciò che prima arrivava solo a un PC tramite nxlink), ogni riga con la sua ora e il suo livello (`E` errore, `W` avviso, `I` informazione), e un'ultima riga che dice come si è conclusa la sessione: `=== end: clean exit ===`, oppure `=== end: crash, see logs/crashes/… ===`. Un file smette di crescere a 1 MiB.
- `crashes/crash_<date>.txt` — **un file per crash**, la cui prima riga dice cosa è successo:
  - un'**eccezione della CPU** (un puntatore errato, un'istruzione non definita, uno stack overflow): il tipo di errore e l'indirizzo toccato, il backtrace e i registri. Gli indirizzi sono offset in `playguard.nro` (`playguard+0x1a2b3c`): la mappa del linker della build (l'artefatto `-linker-map` della sua esecuzione CI) indica la funzione. PlayGuard poi termina come prima: la schermata di errore della console, e il rapporto di Atmosphère stesso in `sd:/atmosphere/crash_reports/`;
  - un'**eccezione non gestita**: il suo tipo e il suo messaggio, e il backtrace;
  - una **sessione che non si è conclusa**: rilevata all'avvio successivo, quando il file della sessione precedente non ha la riga finale (un crash che PlayGuard non ha potuto intercettare, una chiusura dal menu HOME mentre era bloccato, o la console spenta), con le ultime 60 righe di quella sessione.

  I file di crash intercettati mentre PlayGuard è in esecuzione terminano con le ultime 80 righe registrate prima del crash.
- `reports/`, `uploads/`, `play_timer/` — i rapporti salvati, i link dei rapporti inviati online, la registrazione del tempo di gioco.

*Informazioni › Aiuto e diagnostica* stabilisce quanti rapporti, file di crash, registri di sessione e link vengono conservati (ogni cartella per conto suo; il più recente resta sempre).

## Modalità sviluppatore (esaminare un nuovo firmware)

Premi Ⓨ sette volte su *Informazioni › Versione*; gli strumenti per sviluppatori compaiono in fondo a *Informazioni › Aiuto e diagnostica*. Aggiunge:

- un interruttore **sola lettura** — quando è attivo, l'app non può modificare nulla: il modo sicuro per esaminare un nuovo firmware (la schermata del firmware lo propone direttamente);
- il rapporto di diagnostica sullo schermo (Ⓨ lo salva, Ⓧ lo invia online), e una scorciatoia per esportarlo dalla scheda Tempo di gioco;
- **Autotest della console**: ciò che PlayGuard controlla da solo su questa console prima di percorrere a mano [la checklist sulla console](../hardware-test.md) (in inglese) — il PIN, le patch di firma, la cartella dei dati, ogni forwarder creato da PlayGuard (nel menu HOME, protetto, visto funzionare attraverso il suo cancello), il cancello sulla scheda SD, ciò che Atmosphère legge del blocco del menu homebrew, le vie di ritorno, il sysmodule di ripristino. Non cambia nulla, e scrive il suo risultato in Markdown in `config/playguard/self-test.md`, da incollare nella pull request;
- **Installa un'altra build** al posto di quella attuale, per provare una correzione prima che venga pubblicata ([sotto](#installare-unaltra-build));
- **Confronta blocco tempo di gioco** (solo mentre l'app Filtro famiglia è collegata e può sincronizzarsi — non con un file hosts che blocca Nintendo; anche in *PIN*), per decifrare le impostazioni che PlayGuard non mostra ancora: salva il blocco grezzo come riferimento, cambia un'impostazione nell'app Filtro famiglia, torna qui — PlayGuard elenca i valori che sono cambiati (salvati in `logs/reports/` su richiesta). Orari diversi di Sogni d'oro per ogni giorno, impostati dall'app Filtro famiglia, potrebbero essere confermati così. Allega quel file a una issue, o invialo con il rapporto (*Invia un rapporto online*).
- **Registra il tempo di gioco**: ogni 30 s mentre PlayGuard è aperto, una riga di ciò che riporta il timer (tempo rimanente, tempo trascorso, il blocco grezzo delle impostazioni…) in `logs/play_timer/log.csv`, un file pronto per un foglio di calcolo, più una riga subito dopo ogni modifica fatta da PlayGuard, che dice quale (la sua ultima colonna, `event`), e una nota quando l'orologio si è spostato più del tempo trascorso. Lasciato aperto oltre la mezzanotte, o fino allo scadere del tempo, mostra ciò che un solo rapporto non può mostrare: quando il tempo trascorso si azzera, cosa dice la console verso la fine. L'interruttore viene ricordato; registra solo in modalità sviluppatore. Ciò che si sa finora è in [parental-controls.md](../parental-controls.md) (in inglese).
- **Svuota la cache** ed **Elimina i file di log**, ognuno dopo una conferma che dice quanti file e quanto spazio: la cache (`cache/` e il `nand/<system>/cache/` di ogni sistema: nomi e icone dei giochi, gli ultimi dati di Attività, l'elenco delle build) viene riletta dalla console quando serve; i file di log sono i file di crash, i registri di sessione (tranne quello di questa sessione, ancora in scrittura), i rapporti salvati e la registrazione del tempo di gioco, mentre `uploads/` (i link per rimuovere i rapporti inviati online) e il riferimento del blocco restano. Impostazioni, cronologia, backup, profili ed esportazioni non vengono mai toccati.
- **Riesegui le migrazioni**: gli spostamenti delle organizzazioni precedenti — la cartella dei dati accanto al `.nro`, i log di prima delle loro sottocartelle, ciò che tutti i sistemi condividevano — ognuno su una riga da spuntare o togliere (tutte spuntate all'inizio; A ne cambia una), poi *Riesegui le N scelte* e una conferma che le elenca con ciò che ognuna sposta. Vengono eseguite subito, qualunque cosa registri `migrations.json` (altrimenti ognuna viene eseguita una sola volta, al primo avvio di una build che ne ha bisogno; il livello registrato non viene mai abbassato); un file già al suo posto non viene mai sostituito. Una finestra dice quanti elementi sono stati spostati; ogni passaggio è nel registro di sessione.
- **Statistiche dei fotogrammi**: una riga al secondo in `logs/frame_stats.txt`, il tempo impiegato per disegnare la schermata e i fotogrammi arrivati in ritardo, per confrontare una schermata con un'altra sulla console. **Alone di sfondo** spegne la luce soffusa dietro le schermate (la modalità indicata nella riga finisce allora con `-noglow`), per misurare quanto costa.

### Installare un'altra build

*Installa un'altra build* mette una build al posto del `.nro` di PlayGuard stesso, per provare una correzione prima che venga pubblicata.

**Cosa elenca**, ogni tipo in una scheda a parte (Ⓛ / Ⓡ, mostrati ai due lati delle schede, passano dall'una all'altra; la schermata si apre su quella della build installata, e ogni scheda conta le build più recenti di quella installata):

- una delle ultime 20 versioni pubblicate (pre-release comprese, etichettate);
- uno degli ultimi 20 commit di `main`;
- la build più recente di una pull request aperta da un branch di questo repository (mai da un fork: il suo codice non è ancora stato revisionato). A sulla scheda delle pull request, quando è mostrata, le ordina nell'altro senso (prima la build più recente, o per numero).

**Ogni build** mostra il suo titolo (l'oggetto di un commit o di una pull request; il messaggio completo di un commit è la sua descrizione) sopra delle etichette: il tipo di modifica (*Novità*, *Correzione*, *Docs*…) e il suo ambito, *Incompatibile*, la pull request da cui proviene un commit, l'esito dei controlli (*CI OK*, *CI fallita*, *CI in corso*), pre-release, *installata* o *nuova*; a destra, quanto è vecchia e il suo commit. Una build apre prima la sua pagina: etichette (il suo tipo, i suoi controlli, installata o nuova), una scheda di dati (compilazione, dimensione, commit, checksum, quando GitHub la elimina), **Installa**, **Vedi su GitHub** (la sua pagina come codice QR per un telefono), poi le note di una versione o la descrizione di una pull request, rese dal Markdown di GitHub (titoli, elenchi, caselle di controllo, codice, citazioni e avvisi, tabelle).

**Lettura sicura:** tutto ciò che viene da GitHub è mostrato come testo semplice: nessun link viene aperto, nessun HTML o script viene eseguito, e i caratteri invisibili che potrebbero camuffare un titolo (inversioni da destra a sinistra, caratteri a larghezza zero, caratteri di controllo, UTF-8 non valido) vengono rimossi.

**Accesso.** Le versioni pubblicate non richiedono nulla; le altre sono gli artefatti del workflow di build, che GitHub dà solo agli utenti che hanno effettuato l'accesso. **Account GitHub** effettua l'accesso con un codice e un codice QR da scansionare con un telefono. Le sole autorizzazioni chieste sono leggere i file del workflow di build (Actions) e creare gist, per *Invia un rapporto online*; per il resto il token può leggere solo ciò che è pubblico. È conservato in `github_token`, mai inviato con un rapporto, e *Account GitHub* permette di uscire. Con l'accesso effettuato, Ⓨ (*Cerca*) accetta il numero di una pull request (aperta o chiusa) o un commit (da 7 a 40 cifre esadecimali) e ne trova la build, solo da un branch di questo repository.

**Installazione:** PlayGuard scarica la build, la verifica (dimensione, lo SHA-256 registrato da GitHub, l'intestazione NRO), la mette al posto del proprio `.nro` e si riavvia con essa (il PIN viene chiesto prima, ogni volta che ne è impostato uno).

- L'elenco viene conservato per 10 minuti e mostrato subito, con la sua età (Ⓧ lo recupera di nuovo); mentre viene recuperato, una barra mostra l'avanzamento.
- Un download interrotto (rete persa) riprende da dove si era fermato la volta successiva che si installa la stessa build.
- Gli artefatti scadono dopo 90 giorni.

**Tornare indietro.** La build sostituita viene conservata (`playguard.nro.previous`): **Torna alla build precedente** nello stesso elenco le scambia di nuovo, dopo il PIN. Se una build installata da qui (o dall'aggiornamento *In PlayGuard*) va in crash prima della sua prima schermata, l'avvio successivo di PlayGuard propone di tornare alla build precedente su una semplice schermata di testo, prima che venga caricata qualsiasi altra cosa (A torna indietro, B la avvia comunque), senza il PIN, perché ad andare in crash potrebbe essere proprio la schermata del PIN. Lo stesso elenco permette di tornare a qualsiasi versione pubblicata in qualsiasi momento.

**Quale build è in esecuzione:** *Informazioni › Versione* mostra il commit in modalità sviluppatore. L'intestazione di ogni schermata mostra la versione in esecuzione accanto al titolo: quella di una versione pubblicata in un'etichetta grigia (`1.2.0`), quella di qualsiasi altra build in un'etichetta ambra seguita dal suo commit, così una build di prova non viene mai scambiata per la versione pubblicata.

<!-- translated from docs/compatibility.md, sha256 1fe10bbb78d4 -->
# Compatibilità

Firmware, Atmosphère e launcher supportati, emuMMC e sysMMC, e come PlayGuard si confronta con un sysmodule sostitutivo. Torna al [README](../../README.md).

## Versioni supportate

**Testato su una console** — PlayGuard è stato usato su:

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira e l'Homebrew App Store

**Dovrebbe funzionare, non testato** — nulla in PlayGuard li esclude, ma nessuno li ha ancora segnalati ([segnalazioni benvenute](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Ogni firmware richiede un Atmosphère che lo avvii: quello indicato o uno più recente.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Stessi comandi della 23.0.1 nella tabella di [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Tutte le funzioni.

**Prima della 21.0.0, tutto tranne il tempo di gioco** — limitazioni, PIN, sblocco temporaneo, attività, orologio di rete, app Filtro famiglia e cronologia funzionano. Il blocco di impostazioni del timer (0x44 byte) ha questa struttura solo dalla 21.0.0, quindi PlayGuard disattiva ciò che ne dipende: i limiti giornalieri, Sogni d'oro, il tempo extra e il blocco della console.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x e precedenti** — Atmosphère 1.0.0 e precedenti

PlayGuard controlla che Atmosphère sia in esecuzione, non quale versione: conta il firmware che avvia. *Console* mostra entrambi.

**Launcher.** È consigliato l'avvio sopra un gioco (title override). L'app indica se gira come applicazione o come applet (album). Sopra un gioco, la console conta il tempo di PlayGuard come tempo di quel gioco. Nell'attività, va all'utente scelto all'avvio: aprilo con l'utente di un genitore, non di un bambino. Il timer è quello della console, lo stesso per tutti gli utenti: conta quel tempo chiunque abbia aperto PlayGuard (a meno che il timer non sia disattivato).

**Nintendo Switch 2.** Non supportata: Atmosphère non esiste per lei.

<a id="firmware-piu-recente"></a>**Firmware più recente?** Oltre il firmware più recente testato su una console, PlayGuard si apre in **sola lettura** e controlla se una versione più recente lo supporta. Se c'è, propone di aggiornare (in PlayGuard stesso, con sphaira o con l'Homebrew App Store). Altrimenti scegli tu: sola lettura, sola lettura con gli strumenti per sviluppatori (per indagare sul firmware), o tutte le funzioni a tuo rischio. La scelta può essere ricordata per quel firmware e quella versione dell'app; *Console › Compatibilità* riapre la schermata.

**emuMMC e sysMMC.** Ognuno ha il proprio filtro famiglia, e PlayGuard vede solo quello del sistema su cui gira. Su emuMMC, *Console* e *Primi passi* lo dicono; anche su sysMMC, quando sulla scheda SD è configurata un'emuMMC. Imposta il filtro famiglia su entrambi, o nascondi il menu di avvio di hekate perché un bambino non possa scegliere l'altro sistema. PlayGuard tiene separati la cronologia, i backup e lo stato di ogni sistema (il sysMMC e l'emuMMC, in `nand/`); le preferenze e i profili sono comuni.

**La console rischia il ban?** PlayGuard non contatta mai Nintendo: online raggiunge solo server dell'ora pubblici e GitHub (aggiornamenti, e i rapporti che invii). A far bannare le console è un firmware personalizzato o un'emuMMC che si collega a Nintendo ([FAQ di switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), in inglese). Nessun ban legato a modifiche del filtro famiglia o dell'orologio è documentato; non è una garanzia.

**Una console senza Internet?** È proprio lo scopo di PlayGuard, e il timer ha comunque bisogno di un orologio di rete che sia stato impostato. Senza un server dell'ora da interrogare, *Console › Orologio di rete › Misura la differenza oraria* propone invece di impostarlo sull'orologio della console: controlla prima l'orologio della console, con *Sincronizza l'orologio tramite Internet* disattivato, e attiva quell'impostazione solo dopo (vedi [Console](features.md#console--lorologio-di-rete-i-giochi-installati-cosa-esegue-la-console)).

**Notifiche sul telefono.** Dalla 22.0.0, una console ancora collegata all'app Filtro famiglia può avvisare il genitore quando il PIN viene inserito sulla console ([switchbrew](https://switchbrew.org/wiki/22.0.0), in inglese). Non si sa se gli sblocchi di PlayGuard lo attivino.

## Perché PlayGuard

I bambini amano i videogiochi, e i videogiochi vogliono dire schermi: un limite fa parte del prendersi cura di loro. Su una Switch non modificata, se ne occupava l'app di Nintendo. Una volta che la console usa un firmware personalizzato, si apre un mondo nuovo — e il filtro famiglia salta, perché l'app non raggiunge più la console. Sapere quanto hanno giocato, impostare un limite, far smettere di giocare senza litigi né l'ennesimo "altri cinque minuti" era diventato difficile.

I pochi strumenti esistenti non facevano al caso: resoconti di attività poco mantenuti e poco dettagliati, un filtro famiglia rudimentale e incompleto. E non volevo nemmeno un sostituto fatto in casa, né un sysmodule sempre in esecuzione in background. L'idea era **riutilizzare il più possibile il filtro famiglia della console stessa**, con tutto ciò che già fa — il limite, il PIN, gli avvisi, la sospensione — e riportare le sue impostazioni sulla console. Questo è PlayGuard.

Gestire il filtro famiglia della console stessa apre anche la porta a molto di più: inviare il tempo di gioco a un server di casa, riceverne ordini, Home Assistant, automazioni. Dove potrebbe portare è nell'[orizzonte della roadmap](../../ROADMAP.md#horizon) (in inglese).

## PlayGuard o un sysmodule sostitutivo?

PlayGuard gestisce il filtro famiglia integrato nella console (il servizio `pctl`), offline. L'altro approccio lo sostituisce con un sysmodule, per esempio [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (un sysmodule e un overlay Tesla / Ultrahand).

- **Limite di tempo di gioco** — PlayGuard: Uno per la console (il timer della console stessa). Un sysmodule sostitutivo: Uno per utente.
- **PIN, avvisi, sospensione** — PlayGuard: Quelli della console. Un sysmodule sostitutivo: I propri.
- **In background** — PlayGuard: Niente (il sysmodule di ripristino facoltativo agisce solo all'avvio della console, quando un file `RESCUE` glielo chiede). Un sysmodule sostitutivo: Un sysmodule, dall'avvio.
- **Dopo un aggiornamento del firmware** — PlayGuard: Il filtro famiglia della console continua a funzionare; PlayGuard si apre in sola lettura finché una versione non supporta il firmware. Un sysmodule sostitutivo: Il controllo dipende dal fatto che il sysmodule funzioni ancora.
- **Eliminato dalla scheda SD** — PlayGuard: I limiti restano sulla console. Un sysmodule sostitutivo: Il controllo sparisce con lui.

**Serve un limite diverso per ogni bambino?** Scegli un sysmodule sostitutivo — il timer della console ha un solo limite per tutta la console. Altrimenti, PlayGuard mantiene il filtro famiglia di Nintendo e porta semplicemente le sue impostazioni sulla console.

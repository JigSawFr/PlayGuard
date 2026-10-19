<!-- translated from docs/install.md, sha256 f12b746bb6b9 -->
# Installazione

Come installare PlayGuard, e cosa scrive sulla scheda SD. Torna al [README](../../README.md).

## Dove trovarlo

A scelta:

- **Homebrew App Store** / **App Store di sphaira** (stesso catalogo) — Cerca *PlayGuard* una volta approvata la scheda. Le nuove versioni vengono riprese automaticamente.
- **sphaira › GitHub** — Lo zip della versione contiene già la voce (`/config/sphaira/github/playguard.json`): dopo una prima installazione, aggiorna da *GitHub* in sphaira.
- **Manualmente** — Scarica `playguard.zip` dall'[ultima versione](https://github.com/JigSawFr/PlayGuard/releases/latest) ed estrailo nella **radice** della scheda SD. L'app finisce in `sd:/switch/playguard/`, i suoi file in `sd:/config/playguard/`.

Il **sysmodule di ripristino** facoltativo (`playguard-rescue.zip`) si scarica a parte — vedi [Console bloccata?](troubleshooting.md#console-bloccata).

## File sulla scheda SD

Tutti in `sd:/config/playguard/` (l'app stessa resta in `sd:/switch/playguard/`: rimuoverla o reinstallarla li conserva). Le versioni che li tenevano accanto all'app li hanno spostati lì al primo avvio.

- **`config.json`** — Preferenze (lingua, tema, server NTP, *Proteggi PlayGuard con il PIN*…); ogni chiave in [config.md](../config.md) (in inglese). Se non si può leggere, viene conservato come `config.json.bad` (il precedente come `.bad.1`) e le impostazioni ripartono dai valori predefiniti; PlayGuard lo segnala una volta all'avvio
- **`profiles/`** — I profili di limiti del tempo di gioco salvati
- **`exports/`** — Le esportazioni dell'attività
- **`nand/<system>/`** — Ciò che appartiene a un sistema, ognuno con il proprio filtro famiglia: `sysmmc/` per il sysMMC, `emummc/` per l'emuMMC. Dentro: `history.json` la cronologia delle modifiche (le 500 più recenti, comprese quelle rifiutate dalla console; se non si può leggere, viene conservata come `history.json.bad`, la precedente come `.bad.1`, invece di essere sovrascritta, e PlayGuard lo segnala una volta); `state.json` ciò che *Blocca tutti i giochi*, il tempo extra, un nuovo blocco in sospeso e la scelta del firmware devono ricordare ([config.md](../config.md#state-kept-between-launches), in inglese); `watch.json` ciò che PlayGuard ha visto per ultimo della console, per segnalare cosa è cambiato fuori da lui ([config.md](../config.md#watchjson), in inglese; eliminarlo non fa danni); `backups/` i backup delle impostazioni (mai il PIN); `cache/` l'ultima attività di gioco letta (tutti gli utenti, e ogni utente consultato), mostrata subito all'avvio successivo mentre il registro viene riletto. I file di una versione precedente a queste cartelle vanno al primo sistema su cui PlayGuard si avvia
- **`cache/`** — Le icone dei giochi (`icons/`, 16 MB al massimo, svuotata quando cambia la lingua della console), comuni a tutti i sistemi, per non rileggere nome e icona di ogni gioco a ogni avvio; le note di rilascio di Atmosphère, hekate e del firmware (`sys_notes.json`, per *Console*) e quelle di PlayGuard (per *Informazioni*); in modalità sviluppatore, l'elenco di *Installa un'altra build* (`dev_builds.json`)
- **`github_token`** — L'accesso a GitHub di *Invia un rapporto online* e, in modalità sviluppatore, di *Installa un'altra build*; eliminato uscendo dall'account (*Informazioni › Aiuto e diagnostica › Account GitHub*, mostrato una volta effettuato l'accesso)
- **`previous_build.json`**, **`boot_check.json`** — Dopo l'installazione di una build al posto di quella attuale: il nome della build sostituita (conservata accanto al `.nro` come `playguard.nro.previous`), e il controllo dell'avvio successivo (eliminato appena compare la sua prima schermata)
- **`rescue_report.txt`** — Lasciato dal sysmodule di ripristino dopo il suo intervento, finché PlayGuard non lo mostra all'avvio (se non si può leggere viene rimosso, e PlayGuard lo segnala)
- **`rescue_nonce`** — Una sfida casuale a cui il sysmodule di ripristino risponde nel suo rapporto, perché PlayGuard distingua un rapporto scritto da lui da uno scritto a mano ([sysmodule/README.md](../../sysmodule/README.md), in inglese); uno nuovo a ogni avvio
- **`logs/`** — `reports/` i rapporti di diagnostica salvati (mai il PIN né il numero di serie); `crashes/` un file per crash (ciò che ha fermato PlayGuard); `uploads/` un collegamento Internet (`.url`) per ogni rapporto inviato online; `play_timer/` i file del tempo di gioco degli strumenti per sviluppatori. Quanti rapporti, file di crash e link vengono conservati: *Informazioni › Aiuto e diagnostica* (per numero, età o dimensione; il più recente resta sempre)

Maggiori dettagli in [packaging/README.md](../../packaging/README.md) (in inglese).

<!-- translated from docs/getting-started.md, sha256 395c2eb35838 -->
# Per iniziare

Dalla scheda SD a una console configurata per tuo figlio, in una decina di minuti, poi le attività di tutti i giorni. Torna al [README](../../README.md).

## Prima di cominciare

- **Una console con Atmosphère.** PlayGuard gestisce il filtro famiglia della console stessa tramite un servizio riservato: richiede un firmware personalizzato. Console non modificata? Non può aiutare ([risoluzione dei problemi](troubleshooting.md#console-non-modificata)).
- **Un firmware supportato.** Dalla 21.0.0 alla 23.0.1 per tutte le funzioni; con i precedenti, tutto tranne il tempo di gioco; uno più recente si apre in sola lettura finché una versione non lo supporta ([compatibilità](compatibility.md)).
- **Un solo limite per tutta la console.** Il timer è quello della console: è lo stesso per tutti gli utenti. Per limiti diversi per ogni bambino serve un altro strumento ([perché](compatibility.md#playguard-o-un-sysmodule-sostitutivo)).

Vuoi dare prima un'occhiata? [Prova PlayGuard nel browser](https://playguard.link/simulator/?not_set_up=1), su una console simulata al suo primo avvio.

## 1. Installalo

Estrai `playguard.zip` dall'[ultima versione](https://github.com/JigSawFr/PlayGuard/releases/latest) nella radice della scheda SD ([installazione](install.md)). PlayGuard non è ancora nell'Homebrew App Store né in sphaira: la sua scheda è ancora in attesa di approvazione.

> [!TIP]
> Estrai anche `playguard-rescue.zip`, il sysmodule di ripristino facoltativo. Non fa nulla finché non ti serve, ed è l'unico modo per rientrare se un limite di 0 minuti blocca tutto e il PIN è dimenticato ([Console bloccata?](troubleshooting.md#console-bloccata)).

## 2. Aprilo nel modo giusto

Apri PlayGuard **sopra un gioco** (tieni premuto R mentre avvii un gioco, con hbmenu, sphaira o l'Homebrew App Store), con **l'utente di un genitore**:

- sopra un gioco, PlayGuard gira come applicazione, con tutta la sua memoria; dall'album gira come applet, con meno memoria;
- la console conta il tempo passato in PlayGuard come tempo di quel gioco, e lo attribuisce all'utente scelto all'avvio: quello di un genitore, non di un bambino.

## 3. Segui *Primi passi*

Al primo avvio compare la guida *Primi passi* (in seguito: *Preferenze › Primi passi*). *Oggi* mostra gli stessi passaggi come riquadro finché non sono fatti (*1 di 3*):

1. **Il PIN.** Il PIN della console stessa, chiesto dal menu HOME per avviare un gioco oltre il limite o per cambiare le impostazioni. Evita 0000, 1234 o 2580: PlayGuard segnala un PIN che chiunque proverebbe per primo.
2. **Un limite giornaliero.** Lo stesso ogni giorno, oppure uno per i giorni feriali e uno per il weekend. Cambia un singolo giorno più tardi in *Tempo di gioco*.
3. **L'orologio di rete.** Il timer conta con esso. Una console che non raggiunge mai i server di Nintendo lo mantiene impreciso: vedi il passaggio 5.

## 4. Scollega l'app Filtro famiglia

Ancora collegata all'app *Filtro famiglia* di Nintendo (o una console usata collegata a quella di qualcun altro)? *Sicurezza › Scollega l'app Filtro famiglia*. Altrimenti la prossima sincronizzazione dell'app sovrascrive ciò che imposti in PlayGuard. Funziona offline.

## 5. Imposta l'orologio di rete

*Console › Orologio di rete › Misura la differenza oraria* interroga tre server dell'ora pubblici, poi *Imposta l'orologio di rete*. Impostare l'orologio azzera il tempo di gioco di oggi, quindi fallo prima che il bambino giochi.

Una console senza Internet: quando nessun server risponde, PlayGuard propone invece di impostare l'orologio di rete sull'orologio della console. Controlla prima l'orologio della console con un telefono o un orologio, con *Impostazioni di sistema › Sistema › Data e ora › Sincronizza l'orologio tramite Internet* disattivato ([dettagli](features.md#orologio-di-rete)).

## 6. Scegli cosa succede quando il tempo è scaduto

*Tempo di gioco › Quando il tempo è scaduto*: la console mostra "Tempo scaduto", poi **sospende il gioco** oppure **si limita ad avvisare**. La sospensione ferma il gioco senza litigi, ma i progressi non salvati possono andare persi; l'avviso lascia la decisione al bambino. Aggiungi **Sogni d'oro** nella stessa scheda, se lo vuoi.

## 7. Proteggi PlayGuard stesso

*Sicurezza › Proteggi PlayGuard con il PIN*:

- **Prima di una modifica** (l'impostazione predefinita) — tutti possono consultare, solo il genitore modifica;
- **Per aprire PlayGuard** — una schermata di blocco all'avvio;
- **Mai** — PlayGuard lo segnala nella campanella finché è impostato un PIN.

## 8. Consegna la console

*Oggi › Mostra al bambino* mostra solo il tempo rimanente e quando finisce il gioco, abbastanza grande da leggerlo dall'altra parte della stanza. Ⓐ e il PIN riportano le schermate del genitore.

> [!IMPORTANT]
> **Un gioco lasciato aperto nel menu HOME continua a contare.** Chiudilo, o metti la console in modalità standby ([cosa conta](features.md#cosa-conta-come-tempo-di-gioco)).

## Attività di tutti i giorni

| Per… | Dove |
|---|---|
| Dare più tempo oggi | *Oggi › Tempo extra* (+15 min, +30 min, +1 h). Torna com'era da solo il giorno dopo. |
| Dare più tempo in un altro giorno | *Oggi › Tempo extra › Un altro giorno…* |
| Fermare il gioco per oggi | *Oggi › Basta per oggi*: adesso, o tra 5 minuti per salvare. |
| Cambiare il limite di un giorno | *Tempo di gioco*: Ⓐ sul giorno, ↑/↓, poi **+** per salvare. |
| Passare dal periodo scolastico alle vacanze | *Tempo di gioco › Profili*; *Profilo programmato* per un periodo scelto in anticipo. |
| Vedere chi ha giocato a cosa | *Attività*: per gioco, per utente, oggi, 7 giorni, da sempre; esportazione o un codice QR. |
| Bloccare tutti i giochi per ora | *Tempo di gioco › Blocca tutti i giochi*. Una volta disattivato, i limiti precedenti tornano. |
| Annullare una modifica | **ZR** *Annulla* subito dopo, o *Cronologia › Modifiche fatte da PlayGuard* in qualsiasi momento. |
| Conservare una copia delle impostazioni | *Cronologia › Esegui il backup delle impostazioni* (mai il PIN). |
| Sapere cosa richiede la tua attenzione | La campanella (Ⓨ): ciò che va fatto in ambra, ciò che è bene sapere con una "i". |

## E poi

- Ogni scheda e impostazione: [funzioni](features.md).
- Qualcosa non funziona come previsto: [risoluzione dei problemi](troubleshooting.md#problemi-comuni).
- Cosa fa la console, e con quale certezza lo sappiamo: [come funziona il filtro famiglia](../parental-controls.md) (in inglese).

<!-- translated from docs/install.md, sha256 7d2be6a20dff -->
# Installation

Comment installer, mettre à jour et supprimer PlayGuard, et ce qu'il écrit sur la carte SD. Une fois installé : [prise en main](getting-started.md). Retour au [README](../../README.md).

## Où le trouver

> [!WARNING]
> PlayGuard n'est **pas encore dans le Homebrew App Store (ni celui de sphaira)** : sa fiche attend toujours d'être validée. Installez-le à la main pour le moment.

Au choix :

- **Manuellement** (recommandé pour le moment) — Téléchargez `playguard.zip` dans la [dernière version](https://github.com/JigSawFr/PlayGuard/releases/latest) et extrayez-le à la **racine** de la carte SD. L'application arrive dans `sd:/switch/playguard/`, ses fichiers dans `sd:/config/playguard/`.
- **sphaira › GitHub** — Le zip de la version contient déjà l'entrée (`/config/sphaira/github/playguard.json`) : après une première installation, mettez à jour depuis *GitHub* dans sphaira.
- **Homebrew App Store** / **App Store de sphaira** (même catalogue) — **Pas encore référencé.** Une fois la fiche validée, cherchez *PlayGuard*. Les nouvelles versions sont alors reprises automatiquement, après une vérification : souvent quelques jours après GitHub. PlayGuard sait qu'il a été installé par là et recommande de se mettre à jour par le store.

Le **sysmodule de récupération** optionnel (`playguard-rescue.zip`) se télécharge à part : extrayez-le lui aussi à la racine de la carte SD, puis redémarrez la console. Il arrive dans `atmosphere/contents/4200000000505247/` et n'agit que quand un fichier `RESCUE` le demande — voir [Console bloquée ?](troubleshooting.md#console-bloquée-).

Ouvrez ensuite PlayGuard **par-dessus un jeu**, avec le compte d'un parent ([pourquoi](compatibility.md#lanceurs)).

## Mettre à jour

- **Dans PlayGuard :** *Préférences › Mettre à jour avec* choisit comment — dans PlayGuard même (il télécharge la version, la vérifie, se remplace et redémarre), sphaira, le Homebrew App Store ou à la main. Avec *Rechercher les mises à jour chaque jour* activé, PlayGuard s'ouvre sur la nouvelle version quand il y en a une.
- **Le sysmodule de récupération** suit la version de PlayGuard : après chaque mise à jour, extrayez à nouveau le `playguard-rescue.zip` de cette version (ou *Mettre à jour* sur l'entrée *Le sysmodule de récupération n'est pas à jour* de la cloche), puis redémarrez la console.
- Les réglages, l'historique, les profils et les sauvegardes restent : ils sont dans `sd:/config/playguard/`, pas à côté de l'application.

## Le supprimer

Supprimez `sd:/switch/playguard/` (l'application) et, pour retirer aussi ses fichiers, `sd:/config/playguard/` ; avec le sysmodule de récupération, aussi `sd:/atmosphere/contents/4200000000505247/`. Les limites, le code PIN et les restrictions sont ceux de la console : ils **restent sur la console**. Pour les retirer aussi, passez d'abord par *Sécurité › Supprimer tout le contrôle parental*.

## Fichiers sur la carte SD

Tous dans `sd:/config/playguard/` (l'application elle-même reste dans `sd:/switch/playguard/` : la supprimer ou la réinstaller les conserve). Les versions qui les gardaient à côté de l'application les y ont déplacés à leur premier lancement.

- **`config.json`** — Préférences (langue, thème, serveur NTP, *Protéger PlayGuard par le code PIN*…) ; chaque clé dans [config.md](../config.md) (en anglais). S'il est illisible, il est gardé en `config.json.bad` (le précédent en `.bad.1`) et les réglages repartent de leurs valeurs par défaut ; PlayGuard le signale une fois au démarrage
- **`profiles/`** — Profils de limites enregistrés
- **`exports/`** — Exports de l'activité
- **`nand/<système>/`** — Ce qui appartient à un système, chacun ayant son propre contrôle parental : `sysmmc/` pour le sysMMC, `emummc/` pour l'emuMMC. Dedans : `history.json` l'historique des modifications (les 500 dernières, y compris celles que la console a refusées ; s'il est illisible, il est gardé en `history.json.bad`, le précédent en `.bad.1`, plutôt qu'écrasé, et PlayGuard le signale une fois) ; `state.json` ce que *Bloquer tous les jeux*, le temps en plus, un reverrouillage en attente et le choix de firmware doivent retenir ([config.md](../config.md#state-kept-between-launches), en anglais) ; `watch.json` ce que PlayGuard a vu en dernier de la console, pour signaler ce qui a changé en dehors de lui ([config.md](../config.md#watchjson) ; le supprimer est sans danger) ; `backups/` les sauvegardes des réglages (jamais le code PIN) ; `cache/` la dernière activité de jeu lue (tous les comptes, et chaque compte consulté), affichée dès le lancement suivant pendant que le journal est relu. Les fichiers d'une version d'avant ces dossiers vont au premier système sur lequel PlayGuard démarre
- **`cache/`** — Les icônes des jeux (`icons/`, 16 Mo au plus, vidé quand la langue de la console change), communes à tous les systèmes, pour ne pas relire le nom et l'icône de chaque jeu à chaque lancement ; les notes de version d'Atmosphère, de hekate, de sys-patch et du firmware (`sys_notes.json`, pour *Console*) et celles de PlayGuard (pour *À propos*) ; en mode développeur, la liste d'*Installer un autre build* (`dev_builds.json`)
- **`github_token`** — La connexion GitHub d'*Envoyer un rapport en ligne* et, en mode développeur, d'*Installer un autre build* ; supprimé à la déconnexion (*À propos › Aide et diagnostic › Compte GitHub*, affiché une fois connecté)
- **`previous_build.json`**, **`boot_check.json`** — Après l'installation d'un build sur place : le nom du build remplacé (gardé à côté du `.nro` sous `playguard.nro.previous`), et le contrôle du lancement suivant (supprimé dès que son premier écran s'affiche)
- **`rescue_report.txt`** — Laissé par le sysmodule de récupération après son intervention, jusqu'à ce que PlayGuard l'affiche au démarrage (illisible, il est supprimé et PlayGuard le signale)
- **`rescue_nonce`** — Un défi aléatoire auquel le sysmodule de récupération répond dans son rapport, pour que PlayGuard distingue un rapport qu'il a écrit d'un rapport écrit à la main ([sysmodule/README.md](../../sysmodule/README.md)) ; un nouveau à chaque lancement
- **`logs/`** — `reports/` les rapports de diagnostic enregistrés (jamais le code PIN ni le numéro de série) ; `crashes/` un fichier par plantage (ce qui a arrêté PlayGuard) ; `uploads/` un raccourci Internet (`.url`) par rapport envoyé en ligne ; `play_timer/` les fichiers du minuteur des outils développeur. Combien de rapports, de fichiers de plantage et de liens sont gardés : *À propos › Aide et diagnostic* (par nombre, âge ou taille ; le plus récent reste toujours)

Plus de détails dans [packaging/README.md](../../packaging/README.md) (en anglais).

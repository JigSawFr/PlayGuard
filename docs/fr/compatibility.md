<!-- translated from docs/compatibility.md, sha256 f5648921d1b6 -->
# Compatibilité

Firmwares, Atmosphère et lanceurs pris en charge, emuMMC et sysMMC, et la comparaison avec un sysmodule de remplacement. Retour au [README](../../README.md).

## Versions prises en charge

**Testé sur console** — PlayGuard a tourné sur :

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira et le Homebrew App Store

**Devrait fonctionner, non testé** — rien dans PlayGuard ne l'empêche, mais personne ne l'a encore signalé ([retours bienvenus](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Chaque firmware demande un Atmosphère qui le démarre : celui indiqué ou un plus récent.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Mêmes commandes que 23.0.1 dans la table de [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Toutes les fonctions.

**Avant 21.0.0, tout sauf le temps de jeu** — restrictions, code PIN, déverrouillage temporaire, activité, horloge réseau, application mobile et historique fonctionnent. Le bloc de réglages du minuteur (0x44 octets) n'a cette structure que depuis 21.0.0, donc PlayGuard désactive ce qui en dépend : les limites quotidiennes, l'alarme du coucher, le temps en plus et *Bloquer tous les jeux*.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x et antérieur** — Atmosphère 1.0.0 et antérieur

PlayGuard vérifie qu'Atmosphère tourne, pas sa version : c'est le firmware qu'il démarre qui compte. *Console* affiche les deux.

## À savoir

### Lanceurs

Le lancement par-dessus un jeu (title override) est recommandé. L'application indique si elle tourne en application (par-dessus un jeu, ou depuis son propre forwarder sur le menu HOME) ou en applet (album). Par-dessus un jeu, la console compte le temps passé dans PlayGuard comme celui de ce jeu. Dans l'activité, il va au compte choisi au lancement : ouvrez-le avec le compte d'un parent, pas celui d'un enfant. Le minuteur est celui de la console, le même pour tous les comptes : il compte ce temps quel que soit le compte, sauf minuteur désactivé ou contrôle parental mis en pause par le code PIN ([détails](features.md#le-code-pin-met-le-contrôle-parental-en-pause-pendant-votre-visite)).

### Nintendo Switch 2

Non prise en charge : Atmosphère n'existe pas pour elle.

### Firmware plus récent

Au-delà du plus récent firmware testé sur console, PlayGuard s'ouvre **en lecture seule** et vérifie si une version plus récente le prend en charge. Si c'est le cas, il propose de mettre à jour (dans PlayGuard même, avec sphaira ou le Homebrew App Store). Sinon, vous choisissez : lecture seule, lecture seule avec les outils développeur (pour diagnostiquer le firmware), ou toutes les fonctions à vos risques. Le choix peut être mémorisé pour ce firmware et cette version de l'application ; *Console › Compatibilité* rouvre l'écran.

### emuMMC et sysMMC

Chacune a son propre contrôle parental, et PlayGuard ne voit que celui du système sur lequel il tourne. En emuMMC, *Console* et *Premiers pas* le signalent ; en sysMMC aussi, quand une emuMMC est configurée sur la carte SD. Réglez le contrôle parental sur les deux, ou masquez le menu de démarrage d'hekate pour qu'un enfant ne puisse pas choisir l'autre système. PlayGuard garde à part l'historique, les sauvegardes et l'état de chaque système (le sysMMC et l'emuMMC, dans `nand/`) ; les préférences et les profils sont communs.

### Risque de bannissement ?

PlayGuard ne contacte jamais Nintendo : en ligne, il ne joint que des serveurs de temps publics, GitHub (mises à jour, et les rapports que vous envoyez). Ce qui fait bannir une console, c'est un firmware modifié ou une emuMMC qui se connecte à Nintendo ([FAQ switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), en anglais). Aucun bannissement lié à un changement du contrôle parental ou de l'horloge n'est documenté ; ce n'est pas une garantie.

### Une console sans Internet

C'est la cible de PlayGuard, et le minuteur a quand même besoin d'une horloge réseau réglée. Sans serveur de temps à interroger, *Console › Horloge réseau › Mesurer l'écart* propose de la régler sur l'horloge de la console à la place : vérifiez d'abord l'horloge de la console, avec *Synchroniser l'horloge via Internet* désactivé, et n'activez ce réglage qu'ensuite (voir [horloge réseau](features.md#aucun-serveur-ne-répond)).

### Notifications sur le téléphone

Depuis 22.0.0, une console encore associée à l'application mobile peut prévenir le parent quand le code PIN est saisi sur la console ([switchbrew](https://switchbrew.org/wiki/22.0.0), en anglais). On ne sait pas si les déverrouillages de PlayGuard la déclenchent.

## Pourquoi PlayGuard

Les enfants aiment les jeux vidéo, et les jeux vidéo, ce sont des écrans : les limiter fait partie de prendre soin d'eux. Sur une Switch d'origine, l'application mobile de Nintendo s'en chargeait. En passant la console sous firmware personnalisé, le champ des possibles s'ouvre — mais le contrôle parental saute, car l'application mobile n'atteint plus la console. Savoir combien de temps ils ont joué, fixer une limite, faire arrêter le jeu sans bataille ni éternel « encore cinq minutes » devenait compliqué.

Les rares outils existants ne faisaient pas l'affaire : un suivi d'activité peu maintenu et peu détaillé, un contrôle parental peu développé et inabouti. Je ne voulais pas non plus d'un remplacement maison, ni d'un sysmodule qui tourne en permanence en arrière-plan. L'objectif : **réutiliser au maximum le contrôle parental de la console**, avec tout ce qu'il sait déjà faire — la limite, le code PIN, les avertissements, la suspension — et ramener ses réglages sur la console. C'est PlayGuard.

Piloter le contrôle parental de la console ouvre aussi la porte à bien plus : remonter le temps de jeu vers un serveur à la maison, recevoir des ordres, Home Assistant, des automatisations. La première étape est là, un lien facultatif avec Home Assistant par MQTT ([guide](../home-assistant.fr.md)) ; la suite possible est dans [l'horizon de la feuille de route](../../ROADMAP.md#horizon) (en anglais).

## PlayGuard ou un sysmodule de remplacement ?

PlayGuard pilote le contrôle parental intégré à la console (le service `pctl`), hors ligne. L'autre approche le remplace par un sysmodule, par exemple [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (un sysmodule et un overlay Tesla / Ultrahand).

| | PlayGuard | Un sysmodule de remplacement |
|---|---|---|
| **Limite de temps de jeu** | Une pour la console (le minuteur de la console) | Une par compte utilisateur |
| **Code PIN, avertissements, suspension** | Ceux de la console | Les siens |
| **En arrière-plan** | Rien (le sysmodule de récupération optionnel n'agit qu'au démarrage, quand un fichier `RESCUE` le demande ; l'agent du lien Home Assistant, facultatif lui aussi, ne fonctionne qu'une fois installé) | Un sysmodule, dès le démarrage |
| **Après une mise à jour du firmware** | Le contrôle de la console continue de fonctionner ; PlayGuard s'ouvre en lecture seule jusqu'à une version qui prend le firmware en charge | Le contrôle dépend du bon fonctionnement du sysmodule |
| **Supprimé de la carte SD** | Les limites restent sur la console | Le contrôle disparaît avec lui |

**Besoin d'une limite différente pour chaque enfant ?** Choisissez un sysmodule de remplacement — le minuteur de la console n'a qu'une limite pour toute la console. Sinon, PlayGuard garde le contrôle parental de Nintendo et ramène simplement ses réglages sur la console.

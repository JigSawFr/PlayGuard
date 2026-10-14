# Compatibilité

Firmwares, Atmosphère et lanceurs pris en charge, emuMMC et sysMMC, et la comparaison avec un sysmodule de remplacement. Retour au [README](../../README.fr.md).

## Versions prises en charge

| | Pris en charge | Remarques |
|---|---|---|
| **Firmware** | **21.0.0 → 22.5.0** | La structure de la limite de temps de jeu (0x44 octets) existe depuis 21.0.0 ; en dessous, tous les onglets fonctionnent sauf le temps de jeu. 23.x passe par l'écran [firmware plus récent](#firmware-plus-recent) tant qu'il n'a pas été testé sur console. |
| **Atmosphère** | **1.11.x → 1.12.0** | 1.12.0 ajoute 23.0.0. L'application affiche la version détectée. |
| **Lanceurs** | hbmenu, **sphaira**, **Homebrew App Store** | Le lancement par-dessus un jeu (title override) est recommandé. L'application indique si elle tourne en application ou en applet (album). Par-dessus un jeu, la console compte le temps passé dans PlayGuard comme celui de ce jeu. Dans l'activité, il va au compte choisi au lancement : ouvrez-le avec le compte d'un parent, pas celui d'un enfant. Le minuteur est celui de la console, le même pour tous les comptes : il compte ce temps quel que soit le compte (sauf minuteur désactivé). |
| **Testé sur console** | 22.0.0, 22.1.0, 22.5.0 | 23.0.1 / 1.12.0 : la table des commandes ([switchbrew](https://switchbrew.org/wiki/Parental_Control_services)) a les mêmes commandes, mais rien n'a encore tourné sur console, donc PlayGuard demande d'abord (lecture seule tant que vous n'avez pas choisi) — vos retours sont bienvenus. |
| **Nintendo Switch 2** | Non prise en charge | Atmosphère n'existe pas pour elle. |

<a id="firmware-plus-recent"></a>**Firmware plus récent ?** Au-delà du plus récent firmware testé sur console, PlayGuard s'ouvre **en lecture seule** et vérifie si une version plus récente le prend en charge. Si c'est le cas, il propose de mettre à jour (dans PlayGuard même, avec sphaira ou le Homebrew App Store). Sinon, vous choisissez : lecture seule, lecture seule avec les outils développeur (pour diagnostiquer le firmware), ou toutes les fonctions à vos risques. Le choix peut être mémorisé pour ce firmware et cette version de l'application ; *Outils › Compatibilité* rouvre l'écran.

**emuMMC et sysMMC.** Chacune a son propre contrôle parental, et PlayGuard ne voit que celui du système sur lequel il tourne. En emuMMC, *Outils* et *Premiers pas* le signalent ; en sysMMC aussi, quand une emuMMC est configurée sur la carte SD. Réglez le contrôle parental sur les deux, ou masquez le menu de démarrage d'hekate pour qu'un enfant ne puisse pas choisir l'autre système. PlayGuard garde à part l'historique, les sauvegardes et l'état de chaque système (le sysMMC et l'emuMMC, dans `nand/`) ; les préférences et les profils sont communs.

**Risque de bannissement ?** PlayGuard ne contacte jamais Nintendo : en ligne, il ne joint que des serveurs de temps publics, GitHub (mises à jour, et les rapports que vous envoyez). Ce qui fait bannir une console, c'est un firmware modifié ou une emuMMC qui se connecte à Nintendo ([FAQ switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), en anglais). Aucun bannissement lié à un changement du contrôle parental ou de l'horloge n'est documenté ; ce n'est pas une garantie.

**Une console sans Internet ?** C'est la cible de PlayGuard, et le minuteur a quand même besoin d'une horloge réseau réglée. Sans serveur de temps à interroger, *Horloge réseau › Mesurer l'écart* propose de la régler sur l'horloge de la console à la place : vérifiez d'abord l'horloge de la console, avec *Synchroniser l'horloge via Internet* désactivé, et n'activez ce réglage qu'ensuite (voir [Horloge réseau](features.md#horloge-réseau--lhorloge-dont-dépend-le-minuteur)).

**Notifications sur le téléphone.** Depuis 22.0.0, une console encore associée à l'application mobile peut prévenir le parent quand le code PIN est saisi sur la console ([switchbrew](https://switchbrew.org/wiki/22.0.0), en anglais). On ne sait pas si les déverrouillages de PlayGuard la déclenchent.

## PlayGuard ou un sysmodule de remplacement ?

PlayGuard pilote le contrôle parental intégré à la console (le service `pctl`), hors ligne. L'autre approche le remplace par un sysmodule, par exemple [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (un sysmodule et un overlay Tesla / Ultrahand).

| | PlayGuard | Un sysmodule de remplacement |
|---|---|---|
| Limite de temps de jeu | Une pour la console (le minuteur de la console) | Une par compte utilisateur |
| Code PIN, avertissements, suspension | Ceux de la console | Les siens |
| En arrière-plan | Rien (le module de récupération optionnel n'agit qu'au démarrage, quand un fichier `RESCUE` le demande) | Un sysmodule, dès le démarrage |
| Après une mise à jour du firmware | Le contrôle de la console continue de fonctionner ; PlayGuard s'ouvre en lecture seule jusqu'à une version qui prend le firmware en charge | Le contrôle dépend du bon fonctionnement du sysmodule |
| Supprimé de la carte SD | Les limites restent sur la console | Le contrôle disparaît avec lui |

**Besoin d'une limite différente pour chaque enfant ?** Choisissez un sysmodule de remplacement — le minuteur de la console n'a qu'une limite pour toute la console. Sinon, PlayGuard garde le contrôle parental de Nintendo et ramène simplement ses réglages sur la console.

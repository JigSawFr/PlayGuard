<!-- translated from docs/getting-started.md, sha256 395c2eb35838 -->
# Prise en main

De la carte SD à une console réglée pour votre enfant, en une dizaine de minutes, puis les tâches du quotidien. Retour au [README](../../README.md).

## Avant de commencer

- **Une console sous Atmosphère.** PlayGuard pilote le contrôle parental de la console elle-même par un service restreint : il lui faut un firmware personnalisé. Console non modifiée ? Il ne peut rien faire ([dépannage](troubleshooting.md#console-non-modifiée)).
- **Un firmware pris en charge.** De 21.0.0 à 23.0.1 pour toutes les fonctions ; les plus anciens, tout sauf le minuteur ; un plus récent s'ouvre en lecture seule jusqu'à ce qu'une version le prenne en charge ([compatibilité](compatibility.md)).
- **Une seule limite pour toute la console.** Le minuteur est celui de la console : il est le même pour tous les comptes. Des limites différentes pour chaque enfant demandent un autre outil ([pourquoi](compatibility.md#playguard-ou-un-sysmodule-de-remplacement-)).

Envie de faire le tour d'abord ? [Essayez PlayGuard dans votre navigateur](https://playguard.link/simulator/?not_set_up=1), face à une console simulée à son premier lancement.

## 1. L'installer

Extrayez `playguard.zip` de la [dernière version](https://github.com/JigSawFr/PlayGuard/releases/latest) à la racine de la carte SD ([installation](install.md)). PlayGuard n'est pas encore dans le Homebrew App Store ni sphaira : sa fiche attend toujours d'être validée.

> [!TIP]
> Extrayez aussi `playguard-rescue.zip`, le sysmodule de récupération optionnel. Il ne fait rien tant que vous n'en avez pas besoin, et c'est le seul moyen de reprendre la main si une limite à 0 minute bloque tout et que le code PIN est oublié ([Console bloquée ?](troubleshooting.md#console-bloquée-)).

## 2. L'ouvrir comme il faut

Ouvrez PlayGuard **par-dessus un jeu** (maintenez R en lançant un jeu, avec hbmenu, sphaira ou le Homebrew App Store), avec **le compte d'un parent** :

- par-dessus un jeu, PlayGuard tourne en application, avec toute sa mémoire ; depuis l'album, il tourne en applet, avec moins ;
- la console compte le temps passé dans PlayGuard comme celui de ce jeu, et l'attribue au compte choisi au lancement : celui d'un parent, pas d'un enfant.

## 3. Suivre *Premiers pas*

Au premier lancement, le guide *Premiers pas* s'ouvre (plus tard : *Préférences › Premiers pas*). *Aujourd'hui* montre les mêmes étapes dans une carte tant qu'elles ne sont pas faites (*1 sur 3*) :

1. **Le code PIN.** Celui de la console elle-même, que le menu HOME demande pour lancer un jeu au-delà de la limite ou pour changer les réglages. Évitez 0000, 1234 ou 2580 : PlayGuard signale un code PIN que n'importe qui essaierait en premier.
2. **Une limite quotidienne.** La même tous les jours, ou une pour la semaine et une pour le week-end. Changez un jour en particulier plus tard dans *Temps de jeu*.
3. **L'horloge réseau.** Le minuteur compte avec elle. Une console qui n'atteint jamais les serveurs de Nintendo la garde imprécise : voir l'étape 5.

## 4. Dissocier l'application mobile

Encore associée à l'application *Contrôle parental* de Nintendo (ou une console d'occasion associée à celle de quelqu'un d'autre) ? *Sécurité › Dissocier l'application mobile*. Sinon, la prochaine synchronisation de l'application écrase ce que vous avez réglé dans PlayGuard. Cela fonctionne hors ligne.

## 5. Régler l'horloge réseau

*Console › Horloge réseau › Mesurer l'écart* interroge trois serveurs de temps publics, puis *Régler l'horloge réseau*. Régler l'horloge remet à zéro le temps de jeu du jour : faites-le avant que l'enfant ne joue.

Une console sans Internet : quand aucun serveur ne répond, PlayGuard propose de régler l'horloge réseau sur l'horloge de la console à la place. Vérifiez d'abord l'horloge de la console avec un téléphone ou une montre, avec *Paramètres de la console › Console › Date et heure › Synchroniser l'horloge via Internet* désactivé ([détails](features.md#horloge-réseau)).

## 6. Choisir ce qui se passe quand le temps est écoulé

*Temps de jeu › Quand le temps est écoulé* : la console affiche « temps écoulé », puis soit **suspend le jeu**, soit **se contente de prévenir**. La suspension arrête le jeu sans bataille, mais la progression non sauvegardée peut être perdue ; l'avertissement s'en remet à l'enfant. Ajoutez une **alarme du coucher** dans le même onglet si vous en voulez une.

## 7. Protéger PlayGuard lui-même

*Sécurité › Protéger PlayGuard par le code PIN* :

- **Avant une modification** (par défaut) — tout le monde peut regarder, seul le parent modifie ;
- **Pour ouvrir PlayGuard** — un écran verrouillé au lancement ;
- **Jamais** — PlayGuard le signale dans la cloche tant qu'un code PIN est défini.

## 8. Confier la console

*Aujourd'hui › Montrer à l'enfant* n'affiche que le temps restant et l'heure de fin du jeu, assez grand pour se lire de l'autre bout de la pièce. Ⓐ et le code PIN ramènent les écrans du parent.

> [!IMPORTANT]
> **Un jeu laissé ouvert sur le menu HOME continue d'être décompté.** Fermez-le, ou mettez la console en veille ([ce qui compte](features.md#ce-qui-compte-comme-temps-de-jeu)).

## Les tâches du quotidien

| Pour… | Où |
|---|---|
| Donner plus de temps aujourd'hui | *Aujourd'hui › Temps en plus* (+15 min, +30 min, +1 h). Il est retiré de lui-même le lendemain. |
| Donner plus de temps un autre jour | *Aujourd'hui › Temps en plus › Un autre jour…* |
| Arrêter pour aujourd'hui | *Aujourd'hui › Arrêter pour aujourd'hui* : maintenant, ou dans 5 minutes pour sauvegarder. |
| Changer la limite d'un jour | *Temps de jeu* : Ⓐ sur le jour, ↑/↓, puis **+** pour enregistrer. |
| Passer de la période scolaire aux vacances | *Temps de jeu › Profils* ; *Profil programmé* pour une période choisie à l'avance. |
| Voir qui a joué à quoi | *Activité* : par jeu, par compte, aujourd'hui, 7 jours, depuis toujours ; export ou QR code. |
| Bloquer tous les jeux pour l'instant | *Temps de jeu › Bloquer tous les jeux*. Une fois désactivé, les limites précédentes reviennent. |
| Annuler une modification | **ZR** *Annuler* juste après, ou *Historique › Historique des modifications* à tout moment. |
| Garder une copie des réglages | *Historique › Sauvegarder les réglages* (jamais le code PIN). |
| Savoir ce qui demande votre attention | La cloche (Ⓨ) : ce qu'il faut faire en orange, ce qu'il faut savoir avec un « i ». |

## Ensuite

- Chaque onglet et chaque réglage : [fonctions](features.md).
- Quelque chose ne fonctionne pas comme prévu : [dépannage](troubleshooting.md#problèmes-courants).
- Ce que fait la console, et avec quelle certitude : [fonctionnement du contrôle parental](../parental-controls.md) (en anglais).

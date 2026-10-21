<!-- translated from docs/features.md, sha256 1cf484ad9b39 -->
# Fonctions

Ce que fait chaque onglet, comment PlayGuard écrit sans risque, et ce que la console compte comme temps de jeu. Nouveau sur PlayGuard ? Commencez par la [prise en main](getting-started.md). Retour au [README](../../README.fr.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

Les captures montrent le thème sombre, celui par défaut (en anglais).

| Aujourd'hui | Console |
|---|---|
| ![Aujourd'hui](../../images/screenshots/dashboard.png) | ![Console](../../images/screenshots/console.png) |
| **Temps de jeu** | **Temps de jeu › un jour en modification** |
| ![Temps de jeu](../../images/screenshots/play_timer.png) | ![Un jour en modification](../../images/screenshots/per_day.png) |
| **Activité** | **Un jeu** |
| ![Activité](../../images/screenshots/activity.png) | ![Un jeu](../../images/screenshots/activity_game.png) |
| **Restrictions** | **Console › Horloge réseau** |
| ![Restrictions](../../images/screenshots/restrictions.png) | ![Horloge réseau](../../images/screenshots/clock.png) |
| **Code PIN** | **Préférences** |
| ![Code PIN](../../images/screenshots/security.png) | ![Préférences](../../images/screenshots/preferences.png) |

## Commandes

| Bouton | Ce qu'il fait |
|---|---|
| ↑/↓ | Déplacer |
| Ⓐ | Valider |
| Ⓑ | Retour ou annuler (deux fois sur la barre latérale pour quitter) |
| Ⓧ | Actualiser (sur les onglets qui relisent la console) |
| Ⓨ | Notifications, depuis n'importe quel onglet |
| **−** | Langue et thème, depuis n'importe quel onglet (le pied de page ne les montre pas ; la cloche, la langue et le thème de l'en-tête le font) |
| **+** | Enregistre les limites par jour (et la semaine modifiée dans *Temps de jeu*) |
| **ZR** | *Annuler*, pendant quelques secondes après l'enregistrement d'une modification |

L'application est organisée en onglets, comme les paramètres de la console ; chacun a sa section ci-dessous.

- **Ce qui a échoué est dit dans une boîte de dialogue, ce qui a réussi dans une notification.** Un dialogue ne dit que ce qui s'applique à cette console maintenant : que le jeu en cours serait suspendu, seulement quand un jeu est ouvert ; que l'application compagnon pourrait écraser une modification, seulement quand elle est associée et peut se synchroniser.
- **L'en-tête** indique, sur chaque onglet, que le contrôle parental est déverrouillé temporairement (sa pastille), et affiche un crayon barré à droite du titre quand l'application est en lecture seule ; toucher l'un ou l'autre mène là où on l'annule (*Code PIN › Réactiver le contrôle parental*, l'écran du firmware).
- **En lecture seule**, les actions restent à leur place, grisées, et disent pourquoi quand on les choisit.

## L'apparence : Cadran

L'apparence de PlayGuard s'appelle Cadran : une barre d'icônes pour les onglets, une palette bleu nuit ou blanc froid (selon le thème), des cartes au lieu des listes. Chaque onglet s'ouvre sur l'essentiel (un anneau, des tuiles, des cartes, des puces), ses autres réglages en dessous.

- **La pastille de l'en-tête** dit de quoi parle l'onglet (*Contrôle parental actif*, *Minuteur actif*…) ; contrôle parental déverrouillé, elle le dit sur chaque onglet, et la toucher ouvre *Code PIN › Réactiver le contrôle parental*. Le texte secondaire ne descend jamais sous 15 px, pour se lire depuis le canapé sur une télé.
- **Les dialogues** sont des cartes aux boutons en pilules, la confirmation en turquoise (en corail quand elle supprime ou arrête quelque chose).
  - Les choix s'ouvrent en carte au-dessus des aides, le choix actuel sur un turquoise léger avec sa coche. Le contour turquoise n'est qu'au focus, partout : un niveau, un compte, un choix en vigueur garde le fond léger.
  - Un long dialogue s'estompe en bas tant que la suite continue, et son avertissement (une partie sur le point d'être suspendue) vient en premier, dans une carte orange.
  - Un échec a pour titre ce qui a échoué, le code d'erreur en petit sous la raison, et propose *Envoyer un rapport* quand c'est utile.
  - Les messages apparaissent en cartes sous l'en-tête, avec une coche turquoise pour ce qui a été fait et une marque orange pour ce qui a été refusé.
- **Les autres écrans** prennent la même allure : l'éditeur par jour d'un profil montre la semaine comme *Temps de jeu* (modifiée de la même façon, les remplissages rapides en tuiles) ; *Vérifier les jeux installés* et *Tous les jeux* mettent leurs lignes en cartes ; *Profils* et *Aide et diagnostic* donnent son icône à chaque ligne.
- **Taille du texte** (*Préférences*, ou **AA** dans l'en-tête) :
  - *Automatique*, par défaut, agrandit tous les textes de 25 % quand la console est sur sa station, sur la TV, et les remet en mode portable ; *Plus grande* les garde agrandis, *Normale* jamais.
  - L'en-tête, la barre latérale et le pied de page gardent leur taille (ils n'ont pas la place), comme les noms des niveaux de *Restrictions*, déjà grands.
  - Le changement est immédiat, sur tous les écrans, sans relancer.
  - Les 25 % viennent d'une TV 55" en 4K : le plus petit texte (15 px) y fait alors 14,6 mm de haut en capitales, 20′ d'arc à 2,5 m, où la lecture est confortable (16′ en taille normale, le strict minimum).
- **Effets visuels** (*Préférences*) : *Allégés sur batterie*, par défaut, retire le halo de fond, le battement de l'anneau de l'écran enfant dans ses dernières minutes et le glissement de l'arc vers une nouvelle valeur tant qu'aucun chargeur n'est branché (en portable, débranchée) ; *Toujours complets* les garde. Relu toutes les 5 secondes : brancher la console les fait revenir.
- **Annuler :** après l'enregistrement d'une modification, **ZR** *Annuler* reste quelques secondes dans le pied de page : il ouvre cette modification dans l'historique, pour remettre la valeur précédente. Un historique vide le dit dans une carte.

## Notifications — ce qui demande votre attention, depuis n'importe quel onglet

Une cloche dans l'en-tête, à gauche de la version, regroupe ce qui demande l'intervention du parent ; Ⓨ ouvre la liste depuis n'importe quel onglet (le badge de la cloche indique *2* quand deux choses sont à faire).

À gauche de la cloche, la langue (son code, *FR*, *EN*…) et le thème (un soleil pour clair, une lune pour sombre, un cercle à moitié plein quand il suit la console), puis la taille du texte (**AA**, son grand A en turquoise quand le texte est agrandi) : un appui ouvre la même liste que dans *Préférences*, et **−** les propose depuis n'importe quel onglet. Un nouveau thème ou une nouvelle langue prend effet tout de suite, chaque écran refait sur le même onglet avec la sélection sur la même ligne, chinois et sa police compris.

### À faire

Chacune avec un triangle orange, comptée en orange sur la cloche ; elle reste jusqu'à ce que ce soit réglé :

- pas encore de code PIN, ou un code PIN que n'importe qui essaierait en premier (0000, 1234, 1212, 2580… : vérifié sur la console, jamais affiché) ;
- un code PIN sur la console alors que PlayGuard s'ouvre sans lui (*Protéger PlayGuard par le code PIN* sur *Jamais*) ;
- le contrôle parental déverrouillé temporairement (*Réactiver*) ;
- du temps en plus d'un jour précédent à retirer ;
- l'alarme « temps écoulé » désactivée ;
- des modifications faites en dehors de PlayGuard ;
- la console qui ne compte pas le temps de jeu ;
- une horloge réseau imprécise.

### Pour information

Chacune avec un « i », un point sur la cloche ; elle peut être marquée comme lue :

- une limite encore abaissée par *Arrêter le jeu pour aujourd'hui* un jour précédent (*Remettre*) ;
- une nouvelle version installée (*Nouveautés* ; après une mise à jour sur place, la version précédente gardée), ou une nouvelle version de PlayGuard ;
- un rapport envoyé en ligne (*Afficher le lien*), ou PlayGuard fermé de façon inattendue la dernière fois (*Envoyer un rapport*) ;
- le temps de jeu de la semaine (tous les 7 jours jusqu'à lecture ; avec la comparaison par jour avec la semaine d'avant ; *Afficher son QR code* ouvre le résumé de la semaine sur *Activité*), et une limite dépassée deux jours ou plus sur les 7 derniers quand la console se contente de prévenir ;
- une sauvegarde des réglages à faire (aucune, ou aucune depuis 30 jours) ;
- tous les jeux bloqués depuis une semaine ;
- un nouveau compte utilisateur ;
- l'application mobile associée ;
- l'horloge réseau trouvée décalée au lancement.

### Console

Les entrées qui concernent la console plutôt que le contrôle parental viennent en dernier, sur une ligne que Ⓐ ouvre (ouverte d'emblée quand il n'y a rien d'autre). Celles à faire sont comptées sur la pastille de l'onglet Console, pas sur la cloche :

- des jeux installés qui ne peuvent pas démarrer (*Vérifier les jeux installés*), des patchs de jeux à vérifier ;
- la carte SD presque pleine (moins de 500 Mo) ;
- un fichier de PlayGuard qui n'a pas pu être mis de côté ou écrit sur la carte SD ;
- le sysmodule de récupération pas à jour (pas celui fourni avec cette version, ou installé avant qu'il n'indique sa version ; *Mettre à jour* installe celui que cette version contient), puis la console à redémarrer pour qu'il serve, jusqu'à ce qu'elle l'ait été ;
- une version qui n'a pas fini de démarrer et dont PlayGuard est revenu (*Envoyer un rapport*).

Celles pour information : un retour à la version précédente, un firmware non testé avec cette version, une nouvelle version d'Atmosphère, de hekate ou du firmware (vérifiée avec *Rechercher les mises à jour chaque jour* ; l'entrée du firmware conseille d'attendre qu'Atmosphère le prenne en charge), le numéro de série non masqué sur emuMMC, le contrôle parental propre à l'autre système, un fichier de PlayGuard trouvé endommagé et mis de côté (une notification au lancement y renvoie).

### Comportement des entrées

- Chaque section liste ses entrées dans un ordre fixe, quelle que soit la vérification qui a fini la première.
- **Dans la barre latérale**, une pastille ambre à côté du nom d'un onglet compte ses entrées à faire, là où elles se règlent : l'onglet Code PIN (pas de code PIN, un code PIN trop simple, PlayGuard sans son code PIN, déverrouillé temporairement), Temps de jeu (l'alarme désactivée, la console qui ne compte pas), *Aujourd'hui* (des changements faits hors de PlayGuard, du temps en plus à retirer), la Console (ses entrées ci-dessus, et l'horloge réseau). Les entrées ignorées et celles pour information ne sont pas comptées.
- Dans l'onglet, la même pastille est sur la ligne où chacune se règle (*Horloge réseau*, *Vérifier les jeux installés*, *Stockage*, *Patchs de jeux* ; *Définir ou changer le code PIN*, *Protéger PlayGuard par le code PIN*, *Réactiver le contrôle parental* ; *Avancé* du Temps de jeu pour l'alarme), et l'onglet s'ouvre sur la première d'entre elles.
- **Ⓐ** sur une entrée fait la correction (*L'activer*, *La régler*, *Mettre à jour*…) ou ouvre l'onglet où elle se fait. Une entrée disparaît d'elle-même une fois réglé ce qu'elle signale.
- **Ⓧ** marque une entrée pour information comme lue ; **Ⓨ** les marque toutes. Une entrée marquée comme lue le reste au lancement suivant, jusqu'à ce que son texte change.
- **Ignorer :** quelques entrées à faire peuvent être ignorées avec Ⓧ, après une confirmation et le code PIN : un code PIN que tout le monde essaierait en premier, PlayGuard qui s'ouvre sans le code PIN, l'alarme « temps écoulé » désactivée, la carte SD presque pleine (jamais l'absence de PIN, le contrôle parental déverrouillé, l'horloge, le minuteur qui ne compte pas ni les fichiers de PlayGuard). Une entrée ignorée n'est plus comptée et s'affiche en dernier sous *Ignorés*, où Ⓐ la fait compter de nouveau. Elle revient d'elle-même 30 jours plus tard (sauf si *Rappeler les avertissements ignorés après 30 jours* est désactivé), dès que le problème a été corrigé puis se reproduit, ou, pour le code PIN, avec un nouveau code d'une autre longueur.
- *Préférences › Notifications* désactive trois types : versions d'Atmosphère, de hekate et du firmware ; temps de jeu de la semaine ; rappels (sauvegardes, autre système, jeux bloqués).
- L'état derrière est relu au lancement de PlayGuard, toutes les minutes sur l'écran principal, et toutes les 5 s tant qu'*Aujourd'hui* est affiché.

## Aujourd'hui — la journée en un coup d'œil

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/)*

### L'anneau et la ligne

**Le temps restant** en anneau : un jour sans jeu dit *Pas de jeu*, pas *0 min* ; le temps en plus donné aujourd'hui y forme un segment plus clair à son extrémité, et l'anneau glisse vers chaque nouvelle valeur.

Dessous, ce qui arrêtera le jeu en premier :

- avant la fin du temps ou l'alarme du coucher, le reste de la journée sur une ligne, maintenant à gauche, chacune d'elles une marque avec son heure (*16:40 temps écoulé*, *21:00 coucher*), en sarcelle jusqu'à la première, en orange dans ses 15 dernières minutes ;
- sinon en mots : l'alarme du coucher une fois le temps du jour épuisé, l'heure où le jeu est de nouveau permis ; contrôle parental déverrouillé, la limite du jour en pause et pourquoi.

### Les actions

- **Temps en plus**, avec ses montants à portée d'un appui : un montant sélectionné déplace la fin du temps sur la ligne (*avec 15 min de plus*), ou dit sur la marque de l'alarme du coucher qu'elle arrive d'abord ; le premier ressort en sarcelle dès qu'il reste 15 min ou moins, ou plus rien.
- **Arrêter pour aujourd'hui**, **Montrer à l'enfant**.
- **Limite d'aujourd'hui…** : la limite du jour choisie sur place, écrite après une confirmation.
- **Le profil en vigueur**, par son nom, dès qu'un profil est enregistré (*Profils…* quand aucun ne correspond) : Ⓐ les liste, celui en vigueur coché, pour en appliquer un autre après une confirmation (les jours qui changent), ou *Gérer les profils…*.
- Contrôle parental déverrouillé, **Réactiver** passe en premier, sur toute la largeur, et *Temps en plus* s'efface.
- Sans limite sur aucun jour, **Définir les limites de la semaine** ouvre la semaine de *Temps de jeu*.

### Les cartes et les pastilles

- L'**alarme du coucher** (l'heure où le jeu est de nouveau permis, ou dans combien de temps elle sonne quand la console ne le dit pas ; désactivée, *Désactivée* en grand et Ⓐ *La régler*), le **contrôle parental** et l'**horloge réseau**, en cartes.
- L'état de la console en **pastilles** :
  - l'une dit si la redirection DNS d'Atmosphère bloque les serveurs de Nintendo, et Ⓐ dessus lesquels et ce que cela change : plus de synchro pour l'application mobile, une horloge réseau qui ne se règle plus seule ;
  - une autre, ouvert par-dessus un jeu, que le temps passé dans PlayGuard est décompté, Ⓐ disant comment ;
  - un jeu laissé ouvert sur le menu HOME apparaît en pastille orange (*toujours décompté*, Ⓐ disant pourquoi), quand la pastille sur PlayGuard ouvert par-dessus un jeu ne le dit pas déjà.
- Les pastilles mettent d'abord ce qui demande le parent (orange, avec un triangle), puis ce qui est actif (sarcelle, avec une coche), puis le reste (gris, avec un point) : la forme le dit autant que la couleur.
- Sous les pastilles, le temps en plus encore à retirer (Ⓐ le retire) et l'alarme « temps écoulé » tant qu'elle est désactivée (Ⓐ la réactive).

### Premiers pas

Tant que le code PIN et les limites de la semaine ne sont pas définis, une carte au-dessus des cartes liste les trois étapes (le code PIN, une limite quotidienne, l'horloge réseau), chacune cochée en sarcelle une fois faite ou cerclée d'orange, avec combien sont faites (*1 sur 3*) ; Ⓐ sur l'une la fait sur place.

### La console décompte-t-elle ?

Le temps de PlayGuard lui-même compte quand il tourne comme une application : avec une limite aujourd'hui, le temps restant doit donc baisser tant qu'il est ouvert. S'il n'a pas bougé pendant environ 90 s à l'écran (minuteur actif, limite aujourd'hui, pas de déverrouillage, pas d'ouverture depuis l'album), une ligne orange sous le temps joué indique *La console ne décompte pas le temps de jeu en ce moment*, avec les causes probables :

- l'horloge a été reculée (le décompte reprendra plus tard) ;
- l'horloge réseau n'a jamais été réglée (*Console › Horloge réseau*) ;
- une limite vient d'être écrite (lancez un jeu une minute, puis vérifiez).

Elle disparaît dès que le temps restant bouge de nouveau. *Temps de jeu* l'indique aussi.

### Modifié en dehors de PlayGuard

Le temps de jeu du jour reparti de zéro le même jour (un changement d'horloge fait cela), l'horloge de la console déplacée de plus de 5 minutes, des limites différentes de celles que PlayGuard a vues en dernier (l'application mobile, un autre outil) : une carte orange au-dessus de l'anneau jusqu'à ce qu'elle soit masquée (Ⓐ) ou jusqu'au lendemain, et une entrée dans l'historique des modifications. PlayGuard ne tourne pas en arrière-plan : il ne voit que ce qui a changé pendant qu'il était ouvert, ou depuis sa dernière ouverture, quand il relit la console.

### À la limite

La console affiche « temps écoulé », puis suspend le jeu (sans « continuer ») ou se contente de prévenir : *Quand le temps est écoulé* indique lequel et permet de le changer, une fois une limite ou une heure du coucher réglée ([détails, en anglais](../parental-controls.md#when-the-time-runs-out)).

### Temps en plus et arrêt du jeu pour aujourd'hui

- **Temps en plus aujourd'hui** : +15 min, +30 min, +1 h par défaut.
- **Arrêter le jeu pour aujourd'hui** : limite à 0 pour aujourd'hui seulement, *maintenant* ou *dans 5 minutes* quand le temps joué est connu (la limite devient alors le temps joué + 5 min, le temps de sauvegarder). Au reverrouillage, le « temps écoulé » de la console apparaît, et la progression non sauvegardée peut être perdue — la confirmation le signale.
- **Le lendemain** — au lancement, ou à minuit si l'application est ouverte — PlayGuard remet la limite habituelle :
  - après du temps en plus, de lui-même, sans question ni code PIN (il ne fait que baisser la limite, comme la réactivation du contrôle parental), en déverrouillant un instant si le minuteur décompte et en reverrouillant aussitôt. Il attend tant qu'il ne peut pas écrire (mode lecture seule, firmware non pris en charge, blocage de la console activé) et, une semaine plus tard le même jour, tant que la limite plus basse arrêterait le jeu en cours (alors le lendemain). Pour garder plus de temps un jour donné, changez la limite de ce jour ;
  - après *Arrêter le jeu pour aujourd'hui*, il demande s'il faut garder 0 ou remettre la limite (avec le code PIN, comme tout changement qui relève une limite), ou la remet comme tout changement (*Temps de jeu › Remettre la limite d'office après un arrêt du jeu*) ; remettre « sans limite » est toujours demandé.
- **Du temps en plus un autre jour** (*Temps en plus › Un autre jour…*, en fin de liste) : demain ou l'un des cinq jours suivants, chacun avec sa limite, puis la durée. La limite de ce jour est relevée tout de suite, pour cette date seulement, et remise d'elle-même (elle ne fait que baisser) à la prochaine ouverture de PlayGuard une fois la journée passée — pas sur une console où PlayGuard n'est pas ouvert, où le même jour suivant la garde aussi. Un seul à la fois : la liste propose alors de le retirer (sa limite remise aussitôt). Une modification de la limite de ce jour entre-temps (la semaine, un profil…) l'emporte : rien n'est remis.

### L'écran enfant

*Montrer à l'enfant* sur *Aujourd'hui*, pour confier la console :

- seulement le temps restant (contrôle parental déverrouillé, la limite du jour, en pause), l'heure de fin (*Fin vers 16:40* ; quand l'alarme du coucher arrive avant, l'anneau fait le compte à rebours jusqu'à elle, *45 min avant le coucher*, comme dans la dernière heure d'un jour sans limite), le coucher et ce qui se passe quand le temps est écoulé ;
- fait pour se lire de l'autre bout de la pièce, sur la TV ; son texte garde sa taille quelle que soit la taille du texte ;
- dans les 5 dernières minutes (de jeu ou avant le coucher), la fin passe en orange et l'anneau pulse lentement ; le coucher passe en orange dans ses 30 dernières ;
- Ⓐ et Ⓨ, les touches du parent, sont indiquées en petit en haut à droite, pour que le pied de page ne montre que celle de l'enfant ;
- Ⓐ revient aux écrans du parent avec le code PIN ; Ⓨ donne du temps en plus depuis là, après le code PIN (les montants de *Temps en plus*, l'écran enfant restant affiché) ; Ⓑ quitte PlayGuard (en proposant d'abord de réactiver le contrôle parental, comme en quittant).

En texte plus grand, *Aujourd'hui* tient toujours sur un écran : la ligne au-dessus du titre ne dit plus que le jour et sa limite (*JEUDI · 2 h*), les boutons et les cartes se resserrent un peu. Actualisation toutes les 5 s (Ⓧ pour actualiser tout de suite).

## Temps de jeu — la semaine sert d'éditeur

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/?tab=play_timer)*

### Modifier la semaine

Ⓐ sur un jour le modifie :

| Bouton | Pendant la modification |
|---|---|
| ↑/↓ | Le changer de 15 min, par heure une fois maintenus |
| ←/→ | Passer au jour suivant |
| Ⓛ | Le copier du lundi au vendredi |
| Ⓡ | Le copier sur le week-end |
| Ⓨ | Pas de limite |
| Ⓧ | Une durée exacte (*Saisir une durée…* : ↑/↓ par 15 min, par heure une fois maintenus, Ⓛ/Ⓡ par 5 min, OK sélectionné) |
| Ⓐ ou Ⓑ | Terminer la modification |

- Au toucher, un jour touché passe en modification, puis toucher son haut ou son bas le change de 15 min.
- Les modifications s'enregistrent ensemble, avec **+** ou *Enregistrer*, comme dans l'éditeur par jour (la confirmation liste les jours modifiés, avant → après) ; *Annuler* les remet.
- Dans la barre de chaque jour, le temps joué à sa dernière occurrence (aujourd'hui compris, tous comptes confondus), en corail une fois la limite atteinte, sa limite aussi, et une légende sous la semaine le dit (*limite dépassée*).
- Un jour modifié et pas encore enregistré a sa limite en orange et sa barre cerclée d'orange ; sous la semaine, les chiffres du jour sélectionné (*jeudi : 1 h 20 joué sur 2 h*).
- Au-dessus de la semaine, en orange, une limite atteinte et ce que fait alors la console, ou la console qui ne décompte pas le temps de jeu.
- `0` minute signifie *pas de jeu ce jour-là* ; *Supprimer la limite de temps de jeu* désactive le minuteur.

### Puces et réglages rapides

**Des puces** au-dessus et en dessous de la semaine ouvrent directement leurs choix : les profils (combien sont enregistrés), la même limite tous les jours, l'alarme du coucher, *Quand le temps est écoulé* et la suppression.

- **Même limite tous les jours :** liste rapide ou durée exacte (*Saisir une durée…* : ↑/↓ par 15 min, par heure une fois maintenus, Ⓛ/Ⓡ par 5 min).
- **Semaine et week-end › :** l'éditeur, qui demande la limite du lundi au vendredi puis celle du week-end.
- **Temps en plus aujourd'hui** et **arrêter le jeu pour aujourd'hui**, comme dans *Aujourd'hui*. Une note en dessous dit ce que fait la console à la limite. Pendant un déverrouillage temporaire du contrôle parental, le temps de jeu n'est *peut-être* pas décompté : la question reste ouverte ([docs, en anglais](../parental-controls.md#still-open)).
- Chaque confirmation liste les jours qui changent, avant → après.

### Profils

- **Profils** enregistrés sur la carte SD : appliquer (un profil déjà en vigueur le dit simplement), modifier, renommer ou supprimer un profil ; enregistrer les limites actuelles ou en créer un nouveau.
- N'importe quel nom, dans n'importe quelle écriture (*École*, *周末*, *Выходные* …) — un nom sans lettres latines reçoit son propre nom de fichier, et deux noms qui donneraient le même fichier sont repérés.
- **Profil programmé** (*Profils › Profil programmé*) : un profil pour une période — vacances, voyage — dont le premier et le dernier jour se choisissent parmi les 60 jours à venir.
  - Quand la période commence, PlayGuard propose de l'appliquer (les jours qu'il change affichés sous la question) ; une fois finie, de remettre les limites qu'il a remplacées, ou de les garder.
  - Demandé une fois par jour, sur l'écran principal, quand rien d'autre n'est devant : PlayGuard n'agit qu'à son ouverture, donc une console où il n'est pas ouvert garde ses limites. Une période passée sans que le profil ait été appliqué est oubliée.
  - Un seul à la fois ; la ligne le modifie, le retire ou y met fin.

### Alarme du coucher

- L'heure de l'alarme (16:00 à 23:45, ou désactivée) et l'heure où le jeu est de nouveau permis (05:00 à 09:00), la même tous les jours.
- Ou, depuis *Par jour de la semaine…* en fin de liste de l'alarme, du lundi au vendredi, le week-end ou un seul jour, les autres jours gardant la leur (chaque groupe de jours listé avec son heure). Quand les jours diffèrent, l'onglet indique *Varie selon le jour*, et la semaine de *Temps de jeu* montre l'alarme de chaque jour sous son nom.
- Sa place dans les réglages du minuteur, déduite des réglages de l'application mobile, est vérifiée sur une console (22.0.0) ; PlayGuard ne la modifie toujours que si la console indique ce qu'il y lit, vérifie la réponse de la console après la modification et remet les réglages précédents si elle diffère.

### Réglages du temps en plus et Avancé

- **Temps en plus :** les montants proposés (+15/+30/+1 h, +10/+20/+30 min…) et la remise d'office de la limite habituelle après *Arrêter le jeu pour aujourd'hui* (le temps en plus revient toujours sans rien demander). L'une comme l'autre se fait la prochaine fois que PlayGuard est ouvert, à partir du lendemain : tant qu'il n'est pas ouvert, la console garde la limite de ce jour-là.
- **Avancé ›**, un écran à part : l'interrupteur des actions de débogage et, tant qu'il est activé, l'alarme « temps écoulé » et la pause / reprise du décompte. Le même interrupteur permet à une sauvegarde ou à l'historique de remettre l'alarme.

## Activité — qui a joué à quoi, et combien de temps

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/?tab=activity)*

Temps passé sur chaque jeu **aujourd'hui**, sur les **7 derniers jours** et **depuis toujours**, lu dans le journal d'activité de la console — pour tous les comptes ou **un seul compte utilisateur** (les comptes en puces en haut, quand la console en a plusieurs). PlayGuard s'ouvre sur le dernier compte affiché, ou sur celui choisi dans *Compte par défaut* (tous les comptes, ou un seul).

### En haut

- Les comptes en puces, la période et l'export.
- Aujourd'hui, les 7 derniers jours et depuis toujours en chiffres. Sous les 7 derniers jours, la comparaison avec les 7 d'avant, par jour : ▲ *+20 min/jour vs semaine d'avant* en orange, ▼ de moins en sarcelle, *comme la semaine d'avant* à 5 minutes près.
- Les quatre jeux les plus joués en cartes (Ⓐ en ouvre un) ; sur les 7 derniers jours, celui qui a le plus augmenté par rapport à la semaine d'avant le dit, dès une demi-heure (▲ *+2 h*).
- Rien de joué sur la période : une carte le dit, avec *Voir depuis toujours* quand il y a quelque chose à voir.

### Graphiques et résumé

- **Un graphique des sept derniers jours** (aujourd'hui à droite, une légende dessous), avec la limite de cette semaine pour chaque jour en trait et le temps au-delà en orange, marqué ! ; totaux du jour, de la semaine et depuis toujours.
- **Un résumé** sur la période choisie (aujourd'hui, les 7 derniers jours, les **30 derniers jours** ou depuis toujours), pour le compte affiché : moyenne par jour, le jeu le plus joué, les jours joués et le plus chargé (7 derniers jours), la session moyenne (depuis toujours).
- **Heures de jeu, 7 derniers jours** : un carré par heure de chaque jour, lundi en premier, plus il est vif plus on a joué (tous les jeux, le compte affiché) ; puis les sessions (le jeu entrecoupé de pauses de moins de 5 minutes compte pour une session, changement de jeu compris) et leur durée moyenne, la plus longue et son jour, les deux heures d'affilée les plus jouées, et le temps joué entre 21:00 et 06:00.

### Tous les jeux et la page d'un jeu

- **Tous les jeux**, sur un écran à part : chaque jeu joué sur la période, les plus joués d'abord, en **grille de jaquettes** ou en **liste** (Ⓨ bascule, et le choix est gardé), avec les chiffres du jeu sélectionné toujours en haut : son icône et son nom, le temps sur la période, depuis toujours, les lancements et la dernière partie. Ⓧ choisit la période (l'onglet Activité suit).
- Les icônes sont lues autour du jeu sélectionné (pas en mode applet, pour économiser la mémoire) ; une manette grise les remplace en attendant, et pour un jeu qui n'en a pas.
- Une grande bibliothèque affiche ses 50 premiers jeux, puis *Afficher tous les jeux* (100 jeux au plus en mode applet) ; l'export les contient toujours tous. PlayGuard lit 256 jeux au plus (ceux de la semaine d'abord) : au-delà, l'onglet Activité, Tous les jeux et l'export (sauf CSV) signalent qu'il en manque, et le total depuis toujours ne les compte pas.
- **Ⓐ sur un jeu :** ses sept derniers jours en barres, ses lancements, ses première et dernière parties, le temps de chaque compte. Les jeux supprimés gardent leurs chiffres depuis toujours, même un jeu supprimé avant l'installation de PlayGuard tant que le journal de jeu de la console le cite encore, et leur nom dès que PlayGuard l'a vu (sinon *Jeu supprimé* et son ID de titre).

### Comptes

- Le temps d'un jeu qu'aucun compte utilisateur ne porte (certains jeux n'en demandent pas, ou le compte a été supprimé depuis) apparaît en *Sans compte*, à partir d'une minute.
- Un compte utilisateur que PlayGuard n'avait jamais vu est signalé en orange dans l'onglet pendant 30 jours (*Nouveau compte depuis le …* : créer un compte est une astuce connue pour repartir de zéro) et noté dans l'historique des modifications ; les comptes présents quand PlayGuard les liste pour la première fois ne sont pas signalés.

### Export et partage

- **Export sur la carte SD** (la puce *Exporter*) en CSV, JSON, XLSX (Excel) ou PDF, une colonne par jour, puis les 7 derniers jours, les 30 derniers jours et depuis toujours.
- **Le résumé de la semaine en QR code :** l'appareil photo d'un téléphone le lit comme quelques lignes de texte, dans la langue affichée — les 7 derniers jours et leur moyenne, la tendance, chaque jour, les trois jeux les plus joués, les sessions, la plus longue et le temps joué entre 21:00 et 06:00. Rien ne passe par Internet.
- S'ouvre aussitôt sur les derniers chiffres lus (gardés sur la carte SD d'un lancement à l'autre), actualisés en arrière-plan au-delà d'une minute ; Ⓧ les relit tout de suite, avec un indicateur de chargement.
- Durées approximatives si l'horloge de la console a été modifiée.

## Restrictions — niveau, classification par âge, communication

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/?tab=restrictions)*

- **Le niveau de restriction :** Jeune enfant, Enfant, Adolescent et Personnalisé côte à côte en tuiles, chacun avec ce qu'il restreint (un cadenas pour restreint, une coche pour autorisé, et les jeux installés qu'il bloque ; Ⓐ le choisit ; sur *Personnalisé* en vigueur, Ⓐ mène à ses réglages en dessous) ; *Aucun* en dessous. Ce qu'impose un niveau (limite d'âge, publications, communication) est redit avant de l'appliquer.
- **En Personnalisé :** classification par âge, publications sur les réseaux sociaux, communication avec d'autres joueurs.
- **Mode VR** et **organisme de classification** (PEGI, ESRB, USK, CERO…) : la liste dit pour quels pays chacun classe les jeux (PEGI : la plupart de l'Europe, ESRB : États-Unis, Canada et Amérique latine, USK : Allemagne, PEGI BBFC : Royaume-Uni…).
- **Le nombre de jeux autorisés à communiquer** (la liste elle-même figure, brute, dans le rapport de diagnostic tant que sa structure n'est pas connue).
- **Jeux installés bloqués :** combien de jeux installés le niveau en vigueur bloque selon leur classification par âge (*3 sur 15*), Ⓐ les liste avec leur classification (*PEGI 16*) ; la tuile de chaque niveau l'indique aussi (*Jeux bloqués · 3*).
  - Les classifications sont celles de chaque jeu (ses données de contrôle, lues par la vérification des jeux installés au démarrage ou depuis *Console › Vérifier les jeux installés*), selon l'organisme qu'utilise la console.
  - Un jeu que cet organisme ne classe pas n'est pas compté, ni les exceptions réglées dans l'application mobile.
  - L'écran d'un jeu dans Activité affiche sa classification, et *bloqué au niveau actuel* le cas échéant.

## Code PIN — code PIN, déverrouillage, blocage des jeux, application mobile

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/?tab=security)*

- **En haut**, trois tuiles : le code PIN, les restrictions et le déverrouillage temporaire, chacune avec une coche turquoise quand tout est comme il faut (code PIN défini, restrictions actives, verrouillé) ou un triangle orange quand il faut intervenir.
- **Définir ou changer le code PIN** (écran système).
- **Afficher le code PIN** : après un avertissement et le code PIN lui-même, demandé à chaque fois ; noté dans l'historique, puis un nouveau code PIN est proposé.
- **Déverrouiller temporairement**, **réactiver le contrôle parental**, et si PlayGuard le réactive de lui-même juste après une modification qui a demandé le déverrouillage (activé par défaut).

### Protéger PlayGuard par le code PIN

Le code PIN dans PlayGuard lui-même :

| Réglage | Effet |
|---|---|
| *Jamais* | Pas de code PIN dans PlayGuard (la cloche le signale tant que la console en a un) |
| *Avant une modification* (par défaut) | Tout le monde peut regarder, seul le parent modifie |
| *Pour ouvrir PlayGuard* | Un écran verrouillé au lancement |

- Une fois saisi, le code vaut jusqu'à la fermeture de PlayGuard ; *Redemander le code PIN après 5 minutes* (désactivé par défaut) le redemande après ce délai, et ramène l'écran verrouillé de *Pour ouvrir PlayGuard* quand PlayGuard revient après 5 min ou plus hors de l'écran.
- Vérifié dans la couche service, donc aucune modification n'y échappe ; reverrouiller ne le demande jamais. *Afficher le code PIN* le demande à chaque fois, dans tous les modes et même juste après sa saisie, et un `config.json` absent ou abîmé compte comme *Avant une modification*.
- **Quitter** alors que le contrôle parental est encore déverrouillé propose de le reverrouiller (*Réactiver et quitter* ferme aussi le jeu par-dessus lequel PlayGuard a été ouvert, retour au menu HOME plutôt qu'à hbmenu) ; fermé depuis le menu HOME à la place, PlayGuard le reverrouille à son prochain lancement, et un redémarrage de la console le reverrouille de lui-même.

### Bloquer tous les jeux

Un interrupteur qui met la limite de chaque jour à 0, donc un code PIN est nécessaire pour lancer un jeu — un blocage léger, sans classification par âge ni limite de communication.

- Il bloque le lancement des jeux, pas le menu HOME, et nécessite un code PIN.
- Les limites précédentes reviennent quand on le désactive.
- Tant qu'il est activé, *temps en plus* et *arrêter le jeu pour aujourd'hui* sont refusés, et des limites réglées autrement (un profil, une sauvegarde, l'historique…) le remplacent.

### Application mobile

- Association de l'application Contrôle parental Nintendo Switch, dernière synchronisation, et **dissociation** (sinon sa prochaine synchronisation écrase les limites réglées ici : tant qu'elle est associée, chaque modification que PlayGuard fait confirmer le rappelle).
- Quand la redirection DNS d'Atmosphère (`atmosphere/hosts`) détourne le serveur du contrôle parental de Nintendo, l'application ne peut pas se synchroniser : PlayGuard indique alors *synchro bloquée (DNS)* ici et ne la mentionne nulle part ailleurs (pas de note dans les dialogues, pas d'entrée dans la cloche, pas d'étape dans les premiers pas, pas de pastille sur *Aujourd'hui*, pas d'aide au décodage, et la dissociation se confirme sans dire ce que l'application écraserait).
- Tant qu'elle est associée et peut se synchroniser, *Avant de dissocier : aidez à décoder…* ouvre la comparaison du bloc (voir [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), en anglais).

### Supprimer tout le contrôle parental

Deux confirmations, irréversible ; une sauvegarde des réglages est d'abord enregistrée.

## Console — l'horloge réseau, les jeux installés, ce que fait tourner la console

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/?tab=console)*

**En haut**, ce que fait tourner la console en tuiles (le firmware, Atmosphère, et hekate quand il est sur la carte SD ; Ⓐ ouvre ses notes de version), puis le diagnostic, puis les lignes de la console, la compatibilité en premier (une coche turquoise seule quand PlayGuard a été vérifié sur ce firmware, sinon un triangle et ce qui ne va pas).

### Horloge réseau

*Horloge réseau ›*, un écran à part — l'horloge dont dépend le minuteur. Une console qui n'atteint jamais les serveurs de Nintendo garde cette horloge imprécise, ce qui fausse le minuteur.

- **Horloges console et réseau**, fuseau horaire, précision. Quand la redirection DNS d'Atmosphère (`atmosphere/hosts`) envoie ailleurs le serveur d'heure de Nintendo (`aauth-lp1.ndas.srv.nintendo.net`), la console ne peut plus régler son horloge réseau elle-même : cet écran, l'entrée de la cloche et la carte d'*Aujourd'hui* le disent et renvoient à la mesure ci-dessous.
- **Choix d'un serveur NTP public** (≈ 50 intégrés, par région, ou le vôtre).
- **Mesure** sur 3 serveurs (médiane ; alerte orange en cas de désaccord), puis **réglage de l'horloge réseau** — une mesure reste utilisable 2 minutes, avec un décompte. La confirmation le dit : changer l'horloge remet à zéro le temps de jeu du jour, toute la limite redevient donc disponible.
- **Vérification de l'horloge réseau au lancement** : une notification si elle a plus d'une minute d'écart ; elle ne règle jamais l'horloge.

#### Aucun serveur ne répond

Sur une console sans Internet, *Régler l'horloge réseau sur l'horloge de la console* apparaît sous le résultat (et sur la carte de l'horloge réseau d'*Aujourd'hui*, quand sa mesure échoue), derrière la même confirmation.

- PlayGuard n'a rien d'autre pour comparer l'horloge de la console : la confirmation affiche son heure et demande de la vérifier d'abord avec un téléphone ou une montre, et une horloge de console antérieure à 2026 (jamais réglée) est refusée.
- Avec *Synchroniser l'horloge via Internet* désactivé, l'horloge de la console ne bouge pas, et activer le réglage ensuite ne la fait plus sauter ; s'il est activé, l'horloge de la console suit déjà l'horloge réseau, et la confirmation indique de le désactiver et de régler l'heure à la main si elle est fausse.
- Seule l'horloge est écrite, pas le contrôle parental : aucun déverrouillage n'est nécessaire (le code PIN de *Protéger PlayGuard par le code PIN* s'applique toujours) ; comme tout changement d'horloge, cela remet à zéro le temps de jeu du jour.
- Pas encore essayé sur une console.

### Jeux installés

*Vérifier les jeux installés* trouve les jeux que le menu HOME ne peut pas lancer ou afficher — une icône qui charge sans fin, un « ! », un cadre en pointillés — avec ce qui ne va pas et quoi faire, sous l'icône du jeu, et le nombre de jeux installés qui ont un problème.

**Ce qu'il trouve :** nom et icône illisibles, jeu absent derrière sa mise à jour, plus rien d'installé, fichiers manquants, données corrompues repérées par la console, firmware plus récent requis par le jeu ou seulement par sa mise à jour, clés trop récentes pour le firmware, licence (ticket) manquante, console qui réclame une mise à jour retirée, carte SD illisible, lancement refusé par la console.

**La liste a une section par solution**, chacune avec son nombre et quoi faire, les jeux au même problème regroupés (deux jeux au même nom se distinguent par leur version ou leur Title ID) :

| Section | Ce qu'elle contient |
|---|---|
| **Réparables ici** | Ce que PlayGuard répare |
| **À vérifier** | Des données dont la console doute : la vérification dit s'il faut réinstaller le jeu |
| **À réinstaller** | Fichiers manquants ou inutilisables, ou données trouvées corrompues : supprimer, puis réinstaller |
| **Firmware ou carte SD** | Le jeu va bien, la console non |

- **Vérifier les données :** le résultat de la vérification des données reste attaché au jeu tant que la même version est installée : corrompues, il passe dans *À réinstaller* en *Corrompu (vérifié)*, avec la date et le code de la console ; intactes, il quitte la liste. *Vérifier ces jeux* vérifie toute la section *À vérifier*, un jeu après l'autre dans un seul dialogue (*jeu 2 sur 5*), puis dit combien sont corrompus, intacts ou n'ont pas pu être vérifiés, avec la suppression des jeux corrompus à portée de main.
- **Plusieurs jeux à la fois :** Ⓨ sélectionne des jeux (une coche devant), et *Plusieurs jeux à la fois*, au-dessus des sections, sélectionne une section entière ; puis une seule action pour tous : vérifier leurs données, ou les supprimer (leur liste enregistrée d'abord, une seule question, un seul code PIN).
- **Sous *À réinstaller*,** leur liste peut être enregistrée (JSON et texte, dans le dossier des exports : nom, Title ID, version, problème), et tous peuvent être supprimés d'un coup après une seule question et un seul code PIN : la liste est d'abord enregistrée, puis les jeux sont supprimés pour qu'un installeur ne les compte plus comme installés.
- **Les actions :** quand une action est la solution, c'est le premier bouton du dialogue ; les autres suivent : accepter à nouveau la version installée (quand la console réclame une mise à jour retirée), supprimer seulement la mise à jour ou un contenu additionnel, vérifier si les données sont corrompues (la vérification de la console, avec sa progression et un bouton Annuler), ou supprimer le jeu.
- Les versions sont affichées comme le jeu les affiche (*version 1.4.0*, *mise à jour n°4*) ; après une réparation, un message dit si le jeu fonctionne de nouveau. Les sauvegardes restent toujours ; chaque modification figure dans l'historique.
- **Quand il vérifie :** la dernière vérification s'affiche tout de suite ; seuls les jeux dont l'entrée a changé sont ensuite revérifiés (tous une fois par jour, après une mise à jour du firmware, ou avec Ⓧ *Vérifier à nouveau*), chacun affiché dès qu'il est vérifié.
- Plusieurs jeux cassés à la fois orientent vers la carte SD ou vers un jeu installé depuis l'autre mémoire (sysMMC / emuMMC).
- *Affichage*, au-dessus des sections, présente les jeux en **liste** ou en **jaquettes** (icône, nom et problème ; le choix est gardé), comme *Tous les jeux*.

### Forwarders

*Forwarders ›*, un écran à part sous *Homebrew* — un homebrew sur le menu HOME, avec son icône et son nom, lancé de là comme un jeu (un forwarder de NRO, comme sphaira les crée).

- **Une carte en haut** dit où en est le menu homebrew : verrouillé ou ouvert, ses chemins de retour (le forwarder de PlayGuard, les menus protégés, ceux jamais lancés depuis le menu HOME), ce qui l'ouvre encore sans le code PIN, et si le module rescue est installé. Ⓐ dessus liste quoi faire, chaque action à portée d'un appui (l'étape suivante vers le verrou, protéger ou supprimer un forwarder qui l'ouvre sans code PIN, la vérification, la fiche de secours).
- **La liste par groupes :** celui de PlayGuard, ceux protégés par le code PIN, ceux sans, ceux d'une autre application, ceux qui ne sont plus sur le menu HOME. Un forwarder protégé a un cadenas sur son icône (une coche pour un menu vu en marche à travers son portail), un forwarder créé ici sans code PIN un cadenas ouvert orange.
- **Ⓨ** ouvre cette partie du guide (un QR code). Après avoir supprimé ou oublié un forwarder, ou changé ses heures sans code PIN, **ZR** le remet quelques secondes (réinstallé avec le même ID de programme, ou les heures d'avant).
- **PlayGuard** en un appui : son icône et son nom, le forwarder lançant le `.nro` depuis lequel PlayGuard a été ouvert.
- **Un autre homebrew** (confirmé avec ce que le menu HOME affichera : son icône, son nom et son éditeur) : n'importe quel `.nro` du dossier `/switch` de la carte SD (et d'un dossier en dessous), son nom demandé d'abord (celui du `.nro`, à garder ou à changer). Son icône est celle du `.nro`, convertie dans le JPEG de 256 × 256 qu'affiche le menu HOME quand elle a une autre taille ou est un PNG (une manette à sa place quand le `.nro` n'en a pas), comme le fait sphaira. Un chemin qui contient un espace est passé au homebrew entre guillemets.
- **Les forwarders du menu HOME**, chacun avec son icône : ceux que PlayGuard a créés (Ⓐ : détails, renommer, changer l'éditeur, réinstaller, supprimer) et ceux d'une autre application (détails, supprimer). Celui dont le `.nro` a disparu le dit ; celui supprimé dans les paramètres de la console reste listé jusqu'à ce qu'il soit réinstallé ou oublié. Réinstaller garde son ID de programme : l'entrée du menu HOME est remplacée, pas doublée ; un forwarder créé par un autre outil pour le même `.nro` est remplacé aussi.
- **Protégé par le code PIN** (Ⓐ › *Protéger par le code PIN*, pour un forwarder créé par PlayGuard autre que le sien) : lancé depuis le menu HOME, le forwarder ouvre d'abord une petite porte (`config/playguard/gate.nro`, quelques centaines de kilo-octets que PlayGuard écrit depuis ses propres fichiers), qui affiche aussitôt l'écran du code PIN de la console, puis le homebrew une fois le bon code PIN entré ; B y ramène au menu HOME. Autour de l'écran du code PIN, celui de la porte : l'icône et le nom du homebrew, *Protected by PlayGuard*, et l'issue (lancement, horaires libres, code PIN non saisi), dans la langue de la console, sur le bleu nuit de Cadran (`make gate-preview` le dessine sur un ordinateur). Il garde son ID de programme et son icône, la ligne indique *Code PIN*, et l'historique des modifications le note. Il faut un code PIN sur la console. *Ne plus demander le code PIN* le demande d'abord, quel que soit le réglage *Protéger PlayGuard par le code PIN*. Seule l'icône du menu HOME est protégée : le `.nro` se lance toujours depuis le menu homebrew, sauf s'il est verrouillé (plus bas). Un forwarder protégé par un PlayGuard antérieur lance PlayGuard lui-même comme porte, ce qui fonctionne toujours ; *Tout réinstaller* le fait passer à la petite porte.
  - **Horaires sans code PIN** (Ⓐ sur un forwarder protégé) : tous les jours, du lundi au vendredi ou le week-end, d'une heure à une autre (y compris après minuit) ; la ligne indique quand. Les régler demande le code PIN.
  - **Chaque tentative figure dans l'historique** : ouvert avec le code PIN, ouvert dans ses horaires, ou refusé (B sur l'écran du code PIN), à l'heure où c'est arrivé ; les tentatives d'un même type, un même jour, pour un même forwarder forment une seule entrée, avec leurs heures. La porte les note sur la carte SD ; PlayGuard les lit à son ouverture suivante. *Historique › Afficher › Homebrews et forwarders* ne montre que celles-ci, les forwarders et le verrou.
  - **L'écran de la porte en dit plus :** ses heures sans code PIN pendant qu'il le demande, et les refus du jour (en orange) pendant qu'il le demande et une fois refusé, sous *Demandez à un parent de saisir le code PIN*.
- **Icônes et langues :** une icône d'une autre taille, un PNG, ou un JPEG avec des données EXIF ou un codage progressif (le menu HOME les affiche en « ? ») devient un JPEG 256 × 256 simple. Les données de contrôle du forwarder ne déclarent que l'anglais américain, la langue de sa seule icône : l'icône s'affiche donc sur une console réglée dans n'importe quelle langue.
- **Protéger tous les forwarders par le code PIN** et **Tout réinstaller** (l'icône et la version d'un nouveau `.nro`, la porte de ce PlayGuard, un forwarder supprimé du menu HOME remis) : en un appui chacun.
- **Verrouiller le menu homebrew** (*Menu homebrew*, en bas de l'écran) : ni l'album ni un jeu lancé en maintenant R n'ouvrent plus le menu homebrew, donc un homebrew ne se lance plus que depuis son forwarder, qui peut demander le code PIN.
  - **Étape par étape :** tant qu'il ne peut pas être verrouillé, Ⓐ liste les étapes, numérotées, la suivante en orange et à un appui (*Étape suivante*) : un code PIN, les patchs de signature, le forwarder de PlayGuard, un menu homebrew protégé par le code PIN (sphaira trouvé dans `/switch`, sinon hbmenu, créé protégé d'un coup), lancé une fois depuis le menu HOME ; la liste revient après chaque étape qui crée un forwarder.
  - Il faut deux accès vers PlayGuard depuis le menu HOME : son propre forwarder, et un forwarder d'un menu homebrew (sphaira ou hbmenu, d'après le nom du `.nro`) protégé par le code PIN, qui garde tous les homebrews, boutique comprise, à un code PIN près. Il faut aussi un code PIN sur la console et les patchs de signature dont les forwarders ont besoin. D'ici là, Ⓐ dit ce qui manque.
  - L'accès par le menu ne compte qu'une fois vu en marche : lancé une fois depuis le menu HOME, le code PIN saisi à son portail (le journal du portail le dit, lu à l'ouverture de PlayGuard et au verrouillage). Un menu protégé jamais lancé ainsi est signalé comme tel.
  - Verrouiller affiche les vérifications sous forme de liste (une coche turquoise pour ce qui est prêt, un triangle orange ou corail pour ce qui ne l'est pas) et demande de maintenir Ⓐ 2 secondes sur *Verrouiller* (ou le doigt dessus), une barre se remplissant pendant ce temps ; relâché avant, rien ne change. Vient ensuite la **fiche de secours** : comment l'annuler depuis un ordinateur, ou avec le module rescue s'il est installé, et un QR code vers l'aide ([plus bas](troubleshooting.md#menu-homebrew-verrouillé)) ; *Vérifier le menu homebrew* la propose aussi.
  - Sur emuMMC (ou sur un sysMMC avec un emuMMC configuré), il prévient que le verrou est sur la carte SD, donc lu par les deux systèmes, alors que les forwarders sont propres à chacun : l'autre système n'a aucun de ces chemins de retour.
  - Tant que c'est verrouillé, supprimer le forwarder de PlayGuard, ou le dernier menu protégé (ou lui retirer son code PIN), est refusé : déverrouillez d'abord. Supprimer l'un de plusieurs menus protégés demande confirmation, en corail, en nommant ceux qui restent.
  - Avant de verrouiller, il propose de protéger d'un coup les forwarders créés par PlayGuard qui ne le sont pas encore, et indique ceux qui lanceraient encore des homebrews sans le code PIN (un menu non protégé, ou un forwarder créé par une autre application), ceux qui écrivent sur la carte SD (un explorateur de fichiers, un installeur comme Cyberfoil ou Goldleaf, un serveur FTP : ils pourraient modifier le fichier du verrou), et si uLaunch est installé (il lance les homebrews à sa façon, que le verrou n'atteint pas).
  - Le code PIN refusé trois fois ou plus le même jour au portail d'un forwarder est signalé dans la cloche (quel forwarder, quel jour, combien de fois).
  - Quand l'écriture d'un fichier échoue, l'erreur dit pourquoi quand c'est possible : la carte SD pleine, ou rien ne peut y être écrit (son verrou, ou le bit d'archive que corrige *Tools* de hekate).
  - *Vérifier le menu homebrew* montre ce qu'Atmosphère lira (si l'album et un jeu lancé avec R ouvrent le menu homebrew), les accès de secours, les patchs de signature, la version d'Atmosphère du verrouillage et le module rescue ; le rapport de diagnostic contient la même chose.
  - Ce sont quelques lignes à la fin de `atmosphere/config/override_config.ini`, entre deux commentaires de PlayGuard ; le reste du fichier n'est pas touché, et l'original est gardé dans `config/playguard/override_config.ini.backup`. Atmosphère relit le fichier à chaque lancement d'un programme : pas de redémarrage. Déverrouiller retire ces lignes et réécrit les valeurs d'Atmosphère pour les clés que le reste du fichier ne fixe pas (Atmosphère garde ce qu'il a lu jusqu'au redémarrage de la console).
  - Verrouiller et déverrouiller demandent le code PIN, quel que soit le réglage *Protéger PlayGuard par le code PIN*, et figurent dans l'historique. Tant que c'est verrouillé, PlayGuard vérifie à chaque ouverture que les deux accès sont toujours là ; la cloche signale celui qui manque. Un verrou retiré du fichier en dehors de PlayGuard (un ordinateur, un homebrew) est signalé dans la cloche et l'historique ; PlayGuard ne le remet jamais de lui-même, supprimer le fichier étant la sortie de secours depuis un ordinateur. Supprimer un forwarder (Paramètres › Gestion des données, sans code PIN) ne lève jamais le verrou : cela retire seulement un accès.
  - Depuis un ordinateur : supprimez `atmosphere/config/override_config.ini` (ou remettez la sauvegarde), puis redémarrez la console.
  - Avec le module rescue installé (facultatif, [sysmodule/README.md](../../sysmodule/README.md)), une requête `RESCUE` le lève aussi au démarrage, tout comme une mise à jour d'Atmosphère depuis le verrouillage (les patchs de signature peuvent avoir du retard, et aucun forwarder ne démarre sans eux) : PlayGuard le signale à l'ouverture suivante et propose de reverrouiller.
- **Un forwarder supprimé en dehors de PlayGuard** (dans les Paramètres) : noté une fois dans l'historique, et dans la cloche tant que ça dure, avec *Réinstaller* ou *Oublier* sur sa ligne.
- **Patchs de signature :** la console ne lance un forwarder qu'avec des patchs de signature de fs et du loader (sys-patch, ou des fichiers sigpatch) : sans eux, l'écran le dit au-dessus de la liste (*Console › Patchs de jeux* donne le détail). Le `.nro` reste où il est ; supprimer un forwarder n'y touche jamais.
- **Temps de jeu :** lancé depuis son icône, un homebrew tourne comme un jeu : le temps de jeu le compte et le suspend comme n'importe quel jeu une fois le temps écoulé, et Activité le liste. PlayGuard reste accessible depuis le menu des homebrews.
- Chaque modification demande le code PIN quand *Protéger PlayGuard par le code PIN* le prévoit, est refusée en lecture seule et figure dans l'historique. Ce que PlayGuard a créé est gardé par système (sysMMC / emuMMC), dans `forwarders.json`.

### Les lignes de la console

- **Firmware, Atmosphère et hekate** (quand son `bootloader/update.bin` est sur la carte SD), chacun avec la version plus récente trouvée par la dernière recherche de mises à jour, en orange (*22.0.0 · 23.0.1 disponible*, et le nombre de versions de retard quand il y en a plus d'une), et une icône d'avertissement sur la ligne du firmware quand le firmware le plus récent demande d'abord un Atmosphère ou un hekate plus récent, ou n'est pas encore pris en charge (les notes de version du firmware disent pourquoi, d'après celles d'Atmosphère et de hekate).
- **Notes de version :** Ⓐ sur chacun les ouvre : un onglet pour la version installée, un pour chaque version plus récente (cinq au plus) et des versions plus anciennes dans les onglets restants (six en tout ; Ⓛ / Ⓡ pour passer de l'un à l'autre), ouvert sur la suivante, avec des étiquettes (installée ou plus récente, le firmware le plus récent pris en charge, correctifs de sécurité, contrôle parental modifié), les notes telles que publiées (en anglais), *Voir en ligne* (un QR code) et leur date de récupération. Celles d'Atmosphère et de hekate viennent de leurs releases GitHub, celles du firmware de switchbrew ; elles sont gardées sur la carte SD pour une lecture hors ligne et récupérées à nouveau une fois par jour (Ⓧ pour le faire tout de suite).
- **Compatibilité**, puis **stockage** (emuMMC ou sysMMC, avec une icône d'information quand l'autre système a son propre contrôle parental : Ⓐ explique).
- Le **masquage du numéro de série** par Atmosphère (en partie caché jusqu'à Ⓐ ; en emuMMC, s'il n'est pas masqué, une icône d'avertissement sur la ligne au-dessus, que Ⓐ explique).
- **Patchs de jeux** (sys-patch ou fichiers sigpatches, avec une recommandation de sys-patch quand seuls des fichiers sont utilisés : une icône d'information, ou d'avertissement quand quelque chose est à vérifier, que Ⓐ explique).

## Préférences

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/?tab=preferences)*

- Ce qui règle PlayGuard lui-même :
  - Langue (toutes celles de la console, ou celle de la console), thème (sombre par défaut, clair, ou celui de la console), taille du texte (automatique / normale / plus grande) : les trois tout de suite.
  - L'onglet d'ouverture.
  - **L'intro** au lancement : l'écran de chargement pendant 2 secondes (activée par défaut ; Ⓐ ou Ⓑ la passe ; désactivée, PlayGuard s'ouvre dès qu'il est prêt).
- Les messages qu'il affiche :
  - Les types de notifications (voir [plus haut](#comportement-des-entrées)).
  - Un **rappel mensuel pour soutenir PlayGuard** (activé par défaut, jamais le premier mois ni juste après une mise à jour ; *Ne plus afficher* sur le rappel ou cet interrupteur le désactive pour de bon, mises à jour comprises).
- Un réglage qui ne concerne qu'une fonction est à côté d'elle : le temps en plus dans *Temps de jeu*, la réactivation dans *Code PIN*, la vérification de l'horloge dans *Console › Horloge réseau*. Les mises à jour et *Aide et diagnostic* sont dans *À propos*.

### Modifications et sauvegardes

À la fin de *Préférences* (l'onglet Historique des versions précédentes).

#### Historique des modifications

- Les 500 dernières modifications faites par PlayGuard (limites, niveau de restriction, code PIN, déverrouillages, dissociation, horloge, restaurations, mise en pause ou reprise du décompte…) et ce qu'il a vu changer en dehors de lui, regroupées par jour (*Aujourd'hui*, *Hier*, puis le jour et la date), avec l'heure et depuis où ; une limite changée pour un seul jour nomme ce jour.
- Une modification **refusée** par la console apparaît aussi, en orange, avec l'erreur de la console et ce qui était demandé (jamais remise : rien n'a changé).
- *Afficher* ne garde qu'un type (temps de jeu, restrictions, sécurité et appli, vu en dehors de PlayGuard, refusé par la console) ; *Exporter ce qui est affiché* l'écrit dans `exports/` en CSV, JSON, XLSX ou PDF.
- Ⓐ sur une modification l'affiche et, pour une valeur, **remet la précédente** — avec le même déverrouillage et le même code PIN que toute modification, en signalant si elle a changé depuis.
- Chaque modification garde la version de PlayGuard qui l'a faite ; une modification enregistrée par une version plus récente de PlayGuard apparaît comme *Autre modification*.

#### Sauvegardes

- **Sauvegarder / restaurer les réglages** sur la carte SD : niveau de restriction, réglages personnalisés, mode VR, organisme de classification, limites quotidiennes, alarme « temps écoulé » (avec les actions avancées activées), et pour mémoire le bloc brut du minuteur — jamais le code PIN.
- La restauration ne liste que ce qui changerait (rien à changer : elle le dit simplement, sans déverrouillage ni écriture), et signale quand la sauvegarde avait une alarme du coucher (elle n'est pas réécrite).
- Nombre de sauvegardes conservées au choix.

## À propos

*[L'essayer dans le simulateur ›](https://playguard.link/simulator/?tab=about)*

### Version et mises à jour

- PlayGuard installé : version (en ambre quand une plus récente est sortie ; Ⓐ ouvre ses **notes de version**), mode de lancement et dossier des données.
- **Recherche d'une mise à jour** maintenant ; *Rechercher les mises à jour chaque jour* au lancement.
- *Mettre à jour avec* : dans PlayGuard, qui télécharge la version, la vérifie, se remplace et redémarre, ou sphaira, le Homebrew App Store ou à la main (demandé à la première mise à jour si rien n'est choisi). Installé depuis le Homebrew App Store ou l'App Store de sphaira, c'est le store qui est recommandé, pour qu'il suive la version ; quand son catalogue n'a pas encore la nouvelle version (il la publie après une vérification, souvent quelques jours plus tard), PlayGuard le dit et propose de se mettre à jour lui-même.
- Tant qu'une version plus récente est connue, PlayGuard s'ouvre sur **PlayGuard X.Y.Z est disponible** (sa date et ses notes) : *Mettre à jour*, ou *Plus tard* pour 1 jour, 1 semaine ou 30 jours au plus.
- Les notes de version sont celles de PlayGuard, sur le même écran que celles d'Atmosphère et de hekate dans *Console* : depuis ses versions GitHub, un onglet par version (l'installée, les plus récentes, les plus anciennes), chacune avec ses points forts en cartes et tous ses changements, gardées sur la carte SD pour une lecture hors ligne (celles de la version installée sont aussi dans l'application, quand rien n'a encore été récupéré).
- Après une mise à jour, PlayGuard s'ouvre une fois sur **Nouveautés de la X.Y.Z** (les mêmes points forts, puis les QR codes).

### Aide et diagnostic

*Aide et diagnostic ›*, un écran à part, sous *Mettre à jour avec* :

- **Premiers pas** rouvre le guide (avec une étape *Dissocier l'application mobile* tant qu'elle est associée, une étape *Réactiver l'alarme « temps écoulé »* tant qu'elle est désactivée, une étape *Protéger PlayGuard* une fois le code PIN défini — le choix *Protéger PlayGuard par le code PIN* —, et un interrupteur pour qu'il ne s'ouvre plus au lancement). Sous *Fermer*, *Soutenir PlayGuard* affiche les QR codes de soutien.
- **Exporter un rapport de diagnostic**, ou **l'envoyer en ligne** (voir [Signaler un bug](troubleshooting.md#signaler-un-bug)) ; combien de rapports enregistrés, de fichiers de plantage, de journaux de session et de liens des rapports envoyés en ligne sont gardés, par nombre, âge ou taille.

### Crédits

Les crédits, comment **soutenir PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), affichés en QR codes à scanner avec un téléphone), et un petit *Made in France* 🇫🇷.

## Sûr par conception

**Écriture sûre de la limite.** Écraser la configuration du minuteur pendant son décompte déstabilise Atmosphère. PlayGuard vérifie donc l'état d'abord ; si le minuteur est actif, la confirmation indique que le contrôle parental sera déverrouillé temporairement — avec le code PIN enregistré, **inutile de vous en souvenir**. Un seul appui déverrouille, vérifie que le système confirme bien le déverrouillage, écrit, puis **reverrouille aussitôt** (ou le propose, si *Réactiver le contrôle parental après une modification* est désactivé). La couche service revérifie l'état juste avant d'écrire : aucun écran ne peut contourner cette protection.

> [!CAUTION]
> Une limite inférieure au temps déjà joué aujourd'hui suspend le jeu dès que le contrôle parental est reverrouillé. La confirmation le signale quand c'est le cas.

**Plus de plantage en 22.5.** `pctl:a`, le service privilégié du contrôle parental, n'accepte **qu'une seule session**. Les anciennes versions de l'application d'origine la gardaient ouverte : l'écran PIN du menu HOME (ou l'applet PIN) ne pouvait pas l'obtenir et Atmosphère pouvait planter. PlayGuard ouvre la session pour chaque action et la libère aussitôt ; les rafraîchissements périodiques s'arrêtent quand l'application est en arrière-plan. *(Diagnostic du [fork d'anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Rien ne tourne en arrière-plan.** La limite, le code PIN, les avertissements et la suspension sont ceux de la console ; PlayGuard ne change que leurs réglages.

## Ce qui compte comme temps de jeu

Le minuteur est celui de la console : PlayGuard le lit, il ne décompte rien lui-même. Ce que l'on sait, et avec quelle certitude ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), en anglais, donne les mesures) :

| Situation | Décompté ? | Certitude |
|---|---|---|
| Un jeu en cours | Oui | Mesuré (22.0.0) |
| **Un jeu laissé ouvert sur le menu HOME** (suspendu en arrière-plan) | **Oui** : fermez-le, ou mettez la console en veille | Rapporté par des parents ([Arqade](https://gaming.stackexchange.com/questions/398954/)) ; cohérent avec les mesures (la console décompte tant qu'une application est ouverte) |
| Mode veille | Non | Rapporté ([Arqade](https://gaming.stackexchange.com/questions/398954/)) ; pas mesuré par PlayGuard |
| **PlayGuard ouvert par-dessus un jeu** (en application) | **Oui**, comme ce jeu | Mesuré (22.0.0) : ouvrez-le avec un utilisateur parent, ou depuis l'album pour un coup d'œil |
| PlayGuard ouvert depuis l'album (mode applet) | Non | Déduit : ce n'est pas une application, et le temps hors des applications ne semblait pas décompté |
| Pendant un déverrouillage temporaire du contrôle parental | Peut-être pas | Pas mesuré : question ouverte |
| Après un changement d'horloge | La journée repart de zéro ; après un retour en arrière, le décompte s'arrête jusqu'à ce que l'horloge dépasse l'ancienne heure | Observé (22.0.0) ; *Aujourd'hui* indique quand la console ne décompte pas |

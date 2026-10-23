<!-- translated from docs/hardware-test.md, sha256 5341a83cb2d1 -->
# Tester sur une console : forwarders et menu homebrew

Ce que les simulateurs desktop et web ne peuvent pas montrer : le vrai menu HOME de la console, son écran du code PIN, Atmosphère qui lit `override_config.ini`, le sysmodule de récupération au démarrage. À dérouler sur une console après une modification de *Console › Forwarders* (les fichiers `source/action/forwarder*`, `hbmenu_lock*`, `source/core/gate*`, `hbl_lock*`, `gate/`, `sysmodule/`), en notant dans la pull request ce qui a été vérifié, et sur quels firmware et Atmosphère.

Avant de commencer : un code PIN sur la console, sys-patch (ou des fichiers sigpatch), sphaira dans `/switch/sphaira/`, un ordinateur avec un lecteur de carte SD à portée de main, et de préférence le sysmodule de récupération installé (`playguard-rescue.zip`). Utilisez une console que vous pouvez remettre dans son état d'origine.

D'abord, *Aide et diagnostic › Auto-test de la console* (mode développeur, [dépannage](troubleshooting.md#mode-développeur-examiner-un-nouveau-firmware)) : il vérifie ce que PlayGuard peut vérifier seul et écrit `config/playguard/self-test.md` ; collez-le dans la pull request. Les étapes ci-dessous sont ce qu'il ne peut pas vérifier : elles demandent une main sur la console.

Chaque étape : ce qu'il faut faire, puis ce qui doit se passer. ☐ est à cocher.

## 1. Forwarders

- ☐ *Forwarders › PlayGuard* : sur le menu HOME avec l'icône de PlayGuard ; il lance PlayGuard.
- ☐ *Un autre homebrew › sphaira* : l'aperçu montre l'icône et le nom de sphaira ; une fois ajouté, le menu HOME a la même icône et le même nom, et il lance sphaira.
- ☐ Un homebrew dont le `.nro` a une icône PNG, ou d'une autre taille : son icône sur le menu HOME (pas un « ? »).
- ☐ La console dans une autre langue (Paramètres de la console › Console › Langue) : les icônes restent.
- ☐ *Changer l'icône* sur l'un d'eux : la grille montre aussitôt les captures de l'album, leurs vignettes arrivant au fur et à mesure, chacune avec le jeu où elle a été prise (caps:a ; *Prise hors d'un jeu* pour celles du menu HOME). Une capture déplacée avec R, puis A : le menu HOME montre le carré qu'affichait *Après*.
- ☐ *Supprimer*, puis ZR dans les 8 secondes : de retour sur le menu HOME, avec la même icône.
- ☐ Ⓐ sur un forwarder : la fiche par-dessus la liste, son icône et son état en haut, ses sections ; un toucher sur l'écran en dehors la ferme, comme B. Une longue liste (*Pause après des codes PIN refusés*, la limite d'un jour dans *Temps de jeu*) défile au stick et au doigt, la valeur réglée cochée.
- ☐ *Détails* sur un forwarder protégé : une ligne par règle avec son icône ; Ⓐ sur *Horaires sans code PIN* ouvre ses horaires, sur *Éditeur* son clavier.

## 2. La porte du code PIN

- ☐ *Demander le code PIN* activé pour sphaira, puis le lancer depuis le menu HOME : l'écran de la porte (bleu nuit, *Protected by PlayGuard*, l'icône et le nom de sphaira, dans la langue de la console), puis l'écran du code PIN de la console.
- ☐ L'interrupteur désactivé, puis ZR dans les 8 secondes : de nouveau protégé (le code PIN demandé d'abord pour le désactiver).
- ☐ Le code PIN saisi : sphaira se lance. Dans PlayGuard : l'*Historique* a *Forwarder ouvert avec le code PIN*, et l'icône de sphaira porte une coche.
- ☐ B sur l'écran du code PIN : retour au menu HOME, la porte disant *Code PIN non saisi*, puis *Refus aujourd'hui : 1* ; encore une fois : *Refus aujourd'hui : 2* pendant qu'il le demande.
- ☐ Refusé cinq fois de suite (la pause par défaut) : la cinquième dit *Il redemandera à …* ; relancé avant, il dit qu'il est en pause et n'affiche jamais l'écran du code PIN. Dans PlayGuard : la cloche signale la pause ; *Lever la pause* (code PIN demandé) et il le redemande aussitôt.
- ☐ *Heures sans code PIN* incluant l'heure actuelle : il se lance sans code PIN, la porte le disant ; l'écran de la porte nomme les heures et montre un cadenas ouvert après *Protected by PlayGuard*, la ligne de sphaira dans PlayGuard la même icône, *Détails* les heures.
- ☐ *Heures où il ne s'ouvre jamais* incluant l'heure actuelle : la porte dit *Pas à cette heure-ci* et les heures, même avec un code PIN sur la console ; en dehors, le code PIN de nouveau.
- ☐ *Pause après des codes PIN refusés › Jamais* : refusé dix fois, pas de pause.
- ☐ *Heures sans code PIN › Du lundi au vendredi, et le week-end à part* : un jour de semaine dans sa plage, pas de code PIN ; le week-end, la plage du week-end.
- ☐ *Fermé une fois le temps de jeu du jour écoulé* activé, une minuterie sans temps restant aujourd'hui : la porte dit *Le temps de jeu du jour est écoulé*, même avec le code PIN saisi ; du temps ajouté (PlayGuard ou l'appli du parent), il redemande le code PIN.
- ☐ Depuis un ordinateur, `config/playguard/gate_rules.txt` écrit avec une ligne donnant toutes les heures à sphaira : la porte demande toujours le code PIN (ses règles sont dans le forwarder).
- ☐ Depuis un ordinateur, `config/playguard/gate-….nro` remplacé par un autre `.nro` (celui de hbmenu) : le forwarder de sphaira s'arrête sur l'erreur 2347-0100 et ne le lance pas ; l'ouverture suivante de PlayGuard réécrit la porte.
- ☐ Un forwarder protégé par le PlayGuard précédent : la cloche dit de le refaire ; *Refaire*, puis ses heures sans code PIN fonctionnent.

## 3. Verrouiller le menu homebrew

- ☐ *Verrouiller le menu homebrew* avant d'avoir lancé sphaira à travers sa porte : les étapes, la dernière demandant de le lancer depuis le menu HOME.
- ☐ Les étapes faites, *Verrouiller* : un appui bref sur A ne fait rien ; A maintenu 2 secondes remplit le bouton et verrouille ; le doigt maintenu dessus fait de même.
- ☐ La fiche de secours ; sur un ordinateur, `config/playguard/rescue.html` s'ouvre et s'imprime, son QR code lisible par un téléphone.
- ☐ L'album : il n'ouvre plus le menu homebrew (l'album lui-même s'ouvre). Un jeu lancé en maintenant R : le jeu, pas le menu homebrew. **Pas de redémarrage nécessaire.**
- ☐ sphaira depuis le menu HOME, le code PIN saisi : sphaira s'ouvre, et lance n'importe quel homebrew (PlayGuard compris).
- ☐ Verrouillé, *Supprimer* sur sphaira (le seul menu protégé) ou sur le forwarder de PlayGuard : refusé, *Déverrouillez d'abord*.
- ☐ *Déverrouiller* : l'album rouvre le menu homebrew aussitôt, R sur un jeu aussi.

## 4. Chemins de retour

- ☐ Verrouillé, le forwarder de sphaira supprimé dans Paramètres › Gestion des données : à l'ouverture suivante, la cloche dit qu'un chemin de retour manque ; le verrou reste.
- ☐ Depuis un ordinateur : `atmosphere/config/override_config.ini` supprimé, la console redémarrée : l'album ouvre le menu homebrew ; l'ouverture suivante de PlayGuard signale que le verrou a été retiré (cloche, historique).
- ☐ Avec le sysmodule de récupération : reverrouillé, un fichier vide `config/playguard/RESCUE`, un redémarrage : déverrouillé au démarrage, PlayGuard le signalant à son ouverture suivante.
- ☐ Atmosphère mis à jour pendant le verrou (sysmodule de récupération installé) : déverrouillé au démarrage, PlayGuard le signalant.
- ☐ Avec un emuMMC : verrouiller depuis un système prévient que le verrou est sur la carte SD ; démarré sur l'autre, l'album n'y ouvre plus non plus le menu homebrew, et ce système n'a aucun des forwarders.

## 5. Sauvegardes

- ☐ *Historique › Essais de la semaine* : les refus de la section 2, par jour.
- ☐ *Historique › Faire une sauvegarde* avec des forwarders créés : le fichier (`config/playguard/backups/`) a une partie `forwarders`.
- ☐ Un forwarder supprimé dans les Paramètres, la sauvegarde restaurée : PlayGuard propose de le remettre ; une fois remis, il est sur le menu HOME, protégé comme avant.

## Ensuite

Déverrouillez le menu homebrew, supprimez les forwarders créés pour le test (ou gardez-les), et dites dans la pull request quelles étapes ont été vérifiées et lesquelles ne l'ont pas été.

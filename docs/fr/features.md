# Fonctions

Ce que fait chaque onglet, comment PlayGuard écrit sans risque, et ce que la console compte comme temps de jeu. Retour au [README](../../README.fr.md).

| Temps de jeu | Limites par jour |
|---|---|
| ![Temps de jeu](../../images/screenshots/play_timer.png) | ![Limites par jour](../../images/screenshots/per_day.png) |
| **Activité** | **Un jeu** |
| ![Activité](../../images/screenshots/activity.png) | ![Un jeu](../../images/screenshots/activity_game.png) |
| **Restrictions** | **Horloge réseau** |
| ![Restrictions](../../images/screenshots/restrictions.png) | ![Horloge réseau](../../images/screenshots/clock.png) |
| **Sécurité et appli** | **Préférences** |
| ![Sécurité et appli](../../images/screenshots/security.png) | ![Préférences](../../images/screenshots/preferences.png) |

## Commandes

↑/↓ déplacer · Ⓐ valider · Ⓑ retour ou annuler (deux fois sur la barre latérale pour quitter) · Ⓧ actualiser · **+** enregistre les limites par jour.

Ce qui a échoué est dit dans une boîte de dialogue, ce qui a réussi dans une notification. Le titre indique quand l'application est en lecture seule ou tant que le contrôle parental est déverrouillé temporairement ; en lecture seule, les actions restent à leur place, grisées, et disent pourquoi quand on les choisit.

L'application est organisée en onglets, comme les paramètres de la console ; chacun a sa section ci-dessous.

## Vue d'ensemble — la journée en un coup d'œil

- **D'abord la journée :** jauge du temps de jeu (quand aucun jeu ne tourne, le temps lu dans le journal d'activité, marqué ≈), limite du jour, temps restant, alarme du coucher. L'alarme « temps écoulé » apparaît en orange tant qu'elle est désactivée — Ⓐ la réactive. Sous la jauge, un rappel : un jeu laissé ouvert sur le menu HOME continue d'être décompté (voir [Ce qui compte comme temps de jeu](#ce-qui-compte-comme-temps-de-jeu)).
- **La console décompte-t-elle ?** Le temps de PlayGuard lui-même compte quand il tourne comme une application : avec une limite aujourd'hui, le temps restant doit donc baisser tant qu'il est ouvert. S'il n'a pas bougé pendant environ 90 s à l'écran (minuteur actif, limite aujourd'hui, pas de déverrouillage, pas d'ouverture depuis l'album), une ligne orange indique *La console ne décompte pas le temps de jeu en ce moment*, avec les causes probables : l'horloge a été reculée (le décompte reprendra plus tard), l'horloge réseau n'a jamais été réglée (onglet *Horloge réseau*), ou une limite vient d'être écrite (lancez un jeu une minute, puis vérifiez). Elle disparaît dès que le temps restant bouge de nouveau. La ligne d'état de Temps de jeu l'indique aussi.
- **Modifié en dehors de PlayGuard :** le temps de jeu du jour reparti de zéro le même jour (un changement d'horloge fait cela), l'horloge de la console déplacée de plus de 5 minutes, des limites différentes de celles que PlayGuard a vues en dernier (l'application mobile, un autre outil). Un avis orange jusqu'à ce qu'il soit masqué ou jusqu'au lendemain, et une entrée dans l'historique des modifications. PlayGuard ne tourne pas en arrière-plan : il ne voit que ce qui a changé pendant qu'il était ouvert, ou depuis sa dernière ouverture, quand il relit la console.
- **À la limite**, la console affiche « temps écoulé », puis suspend le jeu (sans « continuer ») ou se contente de prévenir : *Quand le temps est écoulé* indique lequel et permet de le changer, une fois une limite ou une heure du coucher réglée ([détails, en anglais](../parental-controls.md#when-the-time-runs-out)).
- **Puis l'état :** contrôle parental, code PIN (*Défini* ou *Non défini*, jamais sa longueur), niveau de restriction.
- **Puis ce qui demande attention :** précision de l'horloge réseau, association de l'application mobile (en orange tant qu'elle est associée), firmware / compatibilité seulement en cas de problème, avertissements sur le masquage du numéro de série et les patchs de jeux.
- Les lignes qui ouvrent un autre onglet se terminent par un chevron (›). Ⓐ sur *Limite d'aujourd'hui* la modifie sur place ; Ⓐ sur *Horloge réseau › Imprécise* mesure et règle l'horloge directement.
- **Temps en plus aujourd'hui** (+15 min, +30 min, +1 h par défaut) et **Arrêter le jeu pour aujourd'hui** (limite à 0 pour aujourd'hui seulement, *maintenant* ou *dans 5 minutes* quand le temps joué est connu : la limite devient alors le temps joué + 5 min, le temps de sauvegarder ; au reverrouillage, le « temps écoulé » de la console apparaît, et la progression non sauvegardée peut être perdue — la confirmation le signale). Le lendemain — au lancement, ou à minuit si l'application est ouverte — PlayGuard propose de remettre la limite habituelle, ou le fait de lui-même (*Préférences › Remettre d'office la limite habituelle le lendemain*).
- Un bandeau **Verrouiller maintenant** tant que le contrôle parental est déverrouillé temporairement.
- Tant qu'aucun code PIN n'est défini, une ligne **Premiers pas** ouvre le guide en trois étapes, qui s'affiche aussi au lancement.
- Actualisation toutes les 5 s, avec l'heure de la dernière actualisation (Ⓧ pour actualiser tout de suite).

## Temps de jeu — le graphique de la semaine sert d'éditeur

- Une ligne d'état : actif, temps restant, limite du jour, profil correspondant (en rouge tant que la limite est atteinte).
- **Le graphique de la semaine :** ←/→ choisit un jour, Ⓐ change sa limite.
- **Même limite tous les jours :** liste rapide ou valeur libre, en minutes (`90`, `90 min`) ou en heures (`1:30`, `1h30`).
- **Limite différente selon le jour :** jours non enregistrés en orange et marqués \*, valeurs rapides, préréglages lundi–vendredi / week-end, « pas de limite » par jour ; **+** enregistre depuis n'importe où.
- **Suppression de la limite**, **temps en plus aujourd'hui**, **arrêter le jeu pour aujourd'hui** (comme dans la Vue d'ensemble).
- Une note sous la limite quotidienne dit ce que fait la console à la limite (comme dans la Vue d'ensemble). Pendant un déverrouillage temporaire du contrôle parental, le temps de jeu n'est *peut-être* pas décompté : la question reste ouverte ([docs, en anglais](../parental-controls.md#still-open)).
- **Profils** enregistrés sur la carte SD : appliquer, modifier, renommer ou supprimer un profil ; enregistrer les limites actuelles ou en créer un nouveau. N'importe quel nom, dans n'importe quelle écriture (*École*, *周末*, *Выходные* …) — un nom sans lettres latines reçoit son propre nom de fichier, et deux noms qui donneraient le même fichier sont repérés.
- Chaque confirmation dessine la semaine telle qu'elle sera, les jours qui changent en orange.
- **Alarme du coucher** : l'heure de l'alarme (16:00 à 23:45, ou désactivée) et l'heure où le jeu est de nouveau permis (05:00 à 09:00), la même tous les jours. Sa place dans les réglages du minuteur a été déduite des réglages de l'application mobile, pas lue sur une console où une heure du coucher était réglée : PlayGuard ne la modifie que si la console indique ce qu'il y lit, vérifie la réponse de la console après la modification et remet les réglages précédents si elle diffère. Avancé, sur activation : alarme « temps écoulé », pause / reprise du décompte.
- `0` minute signifie *pas de jeu ce jour-là* ; *Supprimer la limite de temps de jeu* désactive le minuteur.

## Activité — qui a joué à quoi, et combien de temps

- Temps passé sur chaque jeu **aujourd'hui**, sur les **7 derniers jours** et **depuis toujours**, lu dans le journal d'activité de la console — pour tous les comptes ou **un seul compte utilisateur** (*Compte*, quand la console en a plusieurs).
- Un graphique des sept derniers jours (aujourd'hui à droite, une légende dessous), avec la limite de cette semaine pour chaque jour en trait et le temps au-delà en orange, marqué ! ; totaux du jour, de la semaine et depuis toujours.
- Un **résumé** sur la période choisie, pour le compte affiché : moyenne par jour, le jeu le plus joué, les jours joués et le plus chargé (7 derniers jours), la session moyenne (depuis toujours).
- **À redécouvrir** : les jeux installés, à moins de 3 h au total et pas lancés depuis un mois ou plus, les moins joués d'abord (jusqu'à trois).
- Tri par période ; les premiers jeux affichent leur icône (pas en mode applet, pour économiser la mémoire). Une grande bibliothèque affiche ses 50 premiers jeux, puis *Afficher tous les jeux*, gardé jusqu'à une autre période ou un autre compte (100 jeux au plus en mode applet) ; l'export les contient toujours tous. PlayGuard lit 256 jeux au plus (ceux de la semaine d'abord) : au-delà, la liste et l'export (sauf CSV) signalent qu'il en manque, et le total depuis toujours ne les compte pas.
- Ⓐ sur un jeu : ses sept derniers jours en barres, ses lancements, ses première et dernière parties, le temps de chaque compte. Les jeux supprimés gardent leurs chiffres depuis toujours, même un jeu supprimé avant l'installation de PlayGuard tant que le journal de jeu de la console le cite encore, et leur nom dès que PlayGuard l'a vu (sinon *Jeu supprimé* et son ID de titre).
- **Temps sans compte et nouveaux comptes :** le temps d'un jeu qu'aucun compte utilisateur ne porte (certains jeux n'en demandent pas, ou le compte a été supprimé depuis) apparaît en *Sans compte*, à partir d'une minute ; un compte utilisateur que PlayGuard n'avait jamais vu est signalé en orange sous *Compte* pendant 30 jours (*Nouveau compte depuis le …* : créer un compte est une astuce connue pour repartir de zéro) et noté dans l'historique des modifications ; les comptes présents quand PlayGuard les liste pour la première fois ne sont pas signalés.
- **Export sur la carte SD** en CSV, JSON, XLSX (Excel) ou PDF, une colonne par jour.
- S'ouvre aussitôt sur les derniers chiffres lus (gardés sur la carte SD d'un lancement à l'autre), actualisés en arrière-plan au-delà d'une minute ; Ⓧ les relit tout de suite, avec un indicateur de chargement.
- Durées approximatives si l'horloge de la console a été modifiée.

## Restrictions — niveau, classification par âge, communication

- Niveau de restriction : Aucun, Jeune enfant, Enfant, Adolescent, Personnalisé. Ce qu'impose un préréglage (limite d'âge, publications, communication) est affiché en lecture seule, et dit avant de le choisir.
- En Personnalisé : classification par âge, publications sur les réseaux sociaux, communication avec d'autres joueurs.
- Mode VR, **organisme de classification** (PEGI, ESRB, USK, CERO…).
- Le nombre de jeux autorisés à communiquer (la liste elle-même figure, brute, dans le rapport de diagnostic tant que sa structure n'est pas connue).

## Horloge réseau — l'horloge dont dépend le minuteur

- Horloges console et réseau, fuseau horaire, précision.
- Choix d'un serveur NTP public (≈ 50 intégrés, par région, ou le vôtre).
- **Mesure** sur 3 serveurs (médiane ; alerte orange en cas de désaccord), puis **réglage de l'horloge réseau** — une mesure reste utilisable 2 minutes, avec un décompte. La confirmation le dit : changer l'horloge remet à zéro le temps de jeu du jour, toute la limite redevient donc disponible.
- Une console qui n'atteint jamais les serveurs de Nintendo garde cette horloge imprécise, ce qui fausse le minuteur.
- **Aucun serveur ne répond** (console sans Internet) : *Régler l'horloge réseau sur l'horloge de la console* apparaît sous le résultat (et depuis la ligne *Imprécise* de la Vue d'ensemble, quand sa mesure échoue), derrière la même confirmation. PlayGuard n'a rien d'autre pour comparer l'horloge de la console : la confirmation affiche son heure et demande de la vérifier d'abord avec un téléphone ou une montre, et une horloge de console antérieure à 2026 (jamais réglée) est refusée. Avec *Synchroniser l'horloge via Internet* désactivé, l'horloge de la console ne bouge pas, et activer le réglage ensuite ne la fait plus sauter ; s'il est activé, l'horloge de la console suit déjà l'horloge réseau, et la confirmation indique de le désactiver et de régler l'heure à la main si elle est fausse. Seule l'horloge est écrite, pas le contrôle parental : aucun déverrouillage n'est nécessaire (le code PIN de *Demander le code PIN* s'applique toujours) ; comme tout changement d'horloge, cela remet à zéro le temps de jeu du jour. Pas encore essayé sur une console.

## Sécurité et appli — code PIN, verrous, application mobile

- **Définir ou changer le code PIN** (écran système), **afficher le code PIN** (après un avertissement et le code PIN lui-même, demandé à chaque fois ; noté dans l'historique, puis un nouveau code PIN est proposé), **déverrouiller temporairement**, **verrouiller maintenant**.
- **Demander le code PIN** dans PlayGuard lui-même : *Jamais*, *Avant une modification* (par défaut : tout le monde peut regarder, seul le parent modifie ; redemandé après 5 min) ou *Pour ouvrir PlayGuard* (un écran verrouillé au lancement, et de nouveau quand PlayGuard revient après 5 min ou plus hors de l'écran). Vérifié dans la couche service, donc aucune modification n'y échappe ; reverrouiller ne le demande jamais. *Afficher le code PIN* le demande à chaque fois, dans tous les modes et même pendant les 5 minutes, et un `config.json` absent ou abîmé compte comme *Avant une modification*. Quitter alors que le contrôle parental est encore déverrouillé propose de le reverrouiller ; fermé depuis le menu HOME à la place, PlayGuard le reverrouille à son prochain lancement.
- **Verrou de console :** un interrupteur qui met la limite de chaque jour à 0, donc un code PIN est nécessaire pour lancer un jeu — un verrou léger, sans classification par âge ni limite de communication. Il bloque le lancement des jeux, pas le menu HOME, et nécessite un code PIN. Les limites précédentes reviennent quand on le désactive. Tant qu'il est activé, *temps en plus* et *arrêter le jeu pour aujourd'hui* sont refusés, et des limites réglées autrement (un profil, une sauvegarde, l'historique…) le remplacent.
- **Application mobile :** association de l'application Contrôle parental Nintendo Switch, dernière synchronisation, et **dissociation** (sinon sa prochaine synchronisation écrase les limites réglées ici : tant qu'elle est associée, chaque modification que PlayGuard fait confirmer le rappelle). Tant qu'elle est associée, *Avant de dissocier : aidez à décoder…* ouvre la comparaison du bloc (voir [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), en anglais).
- **Supprimer tout le contrôle parental :** deux confirmations, irréversible ; une sauvegarde des réglages est d'abord enregistrée.

## Préférences

- Langue (toutes celles de la console, ou celle de la console) et thème (clair / sombre, avec proposition de relancer).
- L'onglet d'ouverture.
- **Reverrouiller automatiquement après une modification** (activé par défaut).
- Montants du temps en plus (+15/+30/+1 h, +10/+20/+30 min…) et remise de la limite habituelle le lendemain sans demander.
- **L'intro** au lancement : l'écran de chargement pendant 2 secondes (activée par défaut ; Ⓐ ou Ⓑ la passe ; désactivée, PlayGuard s'ouvre dès qu'il est prêt).
- Vérification de l'horloge réseau au lancement (une notification si elle a plus d'une minute d'écart ; elle ne règle jamais l'horloge).
- Un **rappel mensuel pour soutenir PlayGuard** (activé par défaut, jamais le premier mois ni juste après une mise à jour ; *Ne plus afficher* sur le rappel ou cet interrupteur le désactive pour de bon, mises à jour comprises).
- Actions avancées.

## Outils et À propos — historique, sauvegardes, infos console ; version, mises à jour, nouveautés, crédits

- **Historique des modifications :** ce que PlayGuard a changé (limites, niveau de restriction, code PIN, déverrouillages, dissociation, horloge, restaurations…), quand (*Aujourd'hui*, *Hier* ou la date) et depuis où ; une limite changée pour un seul jour nomme ce jour. Ⓐ sur une modification l'affiche et, pour une valeur, **remet la précédente** — avec le même déverrouillage et le même code PIN que toute modification, en signalant si elle a changé depuis. Une modification enregistrée par une version plus récente de PlayGuard apparaît comme *Autre modification*.
- **Sauvegarder / restaurer les réglages** sur la carte SD : niveau de restriction, réglages personnalisés, mode VR, organisme de classification, limites quotidiennes, alarme « temps écoulé » (avec les actions avancées activées), et pour mémoire le bloc brut du minuteur — jamais le code PIN. La restauration ne liste que ce qui changerait (rien à changer : elle le dit simplement, sans déverrouillage ni écriture), et signale quand la sauvegarde avait une alarme du coucher (elle n'est pas réécrite). Nombre de sauvegardes conservées au choix.
- **Premiers pas** rouvre le guide (avec une étape *Dissocier l'application mobile* tant qu'elle est associée, une étape *Réactiver l'alarme « temps écoulé »* tant qu'elle est désactivée, une étape *Protéger PlayGuard* une fois le code PIN défini — le choix *Demander le code PIN* —, et un interrupteur pour qu'il ne s'ouvre plus au lancement). Sous *Fermer*, *Soutenir PlayGuard* affiche les QR codes de soutien.
- **Exporter un rapport de diagnostic**, ou **l'envoyer en ligne** (voir [Signaler un bug](troubleshooting.md#signaler-un-bug)) ; combien de rapports enregistrés, de fichiers de plantage et de liens des rapports envoyés en ligne sont gardés, par nombre, âge ou taille.
- **Console :** firmware, Atmosphère, compatibilité, stockage (emuMMC ou sysMMC, avec une note quand l'autre système a son propre contrôle parental), **masquage du numéro de série** par Atmosphère (en partie caché jusqu'à Ⓐ ; avertissement en emuMMC s'il n'est pas masqué), **patchs de jeux** (sys-patch ou fichiers sigpatches, avec une recommandation de sys-patch quand seuls des fichiers sont utilisés).
- **À propos** (son propre onglet) : version, mode de lancement et dossier des données ; **mises à jour** (recherche maintenant ou une fois par jour au lancement ; *Mettre à jour avec* : dans PlayGuard, qui télécharge la version, la vérifie, se remplace et redémarre, ou sphaira, le Homebrew App Store ou à la main ; demandé à la première mise à jour si rien n'est choisi). Tant qu'une version plus récente est connue, PlayGuard s'ouvre sur **PlayGuard X.Y.Z est disponible** (sa date et ses notes) : *Mettre à jour*, ou *Plus tard* pour 1 jour, 1 semaine ou 30 jours au plus ; les **nouveautés** de la version installée (ses points forts, en anglais, et *Liste complète des changements* pour son entrée du changelog intégré) ; les crédits, comment **soutenir PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), affichés en QR codes à scanner avec un téléphone), et un petit *Made in France* 🇫🇷. Après une mise à jour, PlayGuard s'ouvre une fois sur **Nouveautés de la X.Y.Z** (les mêmes points forts, puis les QR codes).

## Sûr par conception

**Écriture sûre de la limite.** Écraser la configuration du minuteur pendant son décompte déstabilise Atmosphère. PlayGuard vérifie donc l'état d'abord ; si le minuteur est actif, la confirmation indique que le contrôle parental sera déverrouillé temporairement — avec le code PIN enregistré, **inutile de vous en souvenir**. Un seul appui déverrouille, vérifie que le système confirme bien le déverrouillage, écrit, puis **reverrouille aussitôt** (ou le propose, si *Reverrouiller automatiquement après une modification* est désactivé). La couche service revérifie l'état juste avant d'écrire : aucun écran ne peut contourner cette protection.

> [!CAUTION]
> Une limite inférieure au temps déjà joué aujourd'hui suspend le jeu dès que le contrôle parental est reverrouillé. La confirmation le signale quand c'est le cas.

**Plus de plantage en 22.5.** `pctl:a`, le service privilégié du contrôle parental, n'accepte **qu'une seule session**. Les anciennes versions de l'application d'origine la gardaient ouverte : l'écran PIN du menu HOME (ou l'applet PIN) ne pouvait pas l'obtenir et Atmosphère pouvait planter. PlayGuard ouvre la session pour chaque action et la libère aussitôt ; les rafraîchissements périodiques s'arrêtent quand l'application est en arrière-plan. *(Diagnostic du [fork d'anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Rien ne tourne en arrière-plan.** La limite, le code PIN, les avertissements et la suspension sont ceux de la console ; PlayGuard ne change que leurs réglages.

## Ce qui compte comme temps de jeu

Le minuteur est celui de la console : PlayGuard le lit, il ne décompte rien lui-même. Ce que l'on sait, et avec quelle certitude ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), en anglais, donne les mesures) :

- Un jeu en cours — Oui *(mesuré (22.0.0))*
- **Un jeu laissé ouvert sur le menu HOME** (suspendu en arrière-plan) — **Oui** : fermez-le, ou mettez la console en veille *(rapporté par des parents ([Arqade](https://gaming.stackexchange.com/questions/398954/)) ; cohérent avec les mesures (la console décompte tant qu'une application est ouverte))*
- Mode veille — Non *(rapporté ([Arqade](https://gaming.stackexchange.com/questions/398954/)) ; pas mesuré par PlayGuard)*
- **PlayGuard ouvert par-dessus un jeu** (en application) — **Oui**, comme ce jeu *(mesuré (22.0.0) : ouvrez-le avec un utilisateur parent, ou depuis l'album pour un coup d'œil)*
- PlayGuard ouvert depuis l'album (mode applet) — Non *(déduit : ce n'est pas une application, et le temps hors des applications ne semblait pas décompté)*
- Pendant un déverrouillage temporaire du contrôle parental — Peut-être pas *(pas mesuré : question ouverte)*
- Après un changement d'horloge — La journée repart de zéro ; après un retour en arrière, le décompte s'arrête jusqu'à ce que l'horloge dépasse l'ancienne heure *(observé (22.0.0) ; la Vue d'ensemble indique quand la console ne décompte pas)*

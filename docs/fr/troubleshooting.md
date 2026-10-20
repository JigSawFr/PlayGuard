<!-- translated from docs/troubleshooting.md, sha256 86382aa8effc -->
# Dépannage

Reprendre la main sur une console bloquée, les problèmes courants, signaler un bug, et le mode développeur. Retour au [README](../../README.fr.md).

## Console bloquée ?

PlayGuard ne voit que le contrôle parental du système sur lequel il tourne : **l'emuMMC et la sysMMC ont chacune le leur** (un code PIN supprimé sur l'une est toujours là sur l'autre). Lancez-le sur chacune de celles à corriger.

### Console d'occasion

Vous connaissez le code PIN, mais l'application mobile de l'ancien propriétaire est toujours associée (la dissociation échoue, ou la réinitialisation demande son compte).

*Code PIN › Dissocier l'application mobile*, puis, si vous ne voulez plus du tout de contrôle parental, *Supprimer tout le contrôle parental*. Les deux fonctionnent hors ligne, en emuMMC comme en sysMMC.

### Code PIN oublié

*Afficher le code PIN* demande lui-même le code PIN : il ne peut pas aider ici. Avec le module système de récupération installé (ci-dessous), déposez un fichier `RESCUE` et redémarrez : l'écran de récupération en définit alors un nouveau (il n'affiche jamais l'ancien). Sans lui, *Supprimer tout le contrôle parental* permet de repartir de zéro (une sauvegarde des réglages est d'abord enregistrée ; elle ne contient jamais le code PIN) : avec *Protéger PlayGuard par le code PIN* actif (par défaut, *Avant une modification*), mettez d'abord la carte SD dans un ordinateur et passez `"pin_lock"` à `"off"` dans `sd:/config/playguard/config.json`.

### Le minuteur bloque tout (limite à 0 min) et le code PIN est oublié

PlayGuard lui-même ne peut pas démarrer. Installez **à l'avance** le module système de récupération optionnel (`playguard-rescue.zip`) ; une fois bloqué, déposez un fichier vide `config/playguard/RESCUE` sur la carte SD et démarrez — il déverrouille la console pour que PlayGuard puisse s'ouvrir. Voir [`sysmodule/README.md`](../../sysmodule/README.md) (en anglais).

Le module doit suivre la version de PlayGuard : à chaque mise à jour de PlayGuard, extrayez à nouveau le `playguard-rescue.zip` de cette version (ou *Mettre à jour* sur l'entrée *Le sysmodule de récupération n'est pas à jour* de la cloche), puis redémarrez la console. Un module plus ancien laisse un rapport que PlayGuard ne peut pas vérifier : l'écran de récupération ne propose alors rien, dit que le module est plus ancien, et refaire `RESCUE` donne le même résultat tant qu'il n'est pas mis à jour.

### Console non modifiée

PlayGuard ne peut rien faire : il nécessite Atmosphère. La procédure officielle passe par la clé maîtresse du service client de Nintendo.

## Problèmes courants

### Le temps restant ne baisse pas

*Aujourd'hui* indique *La console ne décompte pas le temps de jeu en ce moment* quand il n'a pas bougé pendant environ 90 s. Les causes probables :

- **L'horloge réseau n'a jamais été réglée.** *Console › Horloge réseau › Mesurer l'écart*, puis réglez-la ([horloge réseau](features.md#horloge-réseau)).
- **L'horloge a été reculée.** Après un retour en arrière, le décompte s'arrête jusqu'à ce que l'horloge dépasse l'ancienne heure ; rien à faire.
- **Une limite vient d'être écrite.** Lancez un jeu une minute, puis vérifiez de nouveau.
- **Le contrôle parental est déverrouillé temporairement.** Le temps de jeu n'est peut-être pas décompté pendant ce temps : *Code PIN › Réactiver le contrôle parental*.

### Mes limites ont changé toutes seules

- **L'application mobile est encore associée.** Sa prochaine synchronisation écrase ce que PlayGuard a réglé : *Code PIN › Dissocier l'application mobile*.
- **Un autre outil, ou un profil.** *Aujourd'hui* affiche une carte orange pour des limites différentes de celles que PlayGuard a vues en dernier, et *Historique › Historique des modifications* liste chaque modification vue en dehors de PlayGuard. Ⓐ sur l'une remet la valeur précédente.
- **Un profil programmé** appliqué, ou les limites qu'il avait remplacées remises une fois sa période finie (*Temps de jeu › Profils › Profil programmé*).

### L'enfant a joué au-delà de la limite

- **La console se contente de prévenir.** *Temps de jeu › Quand le temps est écoulé* : choisissez *Suspendre le logiciel*.
- **Le contrôle parental est resté déverrouillé.** Tant qu'il l'est, la limite du jour est en pause : la pastille de l'en-tête le dit sur chaque onglet ; *Code PIN › Réactiver le contrôle parental*. Un redémarrage de la console le reverrouille aussi.
- **L'horloge a été reculée.** Le décompte s'arrête jusqu'à ce que l'horloge dépasse l'ancienne heure ([ce qui compte](features.md#ce-qui-compte-comme-temps-de-jeu)).
- **L'autre système.** L'emuMMC et la sysMMC ont chacune leur propre contrôle parental : réglez-le sur les deux, ou masquez le menu de démarrage d'hekate ([compatibilité](compatibility.md#emummc-et-sysmmc)).

### Toute la limite est de nouveau disponible aujourd'hui

Changer l'horloge (l'horloge réseau ou celle de la console) remet à zéro le temps de jeu du jour. Réglez l'horloge avant que l'enfant ne joue, pas après.

### PlayGuard s'ouvre en lecture seule

La console fait tourner un firmware plus récent que le plus récent testé : PlayGuard vérifie si une version plus récente le prend en charge et propose de mettre à jour ([firmware plus récent](compatibility.md#firmware-plus-récent)). Un crayon barré à droite du titre indique que l'application est en lecture seule ; le toucher ouvre l'écran du firmware.

### PlayGuard demande le code PIN tout le temps, ou jamais

*Code PIN › Protéger PlayGuard par le code PIN* : *Avant une modification* (par défaut) le demande une fois, avant la première modification, et le code vaut jusqu'à la fermeture de PlayGuard. *Redemander le code PIN après 5 minutes* le redemande après ce délai. *Afficher le code PIN* le demande toujours.

### Un jeu ne démarre pas, ou affiche un « ! » sur le menu HOME

*Console › Vérifier les jeux installés* dit ce qui ne va pas pour chacun et quoi faire ; les sauvegardes restent toujours ([jeux installés](features.md#jeux-installés)).

### La cloche ou un onglet affiche une pastille orange

Quelque chose demande votre attention : Ⓨ ouvre la liste, et Ⓐ sur une entrée fait la correction ou ouvre l'endroit où elle se fait ([notifications](features.md#notifications--ce-qui-demande-votre-attention-depuis-nimporte-quel-onglet)).

## Signaler un bug

Une erreur affiche le code du système (par exemple `0x0001088E`) avec ce que l'on en sait : quoi faire pour les codes dont le sens est documenté, sinon un rapport aide.

1. *À propos › Aide et diagnostic › Exporter un rapport de diagnostic* enregistre un fichier texte dans `sd:/config/playguard/logs/reports/` : firmware, version d'Atmosphère, durée du démarrage et des écrans les plus lents (l'historique des changements, *Tous les jeux*, la vérification des jeux installés, les onglets qui se rafraîchissent seuls), horloges, stockage, masquage du numéro de série, état des patchs de jeux, le résultat brut de chaque requête au contrôle parental, et les journaux : comment la dernière session s'est terminée, la mémoire et la place libre sur la carte SD, les derniers fichiers de plantage et les 80 dernières lignes journalisées. **Il ne contient jamais le code PIN ni le numéro de série.**
2. [Ouvrez un ticket](https://github.com/JigSawFr/PlayGuard/issues/new) (en anglais ou en français) et joignez-le.

### Envoyer un rapport en ligne

Console connectée, *À propos › Aide et diagnostic › Envoyer un rapport en ligne* (Ⓧ sur le rapport de diagnostic en mode développeur) l'envoie dans un gist secret de votre compte GitHub, après avoir indiqué exactement ce qui part. Un rapport enregistré plus tôt dans `logs/reports/` peut être envoyé de la même façon.

- **Ce qui part :** le rapport et chaque fichier dans un fichier à part :
  - les derniers rapports de crash : les 3 derniers de PlayGuard dans `logs/crashes/`, et les 2 derniers d'Atmosphère d'un homebrew de la semaine passée, dans `sd:/atmosphere/crash_reports/` (le crash d'un autre homebrew peut en faire partie : la liste affichée avant l'envoi dit lesquels) ;
  - les journaux de cette session et de la précédente (`logs/sessions/` ; un long : son début et ses 64 derniers Kio) ;
  - les fichiers de débogage : la référence du bloc du minuteur, la fin de l'enregistrement du minuteur tant qu'*Enregistrer le temps de jeu* est activé, les 20 dernières modifications de l'historique (sans noms de jeux, de comptes ou de profils, et avec leur date seulement), et les réglages de PlayGuard.
- **La connexion :** déconnecté, PlayGuard propose d'abord de se connecter ; *Compte GitHub*, juste en dessous une fois connecté, permet de se déconnecter ; si GitHub refuse le gist, PlayGuard propose de se reconnecter.
- **Ensuite :** PlayGuard affiche le lien et deux QR codes : le rapport, et le formulaire de signalement de bug avec le lien, vos versions, la façon dont PlayGuard a été lancé (par-dessus un jeu ou depuis l'Album ; ajoutez le menu) et le système (sysMMC ou emuMMC) déjà remplis. Chaque lien est conservé dans `logs/uploads/` sous forme de raccourci Internet (`.url`) qu'un ordinateur ouvre dans son navigateur.

> [!WARNING]
> Toute personne qui a le lien peut lire le rapport, jusqu'à ce que vous supprimiez le gist sur GitHub, et un signalement de bug ouvert avec lui est public : la confirmation le dit.

### Ce que PlayGuard garde sur la carte SD

Tout est dans `sd:/config/playguard/logs/`, en texte brut qu'un ordinateur ouvre :

- `sessions/session_<date>.txt` — **un fichier par session** : tout ce que PlayGuard a journalisé (ce qui n'arrivait jusqu'ici qu'à un PC par nxlink), chaque ligne avec son heure et son niveau (`E` erreur, `W` avertissement, `I` information), et une dernière ligne qui dit comment la session s'est terminée : `=== end: clean exit ===` (sortie normale), ou `=== end: crash, see logs/crashes/… ===` (plantage). Un fichier s'arrête de grossir à 1 Mio.
- `crashes/crash_<date>.txt` — **un fichier par plantage**, dont la première ligne dit ce qui s'est passé :
  - une **exception du processeur** (un mauvais pointeur, une instruction inconnue, un dépassement de pile) : le type de faute et l'adresse touchée, la pile d'appels et les registres. Les adresses sont des décalages dans `playguard.nro` (`playguard+0x1a2b3c`) : la carte d'édition de liens de la version (l'artefact `-linker-map` de son build CI) donne la fonction. PlayGuard se termine ensuite comme avant : l'écran d'erreur de la console, et le rapport d'Atmosphère dans `sd:/atmosphere/crash_reports/` ;
  - une **exception non rattrapée** : son type et son message, et la pile d'appels ;
  - une **session qui ne s'est pas terminée** : constatée au démarrage suivant, quand le fichier de la session précédente n'a pas de ligne de fin (un plantage que PlayGuard n'a pas pu attraper, une fermeture depuis le menu HOME pendant un blocage, ou la console éteinte), avec ses 60 dernières lignes.

  Les fichiers de plantage écrits pendant que PlayGuard tourne se terminent par les 80 dernières lignes journalisées avant le plantage.
- `reports/`, `uploads/`, `play_timer/` — les rapports enregistrés, les liens des rapports envoyés en ligne, l'enregistrement du temps de jeu.

*À propos › Aide et diagnostic* règle combien de rapports, de fichiers de plantage, de journaux de session et de liens sont gardés (chaque dossier de son côté ; le plus récent reste toujours).

## Mode développeur (examiner un nouveau firmware)

Appuyez sept fois sur Ⓨ sur *À propos › Version* ; les outils développeur apparaissent à la fin de *À propos › Aide et diagnostic*. Il ajoute :

- un interrupteur **lecture seule** — activé, l'application ne peut rien modifier : c'est la façon sûre d'examiner un nouveau firmware (l'écran firmware le propose directement) ;
- le rapport de diagnostic à l'écran (Ⓨ l'enregistre, Ⓧ l'envoie en ligne), et un raccourci pour l'exporter depuis l'onglet Temps de jeu ;
- **Installer un autre build** sur place, pour tester un correctif avant sa publication ([ci-dessous](#installer-un-autre-build)) ;
- **Comparer le bloc du temps de jeu** (seulement tant que l'application mobile est associée et peut se synchroniser — pas avec un fichier hosts qui bloque Nintendo ; aussi dans *Code PIN*), pour décoder les réglages que PlayGuard n'affiche pas encore : enregistrez le bloc brut comme référence, changez un réglage dans l'application mobile, revenez — PlayGuard liste les valeurs qui ont changé (enregistrées dans `logs/reports/` sur demande). Des heures du coucher différentes selon les jours, réglées depuis l'application mobile, pourraient être confirmées ainsi. Joignez ce fichier à un ticket, ou envoyez-le avec le rapport (*Envoyer un rapport en ligne*).
- **Enregistrer le temps de jeu** : toutes les 30 s tant que PlayGuard est ouvert, une ligne de ce que rapporte le minuteur (temps restant, temps écoulé, bloc brut des réglages…) dans `logs/play_timer/log.csv`, un fichier qui s'ouvre dans un tableur, plus une ligne juste après chaque modification faite par PlayGuard, qui dit laquelle (la dernière colonne, `event`), et une note quand l'horloge a bougé plus que le temps écoulé. Laissé ouvert après minuit, ou jusqu'à la fin du temps, il montre ce qu'un seul rapport ne peut pas montrer : quand le temps écoulé repart à zéro, ce que dit la console vers la fin. L'interrupteur est mémorisé ; il n'enregistre qu'en mode développeur. Ce qu'on sait déjà est dans [parental-controls.md](../parental-controls.md) (en anglais).
- **Vider le cache** et **Supprimer les fichiers journaux**, chacun après une confirmation qui indique combien de fichiers et quelle place : le cache (`cache/` et le `nand/<système>/cache/` de chaque système : noms et icônes des jeux, derniers chiffres de l'Activité, liste des builds) est relu sur la console au besoin ; les fichiers journaux sont les fichiers de plantage, les journaux de session (sauf celui de la session en cours, encore en écriture), les rapports enregistrés et l'enregistrement du temps de jeu, tandis que `uploads/` (les liens de suppression des rapports envoyés en ligne) et la référence du bloc restent. Les réglages, l'historique, les sauvegardes, les profils et les exports ne sont jamais touchés.
- **Rejouer les migrations** : les déplacements des anciennes organisations — le dossier de données à côté du `.nro`, les journaux d'avant leurs sous-dossiers, ce que tous les systèmes partageaient — chacun sur une ligne à cocher ou décocher (toutes cochées au départ ; A en bascule une), puis *Rejouer les N choisies* et une confirmation qui les liste avec ce que chacune déplace. Elles sont rejouées tout de suite, quoi qu'indique `migrations.json` (chacune ne tourne sinon qu'une fois, au premier lancement d'une version qui en a besoin ; le niveau enregistré n'est jamais abaissé) ; un fichier déjà en place n'est jamais remplacé. Une fenêtre indique combien d'éléments ont été déplacés ; chaque étape est dans le journal de session.
- **Statistiques d'image** : une ligne par seconde dans `logs/frame_stats.txt`, le temps mis à dessiner l'écran et les images en retard, pour comparer un écran à un autre sur la console. **Halo d'arrière-plan** éteint la lumière douce derrière les écrans (le mode de la ligne finit alors par `-noglow`), pour mesurer ce qu'elle coûte.

### Installer un autre build

*Installer un autre build* met un build à la place du `.nro` de PlayGuard, pour tester un correctif avant sa publication.

**Ce qu'il liste**, chaque catégorie dans un onglet à part (Ⓛ / Ⓡ, affichés de part et d'autre des onglets, passent de l'un à l'autre ; l'écran s'ouvre sur celui du build installé, et chaque onglet compte les builds plus récents que celui installé) :

- l'une des 20 dernières versions publiées (préversions comprises, étiquetées) ;
- l'un des 20 derniers commits de `main` ;
- le dernier build d'une pull request ouverte depuis une branche de ce dépôt (jamais depuis un fork : son code n'a pas encore été relu). A sur l'onglet des pull requests, quand il est affiché, les trie dans l'autre ordre (build le plus récent d'abord, ou par numéro).

**Chaque build** affiche son titre (le sujet du commit ou de la pull request ; le message complet d'un commit sert de description) au-dessus d'étiquettes : le type de changement (*Nouveauté*, *Correctif*, *Docs*…) et sa portée, *Rupture*, la pull request d'origine d'un commit, l'état de la CI (*CI OK*, *CI en échec*, *CI en cours*), préversion, *installé* ou *nouveau* ; son ancienneté et son commit à droite. Un build ouvre d'abord sa page : des étiquettes (son type, ses vérifications, installé ou nouveau), une fiche (compilation, taille, commit, empreinte, date de suppression par GitHub), **Installer**, **Voir sur GitHub** (sa page en QR code, pour un téléphone), puis les notes d'une version publiée ou la description d'une pull request, mises en forme depuis le Markdown de GitHub (titres, listes, cases à cocher, code, citations et encadrés, tableaux).

**Sans risque à lire :** tout ce qui vient de GitHub est affiché comme du simple texte : aucun lien n'est ouvert, aucun HTML ni script n'est exécuté, et les caractères invisibles qui pourraient maquiller un titre (inversion de sens, caractères de largeur nulle, de contrôle, UTF-8 invalide) sont retirés.

**La connexion.** Les versions publiées ne demandent rien ; les autres sont les artefacts du workflow de build, que GitHub ne donne qu'aux utilisateurs connectés. **Compte GitHub** connecte PlayGuard avec un code et un QR code à scanner avec un téléphone. Seules autorisations demandées : lire les fichiers du workflow de build (Actions) et créer des gists, pour *Envoyer un rapport en ligne* ; sinon le jeton ne peut lire que ce qui est public. Il est gardé dans `github_token`, jamais envoyé avec un rapport, et *Compte GitHub* permet de se déconnecter. Une fois connecté, Ⓨ (*Rechercher*) prend le numéro d'une pull request (ouverte ou fermée) ou un commit (7 à 40 chiffres hexadécimaux) et trouve son build, uniquement depuis une branche de ce dépôt.

**L'installation :** PlayGuard télécharge le build, le vérifie (taille, empreinte SHA-256 enregistrée par GitHub, en-tête NRO), le met à la place de son propre `.nro` et redémarre dessus (le code PIN est d'abord demandé dès qu'il y en a un).

- La liste est gardée 10 minutes et s'affiche aussitôt, avec son ancienneté (Ⓧ la recharge) ; pendant son chargement, une barre montre où il en est.
- Un téléchargement interrompu (réseau perdu) reprend là où il s'est arrêté la prochaine fois que le même build est installé.
- Les artefacts expirent au bout de 90 jours.

**Revenir en arrière.** Le build remplacé est gardé (`playguard.nro.previous`) : **Revenir au build précédent**, dans la même liste, les échange à nouveau, après le code PIN. Si un build installé d'ici (ou par *Mettre à jour sur place*) plante avant son premier écran, le lancement suivant de PlayGuard propose de revenir au build précédent sur un simple écran texte, avant de charger quoi que ce soit (A revient en arrière, B le lance quand même), sans le code PIN, car l'écran du code PIN peut lui-même être ce qui plante. La même liste permet de revenir à n'importe quelle version publiée à tout moment.

**Quel build tourne :** *À propos › Version* affiche le commit en mode développeur. L'en-tête de chaque écran affiche la version en cours à côté du titre : celle d'une version publiée dans une étiquette grise (`1.2.0`), celle de tout autre build dans une étiquette orange suivie de son commit, pour ne jamais confondre un build de test avec la version publiée.

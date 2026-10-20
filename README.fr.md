# PlayGuard

![PlayGuard](images/store/banner.png)

[![build](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml/badge.svg)](https://github.com/JigSawFr/PlayGuard/actions/workflows/build.yml)
[![dernière version](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2FJigSawFr%2FPlayGuard%2Fmain%2F.release-please-manifest.json&query=%24%5B%27.%27%5D&prefix=v&label=version&color=blue)](https://github.com/JigSawFr/PlayGuard/releases/latest)
[![téléchargements](https://img.shields.io/github/downloads/JigSawFr/PlayGuard/total)](https://github.com/JigSawFr/PlayGuard/releases)
[![licence : GPLv3](https://img.shields.io/badge/licence-GPLv3-blue.svg)](LICENSE)

[English](README.md) · **Français**

**Le contrôle parental de la Nintendo Switch, directement sur la console — sans application mobile, sans compte Nintendo, sans Internet.**

PlayGuard ramène sur une console modifiée les réglages de l'application mobile [Contrôle parental Nintendo Switch](https://apps.apple.com/fr/app/contr%C3%B4le-parental-nintendo-sw/id1190074407). Il pilote le contrôle parental de la console elle-même (la limite, le code PIN, les avertissements, la suspension) : rien ne tourne en arrière-plan.

[![PlayGuard en 30 secondes](images/video/playguard-tour.gif)](images/video/playguard-tour.gif)

▶️ **[L'essayer dans le navigateur](https://playguard.link/simulator/?lang=fr)**, la vraie app face à une console simulée : [déverrouillé pour l'instant](https://playguard.link/simulator/?lang=fr&unlocked=1) · [temps écoulé](https://playguard.link/simulator/?lang=fr&restricted=1) · [premier lancement, sans code PIN](https://playguard.link/simulator/?lang=fr&not_set_up=1) · [sur la TV, texte plus grand](https://playguard.link/simulator/?lang=fr&docked=1).

> [!WARNING]
> **Nécessite un firmware personnalisé (Atmosphère).** PlayGuard utilise le service restreint `pctl`, en partie via des commandes `*ForDebug`. **À utiliser à vos risques.**

## Fonctions

- ⏱️ **Limite de temps de jeu** — la même tous les jours ou une par jour, des profils, du temps en plus ou l'arrêt du jeu pour aujourd'hui en un appui.
- 📊 **Activité** — temps par jeu et par compte, graphiques, export en CSV, JSON, XLSX ou PDF.
- 🔒 **Restrictions et code PIN** — niveau de restriction, classification par âge, code PIN, déverrouillage temporaire, *Bloquer tous les jeux* en un interrupteur.
- 🕒 **Console** — l'horloge réseau réglée depuis des serveurs NTP publics, pour que le minuteur compte juste sur une console qui n'atteint jamais Nintendo, une vérification des jeux installés, et des homebrews (PlayGuard en premier) placés sur le menu HOME comme des jeux, avec leur icône (forwarders de NRO).
- 📱 **Application mobile** — la dissocier, même sur une console d'occasion.
- ↩️ **Historique et sauvegardes** — chaque modification est consignée et peut être annulée.
- 🔔 **Notifications** — une cloche regroupe ce qui demande votre attention (alarme désactivée, horloge décalée, modifications faites ailleurs), avec la correction à portée d'un appui.
- 🌍 Toutes les langues de la console, thèmes clair et sombre, texte plus grand sur la TV ; des cartes, le temps du jour en anneau, la semaine réglée au stick.

## Compatibilité

- **Testé sur console :** firmware 22.0.0 → 22.5.0 (Atmosphère 1.11.x) et 23.0.1 (Atmosphère 1.12.1)
- **Devrait fonctionner, non testé :** firmware 21.x (Atmosphère 1.10.x) et 23.0.0 (1.12.0) ; firmware 20.x et antérieur avec son Atmosphère, tout sauf le temps de jeu
- **Lanceurs :** hbmenu, sphaira, Homebrew App Store

Un firmware plus récent que 23.0.1 s'ouvre en lecture seule tant qu'aucune version ne le prend en charge. Pas de Switch 2 (Atmosphère n'existe pas pour elle). Détails : [compatibilité](docs/fr/compatibility.md).

## Installation

Cherchez *PlayGuard* dans le Homebrew App Store ou sphaira, ou extrayez `playguard.zip` de la [dernière version](https://github.com/JigSawFr/PlayGuard/releases/latest) à la racine de la carte SD. Lancez-le par-dessus un jeu, avec le compte d'un parent.

## Premiers pas

1. Ouvrez PlayGuard par-dessus un jeu, avec le compte d'un parent : le guide *Premiers pas* règle le code PIN et la limite quotidienne.
2. Encore associé à l'application mobile ? *Code PIN › Dissocier l'application mobile*, sinon sa prochaine synchronisation écrase vos réglages.
3. Horloge réseau imprécise ? *Console › Horloge réseau › Mesurer l'écart*, puis réglez-la. Sans Internet, lisez d'abord [ceci](docs/fr/compatibility.md#une-console-sans-internet).

Le pas-à-pas complet, et les tâches du quotidien : [prise en main](docs/fr/getting-started.md). Code PIN oublié ou console d'occasion : [Console bloquée ?](docs/fr/troubleshooting.md#console-bloquée-)

## Documentation

- **Site :** [playguard.link](https://playguard.link/fr/) — cette documentation dans les 15 langues de la console, et [PlayGuard dans le navigateur](https://playguard.link/simulator/?lang=fr) face à une console simulée
- **Guide :** [prise en main](docs/fr/getting-started.md) · [fonctions](docs/fr/features.md) · [installation](docs/fr/install.md) · [compatibilité](docs/fr/compatibility.md) · [dépannage](docs/fr/troubleshooting.md)
- **Référence (en anglais) :** [application mobile](docs/companion-app.md) · [`config.json`](docs/config.md) · [API de lancement (brouillon)](docs/api.md) · [fonctionnement du contrôle parental](docs/parental-controls.md) · [module de récupération](sysmodule/README.md)
- **Contribuer (en anglais) :** [CONTRIBUTING.md](CONTRIBUTING.md) · [feuille de route](ROADMAP.md) · [changelog](CHANGELOG.md)

## Licence et remerciements

GPLv3, voir [`LICENSE`](LICENSE). Maintenu par [JigSawFr](https://github.com/JigSawFr). Fork de [Pctl Manager](https://github.com/tailiang2008) de Taylor ; correctifs fw 22.5 et synchronisation NTP repris de [anbingxi/NX-Pctl-Manager](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly). Construit avec [borealis](https://github.com/xfangfang/borealis) (Apache 2.0), [QR Code generator](https://github.com/nayuki/QR-Code-generator) (MIT), [Twemoji](https://github.com/jdecked/twemoji) (CC-BY 4.0) et la police [Sora](https://github.com/sora-xor/sora-font) (OFL 1.1) ; référence des commandes : [switchbrew](https://switchbrew.org/wiki/Parental_Control_services). La vérification des jeux installés et ses réparations suivent [Goldleaf](https://github.com/XorTroll/Goldleaf) et [sphaira](https://github.com/ITotalJustice/sphaira) (GPLv3), avec les numéros de commandes ns et es de [libnx-ext](https://github.com/XorTroll/libnx-ext) (MIT). Les forwarders de NRO sont construits comme sphaira les construit, leur programme étant le hbl de sphaira, d'après [nx-hbloader](https://github.com/switchbrew/nx-hbloader) (ISC).

Des assistants de code à base d'IA aident au développement ; chaque changement est relu, couvert par les tests unitaires et vérifié sur une vraie console avant publication.

PlayGuard n'est ni affilié à Nintendo ni approuvé par Nintendo. Nintendo Switch est une marque de Nintendo.

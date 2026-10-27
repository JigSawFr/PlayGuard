<!-- translated from docs/themes.md, sha256 6b5b00247c49 -->
# Thèmes et palettes

PlayGuard dessine chaque écran selon deux choix des *Préférences* :

- **Thème** : clair, sombre (par défaut) ou celui de la console.
- **Palette** : les couleurs. *Par défaut* est celle de PlayGuard (bleu nuit,
  ou blanc froid avec le thème clair, et turquoise), puis viennent les palettes
  fournies avec PlayGuard, puis les fichiers de thème que vous ajoutez. La
  palette est aussi la dernière ligne de la liste des thèmes (**−**, ou le
  bouton du thème dans l'en-tête).

Les deux s'appliquent tout de suite, sur chaque écran, sans relancer
PlayGuard. Une palette faite pour un seul thème (les contrastes élevés,
*Océan*, *Sable*) règle *Thème* sur celui-ci ; choisir ensuite l'autre thème
change aussi la palette, pour son autre moitié quand elle en a une
(*Contraste élevé clair* et *Contraste élevé sombre*), sinon pour *Par défaut*
(la liste des thèmes le dit avant). Les autres palettes suivent *Thème*.

Retour au [README](../../README.md).

## Les palettes fournies avec PlayGuard

| Palette | Thème | Description |
| --- | --- | --- |
| *Contraste élevé sombre* | sombre | Blanc sur noir, un contour de sélection jaune, plus épais |
| *Contraste élevé clair* | clair | Noir sur blanc, un contour de sélection bleu foncé, plus épais |
| *Océan* | sombre | Bleu profond, un accent bleu ciel |
| *Améthyste* | clair et sombre | Violet |
| *Sable* | clair | Papier chaud, un accent terre cuite |
| *Hiver*, *Saint-Valentin*, *Nouvel An lunaire*, *Printemps*, *Été*, *Automne*, *Halloween*, *Noël*, *Nouvel An* | clair et sombre | Une par saison et par fête : givre et argent, rose, laque rouge et or, fleurs de cerisier, mer et coucher de soleil, érable, citrouille sur une nuit violette, sapin et or, minuit et champagne |
| *Au fil des saisons* | clair et sombre | Celle du jour, selon le calendrier : Halloween à partir du 17 octobre, Noël en décembre, et ainsi de suite |

Elles viennent d'Aurore, la bibliothèque sur laquelle PlayGuard est construit,
et leurs noms existent dans toutes les langues de la console.

*Au fil des saisons* change d'elle-même : le jour où commence une saison ou
une fête, les écrans prennent sa palette à minuit, ou dès que la console sort
de veille. Les saisons sont celles de l'hémisphère nord ; le
[guide des thèmes d'Aurore](https://github.com/JigSawFr/Aurore/blob/main/docs/themes.md#the-seasons)
(en anglais) donne les jours exacts.

## Ajouter les vôtres

1. Écrivez un fichier de thème (plus bas), avec un nom qui finit par `.theme`.
2. Copiez-le sur la carte SD, dans `config/playguard/themes/` (créez le dossier
   s'il n'existe pas). Dans le simulateur sur ordinateur, le dossier est
   `playguard_data/themes/`, à côté des données que PlayGuard y garde.
3. *Préférences › Palette* l'affiche sous son `name`, après les palettes
   fournies avec PlayGuard. Choisissez-le : chaque écran le prend tout de
   suite.

Un fichier avec une erreur qui empêche de le lire (un fichier inclus
manquant, un fichier qui n'est pas un thème) n'apparaît pas dans la liste ;
une ligne illisible est ignorée, le reste du fichier utilisé. Les deux sont
notés dans le journal de session (`config/playguard/logs/sessions/`), avec le
fichier et la ligne. Si le fichier choisi disparaît de la carte, les couleurs
de PlayGuard reviennent.

## Écrire un fichier de thème

Un fichier de thème est du texte : des lignes `clé = valeur`, des
`[sections]` et des commentaires `#`. Trois lignes suffisent :

```ini
name = Mint
inherit = dark
accent = #3ddc97
```

- **`name`**, et en option `author`, `version` et `description` : ce que la
  liste affiche.
- **`inherit`** : ce dont le fichier part, `dark` (s'il ne dit rien), `light`,
  une des palettes ci-dessus (`builtin:ocean`, `builtin:amethyst`,
  `builtin:sand`, `builtin:high-contrast-dark`, `builtin:high-contrast-light`)
  ou un autre fichier du dossier. Tout ce qu'il ne précise pas en vient.
- **La palette** : `background` (le fond), `surface` (les cartes), `text`,
  `accent`, `warning` (avertissement) et `danger`. Chacune fixe toutes les
  couleurs qui en découlent : `text` les encres plus discrètes des notes et
  légendes, `accent` le contour de sélection, l'onglet actif, les
  interrupteurs et l'anneau du minuteur, etc. Les encres sont assombries ou
  éclaircies jusqu'à se lire sur le fond et sur les cartes.
- **`[light]` et `[dark]`** : une palette ou des clés pour un seul thème. Un
  fichier avec les deux sections habille les deux thèmes, et le réglage
  *Thème* de PlayGuard choisit celui qui s'affiche.
- **`counterpart`**, pour un fichier qui n'habille qu'un thème : le fichier
  (ou la palette fournie, `builtin:…`) qui a l'autre, vers lequel PlayGuard
  passe quand l'autre thème est choisi (`counterpart = mint-day.theme`).
- **`[colors]`** : n'importe quelle couleur par sa clé (listes ci-dessous),
  par-dessus ce que la palette a donné.
- **Couleurs** : `#rgb`, `#rrggbb`, `#rrggbbaa`, `rgb(r, g, b)` ou
  `rgba(r, g, b, a)` (a de 0 à 1).
- **`@include autre.inc`** : les lignes d'un autre fichier du dossier, à cet
  endroit (nommez-le `.inc`, pour qu'il n'apparaisse pas comme une palette).

Chaque couleur de PlayGuard est une clé de thème : celles d'Aurore
(`brls/…`), celles de Cadran, l'apparence que PlayGuard partage avec PlayShelf
(`cadran/…`), et celles de PlayGuard (`playguard/…`). Le [guide des thèmes](https://github.com/JigSawFr/Aurore/blob/main/docs/themes.md)
d'Aurore (en anglais) liste les deux premières avec ce que chacune colore, et
ce que fixe chaque clé de la palette.

### Les clés de PlayGuard

| Clé | Ce qu'elle colore |
| --- | --- |
| `playguard/loading/glow` | Le cadran de l'écran de chargement : le halo derrière lui |
| `playguard/loading/arc` | Le cadran : l'arc de la journée |
| `playguard/loading/track` | Le cadran : le rail de l'arc |
| `playguard/loading/line` | Le cadran : ses cercles et ses minutes, à pleine intensité (ils sont dessinés atténués) |
| `playguard/ring/tip` | Le point de lumière au bout de l'anneau du minuteur |
| `playguard/badge/ink` | Le « ! » sur le disque corail d'un jeu qui reste (*Console › Vérifier les jeux installés*, suppressions) |

Le fond et le texte de l'écran de chargement sont ceux du thème
(`brls/background`, `cadran/text2`), comme sur chaque écran.

### Un texte lisible

Après avoir lu un fichier, PlayGuard vérifie que ce qui doit rester lisible
l'est toujours : le texte et les encres sur le fond et sur les cartes (4,5:1),
le contour de sélection (3:1), et sa propre clé `playguard/badge/ink` sur
`cadran/coral` (3:1). Une paire sous son minimum est notée dans le journal de
session, par exemple `cadran/text3 on cadran/surf reads badly: 3.9:1, under 4.5:1` ;
le thème est utilisé quand même. Les palettes fournies avec PlayGuard passent
toutes les vérifications.

### Deux exemples

Les deux fichiers sont dans [`docs/themes/`](../themes/), prêts à copier.

[`mint.theme`](../themes/mint.theme) : l'apparence sombre de PlayGuard avec un
autre accent.

```ini
name = Mint
author = PlayGuard
version = 1
description = The dark look, a mint accent.

inherit = dark
accent = #3ddc97
```

[`paper.theme`](../themes/paper.theme) : les deux thèmes, et une des clés de
PlayGuard.

```ini
name = Paper and ink
author = PlayGuard
version = 1
description = Warm paper by day, warm ink by night, a terracotta accent.

[light]
background = #f4efe6
surface = #fffaf2
text = #2b2622
accent = #a4502a
warning = #8a5a00
danger = #b3261e

[dark]
background = #17130f
surface = #221c17
text = #f0e7da
accent = #e39a6b
warning = #f2b84b
danger = #ff8a80

[colors]
playguard/ring/tip = #fff1dc
```

## Ce qui garde ses couleurs

- **La porte du code PIN** d'un forwarder protégé par le code PIN
  (`config/playguard/gate.nro`) est un petit programme à part, sans la
  bibliothèque de PlayGuard : elle garde les couleurs de PlayGuard, en thème
  clair ou sombre.
- **Le logo de PlayGuard** (la marque de l'en-tête, l'icône de l'écran de
  chargement) et son nom dessiné, et le drapeau français dans *À propos*.
- **Les QR codes** restent sombres sur blanc, comme les téléphones les lisent
  le mieux.
- **Les jaquettes et icônes des jeux**.

Le choix est gardé dans `config.json` (`theme`, `theme_file` : voir
[config.md](../config.md), en anglais).

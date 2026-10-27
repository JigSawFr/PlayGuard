<!-- translated from docs/themes.md, sha256 293a65fda593 -->
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
   Un thème avec des images est plutôt un dossier : son fichier s'appelle
   `theme.theme`, ses images sont à côté ([Images](#images)).
2. Copiez le fichier, ou le dossier entier, sur la carte SD, dans
   `config/playguard/themes/` (créez le dossier s'il n'existe pas). Dans le
   simulateur sur ordinateur, le dossier est `playguard_data/themes/`, à côté
   des données que PlayGuard y garde.
3. *Préférences › Palette* l'affiche sous son `name`, après les palettes
   fournies avec PlayGuard. Choisissez-le : chaque écran le prend tout de
   suite.

Un fichier avec une erreur qui empêche de le lire (un fichier inclus ou une
image manquants, un fichier qui n'est pas un thème) n'apparaît pas dans la liste ;
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
  liste affiche. **`icon`** : le motif dessiné sur la ligne de la palette, dans
  son accent sur un échantillon de ses couleurs : `snowflake`, `heart`,
  `lantern`, `blossom`, `sun`, `wave`, `leaf`, `pumpkin`, `tree`, `firework`,
  `gem`, `dune`, `contrast`, `moon`, `bolt` ou `sparkle` (par défaut), ou
  n'importe quelle autre icône de Cadran.
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
(`brls/background`, `cadran/text2`), comme sur chaque écran, et son image de
fond aussi.

### Un texte lisible

Après avoir lu un fichier, PlayGuard vérifie que ce qui doit rester lisible
l'est toujours : le texte et les encres sur le fond et sur les cartes (4,5:1),
le contour de sélection (3:1), et sa propre clé `playguard/badge/ink` sur
`cadran/coral` (3:1). Une paire sous son minimum est notée dans le journal de
session, par exemple `cadran/text3 on cadran/surf reads badly: 3.9:1, under 4.5:1` ;
le thème est utilisé quand même. Les palettes fournies avec PlayGuard passent
toutes les vérifications.

### Deux exemples

Les deux fichiers sont dans [`docs/themes/`](../themes/), prêts à copier (un
dossier de thème avec des images est [plus bas](#un-dossier-de-thème-dusk)).

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

## Images

Un dossier de thème peut apporter des images : un fond derrière chaque écran,
un logo, certaines icônes, l'image de chargement de PlayGuard, une vignette
pour la liste des palettes, et des décorations. PNG ou JPEG ; chaque clé nomme
un fichier du dossier (ou d'un de ses sous-dossiers) : un chemin qui sort du
dossier (`/…`, `sdmc:/…`, `../…`) est refusé, la ligne ignorée et notée dans
le journal de session. Une clé en haut du fichier vaut pour les deux thèmes ;
dans `[light]` ou `[dark]`, pour celui-là seulement. Sans ces clés, rien ne
change.

```
config/playguard/themes/
├── mint.theme          # un simple fichier
└── dusk/               # un dossier de thème
    ├── theme.theme
    ├── night.jpg
    └── icons/settings.png
```

### Le fond

| Clé | Valeurs | Rôle |
| --- | --- | --- |
| `background_image` | un fichier, ou `none` | Une image derrière chaque écran (les cartes, la barre latérale et les dialogues restent par-dessus) |
| `background_fit` | `cover` (par défaut), `contain`, `tile`, `center` | Comment elle remplit l'écran : tout l'écran, coupée aux bords ; l'image entière ; répétée ; telle quelle au milieu |
| `background_dim` | de 0 à 1, 0,35 par défaut | Un voile de la couleur de fond sur l'image, pour que le texte s'y lise |
| `surface_opacity` | de 0 à 1, 1 par défaut | L'opacité des cartes, de la barre latérale, de la sélection et des boutons sur l'image |

Le texte est vérifié sur l'image sous son voile (sa couleur moyenne) : s'il se
lit mal, le journal de session le dit, et un `background` plus sombre ou plus
clair (ou un `background_dim` plus fort) y remédie. Une image de 1920 × 1080
ou moins convient le mieux ; une plus grande est réduite au chargement.

### Le logo, les icônes et les images de PlayGuard

| Clé | Rôle |
| --- | --- |
| `logo` | Le logo de PlayGuard : la marque de l'en-tête, *À propos*, et l'écran de chargement (sauf si le thème donne l'image de chargement ci-dessous). Ajusté dans le carré du logo, dessiné tel quel |
| `icon.<nom>` | Une des icônes de Cadran (`settings`, `info`, `today`, `timer`…), dans la barre latérale, les lignes et les volets |
| `icon.<nom>.tint` | `true` : l'image est une forme dessinée dans la couleur de l'icône (dessinez-la en blanc sur fond transparent) ; `false`, par défaut : dans ses propres couleurs |
| `image.<emplacement>` | Une des images de PlayGuard (ci-dessous) |
| `preview` | La vignette que la liste des palettes montre sur la ligne du thème, au lieu de l'échantillon de ses couleurs (carrée, 72 × 72 ou plus) |

Les images de PlayGuard qu'un thème peut remplacer (ses emplacements
d'image) :

| Emplacement | Rôle | Celle de PlayGuard |
| --- | --- | --- |
| `playguard/loading/art` | L'image au milieu de l'écran de chargement, sur son halo qui respire, un carré de 168 unités (de 256 × 256 à 512 × 512, avec un bord transparent pour une image ronde) | L'icône de l'application |
| `playguard/loading/wordmark` | Le nom en dessous, 360 × 88 unités (un PNG transparent d'environ 720 × 176) | Le nom *PlayGuard* dessiné, dans les encres du thème |

```ini
image.playguard/loading/art = loading.png
[light]
image.playguard/loading/wordmark = name-day.png
[dark]
image.playguard/loading/wordmark = name-night.png
```

### Les décorations

| Clé | Rôle |
| --- | --- |
| `backdrop.header`, `backdrop.sidebar`, `backdrop.dialog` | Une image sur l'en-tête, la barre latérale, ou le panneau des dialogues et des volets, sous leur texte : gardez-la discrète |
| `decor.top`, `decor.bottom` | Une bande le long du bord haut ou bas de l'écran, derrière les écrans (une guirlande, des stalactites) : environ 30 unités de haut en haut, 20 en bas, pour que l'en-tête et le pied de page se lisent par-dessus |
| `<clé>_fit`, `<clé>_opacity`, `decor.top_height`, `decor.bottom_height` | Comment chacune remplit sa place (comme `background_fit`), son opacité, la hauteur d'une bande |
| `particles` | `snow` (neige), `leaves` (feuilles), `petals` (pétales), `embers` (braises), `confetti`, `sparkles` (étincelles), `hearts` (cœurs), `bubbles` (bulles), ou `none` : ce qui flotte derrière les écrans |
| `particles_density` | `low` (par défaut), `medium`, `high` |
| `particles_color` | Jusqu'à cinq couleurs, séparées par des virgules ; sans elle, celles de la palette |
| `particles_speed` | de 0,25 à 4, 1 par défaut |
| `particles_layer` | `back` (par défaut : derrière les écrans, entre les cartes) ou `front` (par-dessus, plus pâles) |

Les palettes de saison fournies avec PlayGuard ont leurs propres particules,
discrètes : de la neige en hiver et à Noël, des pétales au printemps, des
feuilles en automne, des braises à Halloween, des confettis au Nouvel An, des
cœurs à la Saint-Valentin, des étincelles dorées au Nouvel An lunaire
(`particles = none` dans un fichier qui en hérite les retire). **Les
particules ne bougent que si l'interface peut bouger** : avec *Préférences ›
Effets visuels* sur *Mouvements réduits*, ou sur *Automatique* avec la console
en main, il n'y en a aucune, seulement la palette. Après une minute sans
bouton pressé, elles s'estompent et s'arrêtent, jusqu'à la pression suivante.

### Un dossier de thème : Dusk

[`dusk/`](../themes/dusk/) a un ciel étoilé derrière le thème sombre et l'aube
derrière le clair, un logo pour chacun, sa propre image de chargement, une
icône des réglages teintée, des étoiles suspendues en haut de l'écran la nuit,
des étincelles qui flottent derrière les écrans, et une vignette pour la liste
des palettes. Pour l'installer, copiez le dossier `dusk` entier dans
`config/playguard/themes/` (sur ordinateur, `playguard_data/themes/`), de
sorte que le fichier soit `config/playguard/themes/dusk/theme.theme`, puis
choisissez *Dusk* dans *Préférences › Palette*. Ses images sont dessinées par
[`tools/make_example_theme.py`](../../tools/make_example_theme.py), et pèsent
environ 100 Ko en tout.

```ini
# A theme folder: this file is theme.theme, its pictures are next to it.
# Copy the whole dusk/ folder into config/playguard/themes/ (docs/themes.md).
# The pictures are drawn by tools/make_example_theme.py.
name = Dusk
author = PlayGuard
version = 1
description = A starry sky at night, the dawn by day, with a logo and a loading picture of its own.
icon = moon

# Behind every screen: the sky, the cards slightly see-through over it
background_fit = cover
surface_opacity = 0.88

# The palette list shows this picture instead of a swatch of the colours
preview = preview.png

# The loading screen's picture (an image slot of PlayGuard's)
image.playguard/loading/art = loading.png

# Cadran's settings icon (the rail, Preferences), drawn in the icon's colour
icon.settings = icons/settings.png
icon.settings.tint = true

# A few stars twinkling behind the screens (not with reduced motion)
particles = sparkles
particles_density = low

[light]
background = #f1edf4
surface = #fdfbff
text = #2a2238
accent = #6a3a7c
warning = #8a5a00
danger = #b3261e
background_image = day.jpg
background_dim = 0.6
logo = logo-light.png
# Over the picture, a little darker than the palette makes them
cadran/text3 = #574d66
brls/header/subtitle = #574d66

[dark]
background = #1b1838
surface = #26224a
text = #f4eefb
accent = #ffc496
warning = #f2b84b
danger = #ff8a80
background_image = night.jpg
background_dim = 0.3
logo = logo-dark.png
# Stars hanging from the top of the screen, repeated sideways
decor.top = stars.png
decor.top_fit = tile
```

## Ce qui garde ses couleurs

- **La porte du code PIN** d'un forwarder protégé par le code PIN
  (`config/playguard/gate.nro`) est un petit programme à part, sans la
  bibliothèque de PlayGuard : elle garde les couleurs de PlayGuard, en thème
  clair ou sombre.
- **Le logo de PlayGuard** (la marque de l'en-tête, l'icône de l'écran de
  chargement) et son nom dessiné, sauf si le thème donne des images pour eux
  (`logo`, `image.…`, ci-dessus), et le drapeau français dans *À propos*.
- **Les QR codes** restent sombres sur blanc, comme les téléphones les lisent
  le mieux.
- **Les jaquettes et icônes des jeux**.

Le choix est gardé dans `config.json` (`theme`, `theme_file` : voir
[config.md](../config.md), en anglais).

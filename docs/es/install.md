<!-- translated from docs/install.md, sha256 f12b746bb6b9 -->
# Instalación

Cómo instalar PlayGuard y qué escribe en la tarjeta SD. Volver al [README](../../README.md).

## Dónde conseguirlo

Elige una opción:

- **Homebrew App Store** / **App Store de sphaira** (el mismo catálogo) — Busca *PlayGuard* cuando la ficha esté aprobada. Las nuevas versiones se detectan automáticamente.
- **sphaira › GitHub** — El zip de la versión ya contiene la entrada (`/config/sphaira/github/playguard.json`): tras una primera instalación, actualiza desde *GitHub* en sphaira.
- **Manual** — Descarga `playguard.zip` de la [última versión](https://github.com/JigSawFr/PlayGuard/releases/latest) y extráelo en la **raíz** de la tarjeta SD. La aplicación queda en `sd:/switch/playguard/` y sus archivos en `sd:/config/playguard/`.

El **sysmodule de recuperación** opcional (`playguard-rescue.zip`) se descarga aparte: consulta [¿Consola bloqueada?](troubleshooting.md#consola-bloqueada).

## Archivos en la tarjeta SD

Todos están en `sd:/config/playguard/` (la aplicación en sí se queda en `sd:/switch/playguard/`: quitarla o reinstalarla los conserva). Las versiones que los guardaban junto a la aplicación los trasladaron allí en su primer inicio.

- **`config.json`** — Preferencias (idioma, tema, servidor NTP, *Proteger PlayGuard con el PIN*…); cada clave en [config.md](../config.md) (en inglés). Si no se puede leer, se conserva como `config.json.bad` (el anterior como `.bad.1`) y los ajustes vuelven a sus valores predeterminados; PlayGuard lo avisa una vez al iniciarse
- **`profiles/`** — Perfiles de límites de tiempo de juego guardados
- **`exports/`** — Exportaciones de la actividad
- **`nand/<sistema>/`** — Lo que pertenece a un sistema, cada uno con su propio control parental: `sysmmc/` para la sysMMC, `emummc/` para la emuMMC. Dentro: `history.json`, el historial de cambios (los 500 más recientes, incluidos los que la consola rechazó; si no se puede leer, se conserva como `history.json.bad`, el anterior como `.bad.1`, en lugar de sobrescribirlo, y PlayGuard lo avisa una vez); `state.json`, lo que *Bloquear todos los juegos*, el tiempo extra, un rebloqueo pendiente y la elección de firmware necesitan recordar ([config.md](../config.md#state-kept-between-launches), en inglés); `watch.json`, lo último que PlayGuard vio de la consola, para saber qué cambió fuera de él ([config.md](../config.md#watchjson), en inglés; borrarlo no causa ningún problema); `backups/`, las copias de seguridad de los ajustes (nunca contienen el PIN); `cache/`, la última actividad de juego leída (todas las cuentas, y cada cuenta consultada), que se muestra en cuanto se inicia la siguiente vez mientras se vuelve a leer el registro. Los archivos de una versión anterior a estas carpetas pasan al primer sistema en el que se inicia PlayGuard
- **`cache/`** — Los iconos de los juegos (`icons/`, 16 MB como máximo, se vacía cuando cambia el idioma de la consola), compartidos por todos los sistemas, para no volver a leer el nombre y el icono de cada juego en cada inicio; las notas de la versión de Atmosphère, hekate y el firmware (`sys_notes.json`, para *Consola*) y las de PlayGuard (para *Acerca de*); en modo desarrollador, la lista de *Instalar otra compilación* (`dev_builds.json`)
- **`github_token`** — El inicio de sesión de GitHub de *Enviar un informe en línea* y, en modo desarrollador, de *Instalar otra compilación*; se borra al cerrar la sesión (*Acerca de › Ayuda y diagnóstico › Cuenta de GitHub*, visible una vez iniciada la sesión)
- **`previous_build.json`**, **`boot_check.json`** — Tras instalar una compilación en su lugar: el nombre de la compilación sustituida (conservada junto al `.nro` como `playguard.nro.previous`) y la comprobación del siguiente inicio (se borra en cuanto aparece su primera pantalla)
- **`rescue_report.txt`** — Lo deja el sysmodule de recuperación después de actuar, hasta que PlayGuard lo muestra al iniciarse (si no lo puede leer, lo elimina y lo avisa)
- **`rescue_nonce`** — Un desafío aleatorio al que el sysmodule de recuperación responde en su informe, para que PlayGuard distinga un informe escrito por él de uno escrito a mano ([sysmodule/README.md](../../sysmodule/README.md), en inglés); uno nuevo en cada inicio
- **`logs/`** — `reports/`, los informes de diagnóstico guardados (nunca contienen el PIN ni el número de serie); `crashes/`, un archivo por cierre inesperado (lo que hizo que PlayGuard se detuviera); `uploads/`, un acceso directo de Internet (`.url`) por informe enviado en línea; `play_timer/`, los archivos del temporizador de las herramientas de desarrollador. Cuántos informes, archivos de cierres y enlaces se conservan: *Acerca de › Ayuda y diagnóstico* (por número, antigüedad o tamaño; el más reciente siempre se queda)

Más detalles en [packaging/README.md](../../packaging/README.md) (en inglés).

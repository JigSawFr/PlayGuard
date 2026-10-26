<!-- translated from docs/install.md, sha256 900d9a183a17 -->
# Instalación

Cómo instalar, actualizar y quitar PlayGuard, y qué escribe en la tarjeta SD. Una vez instalado: [primeros pasos](getting-started.md). Volver al [README](../../README.md).

## Dónde conseguirlo

Elija una opción:

- **Homebrew App Store** / **App Store de sphaira** (el mismo catálogo) — Busque *PlayGuard* e instálelo. Las nuevas versiones se detectan automáticamente, después de una revisión: a menudo unos días después que en GitHub. PlayGuard sabe que se instaló desde ahí y recomienda actualizar desde la tienda.
- **Manual** — Descargue `playguard.zip` de la [última versión](https://github.com/JigSawFr/PlayGuard/releases/latest) y extráigalo en la **raíz** de la tarjeta SD. La aplicación queda en `sd:/switch/playguard/` y sus archivos en `sd:/config/playguard/`.
- **sphaira › GitHub** — El zip de la versión ya contiene la entrada (`/config/sphaira/github/playguard.json`): después de una primera instalación, actualice desde *GitHub* en sphaira.

El **sysmodule de recuperación** opcional se instala desde PlayGuard (*Consola › Módulos opcionales*, o *Seguridad › ¿Sin acceso?*), o a mano desde su propia descarga (`playguard-rescue.zip`): extráigalo también en la raíz de la tarjeta SD y luego reinicie la consola. Queda en `atmosphere/contents/4200000000505247/` y solo actúa cuando un archivo `RESCUE` se lo pide: consulte [¿Consola bloqueada?](troubleshooting.md#consola-bloqueada).

Luego abra PlayGuard **sobre un juego**, con el usuario de un adulto ([por qué](compatibility.md#lanzadores)).

## Actualizar

- **En PlayGuard:** *Preferencias › Actualizar con* elige cómo: en el propio PlayGuard (descarga la versión, la verifica, se reemplaza a sí mismo y se reinicia), con sphaira, con el Homebrew App Store o a mano. Con *Buscar actualizaciones cada día* activado, PlayGuard se abre mostrando la nueva versión cuando hay una.
- **El sysmodule de recuperación** va a la par de la versión de PlayGuard: después de cada actualización, vuelva a extraer el `playguard-rescue.zip` de esa versión (o use *Actualizar* en *El sysmodule de recuperación no está actualizado*, en la campana, o en *Consola › Módulos opcionales*) y luego reinicie la consola.
- **El agente del enlace remoto**, si está instalado, se ofrece en el siguiente inicio cuando PlayGuard trae uno más reciente ([Home Assistant](../home-assistant.md#keep-the-link-up-when-playguard-is-closed), en inglés).
- La configuración, el historial, los perfiles y los respaldos se conservan: están en `sd:/config/playguard/`, no junto a la aplicación.

## Quitarlo

Elimine `sd:/switch/playguard/` (la aplicación) y, para quitar también sus archivos, `sd:/config/playguard/`; con los sysmodules opcionales, también `sd:/atmosphere/contents/4200000000505247/` (recuperación) y `sd:/atmosphere/contents/4200000000504741/` (el agente), o quítelos antes en *Consola › Módulos opcionales*. Los límites, el PIN y las restricciones son los de la propia consola: **se quedan en la consola**. Para quitarlos también, use antes *Seguridad › Eliminar todo el control parental*.

## Archivos en la tarjeta SD

Todos en `sd:/config/playguard/` (la aplicación en sí queda en `sd:/switch/playguard/`: si la elimina o la reinstala, se conservan). Las versiones que los guardaban junto a la aplicación los movieron ahí en su primer inicio.

- **`config.json`** — Preferencias (idioma, tema, servidor NTP, *Proteger PlayGuard con el PIN*…); cada clave en [config.md](../config.md) (en inglés). Si no se puede leer, se guarda como `config.json.bad` (el anterior como `.bad.1`) y la configuración vuelve a sus valores predeterminados; PlayGuard lo avisa una vez al iniciar
- **`profiles/`** — Perfiles de límite de tiempo de juego guardados
- **`exports/`** — Exportaciones de la actividad
- **`nand/<system>/`** — Lo que pertenece a un sistema, cada uno con su propio control parental: `sysmmc/` para la sysMMC, `emummc/` para la emuMMC. Adentro: `history.json`, el historial de cambios (los 500 más recientes, incluidos los que la consola rechazó; si no se puede leer, se guarda como `history.json.bad`, el anterior como `.bad.1`, en lugar de sobrescribirlo, y PlayGuard lo avisa una vez); `state.json`, lo que *Bloquear todos los juegos*, el tiempo extra, un rebloqueo pendiente y la elección de firmware necesitan recordar ([config.md](../config.md#state-kept-between-launches), en inglés); `forwarders.json`, los forwarders que creó PlayGuard; `watch.json`, lo último que PlayGuard vio de la consola, para indicar qué cambió fuera de él ([config.md](../config.md#watchjson), en inglés; borrarlo no causa problemas); `backups/`, los respaldos de la configuración (nunca contienen el PIN); `cache/`, la última actividad de juego leída (todas las cuentas, y cada cuenta consultada), que se muestra de inmediato en el siguiente inicio mientras se vuelve a leer el registro. Los archivos de una versión anterior a estas carpetas pasan al primer sistema en el que se inicia PlayGuard
- **`cache/`** — Los íconos de los juegos (`icons/`, 16 MB como máximo, se vacía cuando cambia el idioma de la consola), compartidos por todos los sistemas, para no volver a leer el nombre y el ícono de cada juego en cada inicio; las notas de versión de Atmosphère, hekate, sys-patch y el firmware (`sys_notes.json`, para *Consola*) y las de PlayGuard (para *Información*); en modo desarrollador, la lista de *Instalar otra compilación* (`dev_builds.json`)
- **`github_token`** — El inicio de sesión en GitHub de *Enviar un informe…* y, en modo desarrollador, de *Instalar otra compilación*; se borra al cerrar sesión (*Información › Ayuda y diagnóstico › Cuenta de GitHub*, visible una vez iniciada la sesión)
- **`gate.nro`**, **`gate-<hash>.nro`**, **`gate_*.txt`** — La puerta de los forwarders protegidos (*Consola › Forwarders*): su programa, su horario libre, su horario en que nunca se abre y su pausa tras los rechazos, sus intentos hasta que PlayGuard los lee, y cuánto dura su pantalla de apertura (`gate_wait.txt`, que se lee en cada inicio)
- **`previous_build.json`**, **`boot_check.json`** — Después de instalar una compilación en su lugar: el nombre de la compilación reemplazada (guardada junto al `.nro` como `playguard.nro.previous`) y la comprobación del siguiente inicio (se borra en cuanto aparece su primera pantalla)
- **`rescue_report.txt`** — Lo deja el sysmodule de recuperación después de actuar, hasta que PlayGuard lo muestra al iniciar (si no lo puede leer, lo elimina y lo avisa)
- **`rescue_nonce`** — Un desafío aleatorio que el sysmodule de recuperación responde en su informe, para que PlayGuard distinga un informe escrito por el módulo de uno escrito a mano ([sysmodule/rescue/README.md](../../sysmodule/rescue/README.md), en inglés); uno nuevo en cada inicio
- **`sync.conf`** — Solo una vez configurado el *Acceso remoto*: la configuración del enlace, contraseña del broker incluida (nunca en un informe ni en un envío en línea)
- **`sync/`** — Solo con el *Acceso remoto*: los perfiles guardados, para el agente opcional (`profiles.txt`); lo que lee y escribe para un sistema está en el `nand/<system>/sync/` de ese sistema (registros, nombres de los juegos, lo que cambió, sus eventos). Una sysMMC y una emuMMC comparten `sync.conf`, pero son dos dispositivos en Home Assistant
- **`logs/`** — `reports/`, los informes de diagnóstico guardados (nunca contienen el PIN ni el número de serie); `crashes/`, un archivo por fallo (lo que hizo que PlayGuard se detuviera); `uploads/`, un acceso directo de Internet (`.url`) por cada informe enviado en línea; `play_timer/`, los archivos del temporizador de juego de las herramientas de desarrollo. Cuántos informes, archivos de fallo y enlaces se conservan: *Información › Ayuda y diagnóstico* (por cantidad, antigüedad o tamaño; el más reciente siempre se conserva)

Más detalles en [packaging/README.md](../../packaging/README.md) (en inglés).

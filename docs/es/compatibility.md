<!-- translated from docs/compatibility.md, sha256 f5648921d1b6 -->
# Compatibilidad

Firmware, Atmosphère y lanzadores compatibles, emuMMC y sysMMC, y cómo se compara PlayGuard con un sysmodule de sustitución. Volver al [README](../../README.md).

## Versiones compatibles

**Probado en una consola** — PlayGuard se ha ejecutado en:

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira y la Homebrew App Store

**Debería funcionar, sin probar** — nada en PlayGuard lo impide, pero nadie lo ha confirmado todavía ([se agradecen los informes](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Cada firmware necesita un Atmosphère que lo arranque: el indicado o uno más reciente.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Los mismos comandos que 23.0.1 en la tabla de [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Todas las funciones.

**Antes de 21.0.0, todo menos el temporizador de juego** — las restricciones, el PIN, el desbloqueo temporal, la actividad, el reloj de red, la aplicación móvil y el historial funcionan. El bloque de ajustes del temporizador (0x44 bytes) solo tiene esta estructura desde 21.0.0, así que PlayGuard desactiva lo que depende de él: los límites diarios, la alarma de hora de dormir, el tiempo extra y *Bloquear todos los juegos*.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x y anteriores** — Atmosphère 1.0.0 y anteriores

PlayGuard comprueba que Atmosphère está en marcha, no su versión: lo que cuenta es el firmware que arranca. *Consola* muestra ambos.

## Lo que hay que saber

### Lanzadores

Se recomienda abrirlo sobre un juego (title override). La aplicación indica si se ejecuta como aplicación (sobre un juego, o desde su propio forwarder en el menú HOME) o como applet (álbum). Sobre un juego, la consola cuenta el tiempo de PlayGuard como el de ese juego. En la actividad, va al usuario elegido al iniciar: ábrelo con el usuario de un padre o una madre, no con el de un niño. El temporizador de juego es el de la consola, el mismo para todos los usuarios: cuenta ese tiempo sea quien sea quien haya abierto PlayGuard, salvo si el temporizador está desactivado o si el PIN ha puesto en pausa el control parental ([detalles](features.md#el-pin-pone-en-pausa-el-control-parental-mientras-estás-en-playguard)).

### Nintendo Switch 2

No es compatible: no existe Atmosphère para ella.

### Firmware más reciente

Por encima del firmware más reciente probado en una consola, PlayGuard se abre en **solo lectura** y comprueba si una versión más reciente lo admite. Si es así, propone actualizar (en el propio PlayGuard, con sphaira o con la Homebrew App Store). Si no, tú eliges: solo lectura, solo lectura con las herramientas de desarrollador (para investigar el firmware) o todas las funciones bajo tu propia responsabilidad. La elección se puede recordar para ese firmware y esa versión de la aplicación; *Consola › Compatibilidad* vuelve a mostrar la pantalla.

### emuMMC y sysMMC

Cada una tiene su propio control parental, y PlayGuard solo ve el del sistema en el que se ejecuta. En la emuMMC, *Consola* y *Primeros pasos* lo indican; en la sysMMC también, cuando hay una emuMMC configurada en la tarjeta SD. Configura el control parental en ambas, u oculta el menú de arranque de hekate para que un niño no pueda elegir el otro sistema. PlayGuard guarda por separado el historial, las copias de seguridad y el estado de cada sistema (la sysMMC y la emuMMC, en `nand/`); las preferencias y los perfiles son comunes.

### ¿Puede provocar un baneo de la consola?

PlayGuard nunca contacta con Nintendo: en línea, solo llega a servidores de hora públicos y a GitHub (actualizaciones, y los informes que envíes). Lo que hace que baneen una consola es un firmware personalizado o una emuMMC que se conecta a Nintendo ([FAQ de switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), en inglés). No hay ningún baneo documentado por cambios del control parental o del reloj; no es una garantía.

### Una consola sin Internet

Para eso está PlayGuard, y el temporizador de juego sigue necesitando un reloj de red ajustado. Sin ningún servidor de hora al que preguntar, *Consola › Reloj de red › Medir el desfase del reloj* ofrece ajustarlo al reloj de la consola: comprueba antes el reloj de la consola, con *Sincronizar el reloj por Internet* desactivado, y activa ese ajuste solo después (consulta [reloj de red](features.md#ningún-servidor-responde)).

### Notificaciones en el móvil

Desde 22.0.0, una consola que sigue vinculada a la aplicación móvil puede avisar al padre o a la madre cuando se introduce el PIN en la consola ([switchbrew](https://switchbrew.org/wiki/22.0.0), en inglés). No se sabe si los desbloqueos del propio PlayGuard lo provocan.

## Por qué PlayGuard

A los niños les encantan los videojuegos, y los videojuegos son pantallas: ponerles un límite forma parte de cuidarlos. En una Switch sin modificar, de eso se encargaba la aplicación móvil de Nintendo. En cuanto la consola usa un firmware personalizado, se abre todo un mundo nuevo, y el control parental se pierde por el camino, porque la aplicación móvil ya no llega a la consola. Saber cuánto tiempo han jugado, fijar un límite, conseguir que dejen el juego sin pelea ni otro «cinco minutos más» se volvió difícil.

Las pocas herramientas que existían no servían: informes de actividad poco mantenidos y poco detallados, un control parental tosco y sin terminar. Tampoco quería un sustituto hecho a medida, ni un sysmodule siempre en marcha en segundo plano. La idea era **aprovechar al máximo el control parental de la propia consola**, con todo lo que ya hace —el límite, el PIN, los avisos, la suspensión— y volver a llevar sus ajustes a la consola. Eso es PlayGuard.

Manejar el control parental de la propia consola abre además la puerta a mucho más: enviar el tiempo de juego a un servidor en casa, recibir órdenes de él, Home Assistant, automatizaciones. El primer paso ya está aquí, un enlace opcional con Home Assistant por MQTT ([guía](../home-assistant.md), en inglés); hacia dónde podría ir después está en el [horizonte de la hoja de ruta](../../ROADMAP.md#horizon) (en inglés).

## ¿PlayGuard o un sysmodule de sustitución?

PlayGuard maneja el control parental integrado en la consola (el servicio `pctl`), sin conexión. El otro enfoque lo sustituye por un sysmodule, por ejemplo [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (un sysmodule y un overlay de Tesla / Ultrahand).

| | PlayGuard | Un sysmodule de sustitución |
|---|---|---|
| **Límite de tiempo de juego** | Uno para la consola (el temporizador de la propia consola) | Uno por cuenta de usuario |
| **PIN, avisos, suspensión** | Los de la consola | Los suyos |
| **En segundo plano** | Nada (el sysmodule de recuperación opcional solo actúa al arrancar, cuando un archivo `RESCUE` se lo pide; el agente del enlace con Home Assistant, también opcional, solo funciona una vez instalado) | Un sysmodule, desde el arranque |
| **Tras una actualización del firmware** | El control parental de la consola sigue funcionando; PlayGuard se abre en solo lectura hasta que una versión admita el firmware | El control depende de que el sysmodule siga funcionando |
| **Eliminado de la tarjeta SD** | Los límites se quedan en la consola | El control desaparece con él |

**¿Necesitas un límite distinto para cada niño?** Elige un sysmodule de sustitución: el temporizador de la consola tiene un solo límite para toda la consola. Si no, PlayGuard conserva el control parental de Nintendo y simplemente lleva sus ajustes a la consola.

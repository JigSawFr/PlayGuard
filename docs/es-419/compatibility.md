<!-- translated from docs/compatibility.md, sha256 2e539d2ecda1 -->
# Compatibilidad

Firmware, Atmosphère y lanzadores compatibles, emuMMC y sysMMC, y cómo se compara PlayGuard con un sysmodule de reemplazo. Volver al [README](../../README.md).

## Versiones compatibles

**Probado en una consola** — PlayGuard se ejecutó en estas:

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira y el Homebrew App Store

**Debería funcionar, sin probar** — nada en PlayGuard lo impide, pero nadie lo ha reportado todavía ([se agradecen los reportes](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Cada firmware necesita un Atmosphère que lo inicie: el indicado o cualquiera más reciente.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Los mismos comandos que 23.0.1 en la tabla de [switchbrew](https://switchbrew.org/wiki/Parental_Control_services) (en inglés).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Todas las funciones.

**Antes de 21.0.0, todo menos el temporizador de juego** — funcionan las restricciones, el PIN, el desbloqueo temporal, la actividad, el reloj de red, la app para celular y el historial. El bloque de configuración del temporizador de juego (0x44 bytes) solo tiene esta estructura desde 21.0.0, así que PlayGuard desactiva lo que depende de él: los límites diarios, la alarma de hora de dormir, el tiempo extra y *Bloquear todos los juegos*.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x y anteriores** — Atmosphère 1.0.0 y anteriores

PlayGuard comprueba que Atmosphère esté en ejecución, no su versión: lo que cuenta es el firmware que inicia. *Consola* muestra ambos.

## Para tener en cuenta

### Lanzadores

Se recomienda iniciarlo sobre un juego (title override). La aplicación indica si se ejecuta como aplicación o como applet (Álbum). Sobre un juego, la consola cuenta el tiempo en PlayGuard como tiempo de ese juego. En la actividad, se asigna al usuario elegido al iniciar: ábralo con el usuario de un adulto, no con el de un niño. El temporizador de juego es el de la consola, el mismo para todos los usuarios: cuenta ese tiempo sin importar quién abrió PlayGuard (salvo que el temporizador esté desactivado).

### Nintendo Switch 2

No es compatible: no existe Atmosphère para ella.

### Firmware más reciente

Por encima del firmware más reciente probado en una consola, PlayGuard se abre en **solo lectura** y comprueba si una versión más reciente es compatible con él. Si la hay, ofrece actualizar (en el propio PlayGuard, con sphaira o con el Homebrew App Store). Si no, usted elige: solo lectura, solo lectura con las herramientas de desarrollo (para investigar el firmware) o todas las funciones bajo su propia responsabilidad. La elección se puede recordar para ese firmware y esa versión de la aplicación; *Consola › Compatibilidad* vuelve a mostrar la pantalla.

### emuMMC y sysMMC

Cada una tiene su propio control parental, y PlayGuard solo ve el del sistema en el que se ejecuta. En emuMMC, *Consola* y *Primeros pasos* lo indican; en sysMMC también, cuando hay una emuMMC configurada en la tarjeta SD. Configure el control parental en ambas, u oculte el menú de arranque de hekate para que un niño no pueda elegir el otro sistema. PlayGuard guarda por separado el historial, los respaldos y el estado de cada sistema (la sysMMC y la emuMMC, en `nand/`); las preferencias y los perfiles son compartidos.

### ¿Puede hacer que baneen la consola?

PlayGuard nunca se comunica con Nintendo: en línea, solo se conecta a servidores de hora públicos y a GitHub (actualizaciones, y los informes que usted envíe). Lo que hace que baneen una consola es un firmware personalizado o una emuMMC que se conecta en línea con Nintendo ([preguntas frecuentes de switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), en inglés). No hay ningún baneo documentado relacionado con cambios en el control parental o en el reloj; eso no es una garantía.

### Una consola sin Internet

Para eso está PlayGuard, y el temporizador de juego igual necesita un reloj de red ajustado. Sin un servidor de hora al que consultar, *Consola › Reloj de red › Medir la diferencia horaria* ofrece ajustarlo al reloj de la consola: verifique primero el reloj de la consola, con *Sincronizar el reloj por Internet* desactivado, y active esa opción solo después (consulte [reloj de red](features.md#ningún-servidor-responde)).

### Notificaciones en el celular

Desde 22.0.0, una consola que sigue vinculada a la app para celular puede avisar al adulto cuando se ingresa el PIN en la consola ([switchbrew](https://switchbrew.org/wiki/22.0.0), en inglés). No se sabe si los desbloqueos de PlayGuard lo activan.

## Por qué PlayGuard

A los niños les encantan los videojuegos, y los videojuegos son pantallas: ponerles un límite es parte de cuidarlos. En una Switch sin modificar, la app para celular de Nintendo se encargaba de eso. Cuando la consola usa un firmware personalizado, se abre todo un mundo nuevo, y el control parental se pierde, porque la app para celular ya no llega a la consola. Saber cuánto tiempo jugaron, poner un límite, lograr que el juego se detenga sin pelear ni otros "cinco minutitos más" se volvió difícil.

Las pocas herramientas que existían no cumplían: reportes de actividad poco mantenidos y poco detallados, controles parentales básicos e incompletos. Tampoco quería un reemplazo hecho a medida, ni un sysmodule siempre en ejecución en segundo plano. La idea era **reutilizar al máximo el control parental de la propia consola**, con todo lo que ya hace —el límite, el PIN, los avisos, la suspensión— y traer de vuelta su configuración a la consola. Eso es PlayGuard.

Manejar el control parental de la propia consola también abre la puerta a mucho más: enviar el tiempo de juego a un servidor en casa, recibir órdenes de él, Home Assistant, automatizaciones. Hacia dónde podría ir está en el [horizonte de la hoja de ruta](../../ROADMAP.md#horizon) (en inglés).

## ¿PlayGuard o un sysmodule de reemplazo?

PlayGuard maneja el control parental integrado en la consola (el servicio `pctl`), sin conexión. El otro enfoque lo reemplaza con un sysmodule, por ejemplo [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (un sysmodule y un overlay de Tesla / Ultrahand).

| | PlayGuard | Un sysmodule de reemplazo |
|---|---|---|
| **Límite de tiempo de juego** | Uno para la consola (el temporizador de la propia consola) | Uno por cuenta de usuario |
| **PIN, avisos, suspensión** | Los de la consola | Los suyos propios |
| **En segundo plano** | Nada (el sysmodule de recuperación opcional solo actúa al arrancar, cuando un archivo `RESCUE` se lo pide) | Un sysmodule, desde el arranque |
| **Después de una actualización del firmware** | El control parental de la consola sigue funcionando; PlayGuard se abre en solo lectura hasta que una versión sea compatible con el firmware | El control depende de que el sysmodule siga funcionando |
| **Eliminado de la tarjeta SD** | Los límites siguen en la consola | El control desaparece con él |

**¿Necesita un límite distinto para cada niño?** Elija un sysmodule de reemplazo: el temporizador de la consola tiene un solo límite para toda la consola. Si no, PlayGuard conserva el control parental de Nintendo y simplemente lleva su configuración a la consola.

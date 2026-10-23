<!-- translated from docs/features.md, sha256 7e41c41641bb -->
# Funciones

Qué hace cada pestaña, cómo escribe PlayGuard sin riesgos y qué cuenta la consola como tiempo de juego. ¿Nuevo en PlayGuard? Empieza por la [guía de inicio](getting-started.md). Volver al [README](../../README.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

Las capturas muestran el tema oscuro, el predeterminado de PlayGuard (en inglés).

| Hoy | Consola |
|---|---|
| ![Hoy](../../images/screenshots/dashboard.png) | ![Consola](../../images/screenshots/console.png) |
| **Tiempo de juego** | **Tiempo de juego › edición de un día** |
| ![Tiempo de juego](../../images/screenshots/play_timer.png) | ![Edición de un día](../../images/screenshots/per_day.png) |
| **Actividad** | **Un juego** |
| ![Actividad](../../images/screenshots/activity.png) | ![Un juego](../../images/screenshots/activity_game.png) |
| **Restricciones** | **Consola › Reloj de red** |
| ![Restricciones](../../images/screenshots/restrictions.png) | ![Reloj de red](../../images/screenshots/clock.png) |
| **Seguridad** | **Preferencias** |
| ![Seguridad](../../images/screenshots/security.png) | ![Preferencias](../../images/screenshots/preferences.png) |

## Controles

| Botón | Qué hace |
|---|---|
| ↑/↓ | Moverse |
| Ⓐ | Confirmar |
| Ⓑ | Volver o cancelar (dos veces en la barra lateral para salir) |
| Ⓧ | Actualizar (en las pestañas que leen la consola) |
| Ⓨ | Notificaciones, desde cualquier pestaña |
| **−** | Idioma y tema, desde cualquier pestaña (el pie de página no los muestra; la campana, el idioma y el tema de la cabecera sí) |
| **+** | Guarda los límites por día (y la semana editada en *Tiempo de juego*) |
| **ZR** | *Deshacer*, durante unos segundos después de guardar un cambio |

La aplicación está organizada en pestañas, como la configuración de la consola; cada una tiene su sección a continuación.

- **Lo que ha fallado se indica en un cuadro de diálogo; lo que ha funcionado, en un aviso breve.** Un cuadro de diálogo solo dice lo que se aplica a esta consola en ese momento: que el juego en curso se suspendería, solo mientras hay un juego abierto; que la aplicación móvil podría sobrescribir un cambio, solo mientras está vinculada y puede sincronizarse.
- **La cabecera** indica, en todas las pestañas, cuándo el control parental está desbloqueado temporalmente (su píldora), y cuándo la aplicación está en solo lectura (su píldora dice entonces *Solo lectura*, y las acciones que escribirían aparecen en gris y se rechazan indicando dónde desactivarlo); al tocarla, lleva a donde se deshace (*Hoy › Reactivar*, la pantalla del firmware o *Ayuda y diagnóstico*). Un pequeño ⊖ delante del idioma y un Ⓨ delante de la campana indican los botones que los abren desde cualquier pestaña.
- **En solo lectura**, las acciones siguen en su sitio, en gris, y explican por qué al pulsarlas.

## El aspecto: Cadran

El aspecto de PlayGuard se llama Cadran: una barra de iconos para las pestañas, una paleta azul marino o blanco frío (según el tema), tarjetas en lugar de listas. Cada pestaña se abre con lo más importante (un anillo, mosaicos, tarjetas, chips) y sus demás ajustes debajo.

- **La píldora de la cabecera** indica de qué trata la pestaña (*Control parental activo*, *Temporizador activo*…); mientras el control parental está desbloqueado, lo indica en todas las pestañas, y al tocarla se abre *Hoy › Reactivar*. El texto secundario nunca baja de 15 px, para poder leerlo desde el sofá en una tele. El reloj del pie de página muestra las horas y los minutos (*16:00*).
- **Los cuadros de diálogo** son tarjetas con los botones en forma de píldora, la confirmación en turquesa (en coral cuando quita o detiene algo).
  - Las opciones se abren como una tarjeta encima de las indicaciones de los botones, con la actual sobre un turquesa suave con su marca de verificación. El contorno turquesa es solo del foco, en todas partes: un nivel, una cuenta, una opción en vigor conserva el fondo suave.
  - Un cuadro de diálogo largo se difumina en su parte inferior mientras sigue habiendo más, y su advertencia (un juego a punto de suspenderse) va primero, en una tarjeta ámbar.
  - Un fallo lleva como título lo que ha fallado, el código de error en pequeño bajo el motivo, y ofrece *Enviar un informe* cuando sirve de ayuda.
  - Los mensajes aparecen como tarjetas bajo la cabecera, con una marca de verificación turquesa para lo que se ha hecho y una marca ámbar para lo que se ha rechazado.
- **Las demás pantallas** adoptan el mismo aspecto: el editor por día de un perfil muestra la semana como en *Tiempo de juego* (se edita igual, con los atajos como mosaicos); *Comprobar los juegos instalados* y *Todos los juegos* presentan sus líneas en tarjetas; *Perfiles* y *Ayuda y diagnóstico* dan a cada línea su icono.
- **Tamaño del texto** (*Preferencias*, o **AA** en la cabecera):
  - *Automático*, la opción predeterminada, agranda todos los textos un 25 % mientras la consola está en la base, en la tele, y los devuelve a su tamaño en modo portátil; *Más grande* los mantiene agrandados, *Normal* nunca.
  - La cabecera, la barra lateral y el pie de página conservan su tamaño (no tienen sitio para más), igual que los nombres de los niveles de Restricciones, que ya son grandes.
  - Cambia al instante, en todas las pantallas, sin reiniciar.
  - El 25 % sale de una tele de 55" en 4K: el texto más pequeño (15 px) mide entonces 14,6 mm de alto en mayúsculas, 20′ de arco a 2,5 m, donde la lectura es cómoda (16′ en tamaño normal, el mínimo indispensable).
- **Efectos visuales** (*Preferencias*): *Más ligeros con batería*, la opción predeterminada, prescinde del halo de fondo, del latido del anillo de la pantalla del niño en sus últimos minutos y del barrido del arco hasta un nuevo valor mientras no hay ningún cargador conectado (en modo portátil, desenchufada); *Siempre completos* los mantiene. Se vuelve a leer cada 5 segundos, así que al enchufar la consola vuelven.
- **Deshacer:** después de guardar un cambio, **ZR** *Deshacer* permanece unos segundos en el pie de página: abre ese cambio en el historial de cambios, para restablecer el valor anterior. Un historial de cambios vacío lo indica en una tarjeta.

## Notificaciones — lo que requiere tu atención, desde cualquier pestaña

Una campana en la cabecera, a la izquierda de la versión, reúne lo que requiere la atención del padre o la madre; Ⓨ abre la lista desde cualquier pestaña (el contador de la campana indica *2* cuando hay dos cosas por hacer).

A la izquierda de la campana, el idioma (su código, *FR*, *EN*…) y el tema (un sol para el claro, una luna para el oscuro, un círculo medio lleno cuando sigue a la consola), y luego el tamaño del texto (**AA**, con su A grande en turquesa mientras el texto está agrandado): al tocarlos se abre la misma lista que en *Preferencias*, y **−** los ofrece desde cualquier pestaña. Un nuevo tema o idioma se aplica al instante, y cada pantalla se rehace en la misma pestaña con el foco en la misma línea, chino y su propia fuente incluidos.

### Por hacer

Cada uno con una señal de advertencia ámbar, contado en ámbar en la campana; se queda hasta que se resuelve:

- todavía sin PIN, o un PIN que cualquiera probaría primero (0000, 1234, 1212, 2580…: se evalúa en la consola, nunca se muestra);
- un PIN en la consola mientras PlayGuard se abre sin él (*Proteger PlayGuard con el PIN* en *Nunca*);
- el control parental desbloqueado temporalmente (*Reactivar*);
- tiempo extra de un día anterior aún por retirar;
- la alarma de «Se acabó el tiempo» desactivada;
- cambios hechos fuera de PlayGuard;
- la consola que no cuenta el tiempo de juego;
- un reloj de red impreciso.

### Para información

Cada uno con una «i», un punto en la campana; se puede marcar como leído:

- un límite todavía rebajado por *No jugar más hoy* un día anterior (*Restablecer*);
- una versión nueva instalada (*Novedades*; tras una actualización en su lugar, la compilación anterior conservada), o una versión más reciente de PlayGuard;
- un informe enviado en línea (*Mostrar el enlace*), o PlayGuard cerrado de forma inesperada la última vez (*Enviar un informe*);
- el tiempo de juego de la semana (cada 7 días hasta que se lee; con la comparación por día con la semana anterior; *Mostrar su código QR* abre el resumen de la semana en *Actividad*), y un límite superado dos o más de los últimos 7 días mientras la consola solo avisa;
- una copia de seguridad de los ajustes por hacer (ninguna todavía, o ninguna en 30 días);
- todos los juegos bloqueados desde hace una semana;
- una cuenta de usuario nueva;
- la aplicación móvil vinculada;
- el reloj de red desfasado al iniciar.

### Consola

Las entradas que tratan de la consola y no del control parental van al final, en una línea que Ⓐ abre (abierta de entrada cuando no hay nada más). Las que están por hacer se cuentan en la insignia de la pestaña Consola, no en la campana:

- juegos instalados que no se pueden iniciar (*Comprobar los juegos instalados*), parches de juegos que hay que revisar;
- la tarjeta SD casi llena (menos de 500 MB);
- un archivo del propio PlayGuard que no se ha podido apartar o escribir en la tarjeta SD;
- el sysmodule de recuperación no actualizado (no es el que acompaña a esta versión, o se instaló antes de que indicara su versión; *Actualizar* instala el que trae esta versión), y después la consola por reiniciar para que se use, hasta que se reinicie;
- una compilación que no terminó de iniciarse y de la que se volvió atrás (*Enviar un informe*).

Las de información: una compilación de la que se volvió atrás, un firmware no probado con esta versión, un Atmosphère, hekate o firmware más reciente (comprobado con *Buscar actualizaciones cada día*; la entrada del firmware aconseja esperar a que Atmosphère lo admita), el número de serie no oculto en la emuMMC, el control parental propio del otro sistema, un archivo del propio PlayGuard encontrado dañado y apartado (un aviso al iniciar lo señala).

### Cómo se comportan las entradas

- Cada sección enumera sus entradas en un orden fijo, sea cual sea la comprobación que terminó primero.
- **En la barra lateral**, una insignia ámbar junto al nombre de una pestaña cuenta sus entradas por hacer, allí donde se resuelven: la pestaña Seguridad (sin PIN, un PIN débil, PlayGuard sin su PIN, desbloqueado temporalmente), Tiempo de juego (la alarma desactivada, la consola que no cuenta), Hoy (cambios hechos fuera de PlayGuard, tiempo extra por retirar), Consola (sus entradas de arriba, y el reloj de red). Las entradas ignoradas y las de información no se cuentan.
- En la pestaña, la misma insignia está en la fila donde se resuelve cada una (*Reloj de red*, *Comprobar los juegos instalados*, *Almacenamiento*, *Parches de juegos*; *Definir o cambiar el PIN*, *Proteger PlayGuard con el PIN*; *Avanzado* de Tiempo de juego para la alarma), y la pestaña se abre en la primera de ellas. Una fila sin valor propio indica antes, en ámbar, para qué es la insignia (*Fácil de adivinar*, *Impreciso*).
- **Ⓐ** en una entrada aplica la solución (*Activarla*, *Ajustarlo*, *Actualizar*…) o abre la pestaña donde se hace. Una entrada desaparece sola en cuanto se resuelve lo que indica.
- **Ⓧ** marca como leída una entrada de información; **Ⓨ** las marca todas. Una entrada marcada como leída sigue leída en el siguiente inicio, hasta que cambie su texto.
- **Ignorar:** algunas entradas por hacer se pueden ignorar con Ⓧ, tras una confirmación y el PIN: un PIN que cualquiera probaría primero, PlayGuard abriéndose sin el PIN, la alarma de «Se acabó el tiempo» desactivada, la tarjeta SD casi llena (nunca la falta de PIN, el control desbloqueado, el reloj, el temporizador que no cuenta ni los archivos del propio PlayGuard). Una entrada ignorada deja de contarse y aparece al final, en *Ignorados*, donde Ⓐ hace que vuelva a contarse. Vuelve sola 30 días después (salvo si *Volver a contar los avisos ignorados tras 30 días* está desactivado), cuando lo que indica se resuelve y luego vuelve a ocurrir, o, en el caso del PIN, con un PIN nuevo de otra longitud.
- *Preferencias › Notificaciones* desactiva tres tipos: versiones de Atmosphère, hekate y el firmware; el tiempo de juego de la semana; avisos (copias, segundo sistema de la consola, juegos bloqueados).
- El estado que hay detrás se lee al iniciar PlayGuard, cada minuto en la pantalla principal y cada 5 s mientras se muestra *Hoy*.

## Hoy — el día de un vistazo

*[Pruébalo en el simulador ›](https://playguard.link/simulator/)*

### El anillo y la línea

**El tiempo restante** en un anillo: un día sin juego indica *Sin jugar*, no *0 min*; el tiempo extra dado hoy aparece como un segmento más claro en su extremo, y el anillo se desplaza hasta cada nuevo valor.

Debajo, lo que detiene el juego primero:

- antes del final del tiempo o de la alarma de hora de dormir, el resto del día como una línea, con el momento actual a su izquierda, y cada uno de ellos como una marca con su hora (*16:40 tiempo agotado*, *21:00 hora de dormir*), en turquesa hasta la primera, en ámbar en sus últimos 15 minutos;
- si no, en palabras: la alarma de hora de dormir una vez agotado el tiempo del día, cuándo se puede volver a jugar; mientras el control parental está desbloqueado, el límite de hoy en pausa y por qué.

### Las acciones

- **Tiempo extra**, sus cantidades en una fila (+15 min, +30 min, +1 h), cada una a una pulsación, y luego **…** para cualquier otra (una duración, otro día): una cantidad seleccionada desplaza el final del tiempo en la línea (*con 15 min más*), o indica en la marca de la alarma de hora de dormir que esta sigue llegando antes; la primera destaca en turquesa cuando quedan 15 min o menos, o nada.
- **Terminar por hoy**, **Mostrar al niño**.
- **Límite de hoy…**: el límite de hoy elegido en el momento, escrito tras una confirmación.
- **El perfil en vigor**, por su nombre, una vez guardado un perfil (*Perfiles…* cuando no coincide ninguno): Ⓐ los enumera, con el que está en vigor marcado, para aplicar otro tras una confirmación (los días que cambian), o *Gestionar los perfiles…*.
- Mientras el control parental está desbloqueado, **Reactivar** va primero, a todo el ancho de la fila, y *Tiempo extra* se aparta.
- Si no hay límite ningún día, **Definir los límites de la semana** abre la semana en *Tiempo de juego*.

### Las tarjetas y las píldoras

- La **alarma de hora de dormir** (cuándo se puede volver a jugar, o dentro de cuánto suena cuando la consola no lo indica; desactivada, *Desactivada* en grande y Ⓐ *Configurarla*), el **control parental** y el **reloj de red**, como tarjetas.
- El estado de la consola como **píldoras**:
  - una indica si la redirección DNS de Atmosphère bloquea los servidores de Nintendo, y Ⓐ en ella, cuáles y qué cambia eso: la aplicación móvil no se sincroniza y el reloj de red ya no se ajusta solo;
  - otra, al abrirlo sobre un juego, que el tiempo pasado en PlayGuard cuenta, y Ⓐ explica cómo;
  - un juego que se deja abierto en el menú HOME aparece como una píldora ámbar (*sigue contando*, y Ⓐ explica por qué), cuando la píldora de PlayGuard abierto sobre un juego no lo dice ya.
- Las píldoras ponen primero lo que requiere la atención del padre o la madre (ámbar, con un triángulo), luego lo que está activado (turquesa, con una marca de verificación) y después el resto (gris, con un punto): la forma lo dice tanto como el color.
- Lo que está bien y lo que solo se informa se agrupan en una sola píldora después de las que requieren la atención del padre o la madre (*✓ 3 comprobaciones correctas*); Ⓐ sobre ella las muestra, mientras la pestaña siga abierta.
- Bajo las píldoras, el tiempo extra aún por retirar (Ⓐ lo retira) y la alarma de «Se acabó el tiempo» mientras está desactivada (Ⓐ la vuelve a activar).

### Primeros pasos

Hasta que se definen el PIN y los límites de la semana, una tarjeta encima de las demás enumera los tres pasos (el PIN, un límite diario, el reloj de red), cada uno marcado en turquesa una vez hecho o rodeado en ámbar, con cuántos se han hecho (*1 de 3*); Ⓐ en uno lo hace ahí mismo.

### ¿Está contando la consola?

El tiempo del propio PlayGuard cuenta mientras se ejecuta como aplicación, así que con un límite hoy el tiempo restante debería bajar mientras está abierto. Cuando no se ha movido durante unos 90 s en pantalla (temporizador activado, un límite hoy, sin desbloquear, sin abrirlo desde el álbum), una línea ámbar bajo el tiempo jugado indica *La consola no está contando el tiempo de juego ahora mismo*, con las causas probables:

- el reloj se atrasó (volverá a contar más tarde);
- el reloj de red nunca se ajustó (*Consola › Reloj de red*);
- se acaba de escribir un límite (inicia un juego un minuto y vuelve a comprobarlo).

Desaparece en cuanto el tiempo restante vuelve a moverse. Tiempo de juego también lo indica.

### Cambiado fuera de PlayGuard

El tiempo de juego de hoy que vuelve a empezar el mismo día (un cambio de reloj lo provoca), el reloj de la consola movido más de 5 minutos, límites distintos de los que PlayGuard vio por última vez (la aplicación móvil, otra herramienta): una tarjeta ámbar encima del anillo hasta que se descarta (Ⓐ) o hasta el día siguiente, y una entrada en el historial de cambios. PlayGuard no se ejecuta en segundo plano: solo ve lo que cambió mientras estaba abierto, o desde la última vez que lo estuvo, cuando vuelve a leer la consola.

### Al llegar al límite

La consola muestra «Se acabó el tiempo» y luego suspende el juego (sin «continuar») o solo avisa: *Cuando se acabe el tiempo* muestra cuál de las dos y permite cambiarlo, una vez fijado un límite o una hora de dormir ([detalles, en inglés](../parental-controls.md#when-the-time-runs-out)).

### Tiempo extra y no jugar más hoy

- **Tiempo extra hoy**: +15 min, +30 min, +1 h de forma predeterminada.
- **No jugar más hoy**: límite 0 solo para hoy, *ahora* o *dentro de 5 minutos* cuando se conoce el tiempo jugado (el límite pasa entonces a ser el tiempo jugado + 5 min, el tiempo justo para guardar). Al bloquearse aparece el «Se acabó el tiempo» de la consola, y se puede perder el progreso sin guardar; la confirmación lo dice.
- **Al día siguiente** —al iniciar, o a medianoche si la aplicación está abierta— PlayGuard vuelve a poner el límite habitual:
  - tras el tiempo extra, lo hace solo, sin preguntar y sin PIN (solo baja el límite, como al reactivar el control parental), desbloqueando un momento si el temporizador está contando y volviendo a bloquear enseguida. Espera mientras no puede escribir (modo de solo lectura, un firmware que no admite, *Bloquear todos los juegos* activado) y, una semana después el mismo día, mientras el límite más bajo detendría el juego en curso (entonces, al día siguiente). Para conservar más tiempo un día concreto, cambia el límite de ese día;
  - tras *No jugar más hoy*, pregunta si mantener 0 o restablecer el límite (con el PIN, como cualquier cambio que sube un límite), o lo restablece como cualquier cambio (*Tiempo de juego › Tras «No jugar más hoy», restablecer el límite*); restablecer «sin límite» siempre se pregunta.
- **Tiempo extra otro día** (*Tiempo extra › Otro día…*, al final de su lista): mañana o uno de los cinco días siguientes, cada uno con su límite, y luego la cantidad. El límite de ese día se sube ya, solo para esa fecha, y se restablece solo (solo baja el límite) la próxima vez que se abre PlayGuard una vez pasado el día; no en una consola en la que no se abre PlayGuard, donde el mismo día de la semana siguiente también lo conserva. Uno cada vez: la lista ofrece entonces quitarlo (su límite se restablece al instante). Si entretanto cambia el límite de ese día (la semana, un perfil…), ese cambio prevalece: no se restablece nada.

### La pantalla del niño

*Mostrar al niño* en *Hoy*, para entregar la consola:

- solo el tiempo restante (mientras el control parental está desbloqueado, el límite de hoy, en pausa), cuándo termina el juego (*Termina hacia las 16:40*; cuando la alarma de hora de dormir llega antes, el anillo cuenta atrás hasta ella, *45 min antes de dormir*, como en la última hora de un día sin límite), la hora de dormir y lo que pasa cuando se acaba el tiempo;
- pensada para leerse desde el otro lado de la habitación, en la tele; su texto conserva su tamaño sea cual sea el tamaño del texto;
- en los últimos 5 minutos (de juego o antes de la hora de dormir), el final se vuelve ámbar y el anillo late despacio; la hora de dormir se vuelve ámbar en sus últimos 30;
- Ⓐ e Ⓨ, los botones del padre o la madre, aparecen en pequeño arriba a la derecha, para que el pie de página muestre solo el del niño;
- Ⓐ vuelve a las pantallas del padre o la madre con el PIN; Ⓨ da tiempo extra desde ahí, tras el PIN (las cantidades de *Tiempo extra*, sin salir de la pantalla del niño); Ⓑ sale de PlayGuard (preguntando antes si reactivar el control parental, como al salir).

Con el texto más grande, la pestaña Hoy sigue cabiendo en una sola pantalla: la línea encima del titular solo indica el día y su límite (*JUEVES · 2 h*), y los botones y las tarjetas se aprietan un poco. Se actualiza cada 5 s (Ⓧ actualiza ahora).

## Tiempo de juego — la semana es el editor

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=play_timer)*

### Editar la semana

Ⓐ en un día lo edita:

| Botón | En la edición |
|---|---|
| ↑/↓ | Cambiarlo de 15 en 15 min, de hora en hora al mantenerlos pulsados |
| ←/→ | Pasar al día siguiente |
| Ⓛ | Copiarlo de lunes a viernes |
| Ⓡ | Copiarlo al fin de semana |
| Ⓨ | Sin límite |
| Ⓧ | Una duración exacta (*Introducir una duración…*: ↑/↓ de 15 en 15 min, de hora en hora al mantenerlos pulsados, Ⓛ/Ⓡ de 5 en 5 min, con OK seleccionado) |
| Ⓐ o Ⓑ | Terminar la edición (Ⓐ indica qué día: *Listo el lunes*; aún no se guarda nada) |

- En la pantalla táctil, al tocar un día se edita, y luego un toque en su parte de arriba o de abajo lo cambia en 15 min.
- Los cambios se guardan juntos, con **+** o *Guardar*, como en el editor por día (la confirmación enumera los días cambiados, antes → después); *Descartar* los deshace.
- Dentro de la barra de cada día, el tiempo jugado la última vez que tocó ese día (hoy incluido, todas las cuentas). Pasado el límite, la barra está llena y su parte superior en coral, en la misma proporción que el tiempo por encima del límite respecto al tiempo jugado; una leyenda bajo la semana lo indica (*límite superado*), y el límite también está en coral. Los días posteriores a hoy muestran el juego de la semana pasada, más tenue (seleccionado: *El viernes pasado: 2 h 10 jugado de 2 h*).
- Un día cambiado y aún sin guardar tiene su límite en ámbar y su barra rodeada de ámbar; bajo la semana, las cifras del día seleccionado (*jueves: 1 h 20 jugado de 2 h*).
- Encima de la semana, en ámbar, un límite alcanzado y lo que hace entonces la consola, o la consola que no cuenta el tiempo de juego.
- `0` minutos significa *sin juego ese día*; *Quitar el límite de tiempo de juego* desactiva el temporizador.

### Chips y ajustes rápidos

**Chips** encima y debajo de la semana abren directamente sus opciones: los perfiles (cuántos hay guardados), el mismo límite todos los días, la alarma de hora de dormir y *Cuando se acabe el tiempo*. *Quitar el límite* es la última línea de la pestaña, aparte (el historial lo deshace).

- **Mismo límite todos los días:** una lista rápida o una duración exacta (*Introducir una duración…*: ↑/↓ de 15 en 15 min, de hora en hora al mantenerlos pulsados, Ⓛ/Ⓡ de 5 en 5 min).
- **Entre semana y fin de semana ›:** el editor, que pide el límite de lunes a viernes y luego el del fin de semana.
- **Tiempo extra o no jugar más hoy ›** abre *Hoy*, donde están los dos. Una nota debajo dice qué hace la consola al llegar al límite. Mientras el control parental está desbloqueado temporalmente, es *posible* que el tiempo de juego no se cuente: sigue siendo una pregunta abierta ([documentación, en inglés](../parental-controls.md#still-open)).
- Cada confirmación enumera los días que cambian, antes → después.

### Perfiles

- **Perfiles** guardados en la tarjeta SD: aplicar (un perfil que ya está en vigor simplemente lo indica), editar, renombrar o eliminar uno; guardar los límites actuales o crear uno nuevo.
- Cualquier nombre, en cualquier alfabeto (*École*, *周末*, *Выходные* …): un nombre sin letras latinas recibe un nombre de archivo propio, y se detectan dos nombres que darían el mismo archivo.
- **Perfil programado** (*Perfiles › Perfil programado*): un perfil para un periodo —vacaciones, un viaje—, con su primer y su último día elegidos entre los 60 días siguientes.
  - Cuando empieza el periodo, PlayGuard pregunta si aplicarlo (con los días que cambia dibujados bajo la pregunta); cuando termina, si restablecer los límites que reemplazó o conservarlos.
  - Se pregunta una vez al día, en la pantalla principal, cuando no hay nada más delante: PlayGuard solo actúa cuando se abre, así que una consola en la que no se abre conserva los límites que tiene. Un periodo que pasó sin que el perfil se llegara a aplicar se olvida.
  - Uno cada vez; la línea lo cambia, lo quita o lo termina.

### Bloquear todos los juegos

*Bloquear todos los juegos*, bajo los ajustes de la semana: un interruptor que pone a 0 el límite de cada día, de modo que hace falta un PIN para iniciar un juego; un bloqueo ligero sin clasificación por edades ni límites de comunicación.

- Bloquea el inicio de los juegos, no el menú HOME, y necesita un PIN.
- Los límites anteriores vuelven al desactivarlo.
- Mientras está activado, se rechazan *tiempo extra* y *no jugar más hoy*, y los límites fijados de otra forma (un perfil, una copia de seguridad, el historial…) lo sustituyen.

### Alarma de hora de dormir

- La hora de la alarma (de 16:00 a 23:45, o *Desactivado*) y cuándo se puede volver a jugar (de 05:00 a 09:00), la misma todos los días, ajustadas con la palanca: arriba y abajo un cuarto de hora, L y R una hora.
- O, con Ⓨ *Por día* en el cuadro de diálogo de la alarma, de lunes a viernes, el fin de semana o un solo día, y los demás días conservan la suya (cada grupo de días aparece con su hora). Cuando los días difieren, la pestaña indica *Varía según el día*, y la semana en *Tiempo de juego* muestra la alarma de cada día bajo su nombre.
- Su posición en los ajustes del temporizador, deducida de los ajustes de la aplicación móvil, está verificada en una consola (22.0.0); aun así, PlayGuard solo la cambia cuando la consola indica lo que PlayGuard lee ahí, comprueba la respuesta de la consola tras el cambio y restablece los ajustes anteriores si no coincide.

### Ajustes del tiempo extra y Avanzado

- **Tiempo extra:** las cantidades que se ofrecen (+15/+30/+1 h, +10/+20/+30 min …) y restablecer el límite habitual sin preguntar tras *No jugar más hoy* (el tiempo extra siempre se retira sin preguntar). Ambas cosas se hacen la próxima vez que se abre PlayGuard, a partir del día siguiente: una consola en la que no se abre conserva el límite de ese día.
- **Avanzado ›**, una pantalla propia: el interruptor de las acciones de depuración y, mientras está activado, activar o desactivar la alarma de «Se acabó el tiempo» y pausar / reanudar la cuenta atrás. El mismo interruptor permite que una copia de seguridad o el historial restablezcan la alarma.

## Actividad — quién ha jugado a qué, y cuánto tiempo

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=activity)*

Tiempo por juego **hoy**, en los **últimos 7 días** y **en total**, a partir del propio registro de actividad de la consola, para todas las cuentas o para **una cuenta de usuario** (las cuentas como chips arriba, cuando la consola tiene varias). PlayGuard empieza en la última cuenta mostrada, o en la elegida en *Cuenta que se muestra primero* (todas las cuentas, o una; al final de la pestaña, con *Todos los juegos*).

### Arriba

- Las cuentas como chips, el periodo y *Exportar o compartir*.
- Hoy, los últimos 7 días y desde siempre como cifras. Bajo los últimos 7 días, la comparación con los 7 anteriores, por día: ▲ *+20 min/día vs. semana anterior* en ámbar, ▼ menos en turquesa, *como la semana anterior* con un margen de 5 minutos.
- Los cuatro juegos más jugados como tarjetas (Ⓐ abre uno); en los últimos 7 días, el que más ha aumentado respecto a la semana anterior indica cuánto, a partir de media hora (▲ *+2 h vs. semana anterior*).
- Si no se ha jugado nada en el periodo, una tarjeta lo indica, con *Ver desde siempre* cuando hay algo que ver.

### Gráficos y resumen

- **Un gráfico de los últimos siete días** (hoy a la derecha, con una leyenda debajo), con el límite de esta semana para cada día como una línea y el tiempo que lo supera en ámbar, marcado con !; totales de hoy, de la semana y de siempre.
- **Un resumen** del periodo elegido (hoy, los últimos 7 días, los **últimos 30 días** o desde siempre), para la cuenta mostrada: media por día, el juego más jugado, los días jugados y el de más actividad (últimos 7 días), la sesión media (desde siempre).
- **Horas de juego, últimos 7 días**: un cuadrado por cada hora de cada día, empezando por el lunes, más intenso cuanto más se ha jugado (todos los juegos, la cuenta mostrada); luego las sesiones (el juego con pausas de menos de 5 minutos cuenta como una sola sesión, aunque se cambie de juego) y su duración media, la más larga y su día, las dos horas seguidas con más juego, y el tiempo jugado entre las 21:00 y las 06:00.

### Todos los juegos y la página de un juego

- **Todos los juegos**, en una pantalla propia: cada juego jugado en el periodo, el más jugado primero, como **cuadrícula de portadas** o como **lista** (Ⓨ cambia, y la elección se conserva), con las cifras del juego seleccionado siempre arriba: su icono y su nombre, el tiempo de juego del periodo, el total, los inicios y la última partida. Ⓧ elige el periodo (la pestaña Actividad lo sigue).
- Los iconos se leen alrededor del juego seleccionado (no en modo applet, para ahorrar memoria); un mando gris los sustituye hasta que llegan, y para un juego que no tiene.
- Una biblioteca grande muestra sus 50 primeros juegos y luego *Mostrar todos los juegos* (100 juegos como máximo en modo applet); la exportación siempre los incluye todos. PlayGuard lee 256 juegos como máximo (primero los de esta semana): por encima, la pestaña Actividad, Todos los juegos y la exportación (salvo CSV) indican que faltan algunos, y el total de siempre los deja fuera.
- **Ⓐ en un juego:** sus últimos siete días en barras, los inicios, la primera y la última partida, el tiempo por cuenta de usuario. Los juegos eliminados conservan sus cifras de siempre, incluso uno eliminado antes de instalar PlayGuard mientras el registro de juego de la consola todavía lo nombre, y su nombre en cuanto PlayGuard lo ha visto (si no, *Juego eliminado* y su ID de título).

### Cuentas

- El tiempo de un juego que no pertenece a ninguna cuenta de usuario (algunos juegos nunca piden una, o la cuenta se ha eliminado desde entonces) aparece como *Sin cuenta*, a partir de un minuto.
- Una cuenta de usuario que PlayGuard no había visto nunca se señala en ámbar en la pestaña durante 30 días (*Cuenta nueva desde el …*: una cuenta nueva es una forma conocida de empezar de cero) y se anota en el historial de cambios; las cuentas que ya estaban cuando PlayGuard las enumera por primera vez no se señalan.

### Exportar y compartir

- **Exportar a la tarjeta SD** (*Exportar o compartir › Guardar en la tarjeta SD*) en CSV, JSON, XLSX (Excel) o PDF, una columna por día, y luego los últimos 7 días, los últimos 30 días y desde siempre.
- **El resumen de la semana en código QR** (*Exportar o compartir*, o *Mostrar su código QR* en la campana): la cámara de un móvil lo lee como unas pocas líneas de texto, en el idioma mostrado: los últimos 7 días y su media, la tendencia, cada día, los tres juegos más jugados, las sesiones, la más larga y el tiempo jugado entre las 21:00 y las 06:00. Nada sale a Internet.
- Se abre al instante con las últimas cifras leídas (guardadas en la tarjeta SD entre ejecuciones), que se actualizan en segundo plano si tienen más de un minuto; Ⓧ las vuelve a leer ahora, con un indicador de carga.
- Los tiempos son aproximados si se cambió el reloj de la consola.

## Restricciones — nivel, clasificación por edades, comunicación

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=restrictions)*

- **El nivel de restricción:** Niños pequeños, Niños, Adolescentes y Personalizado, uno al lado de otro como mosaicos, cada uno con lo que restringe (un candado para lo restringido, *Permitido* escrito en los demás casos, y los juegos instalados que bloquea; Ⓐ lo aplica; en *Personalizado* ya en vigor, Ⓐ lleva a sus ajustes de abajo); *Ninguno* debajo. El nivel en vigor indica *✓ Actual*; el contorno turquesa solo está donde está el foco. Lo que restringe un nivel (límite de edad, publicaciones, comunicación) se vuelve a indicar antes de aplicarlo; *Ninguno* lo pregunta en coral, ya que levanta todas las restricciones.
- **Los ajustes del nivel:** clasificación por edades, publicación en redes sociales, comunicación con otros usuarios. Con un nivel predefinido, son los suyos: cambiar uno convierte el nivel en *Personalizado*, a partir de los ajustes del nivel predefinido con ese cambio, tras preguntar (con los ajustes enumerados).
- **En *Clasificación por edad*,** el **sistema de clasificación** (PEGI, ESRB, USK, CERO …): la lista indica los países para los que clasifica cada uno (PEGI: la mayor parte de Europa, ESRB: Estados Unidos, Canadá y Latinoamérica, USK: Alemania, PEGI BBFC: Reino Unido…).
- **Juegos instalados que bloquea** (también en *Clasificación por edad*): cuántos de los juegos instalados bloquea el nivel en vigor por su clasificación por edades (*3 de 15*), y Ⓐ los enumera con su clasificación (*PEGI 16*); el mosaico de cada nivel también lo indica (*Juegos bloqueados · 3*).
  - Las clasificaciones son las de cada juego (sus datos de control, leídos por la comprobación de los juegos instalados al iniciar o desde *Consola › Comprobar los juegos instalados*), según el sistema de clasificación que usa la consola.
  - Un juego que ese sistema no clasifica no se cuenta, ni tampoco las excepciones definidas en la aplicación móvil.
  - La pantalla de un juego en Actividad muestra su clasificación, y *bloqueado en el nivel actual* cuando lo está.
- **En *Otros*,** el modo RV. Los juegos a los que se permite comunicarse están en el informe de diagnóstico, en bruto, hasta que se entienda su lista.

## Seguridad — el PIN, el desbloqueo, el menú homebrew, la aplicación móvil

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=security)*

- **Arriba**, cuatro mosaicos: el PIN, las restricciones, el desbloqueo temporal y el menú homebrew, cada uno con una marca de verificación turquesa cuando está como debe (un PIN definido, las restricciones activas, bloqueado) o una señal de advertencia ámbar cuando hay que intervenir; el menú homebrew indica *Abierto* o *Bloqueado* (una marca de verificación una vez bloqueado: abierto está bien sin forwarders).
- **Definir / cambiar el PIN** (pantalla de PIN del sistema).
- **Mostrar el PIN**: tras un aviso y el propio PIN, que se pide cada vez; se anota en el historial, y luego se ofrece un PIN nuevo.
- **Desbloquear temporalmente**, y si PlayGuard lo reactiva solo justo después de un cambio que necesitó el desbloqueo (activado de forma predeterminada).

### Proteger PlayGuard con el PIN

El PIN en el propio PlayGuard:

| Ajuste | Efecto |
|---|---|
| *Nunca* | Sin PIN en PlayGuard (la campana lo señala mientras la consola tiene uno) |
| *Antes de un cambio* (predeterminado) | Cualquiera puede mirar, solo el padre o la madre cambia algo |
| *Para abrir PlayGuard* | Una pantalla de bloqueo al iniciar |

- Una vez introducido, el PIN vale hasta que se cierra PlayGuard; *Volver a pedir el PIN pasados 5 minutos* (desactivado de forma predeterminada) lo vuelve a pedir pasado ese tiempo, y hace que la pantalla de bloqueo de *Para abrir PlayGuard* vuelva a aparecer cuando PlayGuard regresa tras 5 min o más en segundo plano.
- Se comprueba en la capa de servicio, así que ningún cambio se lo salta; volver a bloquear nunca lo pide. *Mostrar el PIN* lo pide siempre, en todos los modos e incluso justo después de introducirlo, y un `config.json` que falta o está dañado cuenta como *Antes de un cambio*.
- **Bloquear el menú homebrew ›** abre *Consola › Forwarders*, donde se bloquea tras sus comprobaciones (ver [Forwarders](#forwarders)).
- **Al salir** con el control parental todavía desbloqueado, se pregunta si volver a bloquearlo (*Reactivar y salir* también cierra el juego sobre el que se abrió PlayGuard y vuelve al menú HOME, no a hbmenu); si en cambio se cierra desde el menú HOME, PlayGuard lo vuelve a bloquear la próxima vez que se inicia, y reiniciar la consola lo bloquea por sí solo.

### Aplicación móvil

- Si la aplicación Control parental de Nintendo Switch está vinculada, su última sincronización y **desvincularla** (si no, su próxima sincronización sobrescribe los límites fijados aquí: mientras está vinculada, cada cambio que PlayGuard pide confirmar lo advierte).
- Cuando la redirección DNS de Atmosphère (`atmosphere/hosts`) desvía a otro sitio el servidor del control parental de Nintendo, la aplicación no puede sincronizarse: PlayGuard indica entonces aquí *sincronización bloqueada (DNS)* y no menciona la aplicación en ningún otro sitio (ninguna nota en los cuadros de diálogo, ninguna entrada en la campana, ningún paso en los primeros pasos, ninguna píldora en *Hoy*, ninguna ayuda para descifrar, y la desvinculación se pide sin decir qué sobrescribiría la aplicación).
- Mientras está vinculada y puede sincronizarse, *Antes de desvincular: ayude a descifrar…* abre la comparación del bloque (consulta [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), en inglés).

### Eliminar todo el control parental

Dos confirmaciones, irreversible; antes se guarda una copia de seguridad de los ajustes.

## Historial — lo que ha cambiado PlayGuard, y las copias de seguridad

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=history)*

**Arriba**, como mosaicos, cuántos cambios hay registrados y el día de la última copia de seguridad: una marca de verificación turquesa, o una señal de advertencia ámbar cuando no hay ninguna o ninguna desde hace 30 días.

### Historial de cambios

- Los 500 cambios más recientes que hizo PlayGuard (límites, nivel de restricción, PIN, desbloqueos, desvinculación, el reloj, restauraciones, pausar o reanudar la cuenta atrás …) y lo que vio cambiar fuera de él, agrupados por día (*Hoy*, *Ayer*, y luego el día y la fecha), con la hora y desde dónde; un límite cambiado para un día concreto nombra ese día.
- Un cambio que la consola **rechazó** también aparece, en ámbar, con el error de la consola y lo que se intentó (nunca se restablece: no cambió nada).
- *Mostrar* conserva un solo tipo (tiempo de juego, restricciones, seguridad y aplicación, visto fuera de PlayGuard, rechazado por la consola); *Exportar lo que se muestra* lo escribe en `exports/` como CSV, JSON, XLSX o PDF.
- Ⓐ en un cambio lo muestra y, si es un valor, **restablece el anterior**, con el mismo desbloqueo y el mismo PIN que cualquier cambio, avisando si ha cambiado desde entonces.
- Cada cambio guarda la versión de PlayGuard que lo hizo (una compilación de desarrollo: se indica, con su commit y, para la compilación de una pull request, su número); un cambio registrado por una versión más reciente de PlayGuard aparece como *Otro cambio*.

### Copias de seguridad

- **Hacer una copia de seguridad / restaurar los ajustes** en la tarjeta SD: nivel de restricción, ajustes personalizados, modo RV, sistema de clasificación, límites diarios, la alarma de «Se acabó el tiempo» (con las acciones avanzadas activadas) y, como referencia, el bloque en bruto del temporizador; nunca el PIN.
- Una restauración enumera solo lo que cambiaría (si no hay nada que cambiar, simplemente lo dice, sin desbloquear ni escribir nada), y avisa cuando la copia tenía una alarma de hora de dormir activada (no se vuelve a escribir).
- Elige cuántas copias de seguridad conservar.

## Consola — el reloj de red, los juegos instalados, lo que ejecuta la consola

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=console)*

**Arriba**, las herramientas como mosaicos (*Reloj de red*, *Comprobar los juegos instalados*, *Forwarders*, cada una con lo que requiere atención en su esquina), luego lo que ejecuta la consola como mosaicos (el firmware, Atmosphère, hekate cuando está en la tarjeta SD y sys-patch cuando está instalado; Ⓐ abre sus notas de la versión) y después las líneas de la consola, empezando por la compatibilidad (solo una marca de verificación turquesa cuando PlayGuard se ha comprobado en este firmware; si no, una advertencia y lo que falla).

### Reloj de red

*Reloj de red ›*, una pantalla propia: el reloj del que depende el temporizador de juego. Una consola que nunca llega a los servidores de Nintendo mantiene este reloj impreciso, lo que falsea el temporizador de juego.

- **Relojes de la consola y de red**, zona horaria, precisión. Cuando la redirección DNS de Atmosphère (`atmosphere/hosts`) desvía a otro sitio el servidor de hora de Nintendo (`aauth-lp1.ndas.srv.nintendo.net`), la consola ya no puede ajustar su reloj de red por sí misma: esta pantalla, la entrada de la campana y la tarjeta de *Hoy* lo indican y remiten a la medición de abajo.
- **Elige un servidor NTP público** (≈ 50 integrados, por región, o el tuyo).
- **Medir** con 3 servidores (mediana; un aviso ámbar cuando no coinciden): un cuadro de diálogo muestra el resultado (cuánto va adelantado o atrasado el reloj de red, en grande, la hora de los servidores junto a la del reloj de red, la respuesta de cada servidor) y ajusta el reloj de red desde ahí. Una medición sigue siendo válida durante 2 minutos, con una cuenta atrás bajo *Ajustar el reloj de red*. La confirmación lo dice: cambiar el reloj pone a cero el tiempo de juego de hoy, así que vuelve a estar disponible todo el límite.
- **Una comprobación del reloj de red al iniciar**: un aviso cuando tiene más de un minuto de desfase; nunca ajusta el reloj.

#### Ningún servidor responde

En una consola sin Internet, *Usar el reloj de la consola* se ofrece en el cuadro de diálogo del resultado, y *Ajustar el reloj de red al reloj de la consola* bajo la medición (y en la tarjeta del reloj de red de *Hoy*, cuando su medición falla), tras la misma confirmación.

- PlayGuard no tiene nada más con lo que comparar el reloj de la consola: la confirmación muestra su hora y te pide que la compruebes antes con un móvil o un reloj, y se rechaza un reloj de consola anterior a 2026 (nunca ajustado).
- Con *Sincronizar el reloj por Internet* desactivado, el reloj de la consola se queda como está, y activar el ajuste después ya no lo hace saltar; con él activado, el reloj de la consola ya sigue al reloj de red, así que la confirmación indica que lo desactives y pongas la hora a mano si es incorrecta.
- Solo escribe el reloj, no el control parental, así que no necesita desbloqueo (el PIN de *Proteger PlayGuard con el PIN* sigue aplicándose); como cualquier cambio de reloj, pone a cero el tiempo de juego de hoy.
- Aún no se ha probado en una consola.

### Juegos instalados

*Comprobar los juegos instalados* encuentra los juegos que el menú HOME no puede iniciar ni mostrar (un icono que no termina de cargar, un «!», un marco punteado) con lo que falla y qué hacer, bajo el icono del juego, y cuántos juegos instalados tienen un problema.

**Lo que encuentra:** nombre e icono ilegibles, el propio juego que falta detrás de su actualización, nada instalado ya, archivos que faltan, datos dañados que la consola ha encontrado, un firmware más reciente que exige el juego o solo su actualización, claves demasiado recientes para el firmware, una licencia (ticket) que falta, la consola que pide una actualización que se quitó, la tarjeta SD ilegible, un inicio que la consola rechaza.

**La lista tiene una sección por cada salida**, cada una con su número y qué hacer, con los juegos que tienen el mismo problema juntos (dos juegos con el mismo nombre se distinguen por su versión o su ID de título):

| Sección | Qué contiene |
|---|---|
| **Reparables aquí** | Lo que PlayGuard repara |
| **Por comprobar** | Datos de los que la consola sospecha: la comprobación dice si hay que volver a instalar el juego |
| **Reinstalar** | Archivos que faltan o no sirven, o datos que se han encontrado dañados: quitar y volver a instalar |
| **Firmware o tarjeta SD** | El juego está bien, la consola no |

- **Comprobar los datos:** lo que encontró la comprobación de datos se conserva con el juego hasta que se instale otra versión: si están dañados, pasa a *Reinstalar* como *Dañado (comprobado)*, con la fecha y el código de la consola; si están intactos, sale de la lista. *Comprobar estos juegos* comprueba toda la sección *Por comprobar*, un juego tras otro en un solo cuadro de diálogo (*juego 2 de 5*), y luego dice cuántos están dañados, intactos o no se han podido comprobar, con la opción de quitar los dañados al alcance de la mano.
- **Varios juegos a la vez:** Ⓨ selecciona juegos (una marca delante); desde el primero seleccionado, una línea encima de las secciones dice cuántos, y A en ella también selecciona una sección entera; luego, una sola acción para todos: comprobar sus datos o quitarlos (antes se guarda su lista, una sola pregunta, un solo PIN).
- **En *Reinstalar*,** se puede guardar su lista (JSON y texto, en la carpeta de exportaciones: nombre, ID de título, versión, problema), y quitarlos todos a la vez tras una sola pregunta y un solo PIN: primero se guarda la lista y luego se quitan los juegos para que un instalador ya no los cuente como instalados.
- **Las acciones:** cuando una acción es la solución, es el primer botón del cuadro de diálogo; después vienen las demás: volver a aceptar la versión instalada (cuando la consola pide una actualización que se quitó), quitar solo la actualización o un contenido descargable, comprobar si los datos están dañados (la comprobación de la propia consola, con su progreso y Cancelar) o quitar el juego.
- Las versiones se muestran como las muestra el juego (*versión 1.4.0*, *actualización n.º 4*); tras una reparación, un aviso dice si el juego vuelve a funcionar. Los datos de guardado siempre se conservan; cada cambio queda en el historial de cambios.
- **Cuándo comprueba:** la última comprobación se muestra al instante; después solo se vuelven a comprobar los juegos cuya entrada ha cambiado (todos los juegos una vez al día, tras una actualización del firmware o con Ⓧ *Comprobar de nuevo*), y cada uno se muestra en cuanto se comprueba.
- Varios juegos dañados a la vez apuntan a la tarjeta SD o a un juego instalado desde el otro almacenamiento (sysMMC / emuMMC).
- Ⓡ muestra los juegos como **lista** o como **portadas** (icono, nombre y problema; la elección se conserva), como hace *Todos los juegos*.

### Forwarders

*Forwarders ›*, una pantalla propia, un mosaico en *Herramientas* (también *Seguridad › Bloquear el menú homebrew*): un homebrew en el menú HOME, con su icono y su nombre, que se inicia desde ahí como un juego (un forwarder de NRO, como los crea sphaira).

- **Una tarjeta arriba** indica en qué estado está el menú homebrew: bloqueado o abierto, sus caminos de vuelta (el forwarder de PlayGuard, los menús protegidos, los que aún no se han iniciado nunca desde el menú HOME), lo que todavía lo abre sin el PIN y si el sysmodule de recuperación está instalado. Ⓐ en ella enumera qué hacer, cada cosa a una pulsación (el siguiente paso hacia el bloqueo, proteger o eliminar un forwarder que lo abre sin el PIN, la comprobación, la hoja de rescate).
- **La lista por grupos:** el de PlayGuard, los demás que ha creado (protegidos o no, en una sola tarjeta: el icono indica cuál), los creados por otra aplicación, los que ya no están en el menú HOME. Uno protegido lleva un candado en su icono (una marca de verificación si es un menú que se ha visto funcionar a través de su puerta); uno creado aquí sin el PIN, un candado abierto en ámbar.
- **Ⓨ** abre esta parte de la guía (un código QR). Tras eliminar u olvidar un forwarder, o cambiar sus horas sin el PIN, *Pedir el PIN* o *Cerrado al agotarse el tiempo de juego de hoy*, **ZR** lo restablece durante unos segundos (reinstalado con el mismo ID de programa, o el ajuste como estaba).
- **PlayGuard** con una pulsación: su propio icono y su nombre, y el forwarder inicia el `.nro` desde el que se abrió PlayGuard.
- **Otro homebrew**: cualquier `.nro` de la carpeta `/switch` de la tarjeta SD (y una carpeta más abajo), con su nombre preguntado primero (el del `.nro`, para conservarlo o cambiarlo), y luego confirmado con lo que mostrará el menú HOME: su icono, su nombre y su editor. Su icono es el del `.nro`, convertido al JPEG de 256 × 256 que muestra el menú HOME cuando tiene otro tamaño o es un PNG (un mando en su lugar cuando el `.nro` no tiene), como hace sphaira. Una ruta con un espacio se pasa al homebrew entre comillas.
- **Los forwarders del menú HOME**, cada uno con su icono: los que creó PlayGuard (Ⓐ: detalles, cambiar el nombre, cambiar el editor, reinstalar, eliminar) y los de otra aplicación (detalles, eliminar). Ⓐ abre una hoja por secciones (*Este forwarder*, *Al iniciar*, *Protección*, *Instalar y eliminar*) bajo su icono y su estado, cada línea con su icono y lo que está configurado ahora (sus horas; un interruptor para *Pedir el PIN*, *Cerrado al agotarse el tiempo de juego de hoy* y cada opción de inicio), *Eliminar* en rojo; B o un toque fuera de ella la cierra. *Detalles* enumera sus reglas de la misma forma, y luego su editor, su versión, su `.nro` y su ID de programa; Ⓐ en una regla (o en el editor) la cambia, como en la hoja. Bajo los protegidos, una línea dice qué significa cada icono que aparece ahí. Uno cuyo `.nro` ya no está lo indica; uno eliminado en la configuración de la consola sigue en la lista hasta que se reinstala o se olvida. Reinstalar conserva su ID de programa, así que la entrada del menú HOME se reemplaza, no se duplica; un forwarder creado por otra herramienta para el mismo `.nro` también se reemplaza.
- **Protegido con el PIN** (Ⓐ › *Proteger con el PIN*, para uno creado por PlayGuard que no sea el suyo): iniciado desde el menú HOME, el forwarder abre primero una pequeña puerta (`config/playguard/gate.nro`, unos cientos de kilobytes que PlayGuard escribe a partir de sus propios archivos), que muestra enseguida la pantalla del PIN de la consola, y luego el homebrew cuando el PIN es correcto; B ahí vuelve al menú HOME. Alrededor de la pantalla del PIN, lo propio de la puerta: el icono y el nombre del homebrew, *Protected by PlayGuard* y cómo ha ido (iniciando, en su horario libre, PIN no introducido), en el idioma de la consola, sobre el azul noche de Cadran (`make gate-preview` la dibuja en un ordenador). Conserva su ID de programa y su icono, y queda en el historial de cambios. Su fila muestra sus reglas como iconos, uno por cada una activada: una llave para el PIN, un candado abierto para las horas sin él, un cuadrado (stop) para las horas en que nunca se abre, un temporizador para cerrado al agotarse el tiempo de juego de hoy; *Detalles* (Ⓐ) las da en palabras. Hace falta un PIN en la consola. *Dejar de pedir el PIN* lo pide antes, diga lo que diga *Proteger PlayGuard con el PIN*, igual que eliminarlo o volver a crearlo sin el PIN (*Añadir un forwarder* para el mismo `.nro`). Solo protege el icono del menú HOME: el `.nro` sigue iniciándose desde el menú homebrew, salvo que esté bloqueado (más abajo). Un forwarder protegido por un PlayGuard anterior inicia el propio PlayGuard como puerta, lo que sigue funcionando; *Reinstalar todos* lo pasa a la pequeña puerta.
  - **Horas sin PIN** (Ⓐ en uno protegido): todos los días, de lunes a viernes o el fin de semana, de una hora a otra (también pasada la medianoche), o *De lunes a viernes, y el fin de semana aparte* (una franja para cada uno); *Detalles* indica cuándo. Definirlas pide el PIN. Siguen el reloj de la consola, que la configuración de la consola puede cambiar: en cuanto se aleja más de 5 minutos del reloj de red, la puerta pide el PIN igualmente en ellas (*Sin horario libre: el reloj de la consola no coincide con el de la red*), y lo mismo cuando no se puede leer el reloj de red para comprobarlo (*Sin horario libre: reloj de red no disponible*); *Consola › Reloj de red* pone los dos en hora, también en una consola sin Internet.
  - **Horas en que nunca se abre** (Ⓐ en uno protegido), definidas de la misma forma: entonces la puerta dice *No a esta hora* y las horas, y vuelve al menú HOME, incluso con el PIN introducido (la hora de dormir para los homebrew). Tienen prioridad sobre las horas sin PIN. Con el reloj de la consola a más de 5 minutos del reloj de red, sigue cerrado cuando cualquiera de los dos relojes está en ellas, y una pausa tras los rechazos se cuenta con el reloj de red y con el reloj continuo de la consola (que ningún ajuste mueve), así que cambiar la hora no las esquiva.
  - **Cerrado al agotarse el tiempo de juego de hoy** (Ⓐ en uno protegido): cuando al temporizador de la consola no le queda tiempo hoy, la puerta dice *El tiempo de juego de hoy se ha agotado* y vuelve al menú HOME, con PIN o sin él. Desactivado salvo que se defina.
  - **Las reglas están en el forwarder, no en la tarjeta SD:** las horas, el límite del día y la pausa tras los rechazos van en los propios datos del forwarder (en la memoria de la consola), así que un ordenador no puede añadir horas libres editando un archivo; cambiar una rehace el forwarder, en unos segundos. El forwarder también guarda el SHA-256 de la puerta (`config/playguard/gate-<hash>.nro`): una puerta modificada en la tarjeta SD no se inicia (error 2347-0100, [solución de problemas](troubleshooting.md#un-forwarder-protegido-muestra-el-error-2347-0100)). Uno protegido por un PlayGuard anterior lee sus reglas de la tarjeta: entonces sus horas sin PIN no se respetan, y la campana ofrece *Rehacer*.
  - **Una pausa tras PIN rechazados** (*Pausa tras PIN rechazados*, en *Menú homebrew*): cinco rechazos seguidos (B en la pantalla del PIN) y el forwarder deja de pedirlo durante 15 minutos, con la puerta indicando hasta cuándo; 3, 5 o 10 intentos y 15 minutos o una hora, o nunca. La campana indica qué forwarder está en pausa, y su fila también (un icono de pausa, y hasta cuándo); *Quitar la pausa* (Ⓐ en él, pidiendo el PIN) hace que vuelva a pedirlo enseguida. Cada pausa y cada pausa quitada quedan en el historial de cambios.
  - **Cada intento queda en el historial de cambios**: abierto con el PIN, abierto en su horario o rechazado (B en la pantalla del PIN), a la hora en que ocurrió; los intentos de un mismo tipo, un mismo día y para un mismo forwarder forman una sola entrada, con sus horas. La puerta los anota en la tarjeta SD; PlayGuard los lee la siguiente vez que se inicia. *Historial › Mostrar › Homebrew y forwarders* muestra solo estos, los forwarders y el bloqueo.
  - **Intentos de la semana** (en *Historial* y en *Menú homebrew*): de los últimos siete días, las aperturas de cada forwarder, sus rechazos y en qué días, y las veces que no se abrió; se conservan cuatro semanas. Cuando los rechazos de esta semana superan el doble de los de la semana anterior (cinco como mínimo), la campana lo indica.
  - **La pantalla de la puerta dice más:** sus horas sin PIN mientras lo pide, y los rechazos del día hasta ese momento (en ámbar) mientras lo pide y una vez rechazado, bajo *Pide a un adulto que introduzca el PIN*. Tras *Protected by PlayGuard*, un icono por cada regla que lleva, como su línea en PlayGuard: un candado abierto (horas sin PIN), un cuadrado (horas en las que nunca se abre), un temporizador (el tiempo de juego de hoy). Una vez rechazado, Ⓐ lo vuelve a intentar y Ⓑ vuelve al menú HOME (igual que esperar 15 segundos), hasta que empieza la pausa tras los rechazos. También indica hasta cuándo se abre sin PIN, cuándo termina una pausa (*en 12 min*), el tiempo de juego que queda hoy (con *Cerrado al agotarse el tiempo de juego de hoy*) y el código de error cuando la pantalla del PIN no puede abrirse. El marco del icono toma el color de cómo fue. Si la puerta llegara a detenerse de camino al homebrew, termina a los 10 segundos y vuelve al menú HOME, en lugar de dejar la consola en su pantalla.
- **Su icono desde el álbum** (Ⓐ › *Cambiar el icono*): las 30 capturas más recientes del álbum en una cuadrícula, cada una como el icono que daría, primero el del `.nro` y marcado el que está en uso; al lado, el icono que muestra ahora el menú HOME y el que mostrará, el juego en el que se tomó la captura y cuándo (*Hoy*, *Ayer*, al segundo cuando dos se tomaron en el mismo minuto). L y R mueven el cuadrado del icono a lo largo de la captura (cinco posiciones, de su borde izquierdo al derecho), Y vuelve al del `.nro`, A elige el seleccionado: se convierte en el JPEG de 256 × 256 del menú HOME. Volver a instalarlo lo conserva, con su encuadre.
- **Iconos e idiomas:** un icono de otro tamaño, un PNG, o un JPEG con datos EXIF o codificación progresiva (el menú HOME los muestra como un «?») se convierte en un JPEG de 256 × 256 simple. Los datos de control del forwarder solo declaran el inglés americano, el idioma de su único icono: así el icono se muestra en una consola configurada en cualquier idioma.
- **Al iniciar** (Ⓐ en un forwarder, como las opciones de forwarder de sphaira): *Elegir un usuario al iniciar* (desactivado de forma predeterminada) pregunta qué usuario juega al iniciarlo, como hace un juego, para que su tiempo de juego sea el de ese usuario en *Actividad* (el propio perfil de un padre o una madre deja las estadísticas del niño como están); el límite de tiempo de juego de la consola lo sigue contando, juegue quien juegue, como cuenta cualquier juego. *Capturas de pantalla* y *Captura de vídeo* (activados de forma predeterminada) permiten que el botón de captura las haga. Cada interruptor pide el PIN y vuelve a crear el forwarder (con el mismo ID de programa), y queda en el historial de cambios; *Detalles* los enumera.
- **Proteger todos los forwarders con el PIN** y **Reinstalar todos** (el icono y la versión de un `.nro` nuevo, la puerta tal como la trae este PlayGuard, uno eliminado del menú HOME de vuelta): con una pulsación cada uno.
- **Bloquear el menú homebrew** (*Menú homebrew*, al final de la pantalla): ni el álbum ni un juego iniciado manteniendo R abren ya el menú homebrew, así que un homebrew solo se inicia desde su forwarder, que puede pedir el PIN.
  - **Paso a paso:** mientras no se pueda bloquear, Ⓐ enumera los pasos, numerados, el siguiente en ámbar y a una pulsación (*Siguiente paso*): un PIN, los parches de firma, el forwarder de PlayGuard, un menú homebrew protegido con el PIN (sphaira si está en `/switch`, si no hbmenu, creado ya protegido de una vez), iniciado una vez desde el menú HOME; la lista vuelve tras cada paso que crea un forwarder.
  - Hacen falta dos caminos de vuelta a PlayGuard desde el menú HOME: su propio forwarder, y un forwarder de un menú homebrew (sphaira o hbmenu, según el nombre del `.nro`) protegido con el PIN, que deja todos los homebrew, tienda de homebrew incluida, a un PIN de distancia. Y un PIN en la consola, y los parches de firma que necesitan los forwarders. Mientras tanto, Ⓐ dice qué falta.
  - El camino de vuelta del menú cuenta en cuanto se ha visto funcionar: iniciado una vez desde el menú HOME, con el PIN introducido en su puerta (el registro de la puerta lo indica, y se lee cuando PlayGuard se abre o bloquea). Un menú protegido que nunca se ha iniciado así aparece como tal.
  - Bloquear muestra las comprobaciones en una lista (una marca turquesa para lo que está listo, un triángulo ámbar o coral para lo que no) y pide mantener Ⓐ 2 segundos en *Bloquear* (o un dedo sobre él), con una barra que se llena mientras tanto; si se suelta antes, no cambia nada. Luego viene la **hoja de rescate** (también escrita en `config/playguard/rescue.html`, una página para abrir e imprimir en un ordenador, en el idioma mostrado, con el código QR): cómo deshacerlo desde un ordenador, o con el sysmodule de recuperación cuando está instalado, y un código QR a la ayuda ([más abajo](troubleshooting.md#menú-homebrew-bloqueado)); *Comprobar el menú homebrew* también la ofrece.
  - En la emuMMC (o en una sysMMC con una emuMMC configurada), avisa de que el bloqueo está en la tarjeta SD, así que lo leen los dos sistemas, mientras que los forwarders son de cada sistema: el otro sistema no tiene ninguno de estos caminos de vuelta.
  - Mientras está bloqueado, se rechaza eliminar el forwarder de PlayGuard o el último menú protegido (o quitarle el PIN): desbloquea primero. Eliminar uno de varios menús protegidos pide confirmación, en coral, nombrando los que quedan.
  - Antes de bloquear, ofrece proteger de una vez los forwarders creados por PlayGuard que aún no están protegidos, e indica cuáles seguirían iniciando homebrew sin el PIN (un menú sin proteger, o uno creado por otra aplicación), cuáles escriben en la tarjeta SD (un gestor de archivos, un instalador como Cyberfoil o Goldleaf, un servidor FTP: podrían cambiar el archivo del bloqueo) y cuándo está instalado uLaunch (inicia los homebrew a su manera, y el bloqueo no llega hasta ahí).
  - El PIN rechazado tres veces o más en un mismo día en la puerta de un forwarder se avisa en la campana (qué forwarder, qué día, cuántas veces).
  - Cuando falla la escritura de un archivo, el error dice por qué cuando puede saberlo: la tarjeta SD llena, o que no se puede escribir nada en ella (su bloqueo, o el bit de archivo que corrige *Tools* de hekate).
  - *Comprobar el menú homebrew* muestra lo que leerá Atmosphère (si el álbum y un juego iniciado con R abren el menú homebrew), los caminos de vuelta, los parches de firma, la versión de Atmosphère con la que se bloqueó y el sysmodule de recuperación; el informe de diagnóstico incluye lo mismo. Si no está bloqueado, lo dice primero y ofrece *Bloquear*, con los pasos que faltan (la hoja de rescate se queda); con un sysmodule de recuperación desactualizado, el PlayGuard con el que vino y cómo sustituirlo (*Actualizar el módulo* cuando esta compilación lo incluye). En la emuMMC, o en la sysMMC con una emuMMC configurada, indica que el otro sistema también lee el bloqueo, sin los forwarders instalados aquí.
  - Son unas líneas al final de `atmosphere/config/override_config.ini`, entre dos comentarios de PlayGuard; el resto del archivo no se toca, y se guarda tal como estaba en `config/playguard/override_config.ini.backup`. Atmosphère lee el archivo cada vez que se inicia un programa: no hace falta reiniciar. Desbloquear quita esas líneas y vuelve a escribir los valores propios de Atmosphère para las claves que el resto del archivo no define (Atmosphère conserva lo que leyó hasta que se reinicia la consola).
  - Bloquear y desbloquear piden el PIN diga lo que diga *Proteger PlayGuard con el PIN*, y quedan en el historial de cambios. Mientras está bloqueado, PlayGuard comprueba en cada inicio que los dos caminos de vuelta siguen ahí; la campana avisa cuando falta uno. Un bloqueo quitado del archivo fuera de PlayGuard (desde un ordenador, un homebrew) se avisa en la campana y en el historial de cambios; PlayGuard nunca lo vuelve a poner por sí solo, ya que eliminar el archivo es la salida desde un ordenador. Eliminar un forwarder (Configuración de la consola › Gestión de datos, donde no se pide ningún PIN) nunca quita el bloqueo: solo quita un camino de vuelta.
  - Desde un ordenador: elimina `atmosphere/config/override_config.ini` (o vuelve a poner la copia), y luego reinicia la consola.
  - Con el sysmodule de recuperación instalado (opcional, [sysmodule/README.md](../../sysmodule/README.md), en inglés), una petición `RESCUE` también lo quita al arrancar, igual que una actualización de Atmosphère desde que se bloqueó (sus parches de firma pueden ir con retraso, y sin ellos no se inicia ningún forwarder): el siguiente inicio de PlayGuard lo indica y ofrece volver a bloquearlo.
- **Un forwarder eliminado fuera de PlayGuard** (en la configuración de la consola): se indica una vez en el historial de cambios, y en la campana mientras dure, con *Volver a instalar* u *Olvidar* en su fila.
- **Parches de firma:** la consola solo inicia un forwarder con parches de firma de fs y del loader (sys-patch, o archivos sigpatch), con todos los patrones que sys-patch tiene para este firmware aplicados. Cuando faltan, o sys-patch omitió uno (un sys-patch antiguo en un firmware más reciente), la pantalla lo indica encima de la lista, nombrando lo que no está parcheado, y al añadir, crear el de PlayGuard o reinstalarlos todos avisa primero de que el forwarder aún no se iniciaría: añadirlo de todos modos (se inicia en cuanto los parches estén en su sitio) o cancelar. Bloquear el menú homebrew también los espera (*Consola › Parches de juegos* da los detalles). Se avisa de los archivos sigpatch que quedan junto a sys-patch, y *Consola › Parches de juegos* los aparta (los mueve, nunca los borra). Cada forwarder conserva el cargador con el que se creó: se ofrece volver a crear los hechos por un PlayGuard anterior (la campana, *Consola › Forwarders*). El `.nro` se queda donde está; eliminar un forwarder nunca lo toca.
- **Tiempo de juego:** iniciado desde su icono, un homebrew funciona como un juego: el temporizador lo cuenta y lo suspende como a cualquier juego cuando se acaba el tiempo, y *Actividad* lo muestra. PlayGuard sigue a mano desde el menú de homebrew.
- **En las copias de seguridad de los ajustes** (*Historial › Copias de seguridad*): lo que creó PlayGuard y si el menú homebrew estaba bloqueado. Al restaurar una copia, después del resto, ofrece volver a poner los forwarders que ya no están en el menú HOME (protegidos como estaban, con sus horas y su icono); un bloqueo solo se indica, y hay que volver a hacerlo desde aquí.
- Cada cambio pide el PIN cuando *Proteger PlayGuard con el PIN* lo indica, se rechaza en modo de solo lectura y queda en el historial de cambios. Lo que creó PlayGuard se guarda por sistema (sysMMC / emuMMC), en `forwarders.json`.

### Las líneas de la consola

- **Firmware, Atmosphère, hekate** (cuando su `bootloader/update.bin` está en la tarjeta SD) **y sys-patch** (cuando está instalado; su versión, de su `config/sys-patch/log.ini`), cada uno un mosaico con la versión instalada y luego la versión más reciente que encontró la última búsqueda de actualizaciones en una píldora ámbar (*23.0.1 disponible*, el mosaico con borde ámbar, y debajo cuántas versiones de retraso cuando hay más de una), o *Actualizado*; un aviso en la esquina del mosaico del firmware cuando el firmware más reciente necesita antes un Atmosphère o un hekate más reciente, o todavía no lo admiten (las notas de la versión del firmware explican por qué: sus notas de la versión dicen qué firmware admiten).
- **Notas de la versión:** Ⓐ en cada uno las abre: una pestaña para la versión instalada, una para cada versión más reciente (cinco como máximo) y versiones anteriores en las pestañas restantes (seis en total; Ⓛ / Ⓡ pasan de una a otra), que se abren en la siguiente, con etiquetas (instalada o más reciente, el firmware más reciente que admite, correcciones de seguridad, control parental modificado), las notas tal como se publicaron (en inglés), *Ver en línea* (un código QR) y cuándo se obtuvieron. Las de Atmosphère, hekate y sys-patch vienen de sus versiones en GitHub (las de sys-patch, de `impeeza/sys-patch`), las del firmware de switchbrew; se guardan en la tarjeta SD para leerlas sin conexión y se vuelven a obtener una vez al día (Ⓧ ahora).
- **La compatibilidad**, y luego **el almacenamiento** (emuMMC o sysMMC, con un icono de información cuando el otro sistema tiene su propio control parental: Ⓐ lo explica).
- Si Atmosphère **oculta el número de serie** (*Sí (XAW00000000000)*, el número que ve entonces el sistema; en la emuMMC, cuando no lo oculta, un icono de aviso, que Ⓐ explica), y luego el **número de serie real** de la consola, con su parte central oculta: cuando está oculto, se lee de la copia de PRODINFO que Atmosphère guarda en `atmosphere/automatic_backups/` (la de esta consola, por su ID de dispositivo, cuando la tarjeta SD tiene varias). Ⓐ en su icono de información lo muestra entero y de dónde viene.
- **Parches de juegos** (sys-patch o archivos sigpatch, con la recomendación de sys-patch cuando solo se usan archivos: un icono de información, o de aviso cuando hay algo que revisar, que Ⓐ explica).

## Preferencias

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=preferences)*

- Lo que configura el propio PlayGuard:
  - Primero, como mosaicos: idioma (cualquier idioma de la consola, o el de la propia consola), tema (oscuro de forma predeterminada, claro o el de la consola), tamaño del texto (automático / normal / más grande) y la pestaña con la que empezar.
  - **La introducción** al iniciar: la pantalla de carga durante 2 segundos (activada de forma predeterminada; Ⓐ o Ⓑ la saltan; desactivada, PlayGuard se abre en cuanto está listo).
- Los mensajes que muestra:
  - Los tipos de notificaciones (ver [más arriba](#cómo-se-comportan-las-entradas)).
  - Un **recordatorio mensual para apoyar PlayGuard** (activado de forma predeterminada, nunca durante el primer mes ni justo después de una actualización; *No volver a mostrar* en el recordatorio o este interruptor lo desactivan para siempre, actualizaciones incluidas).
- Cómo se actualiza: *Buscar actualizaciones cada día* al iniciar, y *Actualizar con* (ver [Acerca de](#versión-y-actualizaciones)).
- **Primeros pasos** de nuevo (también en *Acerca de › Ayuda y diagnóstico*).
- Un ajuste que afecta a una sola función está junto a ella: el tiempo extra en *Tiempo de juego*, la reactivación del control parental en *Seguridad*, la comprobación del reloj en *Consola › Reloj de red*. El historial de cambios y las copias de seguridad tienen su pestaña, [Historial](#historial--lo-que-ha-cambiado-playguard-y-las-copias-de-seguridad); buscar actualizaciones ahora y *Ayuda y diagnóstico* están en *Acerca de*.

## Acerca de

*[Pruébalo en el simulador ›](https://playguard.link/simulator/?tab=about)*

### Versión y actualizaciones

- Arriba, su tarjeta: el logotipo, el nombre, la versión.
- El PlayGuard instalado: versión (en ámbar cuando hay una más reciente; Ⓐ abre sus **notas de la versión**).
- **Buscar actualizaciones** ahora; *Buscar actualizaciones cada día* al iniciar está en *Preferencias*.
- *Actualizar con* (en *Preferencias*): en PlayGuard, que descarga la versión, la comprueba, se sustituye y se reinicia, o sphaira, Homebrew App Store o a mano (se pregunta en la primera actualización si no se ha elegido nada). Pasar el relevo a una tienda pide el PIN como un cambio (*Proteger PlayGuard con el PIN › Antes de un cambio*), y siempre mientras el menú homebrew está bloqueado: una tienda inicia cualquier homebrew. Si se instaló desde la Homebrew App Store o la App Store de sphaira, la tienda es la vía recomendada ahí, para que siga la versión; cuando su catálogo aún no tiene la nueva versión (la publica tras una revisión, a menudo unos días después), PlayGuard lo indica y ofrece actualizar en el propio PlayGuard.
- Mientras se conozca una versión más reciente, PlayGuard se abre en **PlayGuard X.Y.Z está disponible** (su fecha y sus notas): *Actualizar*, o *Más tarde* durante 1 día, 1 semana o 30 días como máximo.
- Las notas de la versión son las del propio PlayGuard, en la misma pantalla que las de Atmosphère y hekate en *Consola*: de sus versiones en GitHub, una pestaña por versión (la instalada, las más recientes, las anteriores), cada una con sus novedades destacadas como tarjetas y todos los cambios, guardadas en la tarjeta SD para leerlas sin conexión (las de la versión instalada también están en la aplicación, por si aún no se ha obtenido nada).
- Tras una actualización, PlayGuard se abre una vez en **Novedades de la X.Y.Z** (las mismas novedades destacadas y luego los códigos QR).

### Ayuda y diagnóstico

*Ayuda y diagnóstico ›*, una pantalla propia, debajo de *Buscar actualizaciones*:

- **Primeros pasos** vuelve a abrir la guía (con un paso *Desvincular la app para móvil* mientras está vinculada, un paso *Reactivar la alarma de «Se acabó el tiempo»* mientras está desactivada, un paso *Proteger PlayGuard* una vez definido un PIN, que es la opción *Proteger PlayGuard con el PIN*, y un interruptor para que deje de aparecer al iniciar). Bajo *Cerrar*, *Apoyar PlayGuard* muestra los códigos QR para apoyar el proyecto.
- Cómo se inició PlayGuard (aplicación o applet) y su carpeta de datos.
- **Exportar un informe de diagnóstico** o **enviarlo en línea** (consulta [Informar de un error](troubleshooting.md#informar-de-un-error)); cuántos informes guardados, archivos de cierres, registros de sesión y enlaces de informes enviados en línea se conservan, por número, antigüedad o tamaño.

### Créditos

*Créditos y licencia ›*: quién lo ha hecho, en qué se basa, su licencia y su código fuente como código QR, en un solo diálogo. Luego, cómo **apoyar PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), que se muestran como códigos QR para escanear con un móvil) y un pequeño *Made in France* 🇫🇷.

## Seguro desde el diseño

**Escribir el límite de tiempo de juego sin riesgos.** Sobrescribir la configuración del temporizador mientras está contando desestabiliza Atmosphère. Por eso PlayGuard comprueba antes el estado; si el temporizador está activo, la confirmación indica que el control parental se desbloqueará temporalmente, con el PIN guardado: **no hace falta que lo recuerdes**. Una pulsación desbloquea, comprueba que el sistema confirma de verdad el desbloqueo, escribe y **vuelve a bloquear enseguida** (o lo propone, si *Reactivar el control parental tras un cambio* está desactivado). La capa de servicio vuelve a comprobar el estado justo antes de escribir, así que ninguna pantalla puede saltarse esta protección.

> [!CAUTION]
> Un límite inferior al tiempo ya jugado hoy suspende el juego en cuanto se vuelve a bloquear el control parental. La confirmación lo advierte cuando eso va a ocurrir.

**Se acabaron los cierres en 22.5.** `pctl:a`, el servicio privilegiado del control parental, solo acepta **una sesión**. Las versiones antiguas de la aplicación original la mantenían abierta, así que la petición de PIN del menú HOME (o el applet del PIN) no podía obtenerla y Atmosphère podía cerrarse. PlayGuard abre la sesión para cada acción y la libera de inmediato; las actualizaciones periódicas se detienen mientras la aplicación está en segundo plano. *(Diagnóstico del [fork de anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Nada se ejecuta en segundo plano.** El límite, el PIN, los avisos y la suspensión son los de la propia consola; PlayGuard solo cambia sus ajustes.

## Qué cuenta como tiempo de juego

El temporizador de juego es el de la consola: PlayGuard lo lee, no cuenta nada por sí mismo. Lo que se sabe, y con qué certeza ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), en inglés, tiene las mediciones):

| Situación | ¿Cuenta? | Con qué certeza |
|---|---|---|
| Un juego en marcha | Sí | Medido (22.0.0) |
| **Un juego que se deja abierto en el menú HOME** (suspendido en segundo plano) | **Sí**: ciérralo o pon la consola en modo de reposo | Informado por padres y madres ([Arqade](https://gaming.stackexchange.com/questions/398954/)); coincide con las mediciones (la consola cuenta mientras hay una aplicación abierta) |
| Modo de reposo | No | Informado ([Arqade](https://gaming.stackexchange.com/questions/398954/)); no medido por PlayGuard |
| **PlayGuard abierto sobre un juego** (como aplicación) | **Sí**, como ese juego | Medido (22.0.0): ábrelo con el usuario de un padre o una madre, o desde el álbum para echar un vistazo rápido |
| PlayGuard abierto desde el álbum (modo applet) | No | Deducido: no es una aplicación, y el tiempo fuera de las aplicaciones no parecía contar |
| Mientras el control parental está desbloqueado temporalmente | Quizá no | No medido: pregunta abierta |
| Después de cambiar el reloj | El día vuelve a empezar; tras un salto hacia atrás, la cuenta se detiene hasta que el reloj pasa la hora anterior | Observado (22.0.0); *Hoy* indica cuándo la consola no está contando |

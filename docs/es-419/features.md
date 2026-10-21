<!-- translated from docs/features.md, sha256 0adcb09f77ee -->
# Funciones

Qué hace cada pestaña, cómo escribe PlayGuard sin riesgos y qué cuenta la consola como tiempo de juego. ¿Es nuevo en PlayGuard? Empiece por [primeros pasos](getting-started.md). Volver al [README](../../README.md).

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
| **PIN** | **Preferencias** |
| ![PIN](../../images/screenshots/security.png) | ![Preferencias](../../images/screenshots/preferences.png) |

## Controles

| Botón | Qué hace |
|---|---|
| ↑/↓ | Mover |
| Ⓐ | Confirmar |
| Ⓑ | Volver o cancelar (dos veces en la barra lateral para salir) |
| Ⓧ | Actualizar (en las pestañas que leen la consola) |
| Ⓨ | Notificaciones, desde cualquier pestaña |
| **−** | Idioma y tema, desde cualquier pestaña (el pie de página no los muestra; la campana, el idioma y el tema del encabezado sí) |
| **+** | Guarda los límites por día (y la semana editada en *Tiempo de juego*) |
| **ZR** | *Deshacer*, durante unos segundos después de guardar un cambio |

La aplicación está organizada en pestañas, como la Configuración de la consola; cada una tiene su sección más abajo.

- **Lo que falló se indica en un cuadro de diálogo; lo que funcionó, en un aviso breve.** Un cuadro de diálogo solo indica lo que se aplica a esta consola en ese momento: que el juego en curso se suspendería, solo cuando hay un juego abierto; que la app para celular podría sobrescribir un cambio, solo cuando está vinculada y puede sincronizarse.
- **El encabezado** indica, en todas las pestañas, que el control parental está desbloqueado temporalmente (su píldora), y muestra un lápiz tachado a la derecha del título cuando la aplicación está en solo lectura; al tocar cualquiera de los dos, lleva a donde se deshace (*PIN › Reactivar el control parental*, la pantalla del firmware).
- **En modo de solo lectura**, las acciones siguen en su lugar, en gris, y explican por qué al presionarlas.

## El aspecto: Cadran

El aspecto de PlayGuard se llama Cadran: una barra de íconos para las pestañas, una paleta azul marino o blanco frío (según el tema), tarjetas en lugar de listas. Cada pestaña se abre en lo más importante (un anillo, mosaicos, tarjetas, chips), con sus demás opciones debajo.

- **La píldora del encabezado** indica de qué trata la pestaña (*Control parental activo*, *Temporizador activo*…); mientras el control parental está desbloqueado, lo indica en todas las pestañas, y al tocarla se abre *PIN › Reactivar el control parental*. El texto secundario nunca baja de 15 px, para poder leerlo desde el sillón en una TV.
- **Los cuadros de diálogo** son tarjetas con los botones en forma de píldora, la confirmación en turquesa (en coral cuando elimina o detiene algo).
  - Las opciones se abren como una tarjeta encima de las indicaciones de los botones, con la actual sobre un turquesa suave con su marca de verificación. El contorno turquesa es solo del foco, en todas partes: un nivel, una cuenta, una opción vigente conserva el fondo suave.
  - Un cuadro de diálogo largo se difumina en su parte inferior mientras hay más, y su advertencia (un juego a punto de suspenderse) va primero, en una tarjeta ámbar.
  - Un fallo lleva como título lo que falló, el código de error en pequeño debajo del motivo, y ofrece *Enviar un informe* cuando ayuda.
  - Los mensajes aparecen como tarjetas debajo del encabezado, con una marca de verificación turquesa para lo que se hizo y una marca ámbar para lo que se rechazó.
- **Las demás pantallas** adoptan el mismo aspecto: el editor por día de un perfil muestra la semana como en *Tiempo de juego* (se edita de la misma forma, con el relleno rápido como mosaicos); *Comprobar los juegos instalados* y *Todos los juegos* presentan sus líneas en tarjetas; *Perfiles* y *Ayuda y diagnóstico* le dan a cada línea su ícono.
- **Tamaño del texto** (*Preferencias*, o **AA** en el encabezado):
  - *Automático*, la opción predeterminada, agranda todos los textos un 25 % mientras la consola está en la base, en la TV, y los devuelve a su tamaño en modo portátil; *Más grande* los mantiene agrandados, *Normal* nunca.
  - El encabezado, la barra lateral y el pie de página conservan su tamaño (no tienen lugar para más), al igual que los nombres de los niveles de Restricciones, que ya son grandes.
  - Cambia de inmediato, en todas las pantallas, sin reiniciar.
  - El 25 % sale de una TV de 55" en 4K: el texto más pequeño (15 px) mide entonces 14.6 mm de alto en mayúsculas, 20′ de arco a 2.5 m, donde la lectura es cómoda (16′ en tamaño normal, el mínimo indispensable).
- **Efectos visuales** (*Preferencias*): *Más ligeros con batería*, la opción predeterminada, omite el halo de fondo, el latido del anillo de la pantalla del niño en sus últimos minutos y el barrido del arco hasta un nuevo valor mientras no hay ningún cargador conectado (en modo portátil, desconectada); *Siempre completos* los mantiene. Se vuelve a leer cada 5 segundos, así que al conectar la consola vuelven.
- **Deshacer:** después de guardar un cambio, **ZR** *Deshacer* queda unos segundos en el pie de página: abre ese cambio en el historial de cambios, para restablecer el valor anterior. Un historial de cambios vacío lo indica en una tarjeta.

## Notificaciones — lo que requiere su atención, desde cualquier pestaña

Una campana en el encabezado, a la izquierda de la versión, reúne lo que requiere la intervención del adulto; Ⓨ abre la lista desde cualquier pestaña (el contador de la campana indica *2* cuando hay dos cosas pendientes).

A la izquierda de la campana, el idioma (su código, *FR*, *EN*…) y el tema (un sol para el claro, una luna para el oscuro, un círculo medio lleno cuando sigue a la consola), y luego el tamaño del texto (**AA**, con su A grande en turquesa mientras el texto está agrandado): al tocarlos se abre la misma lista que en *Preferencias*, y **−** los ofrece desde cualquier pestaña. Un nuevo tema o idioma se aplica de inmediato, y cada pantalla se vuelve a crear en la misma pestaña con el foco en la misma línea, chino y su propia fuente incluidos.

### Pendiente

Cada uno con una señal de advertencia ámbar, contado en ámbar en la campana; se mantiene hasta que se resuelve:

- todavía no hay PIN, o un PIN que cualquiera probaría primero (0000, 1234, 1212, 2580…: se evalúa en la consola, nunca se muestra);
- un PIN en la consola mientras PlayGuard se abre sin él (*Proteger PlayGuard con el PIN* en *Nunca*);
- el control parental desbloqueado temporalmente (*Reactivar*);
- tiempo extra de un día anterior que falta restablecer;
- la alarma de “Se acabó el tiempo” desactivada;
- cambios hechos fuera de PlayGuard;
- la consola que no cuenta el tiempo de juego;
- un reloj de red impreciso.

### Para tu información

Cada uno con una “i”, un punto en la campana; se puede marcar como leído:

- un límite que sigue reducido por *No jugar más hoy* de un día anterior (*Restablecer*);
- una nueva versión instalada (*Novedades*; tras una actualización en su lugar, la compilación anterior conservada), o una versión más reciente de PlayGuard;
- un informe enviado en línea (*Mostrar el enlace*), o PlayGuard cerrado de forma inesperada la última vez (*Enviar un informe*);
- el tiempo de juego de la semana (cada 7 días hasta que se lea; con la comparación por día con la semana anterior; *Mostrar su código QR* abre el resumen de la semana en *Actividad*), y un límite superado dos o más de los últimos 7 días cuando la consola solo avisa;
- un respaldo de la configuración por hacer (ninguno todavía, o ninguno en 30 días);
- todos los juegos bloqueados durante una semana;
- una nueva cuenta de usuario;
- la app para celular vinculada;
- el reloj de red desajustado al iniciar.

### Consola

Las entradas sobre la consola más que sobre el control parental van al final, en una línea que Ⓐ abre (abierta de entrada cuando no hay nada más). Las pendientes se cuentan en la insignia de la pestaña Consola, no en la campana:

- juegos instalados que no pueden iniciarse (*Comprobar los juegos instalados*), parches de juegos que hay que revisar;
- la tarjeta SD casi llena (menos de 500 MB);
- un archivo propio de PlayGuard que no se pudo apartar o escribir en la tarjeta SD;
- el sysmodule de recuperación sin actualizar (no es el que trae esta versión, o se instaló antes de que indicara su versión; *Actualizar* instala el que trae esta versión), luego la consola que hay que reiniciar para que se use, hasta que se reinicie;
- una compilación que no terminó de iniciarse y de la que se volvió atrás (*Enviar un informe*).

Las informativas: una compilación de la que se volvió atrás, un firmware no probado con esta versión, una versión más reciente de Atmosphère, hekate o el firmware (comprobada con *Buscar actualizaciones cada día*; la entrada del firmware aconseja esperar a que Atmosphère sea compatible), el número de serie no ocultado en emuMMC, el control parental propio del otro sistema, un archivo propio de PlayGuard encontrado dañado y apartado (un aviso breve al iniciar lo indica).

### Cómo se comportan las entradas

- Cada sección enumera sus entradas en un orden fijo, sin importar qué comprobación terminó primero.
- **En la barra lateral**, una insignia ámbar junto al nombre de una pestaña cuenta sus entradas pendientes, ahí donde se resuelven: la pestaña PIN (sin PIN, un PIN débil, PlayGuard sin su PIN, desbloqueado temporalmente), Tiempo de juego (la alarma desactivada, la consola que no cuenta), Hoy (cambios hechos fuera de PlayGuard, tiempo extra por restablecer), la Consola (sus entradas de arriba, y el reloj de red). Las entradas ignoradas y las informativas no se cuentan.
- Dentro de la pestaña, la misma insignia está en la fila donde se resuelve cada una (*Reloj de red*, *Comprobar los juegos instalados*, *Almacenamiento*, *Parches de juegos*; *Definir o cambiar el PIN*, *Proteger PlayGuard con el PIN*, *Reactivar el control parental*; *Avanzado* de Tiempo de juego para la alarma), y la pestaña se abre en la primera de ellas.
- **Ⓐ** en una entrada aplica la solución (*Activarla*, *Ajustarlo*, *Actualizar*…) o abre la pestaña donde se hace. Una entrada desaparece sola una vez que se resuelve lo que indica.
- **Ⓧ** marca una entrada informativa como leída; **Ⓨ** las marca todas. Una entrada marcada como leída sigue leída en el siguiente inicio, hasta que su texto cambie.
- **Ignorar:** algunas entradas pendientes se pueden ignorar con Ⓧ, tras una confirmación y el PIN: un PIN que cualquiera probaría primero, PlayGuard que se abre sin el PIN, la alarma de “Se acabó el tiempo” desactivada, la tarjeta SD casi llena (nunca la falta de PIN, el control desbloqueado, el reloj, el temporizador de juego que no cuenta ni los archivos propios de PlayGuard). Una entrada ignorada deja de contarse y aparece al final, en *Ignorados*, donde Ⓐ la vuelve a contar. Vuelve sola 30 días después (salvo que *Volver a contar los avisos ignorados después de 30 días* esté desactivado), cuando lo que indica se resuelve y luego vuelve a ocurrir, o, para el PIN, con un PIN nuevo de otra longitud.
- *Preferencias › Notificaciones* desactiva tres tipos: versiones de Atmosphère, hekate y el firmware; el tiempo de juego de la semana; los recordatorios (respaldos, el otro sistema, todos los juegos bloqueados).
- El estado detrás de todo esto se lee al iniciar PlayGuard, cada minuto en la pantalla principal y cada 5 s mientras se muestra *Hoy*.

## Hoy — el día de un vistazo

*[Probarlo en el simulador ›](https://playguard.link/simulator/)*

### El anillo y la línea

**El tiempo restante** en un anillo: un día sin juego indica *Sin jugar*, no *0 min*; el tiempo extra dado hoy aparece como un segmento más claro en su extremo, y el anillo se desplaza hasta cada nuevo valor.

Debajo, lo que detiene el juego primero:

- antes del final del tiempo o de la alarma de hora de dormir, el resto del día como una línea, con el momento actual a su izquierda, y cada uno de ellos como una marca con su hora (*16:40 tiempo agotado*, *21:00 hora de dormir*), en turquesa hasta la primera, en ámbar en sus últimos 15 minutos;
- si no, en palabras: la alarma de hora de dormir una vez agotado el tiempo del día, la hora en que se puede volver a jugar; mientras el control parental está desbloqueado, el límite de hoy en pausa y por qué.

### Las acciones

- **Tiempo extra**, con sus cantidades a una pulsación: una cantidad seleccionada desplaza el final del tiempo en la línea (*con 15 min más*), o indica en la marca de la alarma de hora de dormir que esta sigue llegando antes; la primera se destaca en turquesa cuando quedan 15 min o menos, o nada.
- **Terminar por hoy**, **Mostrar al niño**.
- **Límite de hoy…**: el límite de hoy elegido en el momento, escrito tras una confirmación.
- **El perfil vigente**, por su nombre, una vez guardado un perfil (*Perfiles…* cuando ninguno coincide): Ⓐ los enumera, con el vigente marcado, para aplicar otro tras una confirmación (los días que cambian), o *Administrar los perfiles…*.
- Mientras el control parental está desbloqueado, **Reactivar** va primero, a todo el ancho de la fila, y *Tiempo extra* se hace a un lado.
- Si no hay límite ningún día, **Definir los límites de la semana** abre la semana en *Tiempo de juego*.

### Las tarjetas y las píldoras

- La **alarma de hora de dormir** (la hora en que se puede volver a jugar, o en cuánto tiempo suena cuando la consola no lo indica; desactivada, *Desactivada* en grande y Ⓐ *Configurarla*), el **control parental** y el **reloj de red**, como tarjetas.
- El estado de la consola como **píldoras**:
  - una indica si la redirección DNS de Atmosphère bloquea los servidores de Nintendo, y Ⓐ en ella, cuáles y qué cambia eso: la app para celular no se sincroniza y el reloj de red ya no se ajusta solo;
  - otra, al abrirlo sobre un juego, que el tiempo que se pasa en PlayGuard cuenta, y Ⓐ explica cómo;
  - un juego que se deja abierto en el menú HOME aparece como una píldora ámbar (*sigue contando*, y Ⓐ explica por qué), cuando la píldora de PlayGuard abierto sobre un juego no lo indica ya.
- Las píldoras muestran primero lo que requiere la intervención del adulto (ámbar, con un triángulo), luego lo que está activado (turquesa, con una marca de verificación) y después el resto (gris, con un punto): la forma lo indica tanto como el color.
- Debajo de las píldoras, el tiempo extra que falta restablecer (Ⓐ lo restablece) y la alarma de “Se acabó el tiempo” mientras está desactivada (Ⓐ la vuelve a activar).

### Primeros pasos

Hasta que se definan el PIN y los límites de la semana, una tarjeta encima de las demás enumera los tres pasos (el PIN, un límite diario, el reloj de red), cada uno marcado en turquesa una vez hecho o rodeado en ámbar, con cuántos se completaron (*1 de 3*); Ⓐ en uno lo hace ahí mismo.

### ¿La consola está contando?

El tiempo del propio PlayGuard cuenta mientras se ejecuta como aplicación, así que, con un límite hoy, el tiempo restante debería bajar mientras está abierto. Cuando no se movió durante unos 90 s en pantalla (temporizador activado, un límite hoy, sin desbloqueo, sin abrir desde el Álbum), una línea ámbar debajo del tiempo jugado indica *La consola no está contando el tiempo de juego en este momento*, con las causas probables:

- el reloj se atrasó (volverá a contar más tarde);
- el reloj de red nunca se ajustó (*Consola › Reloj de red*);
- se acaba de escribir un límite (inicie un juego un minuto y vuelva a revisar).

Desaparece en cuanto el tiempo restante vuelve a moverse. Tiempo de juego también lo indica.

### Cambios fuera de PlayGuard

El tiempo de juego de hoy que vuelve a empezar el mismo día (un cambio de reloj lo provoca), el reloj de la consola movido más de 5 minutos, límites distintos de los que PlayGuard vio la última vez (la app para celular, otra herramienta): una tarjeta ámbar encima del anillo hasta que se descarte (Ⓐ) o hasta el día siguiente, y una entrada en el historial de cambios. PlayGuard no se ejecuta en segundo plano: solo ve lo que cambió mientras estaba abierto, o desde la última vez que lo estuvo, cuando vuelve a leer la consola.

### Al llegar al límite

La consola muestra “Se acabó el tiempo” y luego suspende el juego (sin “continuar”) o solo avisa: *Cuando se acabe el tiempo* indica cuál de las dos y permite cambiarlo, una vez que hay un límite o una hora de dormir configurados ([detalles](../parental-controls.md#when-the-time-runs-out), en inglés).

### Tiempo extra y no jugar más hoy

- **Tiempo extra hoy**: +15 min, +30 min, +1 h de forma predeterminada.
- **No jugar más hoy**: límite 0 solo por hoy, *ahora* o *en 5 minutos* cuando se conoce el tiempo jugado (el límite pasa a ser el tiempo jugado + 5 min, tiempo para guardar). Al bloquearse, aparece el “Se acabó el tiempo” de la consola, y el progreso no guardado puede perderse: la confirmación lo indica.
- **Al día siguiente** —al iniciar, o a la medianoche si la aplicación está abierta— PlayGuard restablece el límite habitual:
  - después de tiempo extra, solo, sin preguntar ni pedir el PIN (solo baja el límite, como al reactivar el control parental), desbloqueando un momento si el temporizador está contando y volviendo a bloquear de inmediato. Espera mientras no puede escribir (modo de solo lectura, un firmware no compatible, el bloqueo de la consola activado) y, una semana después el mismo día, mientras el límite más bajo detendría el juego en curso (entonces, al día siguiente). Para conservar más tiempo un día, cambie el límite de ese día;
  - después de *No jugar más hoy*, pregunta si mantener 0 o restablecer el límite (con el PIN, como cualquier cambio que sube un límite), o lo restablece como cualquier cambio (*Tiempo de juego › Tras “No jugar más hoy”, restablecer el límite*); restablecer “sin límite” siempre se pregunta.
- **Tiempo extra otro día** (*Tiempo extra › Otro día…*, al final de su lista): mañana o uno de los cinco días siguientes, cada uno con su límite, y luego la cantidad. El límite de ese día se sube de inmediato, solo para esa fecha, y se restablece solo (solo baja el límite) la próxima vez que se abre PlayGuard una vez terminado el día; no en una consola en la que no se abre PlayGuard, donde el mismo día de la semana siguiente también lo conserva. Uno a la vez: la lista ofrece entonces quitarlo (su límite se restablece de inmediato). Si mientras tanto cambia el límite de ese día (la semana, un perfil…), ese cambio prevalece: no se restablece nada.

### La pantalla del niño

*Mostrar al niño* en *Hoy*, para entregar la consola:

- solo el tiempo restante (mientras el control parental está desbloqueado, el límite de hoy, en pausa), cuándo termina el juego (*Termina hacia las 16:40*; cuando la alarma de hora de dormir llega antes, el anillo hace la cuenta regresiva hasta ella, *45 min antes de dormir*, como en la última hora de un día sin límite), la hora de dormir y lo que pasa cuando se acaba el tiempo;
- pensada para leerse desde el otro lado de la habitación, en la TV; su texto conserva su tamaño, sea cual sea el tamaño del texto;
- en los últimos 5 minutos (de juego o antes de la hora de dormir), el final se vuelve ámbar y el anillo late lentamente; la hora de dormir se vuelve ámbar en sus últimos 30;
- Ⓐ y Ⓨ, los botones del adulto, aparecen en pequeño arriba a la derecha, para que el pie de página muestre solo el del niño;
- Ⓐ vuelve a las pantallas del adulto con el PIN; Ⓨ da tiempo extra desde ahí, tras el PIN (las cantidades de *Tiempo extra*, sin salir de la pantalla del niño); Ⓑ sale de PlayGuard (y antes pregunta si reactivar el control parental, como al salir).

Con el texto más grande, la pestaña Hoy sigue cabiendo en una sola pantalla: la línea encima del título solo indica el día y su límite (*JUEVES · 2 h*), y los botones y las tarjetas se ajustan un poco. Se actualiza cada 5 s (Ⓧ actualiza ahora).

## Tiempo de juego — la semana es el editor

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=play_timer)*

### Editar la semana

Ⓐ en un día lo edita:

| Botón | En la edición |
|---|---|
| ↑/↓ | Lo cambia de 15 en 15 min, de hora en hora al mantenerlos presionados |
| ←/→ | Pasa al día siguiente |
| Ⓛ | Lo copia de lunes a viernes |
| Ⓡ | Lo copia al fin de semana |
| Ⓨ | Sin límite |
| Ⓧ | Una duración exacta (*Ingresar una duración…*: ↑/↓ de 15 en 15 min, de hora en hora al mantenerlos presionados, Ⓛ/Ⓡ de 5 en 5 min, con OK seleccionado) |
| Ⓐ o Ⓑ | Termina la edición |

- En la pantalla táctil, al tocar un día se edita, y luego un toque en su parte de arriba o de abajo lo cambia en 15 min.
- Los cambios se guardan juntos, con **+** o *Guardar*, como en el editor por día (la confirmación enumera los días cambiados, antes → después); *Descartar* los deshace.
- Dentro de la barra de cada día, el tiempo jugado la última vez que tocó ese día (incluido hoy, todas las cuentas), en coral una vez alcanzado el límite, también su límite, y una leyenda debajo de la semana lo indica (*límite superado*).
- Un día cambiado y todavía sin guardar tiene su límite en ámbar y su barra rodeada de ámbar; debajo de la semana, los datos del día seleccionado (*jueves: 1 h 20 jugado de 2 h*).
- Encima de la semana, en ámbar, un límite alcanzado y lo que hace entonces la consola, o la consola que no cuenta el tiempo de juego.
- `0` minutos significa *no jugar ese día*; *Quitar el límite de tiempo de juego* desactiva el temporizador.

### Chips y ajustes rápidos

**Chips** encima y debajo de la semana abren directamente sus opciones: los perfiles (cuántos hay guardados), el mismo límite todos los días, la alarma de hora de dormir, *Cuando se acabe el tiempo* y quitar el límite.

- **Mismo límite todos los días:** una lista rápida o una duración exacta (*Ingresar una duración…*: ↑/↓ de 15 en 15 min, de hora en hora al mantenerlos presionados, Ⓛ/Ⓡ de 5 en 5 min).
- **Entre semana y fin de semana ›:** el editor, que pide el límite de lunes a viernes y luego el del fin de semana.
- **Tiempo extra hoy** y **no jugar más hoy**, como en *Hoy*. Una nota debajo indica qué hace la consola al llegar al límite. Mientras el control parental está desbloqueado temporalmente, es *posible* que el tiempo de juego no se cuente: todavía es una pregunta abierta ([documentación](../parental-controls.md#still-open), en inglés).
- Cada confirmación enumera los días que cambian, antes → después.

### Perfiles

- **Perfiles** guardados en la tarjeta SD: aplicar (un perfil que ya está vigente simplemente lo indica), editar, cambiar el nombre o eliminar uno; guardar los límites actuales o crear uno nuevo.
- Cualquier nombre, en cualquier escritura (*École*, *周末*, *Выходные* …): un nombre sin letras latinas recibe su propio nombre de archivo, y se detectan dos nombres que darían el mismo archivo.
- **Perfil programado** (*Perfiles › Perfil programado*): un perfil para un período —vacaciones, un viaje—, con su primer y su último día elegidos entre los próximos 60 días.
  - Cuando empieza el período, PlayGuard pregunta si aplicarlo (con los días que cambia dibujados debajo de la pregunta); cuando termina, si restablecer los límites que reemplazó o conservarlos.
  - Se pregunta una vez al día, en la pantalla principal, cuando no hay nada más delante: PlayGuard solo actúa cuando se abre, así que una consola en la que no se abre conserva los límites que tiene. Un período que pasó sin que el perfil llegara a aplicarse se olvida.
  - Uno a la vez; la línea lo cambia, lo quita o lo termina.

### Alarma de hora de dormir

- La hora de la alarma (de 16:00 a 23:45, o desactivada) y la hora en que se puede volver a jugar (de 05:00 a 09:00), la misma todos los días.
- O, desde *Por día de la semana…* al final de la lista de la alarma, de lunes a viernes, el fin de semana o un solo día, y los demás días conservan la suya (cada grupo de días aparece con su hora). Cuando los días difieren, la pestaña indica *Varía según el día*, y la semana en *Tiempo de juego* muestra la alarma de cada día debajo de su nombre.
- Su lugar en la configuración del temporizador de juego, deducido de la configuración de la app para celular, está verificado en una consola (22.0.0); aun así, PlayGuard solo la cambia una vez que la consola informa lo que PlayGuard lee ahí, comprueba la respuesta de la consola después del cambio y restablece la configuración anterior si no coincide.

### Ajustes del tiempo extra y Avanzado

- **Tiempo extra:** las cantidades que se ofrecen (+15/+30/+1 h, +10/+20/+30 min …) y restablecer el límite habitual sin preguntar después de *No jugar más hoy* (el tiempo extra siempre se restablece sin preguntar). Ambas cosas se hacen la próxima vez que se abre PlayGuard, a partir del día siguiente: una consola en la que no se abre conserva el límite de ese día.
- **Avanzado ›**, una pantalla propia: el interruptor de las acciones de depuración y, mientras está activado, la alarma de “Se acabó el tiempo” activada/desactivada y pausar / reanudar la cuenta regresiva. El mismo interruptor permite que un respaldo o el historial restablezcan la alarma.

## Actividad — quién jugó a qué, y cuánto tiempo

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=activity)*

Tiempo por juego **hoy**, en los **últimos 7 días** y **en total**, a partir del propio registro de actividad de la consola, para todas las cuentas o para **una cuenta de usuario** (las cuentas como chips arriba, cuando la consola tiene varias). PlayGuard se abre en la última cuenta que se mostró, o en la elegida en *Cuenta predeterminada* (todas las cuentas, o una).

### Arriba

- Las cuentas como chips, el período y la exportación.
- Hoy, los últimos 7 días y el total como cifras. Debajo de los últimos 7 días, la comparación con los 7 anteriores, por día: ▲ *+20 min/día vs. semana anterior* en ámbar, ▼ menos en turquesa, *como la semana anterior* con un margen de 5 minutos.
- Los cuatro juegos más jugados como tarjetas (Ⓐ abre uno); en los últimos 7 días, el que más aumentó respecto de la semana anterior indica cuánto, a partir de media hora (▲ *+2 h*).
- Si no se jugó nada en el período, una tarjeta lo indica, con *Ver el total* cuando hay algo que ver.

### Gráficos y resumen

- **Un gráfico de los últimos siete días** (hoy a la derecha, con una leyenda debajo), con el límite de esta semana para cada día como una línea y el tiempo que lo supera en ámbar, marcado con !; los totales de hoy, de la semana y de siempre.
- **Un resumen** del período elegido (hoy, los últimos 7 días, los **últimos 30 días** o el total), para la cuenta que se muestra: promedio por día, el juego más jugado, los días jugados y el de más actividad (últimos 7 días), la sesión promedio (en total).
- **Horas de juego, últimos 7 días**: un cuadrado por cada hora de cada día, empezando por el lunes, más intenso cuanto más se jugó (todos los juegos, la cuenta que se muestra); luego las sesiones (el juego con pausas de menos de 5 minutos cuenta como una sola sesión, aunque se cambie de juego) y su duración promedio, la más larga y su día, las dos horas seguidas con más juego, y el tiempo jugado entre las 21:00 y las 06:00.

### Todos los juegos y la página de un juego

- **Todos los juegos**, en una pantalla propia: cada juego jugado en el período, los más jugados primero, como **cuadrícula de portadas** o como **lista** (Ⓨ alterna, y la elección se conserva), con los datos del juego seleccionado siempre arriba: su ícono y su nombre, el tiempo de juego del período, el total, los inicios y la última partida. Ⓧ elige el período (la pestaña Actividad lo sigue).
- Los íconos se leen alrededor del juego seleccionado (no en modo applet, para ahorrar memoria); un control gris los reemplaza hasta que llegan, y para un juego que no tiene ícono.
- Una biblioteca grande muestra sus primeros 50 juegos y luego *Mostrar todos los juegos* (100 juegos como máximo en modo applet); la exportación siempre los incluye todos. PlayGuard lee 256 juegos como máximo (primero los de esta semana): más allá de eso, la pestaña Actividad, Todos los juegos y la exportación (salvo CSV) indican que faltan algunos, y el total los deja fuera.
- **Ⓐ en un juego:** sus últimos siete días en barras, los inicios, la primera y la última partida, el tiempo por cuenta de usuario. Los juegos eliminados conservan sus totales, incluso uno eliminado antes de instalar PlayGuard mientras el registro de juego de la consola todavía lo mencione, y su nombre una vez que PlayGuard lo vio (si no, *Juego eliminado* y su Title ID).

### Cuentas

- El tiempo de un juego que no tiene ninguna cuenta de usuario (algunos juegos nunca piden una, o la cuenta se eliminó después) aparece como *Sin cuenta (o una cuenta eliminada)*, a partir de un minuto.
- Una cuenta de usuario que PlayGuard nunca había visto se marca en ámbar en la pestaña durante 30 días (*Cuenta nueva desde el …*: crear una cuenta nueva es una forma conocida de empezar de cero) y se registra en el historial de cambios; las cuentas que ya estaban cuando PlayGuard las lista por primera vez no se marcan.

### Exportar y compartir

- **Exportar a la tarjeta SD** (el chip *Exportar*) en CSV, JSON, XLSX (Excel) o PDF, una columna por día, y luego los últimos 7 días, los últimos 30 días y el total.
- **El resumen de la semana en código QR:** la cámara de un celular lo lee como unas pocas líneas de texto, en el idioma que se muestra: los últimos 7 días y su promedio, la tendencia, cada día, los tres juegos más jugados, las sesiones, la más larga y el tiempo jugado entre las 21:00 y las 06:00. Nada se envía a Internet.
- Se abre de inmediato con los últimos datos leídos (guardados en la tarjeta SD entre una sesión y otra), que se actualizan en segundo plano cuando tienen más de un minuto; Ⓧ los vuelve a leer ahora, con un indicador de carga.
- Los tiempos son aproximados si se cambió el reloj de la consola.

## Restricciones — nivel, clasificación por edad, comunicación

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=restrictions)*

- **El nivel de restricción:** Niños pequeños, Niños, Adolescentes y Personalizado, uno al lado del otro como mosaicos, cada uno con lo que restringe (un candado para lo restringido, una marca de verificación para lo permitido, y los juegos instalados que bloquea; Ⓐ lo aplica; en *Personalizado* ya vigente, Ⓐ lleva a sus opciones de abajo); *Ninguno* debajo. Lo que restringe un nivel (límite de edad, publicaciones, comunicación) se vuelve a indicar antes de aplicarlo.
- **En Personalizado:** clasificación por edad, publicaciones en redes sociales, comunicación con otros usuarios.
- **Modo RV** y el **sistema de clasificación** (PEGI, ESRB, USK, CERO …): la lista indica los países para los que clasifica cada uno (PEGI: la mayor parte de Europa, ESRB: Estados Unidos, Canadá y Latinoamérica, USK: Alemania, PEGI BBFC: Reino Unido…).
- **La cantidad de juegos autorizados a comunicarse** (la lista en sí está en el informe de diagnóstico, sin procesar, hasta que se conozca su estructura).
- **Juegos instalados que bloquea:** cuántos de los juegos instalados bloquea el nivel vigente por su clasificación por edad (*3 de 15*), y Ⓐ los enumera con su clasificación (*PEGI 16*); el mosaico de cada nivel también lo indica (*Juegos bloqueados · 3*).
  - Las clasificaciones son las de cada juego (sus datos de control, leídos por la comprobación de los juegos instalados al iniciar o desde *Consola › Comprobar los juegos instalados*), según el sistema de clasificación que usa la consola.
  - Un juego que ese sistema no clasifica no se cuenta, ni tampoco las excepciones configuradas en la app para celular.
  - La pantalla de un juego en Actividad muestra su clasificación, y *bloqueado en el nivel actual* cuando lo está.

## PIN — el PIN, el desbloqueo, el bloqueo de juegos, la app para celular

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=security)*

- **Arriba**, tres mosaicos: el PIN, las restricciones y el desbloqueo temporal, cada uno con una marca de verificación turquesa cuando está como debe (un PIN definido, las restricciones activas, bloqueado) o una señal de advertencia ámbar cuando hay que intervenir.
- **Definir o cambiar el PIN** (pantalla del PIN del sistema).
- **Mostrar el PIN**: tras una advertencia y el propio PIN, que se pide cada vez; queda registrado en el historial y luego se ofrece un PIN nuevo.
- **Desbloquear temporalmente**, **reactivar el control parental**, y si PlayGuard lo reactiva solo justo después de un cambio que necesitó el desbloqueo (activado de forma predeterminada).

### Proteger PlayGuard con el PIN

El PIN en el propio PlayGuard:

| Opción | Efecto |
|---|---|
| *Nunca* | Sin PIN en PlayGuard (la campana lo señala mientras la consola tiene uno) |
| *Antes de un cambio* (predeterminado) | Cualquiera puede mirar, solo el adulto cambia algo |
| *Para abrir PlayGuard* | Una pantalla de bloqueo al iniciar |

- Una vez ingresado, el PIN vale hasta que se cierra PlayGuard; *Volver a pedir el PIN tras 5 minutos* (desactivado de forma predeterminada) lo vuelve a pedir pasado ese tiempo, y vuelve a mostrar la pantalla de bloqueo de *Para abrir PlayGuard* cuando PlayGuard regresa tras 5 min o más en segundo plano.
- Se comprueba en la capa de servicio, así que ningún cambio lo omite; volver a bloquear nunca lo pide. *Mostrar el PIN* lo pide cada vez, en todos los modos e incluso justo después de ingresarlo, y un `config.json` ausente o dañado cuenta como *Antes de un cambio*.
- **Salir** mientras el control parental sigue desbloqueado pregunta si volver a bloquearlo (*Reactivar y salir* también cierra el juego sobre el que se abrió PlayGuard, y vuelve al menú HOME en lugar de a hbmenu); si en cambio se cierra desde el menú HOME, PlayGuard lo vuelve a bloquear la próxima vez que se inicia, y un reinicio de la consola lo bloquea por sí solo.

### Bloquear todos los juegos

Un interruptor que pone en 0 el límite de cada día, así que se necesita un PIN para iniciar un juego: un bloqueo ligero, sin clasificaciones por edad ni límites de comunicación.

- Bloquea el inicio de los juegos, no el menú HOME, y necesita un PIN.
- Los límites anteriores vuelven cuando se desactiva.
- Mientras está activado, *tiempo extra* y *no jugar más hoy* se rechazan, y los límites configurados de otra forma (un perfil, un respaldo, el historial…) lo reemplazan.

### App para celular

- Si la app Control parental de Nintendo Switch está vinculada, su última sincronización, y **desvincular** (si no, su próxima sincronización sobrescribe los límites configurados aquí: mientras está vinculada, cada cambio que PlayGuard pide confirmar lo indica).
- Cuando la redirección DNS de Atmosphère (`atmosphere/hosts`) desvía a otro lugar el servidor del control parental de Nintendo, la app no puede sincronizarse: PlayGuard indica entonces aquí *sincronización bloqueada (DNS)* y no menciona la app en ningún otro lugar (ninguna nota en los cuadros de diálogo, ninguna entrada en la campana, ningún paso en los primeros pasos, ninguna píldora en *Hoy*, ninguna ayuda para descifrar, y la desvinculación se confirma sin decir qué sobrescribiría la app).
- Mientras está vinculada y puede sincronizarse, *Antes de desvincular: ayude a descifrar…* abre la comparación del bloque (consulte [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), en inglés).

### Eliminar todo el control parental

Dos confirmaciones, irreversible; antes se guarda un respaldo de la configuración.

## Consola — el reloj de red, los juegos instalados, lo que ejecuta la consola

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=console)*

**Arriba**, lo que ejecuta la consola como mosaicos (el firmware, Atmosphère, y hekate cuando está en la tarjeta SD; Ⓐ abre sus notas de la versión), luego el diagnóstico y después las líneas de la consola, empezando por la compatibilidad (solo una marca de verificación turquesa cuando PlayGuard se comprobó en este firmware; si no, una advertencia y lo que falla).

### Reloj de red

*Reloj de red ›*, una pantalla propia: el reloj del que depende el temporizador de juego. Una consola que nunca llega a los servidores de Nintendo mantiene este reloj impreciso, lo que altera el temporizador de juego.

- **Relojes de la consola y de red**, zona horaria, precisión. Cuando la redirección DNS de Atmosphère (`atmosphere/hosts`) desvía a otro lugar el servidor de hora de Nintendo (`aauth-lp1.ndas.srv.nintendo.net`), la consola ya no puede ajustar su reloj de red por sí misma: esta pantalla, la entrada de la campana y la tarjeta de *Hoy* lo indican y remiten a la medición de abajo.
- **Elegir un servidor NTP público** (≈ 50 incluidos, por región, o uno propio).
- **Medir** con 3 servidores (mediana; un aviso ámbar cuando no coinciden) y luego **ajustar el reloj de red**: una medición sigue siendo válida durante 2 minutos, con una cuenta regresiva. La confirmación lo indica: cambiar el reloj pone a cero el tiempo de juego de hoy, así que vuelve a estar disponible todo el límite.
- **Una comprobación del reloj de red al iniciar**: un aviso breve cuando tiene más de un minuto de diferencia; nunca ajusta el reloj.

#### Ningún servidor responde

En una consola sin Internet, *Ajustar el reloj de red al reloj de la consola* aparece debajo del resultado (y en la tarjeta del reloj de red de *Hoy*, cuando falla su medición), detrás de la misma confirmación.

- PlayGuard no tiene nada más con qué comparar el reloj de la consola: la confirmación muestra su hora y le pide que primero la compare con un celular o un reloj, y se rechaza un reloj de consola anterior a 2026 (nunca ajustado).
- Con *Sincronizar el reloj por Internet* desactivado, el reloj de la consola se queda donde está, y activar esa opción después ya no lo hace saltar; con la opción activada, el reloj de la consola ya sigue al reloj de red, así que la confirmación indica que la desactive y ajuste la hora a mano si es incorrecta.
- Solo escribe el reloj, no el control parental, así que no necesita desbloqueo (el PIN de *Proteger PlayGuard con el PIN* sigue aplicándose); como cualquier cambio de reloj, pone a cero el tiempo de juego de hoy.
- Todavía no se probó en una consola.

### Juegos instalados

*Comprobar los juegos instalados* encuentra los juegos que el menú HOME no puede iniciar ni mostrar —un ícono que no termina de cargar, un “!”, un marco punteado— con lo que falla y qué hacer, debajo del ícono del juego, y cuántos juegos instalados tienen un problema.

**Lo que encuentra:** nombre e ícono ilegibles, el juego en sí ausente detrás de su actualización, nada instalado ya, archivos faltantes, datos dañados que encontró la consola, un firmware más reciente que exige el juego o solo su actualización, claves demasiado nuevas para el firmware, una licencia (ticket) faltante, la consola que pide una actualización que se eliminó, la tarjeta SD ilegible, un inicio que la consola rechaza.

**La lista tiene una sección por cada salida**, cada una con su cantidad y qué hacer, con los juegos que tienen el mismo problema juntos (dos juegos con el mismo nombre se distinguen por su versión o su Title ID):

| Sección | Qué contiene |
|---|---|
| **Reparables aquí** | Lo que PlayGuard repara |
| **Por comprobar** | Datos de los que la consola sospecha: la comprobación indica si hay que reinstalar el juego |
| **Reinstalar** | Archivos que ya no están o no se pueden usar, o datos que resultaron dañados: eliminar y luego volver a instalar |
| **Firmware o tarjeta SD** | El juego está bien, la consola no |

- **Comprobar los datos:** lo que encontró la comprobación de datos se guarda con el juego hasta que se instale otra versión: si están dañados, pasa a *Reinstalar* como *Dañado (comprobado)*, con la fecha y el código de la consola; si están intactos, sale de la lista. *Comprobar estos juegos* comprueba toda la sección *Por comprobar*, un juego tras otro en un solo cuadro de diálogo (*juego 2 de 5*), y luego indica cuántos están dañados, intactos o no se pudieron comprobar, con la opción de eliminar los dañados al alcance de la mano.
- **Varios juegos a la vez:** Ⓨ selecciona juegos (una marca de verificación delante), y *Varios juegos a la vez*, encima de las secciones, selecciona una sección entera; luego, una sola acción para todos: comprobar sus datos o eliminarlos (antes se guarda su lista, con una sola pregunta y un solo PIN).
- **En *Reinstalar*,** se puede guardar su lista (JSON y texto, en la carpeta de exportaciones: nombre, Title ID, versión, problema), y eliminarlos todos juntos tras una sola pregunta y un solo PIN: primero se guarda la lista y luego se eliminan los juegos, para que un instalador ya no los cuente como instalados.
- **Las acciones:** cuando una acción es la solución, es el primer botón del cuadro de diálogo; las demás vienen después: volver a aceptar la versión instalada (cuando la consola pide una actualización que se eliminó), eliminar solo la actualización o un contenido adicional, comprobar si los datos están dañados (la comprobación propia de la consola, con su progreso y Cancelar) o eliminar el juego.
- Las versiones se muestran como las muestra el juego (*versión 1.4.0*, *actualización n.º 4*); después de una reparación, un aviso breve indica si el juego vuelve a funcionar. Los datos de guardado siempre se conservan; cada cambio queda en el historial de cambios.
- **Cuándo comprueba:** la última comprobación se muestra de inmediato; luego solo se vuelven a comprobar los juegos cuya entrada cambió (todos los juegos una vez al día, después de una actualización del firmware, o con Ⓧ *Comprobar de nuevo*), cada uno en cuanto se comprueba.
- Varios juegos dañados a la vez apuntan a la tarjeta SD o a un juego instalado desde la otra memoria (sysMMC / emuMMC).
- *Vista*, encima de las secciones, muestra los juegos como **lista** o como **portadas** (ícono, nombre y problema; la elección se conserva), como lo hace *Todos los juegos*.

### Forwarders

*Forwarders ›*, una pantalla propia en *Homebrew*: un homebrew en el menú HOME, con su ícono y su nombre, que se inicia desde ahí como un juego (un forwarder de NRO, como los crea sphaira).

- **PlayGuard** con una sola pulsación: su propio ícono y su nombre, y el forwarder inicia el `.nro` desde el que se abrió PlayGuard.
- **Otro homebrew**: cualquier `.nro` de la carpeta `/switch` de la tarjeta SD (y una carpeta más abajo), con su nombre preguntado primero (el del `.nro`, para conservarlo o cambiarlo). Su ícono es el del `.nro`, convertido al JPEG de 256 × 256 que muestra el menú HOME cuando tiene otro tamaño o es un PNG (un control en su lugar cuando el `.nro` no tiene), como hace sphaira. Una ruta con un espacio se pasa al homebrew entre comillas.
- **Los forwarders del menú HOME**, cada uno con su ícono: los que creó PlayGuard (Ⓐ: detalles, cambiar el nombre, cambiar el editor, reinstalar, eliminar) y los de otra app (detalles, eliminar). Uno cuyo `.nro` ya no está lo indica; uno eliminado en la Configuración de la consola sigue en la lista hasta que se reinstala o se olvida. Reinstalar conserva su ID de programa, así que la entrada del menú HOME se reemplaza, no se duplica; un forwarder creado por otra herramienta para el mismo `.nro` también se reemplaza.
- **Íconos e idiomas:** un ícono de otro tamaño, un PNG, o un JPEG con datos EXIF o codificación progresiva (el menú HOME los muestra como un “?”) se convierte en un JPEG de 256 × 256 simple. Los datos de control del forwarder solo declaran el inglés americano, el idioma de su único ícono: así el ícono se muestra en una consola configurada en cualquier idioma.
- **Parches de firma:** la consola solo inicia un forwarder con parches de firma de fs y del loader (sys-patch, o archivos sigpatch): sin ellos, la pantalla lo indica arriba de la lista (*Consola › Parches de juegos* da los detalles). El `.nro` se queda donde está; eliminar un forwarder nunca lo toca.
- **Tiempo de juego:** iniciado desde su ícono, un homebrew funciona como un juego: el temporizador lo cuenta y lo suspende como a cualquier juego cuando se acaba el tiempo, y *Actividad* lo muestra. PlayGuard sigue a la mano desde el menú de homebrew.
- Cada cambio pide el PIN cuando *Proteger PlayGuard con el PIN* lo indica, se rechaza en modo de solo lectura y queda en el historial de cambios. Lo que creó PlayGuard se guarda por sistema (sysMMC / emuMMC), en `forwarders.json`.

### Las líneas de la consola

- **Firmware, Atmosphère y hekate** (cuando su `bootloader/update.bin` está en la tarjeta SD), cada uno con la versión más reciente que encontró la última búsqueda de actualizaciones, en ámbar (*22.0.0 · 23.0.1 disponible*, y cuántas versiones de atraso cuando son más de una), y un ícono de advertencia en la fila del firmware cuando el firmware más reciente necesita primero un Atmosphère o un hekate más reciente, o todavía no es compatible con ellos (las notas de la versión del firmware explican por qué: las notas de versión de Atmosphère y hekate indican qué firmware admiten).
- **Notas de la versión:** Ⓐ en cada uno las abre: una pestaña para la versión instalada, una por cada versión más reciente (cinco como máximo) y versiones más antiguas en las pestañas restantes (seis en total; Ⓛ / Ⓡ pasan de una a otra), que se abre en la siguiente, con etiquetas (instalada o más reciente, el firmware más reciente que admite, correcciones de seguridad, control parental modificado), las notas tal como se publicaron (en inglés), *Ver en línea* (un código QR) y cuándo se obtuvieron. Las de Atmosphère y hekate vienen de sus versiones en GitHub, las del firmware de switchbrew; se guardan en la tarjeta SD para leerlas sin conexión y se vuelven a obtener una vez al día (Ⓧ ahora).
- **La compatibilidad** y luego el **almacenamiento** (emuMMC o sysMMC, con un ícono de información cuando el otro sistema tiene su propio control parental: Ⓐ lo explica).
- Si Atmosphère **oculta el número de serie** (parcialmente oculto hasta Ⓐ; en emuMMC, cuando no lo oculta, un ícono de advertencia en la fila de arriba, que Ⓐ explica).
- **Parches de juegos** (sys-patch o archivos sigpatch, con la recomendación de sys-patch cuando solo se usan archivos: un ícono de información, o uno de advertencia cuando hay algo que revisar, que Ⓐ explica).

## Historial — cambios y respaldos

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=preferences)*

### Historial de cambios

- Los 500 cambios más recientes que hizo PlayGuard (límites, nivel de restricción, PIN, desbloqueos, desvinculación, el reloj, restauraciones, pausar o reanudar la cuenta regresiva …) y lo que vio cambiar fuera de él, agrupados por día (*Hoy*, *Ayer*, y luego el día y la fecha), con la hora y el origen; un límite cambiado para un solo día nombra ese día.
- Un cambio que la consola **rechazó** también aparece, en ámbar, con el error de la consola y lo que se intentó (nunca se restablece: nada cambió).
- *Mostrar* conserva un solo tipo (tiempo de juego, restricciones, seguridad y aplicación, visto fuera de PlayGuard, rechazado por la consola); *Exportar lo que se muestra* lo escribe en `exports/` como CSV, JSON, XLSX o PDF.
- Ⓐ en un cambio lo muestra y, para un valor, **restablece el anterior**, con el mismo desbloqueo y el mismo PIN que cualquier cambio, e indica si cambió desde entonces.
- Cada cambio guarda la versión de PlayGuard que lo hizo; un cambio registrado por una versión más reciente de PlayGuard aparece como *Otro cambio*.

### Respaldos

- **Respaldar / restaurar la configuración** en la tarjeta SD: nivel de restricción, ajustes personalizados, modo RV, sistema de clasificación, límites diarios, la alarma de “Se acabó el tiempo” (con las acciones avanzadas activadas) y, como registro, el bloque sin procesar del temporizador de juego; nunca el PIN.
- Una restauración solo enumera lo que cambiaría (si no hay nada que cambiar, simplemente lo dice, sin desbloquear ni escribir nada), e indica cuando el respaldo tenía una alarma de hora de dormir activada (no se vuelve a escribir).
- Usted elige cuántos respaldos conservar.

## Preferencias

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=preferences)*

- Lo que configura al propio PlayGuard:
  - Idioma (todos los idiomas de la consola, o el de la propia consola), tema (oscuro de forma predeterminada, claro o el de la consola), tamaño del texto (automático / normal / más grande): los tres de inmediato.
  - La pestaña con la que se abre.
  - **La intro** al iniciar: la pantalla de carga durante 2 segundos (activada de forma predeterminada; Ⓐ o Ⓑ la saltan; desactivada, PlayGuard se abre en cuanto está listo).
- Los mensajes que muestra:
  - Los tipos de notificaciones (vea [más arriba](#cómo-se-comportan-las-entradas)).
  - Un **recordatorio mensual para apoyar PlayGuard** (activado de forma predeterminada, nunca durante el primer mes ni justo después de una actualización; *No volver a mostrar* en el recordatorio o este interruptor lo desactivan para siempre, actualizaciones incluidas).
- Una opción que afecta a una sola función está junto a ella: el tiempo extra en *Tiempo de juego*, la reactivación del control parental en *PIN*, la comprobación del reloj en *Consola › Reloj de red*. Las actualizaciones y *Ayuda y diagnóstico* están en *Información*.

## Información

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=about)*

### Versión y actualizaciones

- El PlayGuard instalado: versión (en ámbar cuando hay una más reciente; Ⓐ abre sus **notas de la versión**), modo de inicio y carpeta de datos.
- **Buscar actualizaciones** ahora; *Buscar actualizaciones cada día* al iniciar.
- *Actualizar con*: en PlayGuard, que descarga la versión, la verifica, se reemplaza a sí mismo y se reinicia, o con sphaira, Homebrew App Store o a mano (se pregunta en la primera actualización si no se eligió nada).
- Mientras se conoce una versión más reciente, PlayGuard se abre en **PlayGuard X.Y.Z está disponible** (su fecha y sus notas): *Actualizar*, o *Más tarde* por 1 día, 1 semana o 30 días como máximo.
- Las notas de la versión son las del propio PlayGuard, en la misma pantalla que las de Atmosphère y hekate en *Consola*: a partir de sus versiones en GitHub, una pestaña por versión (la instalada, las más recientes, las más antiguas), cada una con sus puntos destacados como tarjetas y todos sus cambios, guardadas en la tarjeta SD para leerlas sin conexión (las de la versión instalada también están en la aplicación, para cuando todavía no se obtuvo nada).
- Después de una actualización, PlayGuard se abre una vez en **Novedades de la X.Y.Z** (los mismos puntos destacados, y luego los códigos QR).

### Ayuda y diagnóstico

*Ayuda y diagnóstico ›*, una pantalla propia, debajo de *Actualizar con*:

- **Primeros pasos** vuelve a abrir la guía (con un paso *Desvincular la app para celular* mientras está vinculada, un paso *Reactivar la alarma de “Se acabó el tiempo”* mientras está desactivada, un paso *Proteger PlayGuard* una vez definido el PIN —la opción *Proteger PlayGuard con el PIN*—, y un interruptor para que deje de aparecer al iniciar). Debajo de *Cerrar*, *Apoyar PlayGuard* muestra los códigos QR para apoyar el proyecto.
- **Exportar un informe de diagnóstico**, o **enviarlo en línea** (consulte [Reportar un error](troubleshooting.md#reportar-un-error)); cuántos informes guardados, archivos de fallo, registros de sesión y enlaces de informes enviados en línea se conservan, por cantidad, antigüedad o tamaño.

### Créditos

Los créditos, cómo **apoyar PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), mostrados como códigos QR para escanear con un celular) y un pequeño *Made in France* 🇫🇷.

## Seguro por diseño

**Escribir el límite de tiempo de juego sin riesgos.** Sobrescribir la configuración del temporizador mientras hace la cuenta regresiva desestabiliza Atmosphère. Por eso PlayGuard comprueba primero el estado; si el temporizador está activo, la confirmación indica que el control parental se desbloqueará temporalmente, con el PIN guardado: **no necesita recordarlo**. Luego, con una sola pulsación, desbloquea, comprueba que el sistema realmente informe el desbloqueo, escribe y **vuelve a bloquear de inmediato** (o lo ofrece, si *Reactivar el control parental tras un cambio* está desactivado). La capa de servicio vuelve a comprobar el estado justo antes de escribir, así que ninguna pantalla puede saltarse esta protección.

> [!CAUTION]
> Un límite menor que el tiempo ya jugado hoy suspende el juego en cuanto se vuelve a bloquear el control parental. La confirmación lo indica cuando eso va a ocurrir.

**Se acabaron los fallos en 22.5.** `pctl:a`, el servicio privilegiado del control parental, acepta **una sola sesión**. Las compilaciones antiguas de la aplicación original la mantenían abierta, así que la solicitud del PIN del menú HOME (o el applet del PIN) no podía obtenerla y Atmosphère podía fallar. PlayGuard abre la sesión para cada acción y la libera de inmediato; las actualizaciones periódicas se pausan mientras la aplicación está en segundo plano. *(Diagnóstico del [fork de anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Nada se ejecuta en segundo plano.** El límite, el PIN, los avisos y la suspensión son los de la propia consola; PlayGuard solo cambia su configuración.

## Qué cuenta como tiempo de juego

El temporizador de juego es el de la consola: PlayGuard lo lee, no cuenta nada por sí mismo. Lo que se sabe, y con qué certeza ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), en inglés, tiene las mediciones):

| Situación | ¿Cuenta? | Con qué certeza |
|---|---|---|
| Un juego en ejecución | Sí | Medido (22.0.0) |
| **Un juego que se deja abierto en el menú HOME** (suspendido en segundo plano) | **Sí**: ciérrelo, o ponga la consola en modo de suspensión | Reportado por padres y madres ([Arqade](https://gaming.stackexchange.com/questions/398954/)); coincide con las mediciones (la consola cuenta mientras hay una aplicación abierta) |
| Modo de suspensión | No | Reportado ([Arqade](https://gaming.stackexchange.com/questions/398954/)); no medido por PlayGuard |
| **PlayGuard abierto sobre un juego** (como aplicación) | **Sí**, como ese juego | Medido (22.0.0): ábralo con el usuario de un adulto, o desde el Álbum para echar un vistazo rápido |
| PlayGuard abierto desde el Álbum (modo applet) | No | Deducido: no es una aplicación, y el tiempo fuera de las aplicaciones no parecía contar |
| Mientras el control parental está desbloqueado temporalmente | Quizás no | No medido: pregunta abierta |
| Después de cambiar el reloj | El día vuelve a empezar; tras un salto hacia atrás, el conteo se detiene hasta que el reloj pasa la hora anterior | Observado (22.0.0); *Hoy* indica cuando la consola no está contando |

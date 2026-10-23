<!-- translated from docs/features.md, sha256 584095baaee8 -->
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
| **Seguridad** | **Preferencias** |
| ![Seguridad](../../images/screenshots/security.png) | ![Preferencias](../../images/screenshots/preferences.png) |

## Controles

| Botón | Qué hace |
|---|---|
| ↑/↓ | Mover |
| Ⓐ | Confirmar |
| Ⓑ | Volver o cancelar (dos veces en la barra lateral para salir) |
| Ⓧ | Actualizar (en las pestañas que leen la consola) |
| Ⓨ | Notificaciones, desde cualquier pestaña |
| **−** | Idioma y tema, desde cualquier pestaña (en el pie de página, lo primero que se omite cuando los botones propios de una pantalla necesitan el espacio) |
| **+** | Guarda los límites por día (y la semana editada en *Tiempo de juego*) |
| **ZR** | *Deshacer*, durante unos segundos después de guardar un cambio |

La aplicación está organizada en pestañas, como la Configuración de la consola; cada una tiene su sección más abajo.

- **Lo que falló se indica en un cuadro de diálogo; lo que funcionó, en un aviso breve.** Un cuadro de diálogo solo indica lo que se aplica a esta consola en ese momento: que el juego en curso se suspendería, solo cuando hay un juego abierto; que la app para celular podría sobrescribir un cambio, solo cuando está vinculada y puede sincronizarse.
- **El encabezado** indica, en todas las pestañas, que el control parental está desbloqueado temporalmente (su píldora), y que la aplicación está en solo lectura (su píldora dice entonces *Solo lectura*, y las acciones que escribirían aparecen en gris y se rechazan indicando dónde desactivarlo); al tocarla, lleva a donde se deshace (*Hoy › Reactivar*, la pantalla del firmware o *Ayuda y diagnóstico*). El pie de página indica los botones que los abren desde cualquier pestaña: ⊖ para el idioma y el tema, Ⓨ para las notificaciones (con su número).
- **En modo de solo lectura**, las acciones siguen en su lugar, en gris, y explican por qué al presionarlas.

## El aspecto: Cadran

El aspecto de PlayGuard se llama Cadran: una barra de íconos para las pestañas, una paleta azul marino o blanco frío (según el tema), tarjetas en lugar de listas. Cada pestaña se abre en lo más importante (un anillo, mosaicos, tarjetas, chips), con sus demás opciones debajo.

- **La píldora del encabezado** indica de qué trata la pestaña (*Control parental activo*, *Temporizador activo*…); mientras el control parental está desbloqueado, lo indica en todas las pestañas, y al tocarla se abre *Hoy › Reactivar*; mientras está en pausa para usted (vea [más abajo](#el-pin-pone-en-pausa-el-control-parental-mientras-está-en-playguard)), indica *Control parental en pausa*, sin alarmas, y al tocarla explica qué es. El texto secundario nunca baja de 15 px, para poder leerlo desde el sillón en una TV. El reloj del pie de página muestra las horas y los minutos (*16:00*).
- **Los cuadros de diálogo** son tarjetas con los botones en forma de píldora, la confirmación en turquesa (en coral cuando elimina o detiene algo).
  - Las opciones se abren como una tarjeta encima de las indicaciones de los botones, con la actual sobre un turquesa suave con su marca de verificación. El contorno turquesa es solo del foco, en todas partes: un nivel, una cuenta, una opción vigente conserva el fondo suave.
  - Un cuadro de diálogo largo se difumina en su parte inferior mientras hay más, y su advertencia (un juego a punto de suspenderse) va primero, en una tarjeta ámbar.
  - Un fallo lleva como título lo que falló, el código de error en pequeño debajo del motivo, y ofrece *Enviar un informe* cuando ayuda.
  - Los mensajes aparecen como tarjetas debajo del encabezado, con una marca de verificación turquesa para lo que se hizo y una marca ámbar para lo que se rechazó.
- **Las demás pantallas** adoptan el mismo aspecto: el editor por día de un perfil muestra la semana como en *Tiempo de juego* (se edita de la misma forma, con el relleno rápido como mosaicos); *Juegos instalados* y *Todos los juegos* presentan sus líneas en tarjetas; *Perfiles* y *Ayuda y diagnóstico* le dan a cada línea su ícono.
- **Tamaño del texto** (*Preferencias*, o **AA** en el encabezado):
  - *Automático*, la opción predeterminada, agranda todos los textos un 25 % mientras la consola está en la base, en la TV, y los devuelve a su tamaño en modo portátil; *Más grande* los mantiene agrandados, *Normal* nunca.
  - Los nombres de las pestañas de la barra lateral también crecen, un 25 % igualmente; el título, la etiqueta y la fecha del encabezado y las indicaciones de botones y el reloj del pie de página crecen un poco menos (del 15 al 20 %), porque sus filas ya están llenas. Un nombre demasiado largo para su lugar en un idioma se dibuja un poco más chico, nunca más que en tamaño normal, en vez de cortarse. Los nombres de los niveles de Restricciones conservan su tamaño, que ya es grande.
  - Cambia de inmediato, en todas las pantallas, sin reiniciar.
  - El 25 % sale de una TV de 55" en 4K: el texto más pequeño (15 px) mide entonces 14.6 mm de alto en mayúsculas, 20′ de arco a 2.5 m, donde la lectura es cómoda (16′ en tamaño normal, el mínimo indispensable).
- **Efectos visuales** (*Preferencias*): *Siempre completos*, la opción predeterminada, mantiene el halo de fondo y cada movimiento de la interfaz con batería igual que en la base: las pantallas y sus líneas que suben a su lugar, el anillo que se dibuja y se desliza hasta un nuevo valor, el latido del anillo de la pantalla del niño en sus últimos minutos, las pequeñas respuestas a una pulsación. *Más ligeros con batería* omite todos ellos mientras no hay ningún cargador conectado (en modo portátil, desconectada; se vuelve a leer cada 5 segundos, así que al conectar la consola vuelven). *Menos movimiento* omite todo movimiento decorativo, también en la base, y mantiene el halo: para quienes se cansan con el movimiento.
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
- el tiempo de juego de la semana (cada 7 días hasta que se lea; con la comparación por día con la semana anterior), y un límite superado dos o más de los últimos 7 días cuando la consola solo avisa;
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
- **En la barra lateral**, una insignia ámbar junto al nombre de una pestaña cuenta sus entradas pendientes, ahí donde se resuelven: la pestaña Seguridad (sin PIN, un PIN débil, PlayGuard sin su PIN, desbloqueado temporalmente), Tiempo de juego (la alarma desactivada, la consola que no cuenta), Hoy (cambios hechos fuera de PlayGuard, tiempo extra por restablecer), la Consola (sus entradas de arriba, y el reloj de red). Las entradas ignoradas y las informativas no se cuentan.
- Dentro de la pestaña, la misma insignia está en la fila donde se resuelve cada una (*Reloj de red*, *Juegos instalados*, *Almacenamiento*, *Parches de juegos*; *Definir o cambiar el PIN*, *Proteger PlayGuard con el PIN*; *Avanzado* de Tiempo de juego para la alarma), y la pestaña se abre en la primera de ellas. Una fila sin valor propio indica antes, en ámbar, para qué es la insignia (*Fácil de adivinar*, *Impreciso*).
- **Ⓐ** en una entrada aplica la solución (*Activarla*, *Ajustarlo*, *Actualizar*…) o abre la pestaña donde se hace. Una entrada desaparece sola una vez que se resuelve lo que indica.
- **Ⓧ** marca una entrada informativa como leída; **Ⓨ** las marca todas. Una entrada marcada como leída sigue leída en el siguiente inicio, hasta que su texto cambie.
- **Ignorar:** algunas entradas pendientes se pueden ignorar con Ⓧ, tras una confirmación y el PIN: un PIN que cualquiera probaría primero, PlayGuard que se abre sin el PIN, la alarma de “Se acabó el tiempo” desactivada, la tarjeta SD casi llena (nunca la falta de PIN, el control desbloqueado, el reloj, el temporizador de juego que no cuenta ni los archivos propios de PlayGuard). Una entrada ignorada deja de contarse y aparece al final, en *Ignorados*, donde Ⓐ la vuelve a contar. Vuelve sola 30 días después (salvo que *Volver a contar los avisos ignorados después de 30 días* esté desactivado), cuando lo que indica se resuelve y luego vuelve a ocurrir, o, para el PIN, con un PIN nuevo de otra longitud.
- *Preferencias › Notificaciones* desactiva tres tipos: versiones de Atmosphère, hekate y el firmware; el tiempo de juego de la semana; los avisos (copias, segundo sistema de la consola, juegos bloqueados).
- El estado detrás de todo esto se lee al iniciar PlayGuard, cada minuto en la pantalla principal y cada 5 s mientras se muestra *Hoy*.

## Hoy — el día de un vistazo

*[Probarlo en el simulador ›](https://playguard.link/simulator/)*

### El anillo y la línea

**El tiempo restante** en un anillo: un día sin juego indica *Sin jugar*, no *0 min*; el tiempo extra dado hoy aparece como un segmento más claro en su extremo, y el anillo se desplaza hasta cada nuevo valor.

Debajo, lo que detiene el juego primero:

- antes del final del tiempo o de la alarma de hora de dormir, el resto del día como una línea, con el momento actual a su izquierda, y cada uno de ellos como una marca con su hora (*16:40 tiempo agotado*, *21:00 hora de dormir*), en turquesa hasta la primera, en ámbar en sus últimos 15 minutos;
- si no, en palabras: la alarma de hora de dormir una vez agotado el tiempo del día, la hora en que se puede volver a jugar; mientras el control parental está desbloqueado, el límite de hoy en pausa y por qué.

### Las acciones

- **Tiempo extra**, sus cantidades en una fila (+15 min, +30 min, +1 h), cada una a una pulsación, y luego **…** para cualquier otra (una duración, otro día): una cantidad seleccionada desplaza el final del tiempo en la línea (*con 15 min más*), o indica en la marca de la alarma de hora de dormir que esta sigue llegando antes; la primera se destaca en turquesa cuando quedan 15 min o menos, o nada.
- **Terminar por hoy**, **Mostrar al niño**.
- **Límite de hoy…**: el límite de hoy elegido en el momento, escrito tras una confirmación.
- **El perfil vigente**, por su nombre, una vez guardado un perfil (*Perfiles…* cuando ninguno coincide): Ⓐ los enumera, con el vigente marcado, para aplicar otro tras una confirmación (los días que cambian), o *Administrar los perfiles…*.
- Mientras el control parental está desbloqueado, **Reactivar** va primero, a todo el ancho de la fila, y *Tiempo extra* se hace a un lado.
- Si no hay límite ningún día, **Definir los límites de la semana** abre la semana en *Tiempo de juego*.

### Las tarjetas y las píldoras

- La **alarma de hora de dormir** (la hora en que se puede volver a jugar, o en cuánto tiempo suena cuando la consola no lo indica; desactivada, *Desactivada* en grande y Ⓐ *Configurarla*), el **control parental** y el **reloj de red**, como tarjetas.
- El estado de la consola como **píldoras**:
  - una indica si la redirección DNS de Atmosphère bloquea los servidores de Nintendo, y Ⓐ en ella, cuáles y qué cambia eso: la app para celular no se sincroniza y el reloj de red ya no se ajusta solo;
  - otra, al abrirlo como aplicación, que el tiempo que se pasa en PlayGuard cuenta: como ese juego cuando se abre sobre un juego (*Abierto desde un juego*), con el nombre de su ícono cuando se inicia desde un forwarder del menú HOME (*Abierto desde su ícono de HOME*), y Ⓐ explica cómo;
  - un juego que se deja abierto en el menú HOME aparece como una píldora ámbar (*sigue contando*, y Ⓐ explica por qué), cuando la píldora sobre el tiempo del propio PlayGuard no lo indica ya;
  - ninguna de las dos mientras el control parental está en pausa para usted: el tiempo que se pasa aquí entonces no es el del niño.
- Las píldoras muestran primero lo que requiere la intervención del adulto (ámbar, con un triángulo), luego lo que está activado (turquesa, con una marca de verificación) y después el resto (gris, con un punto): la forma lo indica tanto como el color.
- Lo que está bien y lo que solo se informa se agrupan en una sola píldora después de las que requieren la intervención del adulto (*✓ 3 comprobaciones correctas*); Ⓐ sobre ella las muestra, mientras la pestaña siga abierta.
- Debajo de las píldoras, el tiempo extra que falta restablecer (Ⓐ lo restablece) y la alarma de “Se acabó el tiempo” mientras está desactivada (Ⓐ la vuelve a activar).

### Primeros pasos

Hasta que se definan el PIN y los límites de la semana, una tarjeta encima de las demás enumera los tres pasos (el PIN, un límite diario, el reloj de red), cada uno marcado en turquesa una vez hecho o rodeado en ámbar, con cuántos se completaron (*1 de 3*); Ⓐ en uno lo hace ahí mismo.

### ¿La consola está contando?

El tiempo del propio PlayGuard cuenta mientras se ejecuta como aplicación, así que, con un límite hoy, el tiempo restante debería bajar mientras está abierto. Cuando no se movió durante unos 90 s en pantalla (temporizador activado, un límite hoy, sin desbloqueo ni pausa para usted, sin abrir desde el Álbum), una línea ámbar debajo del tiempo jugado indica *La consola no está contando el tiempo de juego en este momento*, con las causas probables:

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
  - después de tiempo extra, solo, sin preguntar ni pedir el PIN (solo baja el límite, como al reactivar el control parental), desbloqueando un momento si el temporizador está contando y volviendo a bloquear de inmediato. Espera mientras no puede escribir (modo de solo lectura, un firmware no compatible, *Bloquear todos los juegos* activado) y, una semana después el mismo día, mientras el límite más bajo detendría el juego en curso (entonces, al día siguiente). Para conservar más tiempo un día, cambie el límite de ese día;
  - después de *No jugar más hoy*, pregunta si mantener 0 o restablecer el límite (con el PIN, como cualquier cambio que sube un límite), o lo restablece como cualquier cambio (*Tiempo de juego › Tras “No jugar más hoy”, restablecer el límite*); restablecer “sin límite” siempre se pregunta.
- **Tiempo extra otro día** (*Tiempo extra › Otro día…*, al final de su lista): mañana o uno de los cinco días siguientes, cada uno con su límite, y luego la cantidad. El límite de ese día se sube de inmediato, solo para esa fecha, y se restablece solo (solo baja el límite) la próxima vez que se abre PlayGuard una vez terminado el día; no en una consola en la que no se abre PlayGuard, donde el mismo día de la semana siguiente también lo conserva. Uno a la vez: la lista ofrece entonces quitarlo (su límite se restablece de inmediato). Si mientras tanto cambia el límite de ese día (la semana, un perfil…), ese cambio prevalece: no se restablece nada.

### La pantalla del niño

*Mostrar al niño* en *Hoy*, para entregar la consola:

- solo el tiempo restante (mientras el control parental está desbloqueado, el límite de hoy, en pausa), cuándo termina el juego (*Termina hacia las 16:40*; cuando la alarma de hora de dormir llega antes, el anillo hace la cuenta regresiva hasta ella, *45 min antes de dormir*, como en la última hora de un día sin límite), la hora de dormir y lo que pasa cuando se acaba el tiempo;
- pensada para leerse desde el otro lado de la habitación, en la TV; su texto conserva su tamaño, sea cual sea el tamaño del texto (solo el título y el pie de página lo siguen, como en todas las pantallas);
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
| Ⓐ o Ⓑ | Termina la edición (Ⓐ indica qué día: *Listo el lunes*; todavía no se guarda nada) |

- En la pantalla táctil, al tocar un día se edita, y luego un toque en su parte de arriba o de abajo lo cambia en 15 min.
- Los cambios se guardan juntos, con **+** o *Guardar*, como en el editor por día (la confirmación enumera los días cambiados, antes → después); *Descartar* los deshace.
- Dentro de la barra de cada día, el tiempo jugado la última vez que tocó ese día (incluido hoy, todas las cuentas). Pasado el límite, la barra está llena y su parte superior en coral, en la misma proporción que el tiempo por encima del límite respecto del tiempo jugado; una leyenda debajo de la semana lo indica (*límite superado*), y el límite también está en coral. Los días posteriores a hoy muestran el juego de la semana pasada, más tenue (seleccionado: *El viernes pasado: 2 h 10 jugado de 2 h*).
- Un día cambiado y todavía sin guardar tiene su límite en ámbar y su barra rodeada de ámbar; debajo de la semana, los datos del día seleccionado (*jueves: 1 h 20 jugado de 2 h*).
- Encima de la semana, en ámbar, un límite alcanzado y lo que hace entonces la consola, o la consola que no cuenta el tiempo de juego.
- `0` minutos significa *no jugar ese día*; *Quitar el límite de tiempo de juego* desactiva el temporizador.

### Chips y ajustes rápidos

**Chips** encima y debajo de la semana abren directamente sus opciones: los perfiles (cuántos hay guardados), el mismo límite todos los días, la alarma de hora de dormir y *Cuando se acabe el tiempo*. *Quitar el límite* es la última línea de la pestaña, aparte (el historial lo deshace).

- **Mismo límite todos los días:** una lista rápida o una duración exacta (*Ingresar una duración…*: ↑/↓ de 15 en 15 min, de hora en hora al mantenerlos presionados, Ⓛ/Ⓡ de 5 en 5 min).
- **Entre semana y fin de semana ›:** el editor, que pide el límite de lunes a viernes y luego el del fin de semana.
- **Tiempo extra o no jugar más hoy ›** abre *Hoy*, donde están los dos. Una nota debajo indica qué hace la consola al llegar al límite. Mientras el control parental está desbloqueado temporalmente, es *posible* que el tiempo de juego no se cuente: todavía es una pregunta abierta ([documentación](../parental-controls.md#still-open), en inglés).
- Cada confirmación enumera los días que cambian, antes → después.

### Perfiles

- **Perfiles** guardados en la tarjeta SD: aplicar (un perfil que ya está vigente simplemente lo indica), editar, cambiar el nombre o eliminar uno; guardar los límites actuales o crear uno nuevo.
- Cualquier nombre, en cualquier escritura (*École*, *周末*, *Выходные* …): un nombre sin letras latinas recibe su propio nombre de archivo, y se detectan dos nombres que darían el mismo archivo.
- **Perfil programado** (*Perfiles › Perfil programado*): un perfil para un período —vacaciones, un viaje—, con su primer y su último día elegidos entre los próximos 60 días.
  - Cuando empieza el período, PlayGuard pregunta si aplicarlo (con los días que cambia dibujados debajo de la pregunta); cuando termina, si restablecer los límites que reemplazó o conservarlos.
  - Se pregunta una vez al día, en la pantalla principal, cuando no hay nada más delante: PlayGuard solo actúa cuando se abre, así que una consola en la que no se abre conserva los límites que tiene. Un período que pasó sin que el perfil llegara a aplicarse se olvida.
  - Uno a la vez; la línea lo cambia, lo quita o lo termina.

### Bloquear todos los juegos

*Bloquear todos los juegos*, debajo de la configuración de la semana: un interruptor que pone en 0 el límite de cada día, así que se necesita un PIN para iniciar un juego: un bloqueo ligero, sin clasificaciones por edad ni límites de comunicación.

- Bloquea el inicio de los juegos, no el menú HOME, y necesita un PIN.
- Los límites anteriores vuelven cuando se desactiva.
- Mientras está activado, *tiempo extra* y *no jugar más hoy* se rechazan, y los límites configurados de otra forma (un perfil, un respaldo, el historial…) lo reemplazan.

### Alarma de hora de dormir

- La hora de la alarma (de 16:00 a 23:45, o *Desactivado*) y la hora en que se puede volver a jugar (de 05:00 a 09:00), la misma todos los días, configuradas con la palanca: arriba y abajo un cuarto de hora, L y R una hora.
- O, con Ⓨ *Por día* en el cuadro de diálogo de la alarma, de lunes a viernes, el fin de semana o un solo día, y los demás días conservan la suya (cada grupo de días aparece con su hora). Cuando los días difieren, la pestaña indica *Varía según el día*, y la semana en *Tiempo de juego* muestra la alarma de cada día debajo de su nombre.
- Su lugar en la configuración del temporizador de juego, deducido de la configuración de la app para celular, está verificado en una consola (22.0.0); aun así, PlayGuard solo la cambia una vez que la consola informa lo que PlayGuard lee ahí, comprueba la respuesta de la consola después del cambio y restablece la configuración anterior si no coincide.

### Ajustes del tiempo extra y Avanzado

- **Tiempo extra:** las cantidades que se ofrecen (+15/+30/+1 h, +10/+20/+30 min …) y restablecer el límite habitual sin preguntar después de *No jugar más hoy* (el tiempo extra siempre se restablece sin preguntar). Ambas cosas se hacen la próxima vez que se abre PlayGuard, a partir del día siguiente: una consola en la que no se abre conserva el límite de ese día.
- **Avanzado ›**, una pantalla propia: el interruptor de las acciones de depuración y, mientras está activado, la alarma de “Se acabó el tiempo” activada/desactivada y pausar / reanudar la cuenta regresiva. El mismo interruptor permite que un respaldo o el historial restablezcan la alarma.

## Actividad — quién jugó a qué, y cuánto tiempo

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=activity)*

Tiempo por juego **hoy**, en los **últimos 7 días** y **en total**, a partir del propio registro de actividad de la consola, para todas las cuentas o para **una cuenta de usuario** (las cuentas como chips arriba, cuando la consola tiene varias). PlayGuard se abre en la última cuenta que se mostró, o en la elegida en *Cuenta que se muestra primero* (todas las cuentas, o una; al final de la pestaña, con *Todos los juegos*).

### Arriba

- Las cuentas como chips, el período y *Exportar*.
- Hoy, los últimos 7 días y el total como cifras. Debajo de los últimos 7 días, la comparación con los 7 anteriores, por día: ▲ *+20 min/día vs. semana anterior* en ámbar, ▼ menos en turquesa, *como la semana anterior* con un margen de 5 minutos.
- Los cuatro juegos más jugados como tarjetas (Ⓐ abre uno); en los últimos 7 días, el que más aumentó respecto de la semana anterior indica cuánto, a partir de media hora (▲ *+2 h vs. semana anterior*).
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

### Exportar

- **Exportar a la tarjeta SD** (*Exportar*) en CSV, JSON, XLSX (Excel) o PDF, una columna por día, y luego los últimos 7 días, los últimos 30 días y el total.
- Se abre de inmediato con los últimos datos leídos (guardados en la tarjeta SD entre una sesión y otra), que se actualizan en segundo plano cuando tienen más de un minuto; Ⓧ los vuelve a leer ahora, con un indicador de carga.
- Los tiempos son aproximados si se cambió el reloj de la consola.

## Restricciones — nivel, clasificación por edad, comunicación

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=restrictions)*

- **El nivel de restricción:** Niños pequeños, Niños, Adolescentes y Personalizado, uno al lado del otro como mosaicos, cada uno con lo que restringe (un candado para lo restringido, *Permitido* escrito en los demás casos, y los juegos instalados que bloquea; Ⓐ lo aplica; en *Personalizado* ya vigente, Ⓐ lleva a sus opciones de abajo); *Ninguno* debajo. El nivel vigente indica *✓ Actual*; el contorno turquesa solo está donde está el foco. Lo que restringe un nivel (límite de edad, publicaciones, comunicación) se vuelve a indicar antes de aplicarlo; *Ninguno* lo pregunta en coral, ya que quita todas las restricciones.
- **Las opciones del nivel:** clasificación por edad, publicaciones en redes sociales, comunicación con otros usuarios. Con un nivel predefinido, son las suyas: cambiar una convierte el nivel en *Personalizado*, a partir de las opciones del nivel predefinido con ese cambio, después de preguntar (con las opciones enumeradas).
- **En *Clasificación por edad*,** el **sistema de clasificación** (PEGI, ESRB, USK, CERO …): la lista indica los países para los que clasifica cada uno (PEGI: la mayor parte de Europa, ESRB: Estados Unidos, Canadá y Latinoamérica, USK: Alemania, PEGI BBFC: Reino Unido…).
- **Juegos instalados que bloquea** (también en *Clasificación por edad*): cuántos de los juegos instalados bloquea el nivel vigente por su clasificación por edad (*3 de 15*), y Ⓐ los enumera con su clasificación (*PEGI 16*); el mosaico de cada nivel también lo indica (*Juegos bloqueados · 3*).
  - Las clasificaciones son las de cada juego (sus datos de control, leídos por la comprobación de los juegos instalados al iniciar o desde *Consola › Juegos instalados*), según el sistema de clasificación que usa la consola.
  - Un juego que ese sistema no clasifica no se cuenta, ni tampoco las excepciones configuradas en la app para celular.
  - La pantalla de un juego en Actividad muestra su clasificación, y *bloqueado en el nivel actual* cuando lo está.
- **En *Otros*,** el modo RV. Los juegos autorizados a comunicarse están en el informe de diagnóstico, sin procesar, hasta que se entienda su lista.

## Seguridad — el PIN, el desbloqueo, el menú homebrew, la app para celular

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=security)*

- **Arriba**, cuatro mosaicos: el PIN, las restricciones, el desbloqueo temporal y el menú homebrew, cada uno con una marca de verificación turquesa cuando está como debe (un PIN definido, las restricciones activas, bloqueado) o una señal de advertencia ámbar cuando hay que intervenir; el menú homebrew indica *Abierto* o *Bloqueado* (una marca de verificación una vez bloqueado: abierto está bien sin forwarders).
- **Definir o cambiar el PIN** (pantalla del PIN del sistema).
- **Mostrar el PIN**: tras una advertencia y el propio PIN, que se pide cada vez; queda registrado en el historial y luego se ofrece un PIN nuevo.
- **Desbloquear temporalmente**, y si PlayGuard lo reactiva solo justo después de un cambio que necesitó el desbloqueo (activado de forma predeterminada).

### Proteger PlayGuard con el PIN

El PIN en el propio PlayGuard:

| Opción | Efecto |
|---|---|
| *Nunca* | Sin PIN en PlayGuard (la campana lo señala mientras la consola tiene uno) |
| *Antes de un cambio* (predeterminado) | Cualquiera puede mirar, solo el adulto cambia algo |
| *Para abrir PlayGuard* | Una pantalla de bloqueo al iniciar |

- Una vez ingresado, el PIN vale hasta que se cierra PlayGuard; *Volver a pedir el PIN tras 5 minutos* (desactivado de forma predeterminada) lo vuelve a pedir pasado ese tiempo, y vuelve a mostrar la pantalla de bloqueo de *Para abrir PlayGuard* cuando PlayGuard regresa tras 5 min o más en segundo plano.
- Se comprueba en la capa de servicio, así que ningún cambio lo omite; volver a bloquear nunca lo pide. *Mostrar el PIN* lo pide cada vez, en todos los modos e incluso justo después de ingresarlo, y un `config.json` ausente o dañado cuenta como *Antes de un cambio*.
#### El PIN pone en pausa el control parental mientras está en PlayGuard

- Una vez ingresado el PIN (la pantalla de bloqueo de *Para abrir PlayGuard*, el primer cambio con *Antes de un cambio*, el regreso desde la pantalla del niño…), PlayGuard desbloquea temporalmente el control parental, para que el tiempo que se pasa en él no se descuente del tiempo de juego del niño; *Abierto desde un juego* desaparece de Hoy.
- La píldora del encabezado dice *Control parental en pausa*, en turquesa; al tocarla explica qué es, con *Reactivar ahora* (y entonces no vuelve a pausarse hasta que se vuelva a ingresar el PIN). Hoy muestra el día tal como es para el niño, y el mosaico de Seguridad dice *Control parental en pausa*, sin advertencia.
- Se reactiva solo en cuanto se sale de PlayGuard (menú HOME, modo de suspensión) o se cierra, también desde el menú HOME (la salida lo espera), y mientras está abierta la pantalla del niño o la pantalla de bloqueo. De regreso en PlayGuard con el PIN todavía válido, vuelve a pausarse.
- No desde el Álbum (ahí el tiempo no cuenta, y puede quedar un juego abierto detrás), no en modo de solo lectura, y nunca sobre un desbloqueo hecho con *Desbloquear temporalmente*, que queda como está.
- Se registra como cualquier desbloqueo: si PlayGuard se detuviera sin volver a bloquear, su siguiente inicio vuelve a bloquear, y un reinicio de la consola lo hace por sí solo. Se basa en que el tiempo no cuenta mientras está desbloqueado, algo deducido pero aún no medido ([más abajo](#qué-cuenta-como-tiempo-de-juego)).
- **Bloquear el menú homebrew ›** abre *Consola › Forwarders*, donde se bloquea tras sus verificaciones (vea [Forwarders](#forwarders)).
- **Salir** mientras el control parental sigue desbloqueado pregunta si volver a bloquearlo (*Reactivar y salir* también cierra el juego sobre el que se abrió PlayGuard, y vuelve al menú HOME en lugar de a hbmenu); si en cambio se cierra desde el menú HOME, PlayGuard lo vuelve a bloquear la próxima vez que se inicia, y un reinicio de la consola lo bloquea por sí solo. La pausa propia de PlayGuard no pregunta nada: termina al cerrarse PlayGuard.

### App para celular

- Si la app Control parental de Nintendo Switch está vinculada, su última sincronización, y **desvincular** (si no, su próxima sincronización sobrescribe los límites configurados aquí: mientras está vinculada, cada cambio que PlayGuard pide confirmar lo indica).
- Cuando la redirección DNS de Atmosphère (`atmosphere/hosts`) desvía a otro lugar el servidor del control parental de Nintendo, la app no puede sincronizarse: PlayGuard indica entonces aquí *sincronización bloqueada (DNS)* y no menciona la app en ningún otro lugar (ninguna nota en los cuadros de diálogo, ninguna entrada en la campana, ningún paso en los primeros pasos, ninguna píldora en *Hoy*, ninguna ayuda para descifrar, y la desvinculación se confirma sin decir qué sobrescribiría la app).
- Mientras está vinculada y puede sincronizarse, *Antes de desvincular: ayude a descifrar…* abre la comparación del bloque (consulte [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), en inglés).

### Eliminar todo el control parental

Dos confirmaciones, irreversible; antes se guarda un respaldo de la configuración.

## Historial — lo que cambió PlayGuard, y los respaldos

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=history)*

**Arriba**, como mosaicos, cuántos cambios hay registrados y el día del último respaldo: una marca de verificación turquesa, o una señal de advertencia ámbar cuando no hay ninguno o ninguno en 30 días.

### Historial de cambios

- Los 500 cambios más recientes que hizo PlayGuard (límites, nivel de restricción, PIN, desbloqueos, desvinculación, el reloj, restauraciones, pausar o reanudar la cuenta regresiva …) y lo que vio cambiar fuera de él, agrupados por día (*Hoy*, *Ayer*, y luego el día y la fecha), con la hora y el origen; un límite cambiado para un solo día nombra ese día.
- Un cambio que la consola **rechazó** también aparece, en ámbar, con el error de la consola y lo que se intentó (nunca se restablece: nada cambió).
- *Mostrar* conserva un solo tipo (tiempo de juego, restricciones, seguridad y aplicación, visto fuera de PlayGuard, rechazado por la consola); *Exportar lo que se muestra* lo escribe en `exports/` como CSV, JSON, XLSX o PDF.
- Ⓐ en un cambio lo muestra y, para un valor, **restablece el anterior**, con el mismo desbloqueo y el mismo PIN que cualquier cambio, e indica si cambió desde entonces.
- Cada cambio guarda la versión de PlayGuard que lo hizo (una compilación de desarrollo: se indica, con su commit y, para la compilación de una pull request, su número); un cambio registrado por una versión más reciente de PlayGuard aparece como *Otro cambio*.

### Respaldos

- **Respaldar / restaurar la configuración** en la tarjeta SD: nivel de restricción, ajustes personalizados, modo RV, sistema de clasificación, límites diarios, la alarma de “Se acabó el tiempo” (con las acciones avanzadas activadas) y, como registro, el bloque sin procesar del temporizador de juego; nunca el PIN.
- Una restauración solo enumera lo que cambiaría (si no hay nada que cambiar, simplemente lo dice, sin desbloquear ni escribir nada), e indica cuando el respaldo tenía una alarma de hora de dormir activada (no se vuelve a escribir).
- Usted elige cuántos respaldos conservar.

## Consola — el reloj de red, los juegos instalados, lo que ejecuta la consola

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=console)*

**Arriba**, lo que ejecuta la consola como mosaicos, en *Sistema* (el firmware, Atmosphère, hekate cuando está en la tarjeta SD y sys-patch cuando está instalado; Ⓐ abre sus notas de la versión), luego las herramientas como mosaicos, en *Herramientas* (*Reloj de red*, *Juegos instalados* y *Forwarders*; las dos primeras muestran lo que requiere atención en su esquina) y después las líneas de la consola, empezando por la compatibilidad (solo una marca de verificación turquesa cuando PlayGuard se comprobó en este firmware; si no, una advertencia y lo que falla).

### Reloj de red

*Reloj de red ›*, una pantalla propia: el reloj del que depende el temporizador de juego. Una consola que nunca llega a los servidores de Nintendo mantiene este reloj impreciso, lo que altera el temporizador de juego.

- **Relojes de la consola y de red**, zona horaria, precisión. Cuando la redirección DNS de Atmosphère (`atmosphere/hosts`) desvía a otro lugar el servidor de hora de Nintendo (`aauth-lp1.ndas.srv.nintendo.net`), la consola ya no puede ajustar su reloj de red por sí misma: esta pantalla, la entrada de la campana y la tarjeta de *Hoy* lo indican y remiten a la medición de abajo.
- **Elegir un servidor NTP público** (≈ 50 incluidos, por región, o uno propio).
- **Medir** con 3 servidores (mediana; un aviso ámbar cuando no coinciden): un cuadro de diálogo muestra el resultado (cuánto está adelantado o atrasado el reloj de red, en grande, la hora de los servidores junto a la del reloj de red, la respuesta de cada servidor) y ajusta el reloj de red desde ahí. Una medición sigue siendo válida durante 2 minutos, con una cuenta regresiva debajo de *Ajustar el reloj de red*. La confirmación lo indica: cambiar el reloj pone a cero el tiempo de juego de hoy, así que vuelve a estar disponible todo el límite.
- **Una comprobación del reloj de red al iniciar**: un aviso breve cuando tiene más de un minuto de diferencia; nunca ajusta el reloj.

#### Ningún servidor responde

En una consola sin Internet, *Usar el reloj de la consola* se ofrece en el cuadro de diálogo del resultado, y *Ajustar el reloj de red al reloj de la consola* debajo de la medición (y en la tarjeta del reloj de red de *Hoy*, cuando falla su medición), detrás de la misma confirmación.

- PlayGuard no tiene nada más con qué comparar el reloj de la consola: la confirmación muestra su hora y le pide que primero la compare con un celular o un reloj, y se rechaza un reloj de consola anterior a 2026 (nunca ajustado).
- Con *Sincronizar el reloj por Internet* desactivado, el reloj de la consola se queda donde está, y activar esa opción después ya no lo hace saltar; con la opción activada, el reloj de la consola ya sigue al reloj de red, así que la confirmación indica que la desactive y ajuste la hora a mano si es incorrecta.
- Solo escribe el reloj, no el control parental, así que no necesita desbloqueo (el PIN de *Proteger PlayGuard con el PIN* sigue aplicándose); como cualquier cambio de reloj, pone a cero el tiempo de juego de hoy.
- Todavía no se probó en una consola.

### Juegos instalados

*Juegos instalados* (un mosaico en *Consola › Herramientas*) encuentra los juegos que el menú HOME no puede iniciar ni mostrar —un ícono que no termina de cargar, un “!”, un marco punteado— con lo que falla y qué hacer, debajo del ícono del juego, y cuántos juegos instalados tienen un problema.

**Lo que encuentra:** nombre e ícono ilegibles, el juego en sí ausente detrás de su actualización, nada instalado ya, archivos faltantes, datos dañados que encontró la consola, un firmware más reciente que exige el juego o solo su actualización, claves demasiado nuevas para el firmware, una licencia (ticket) faltante, la consola que pide una actualización que se eliminó, la tarjeta SD ilegible, un inicio que la consola rechaza.

**La lista tiene una sección por cada salida**, cada una con su cantidad y qué hacer, con los juegos que tienen el mismo problema juntos (dos juegos con el mismo nombre se distinguen por su versión o su Title ID):

| Sección | Qué contiene |
|---|---|
| **Reparables aquí** | Lo que PlayGuard repara |
| **Por comprobar** | Datos de los que la consola sospecha: la comprobación indica si hay que reinstalar el juego |
| **Reinstalar** | Archivos que ya no están o no se pueden usar, o datos que resultaron dañados: eliminar y luego volver a instalar |
| **Firmware o tarjeta SD** | El juego está bien, la consola no |

- **Comprobar los datos:** lo que encontró la comprobación de datos se guarda con el juego hasta que se instale otra versión: si están dañados, pasa a *Reinstalar* como *Dañado (comprobado)*, con la fecha y el código de la consola; si están intactos, sale de la lista. *Comprobar estos juegos* comprueba toda la sección *Por comprobar*, un juego tras otro en un solo cuadro de diálogo, con sus portadas arriba (*juego 2 de 5*), y luego indica cuántos están dañados, intactos o no se pudieron comprobar, con la opción de eliminar los dañados al alcance de la mano.
- **Varios juegos a la vez:** Ⓨ selecciona juegos (una marca de verificación delante); desde el primero seleccionado, una línea encima de las secciones indica cuántos, y A en ella también selecciona una sección entera; luego, una sola acción para todos: comprobar sus datos o eliminarlos (antes se guarda su lista, con una sola pregunta y un solo PIN).
- **Eliminar varios:** las portadas se alejan flotando a medida que se elimina cada juego (un “!” coral en el que queda), la barra, un segmento por juego (hasta 20); al final, cuántos se eliminaron y cuánto espacio se liberó, y el cuadro de diálogo se cierra solo. Si quedan algunos: sus portadas con un borde coral, y *Reintentar* para ellos.
- **En *Reinstalar*,** se puede guardar su lista (JSON y texto, en la carpeta de exportaciones: nombre, Title ID, versión, problema), y eliminarlos todos juntos tras una sola pregunta y un solo PIN: primero se guarda la lista y luego se eliminan los juegos, para que un instalador ya no los cuente como instalados.
- **Las acciones:** cuando una acción es la solución, es el primer botón del cuadro de diálogo; las demás vienen después: volver a aceptar la versión instalada (cuando la consola pide una actualización que se eliminó), eliminar solo la actualización o un contenido adicional, comprobar si los datos están dañados (la comprobación propia de la consola, con su progreso y Cancelar) o eliminar el juego.
- Las versiones se muestran como las muestra el juego (*versión 1.4.0*, *actualización n.º 4*); después de una reparación, un aviso breve indica si el juego vuelve a funcionar. Los datos de guardado siempre se conservan; cada cambio queda en el historial de cambios.
- **Cuándo comprueba:** la última comprobación se muestra de inmediato; luego solo se vuelven a comprobar los juegos cuya entrada cambió (todos los juegos una vez al día, después de una actualización del firmware, o con Ⓧ *Comprobar de nuevo*), cada uno en cuanto se comprueba. Ⓧ *Comprobar de nuevo* abre un cuadro de diálogo: las portadas de los juegos pasando una tras otra, una barra y el conteo; *Cancelar* se detiene después del juego en curso, y lo ya comprobado se conserva. Si todavía no hay ningún juego en la lista, las portadas van pasando en la tarjeta mientras comprueba.
- Varios juegos dañados a la vez apuntan a la tarjeta SD o a un juego instalado desde la otra memoria (sysMMC / emuMMC).
- Ⓡ muestra los juegos como **lista** o como **portadas** (ícono, nombre y problema; la elección se conserva), como lo hace *Todos los juegos*.

### Forwarders

*Forwarders ›*, una pantalla propia, un mosaico en *Herramientas* (también *Seguridad › Bloquear el menú homebrew*): un homebrew en el menú HOME, con su ícono y su nombre, que se inicia desde ahí como un juego (un forwarder de NRO, como los crea sphaira).

- **Una tarjeta arriba** indica cómo está el menú homebrew: bloqueado o abierto, sus caminos de vuelta (el forwarder de PlayGuard, los menús protegidos, los que todavía no se iniciaron desde el menú HOME), lo que todavía lo abre sin el PIN y si el sysmodule de recuperación está instalado. Ⓐ en ella enumera qué hacer, cada cosa a una pulsación (el siguiente paso hacia el bloqueo, proteger o eliminar un forwarder que lo abre sin el PIN, la comprobación, la hoja de rescate).
- **La lista por grupos:** el de PlayGuard, los demás que creó (protegidos o no, en una sola tarjeta: el ícono indica cuál), los creados por otra app, los que ya no están en el menú HOME. Uno protegido tiene un candado en su ícono (una marca de verificación para un menú que se vio funcionar a través de su puerta de acceso), y uno creado aquí sin el PIN, un candado abierto en ámbar.
- **Ⓨ** abre esta parte de la guía (un código QR). Después de eliminar u olvidar un forwarder, o de cambiar su horario sin el PIN, *Pedir el PIN* o *Cerrado al agotarse el tiempo de juego de hoy*, **ZR** lo restablece durante unos segundos (reinstalado con el mismo ID de programa, o el ajuste como estaba).
- **PlayGuard** con una sola pulsación: su propio ícono y su nombre, y el forwarder inicia el `.nro` desde el que se abrió PlayGuard.
- **Otro homebrew**: cualquier `.nro` de la carpeta `/switch` de la tarjeta SD (y una carpeta más abajo), con su nombre preguntado primero (el del `.nro`, para conservarlo o cambiarlo), y luego se confirma con lo que mostrará el menú HOME: su ícono, su nombre y su editor. Su ícono es el del `.nro`, convertido al JPEG de 256 × 256 que muestra el menú HOME cuando tiene otro tamaño o es un PNG (un control en su lugar cuando el `.nro` no tiene), como hace sphaira. Una ruta con un espacio se pasa al homebrew entre comillas.
- **Los forwarders del menú HOME**, cada uno con su ícono: los que creó PlayGuard (Ⓐ: detalles, cambiar el nombre, cambiar el editor, reinstalar, eliminar) y los de otra app (detalles, eliminar). Ⓐ abre una hoja por secciones (*Este forwarder*, *Al iniciar*, *Protección*, *Instalar y eliminar*) bajo su ícono y su estado, cada línea con su ícono y lo que está configurado ahora (su horario; un interruptor para *Pedir el PIN*, *Cerrado al agotarse el tiempo de juego de hoy* y cada opción de inicio), *Eliminar* en rojo; B o un toque fuera de ella la cierra. *Detalles* enumera sus reglas de la misma forma, y luego su editor, su versión, su `.nro` y su ID de programa; Ⓐ en una regla (o en el editor) la cambia, como en la hoja. Debajo de los protegidos, una línea dice qué significa cada ícono que aparece ahí. Uno cuyo `.nro` ya no está lo indica; uno eliminado en la Configuración de la consola sigue en la lista hasta que se reinstala o se olvida. Reinstalar conserva su ID de programa, así que la entrada del menú HOME se reemplaza, no se duplica; un forwarder creado por otra herramienta para el mismo `.nro` también se reemplaza.
- **Protegido con el PIN** (Ⓐ › *Proteger con el PIN*, para uno creado por PlayGuard que no sea el suyo): iniciado desde el menú HOME, el forwarder abre primero una pequeña puerta de acceso (`config/playguard/gate.nro`, unos cientos de kilobytes que PlayGuard escribe a partir de sus propios archivos), que muestra de inmediato la pantalla del PIN de la consola, y luego el homebrew una vez ingresado el PIN correcto; B ahí regresa al menú HOME. Alrededor de la pantalla del PIN, la de la puerta: el ícono y el nombre del homebrew, *Protected by PlayGuard* y cómo resultó (iniciando, en su horario libre, PIN no ingresado), en el idioma de la consola, sobre el azul noche de Cadran (`make gate-preview` la dibuja en una computadora). Conserva su ID de programa y su ícono, y queda en el historial de cambios. Su fila muestra sus reglas como íconos, uno por cada una activada: una llave para el PIN, un candado abierto para el horario sin él, un cuadrado (stop) para el horario en que nunca se abre, un temporizador para cerrado al agotarse el tiempo de juego de hoy; *Detalles* (Ⓐ) las da en palabras. Necesita un PIN en la consola. *Dejar de pedir el PIN* lo pide primero, diga lo que diga *Proteger PlayGuard con el PIN*, igual que eliminarlo o volver a crearlo sin el PIN (*Agregar un forwarder* para el mismo `.nro`). Solo protege el ícono del menú HOME: el `.nro` todavía se inicia desde el menú homebrew, salvo que esté bloqueado (más abajo). Un forwarder protegido por un PlayGuard anterior inicia el propio PlayGuard como puerta, lo que sigue funcionando; *Reinstalar todos* lo pasa a la pequeña puerta.
  - **Horario sin el PIN** (Ⓐ en uno protegido): todos los días, de lunes a viernes o el fin de semana, de una hora a otra (también pasada la medianoche), o *De lunes a viernes, y el fin de semana aparte* (una franja para cada uno); *Detalles* indica cuándo. Configurarlo pide el PIN. Sigue el reloj de la consola, que la Configuración de la consola puede cambiar: en cuanto se aleja más de 5 minutos del reloj de red, la puerta pide el PIN de todos modos en ese horario (*Sin horario libre: el reloj de la consola no coincide con el de la red*), y lo mismo cuando no se puede leer el reloj de red para verificarlo (*Sin horario libre: reloj de red no disponible*); *Consola › Reloj de red* pone los dos en hora, también en una consola sin Internet.
  - **Horario en que nunca se abre** (Ⓐ en uno protegido), configurado de la misma forma: entonces la puerta dice *No a esta hora* y el horario, y regresa al menú HOME, incluso con el PIN ingresado (la hora de dormir para los homebrew). Tiene prioridad sobre el horario sin el PIN. Con el reloj de la consola a más de 5 minutos del reloj de red, sigue cerrado cuando cualquiera de los dos relojes está en ese horario, y una pausa tras los rechazos se cuenta con el reloj de red y con el reloj continuo de la consola (que ningún ajuste mueve), así que cambiar la hora no lo evita.
  - **Cerrado al agotarse el tiempo de juego de hoy** (Ⓐ en uno protegido): cuando al temporizador de juego de la consola no le queda tiempo hoy, la puerta dice *El tiempo de juego de hoy se agotó* y regresa al menú HOME, con o sin el PIN. Desactivado salvo que se configure.
  - **Las reglas están en el forwarder, no en la tarjeta SD:** el horario, el límite del día y la pausa tras los rechazos van en los propios datos del forwarder (en la memoria de la consola), así que una computadora no puede agregar horario libre editando un archivo; cambiar uno rehace el forwarder, en unos segundos. El forwarder también guarda el SHA-256 de la puerta (`config/playguard/gate-<hash>.nro`): una puerta modificada en la tarjeta SD no se inicia (error 2347-0100, [solución de problemas](troubleshooting.md#un-forwarder-protegido-muestra-el-error-2347-0100)). Uno protegido por un PlayGuard anterior lee sus reglas de la tarjeta: entonces su horario sin el PIN no se respeta, y la campana ofrece *Rehacer*.
  - **Una pausa tras PIN rechazados** (*Pausa tras PIN rechazados*, en *Menú homebrew*): cinco rechazos seguidos (B en la pantalla del PIN) y el forwarder deja de pedirlo durante 15 minutos, y la puerta indica hasta cuándo; 3, 5 o 10 intentos y 15 minutos o una hora, o nunca. La campana indica qué forwarder está en pausa, y su fila también (un ícono de pausa, y hasta cuándo); *Quitar la pausa* (Ⓐ en él, con el PIN) hace que lo vuelva a pedir de inmediato. Cada pausa y cada pausa quitada quedan en el historial de cambios.
  - **Cada intento queda en el historial de cambios**: abierto con el PIN, abierto en su horario o rechazado (B en la pantalla del PIN), a la hora en que ocurrió; los intentos de un mismo tipo, un mismo día, para un mismo forwarder forman una sola entrada, con sus horas. La puerta los anota en la tarjeta SD; PlayGuard los lee la siguiente vez que se inicia. *Historial › Mostrar › Homebrew y forwarders* muestra solo estos, los forwarders y el bloqueo.
  - **Intentos de la semana** (en *Historial* y en *Menú homebrew*): de los últimos siete días, las aperturas de cada forwarder, sus rechazos y en qué días, y las veces que no se abrió; se conservan cuatro semanas. Cuando los rechazos de esta semana superan el doble de los de la semana anterior (cinco como mínimo), la campana lo indica.
  - **La pantalla de la puerta dice más:** su horario sin el PIN mientras lo pide, y los rechazos del día hasta el momento (en ámbar) mientras lo pide y una vez rechazado, debajo de *Pide a un adulto que ingrese el PIN*. Después de *Protected by PlayGuard*, un ícono por cada regla que lleva, como su línea en PlayGuard: un candado abierto (horario sin el PIN), un cuadrado (horario en el que nunca se abre), un temporizador (el tiempo de juego de hoy). Una vez rechazado, Ⓐ lo vuelve a intentar y Ⓑ regresa al menú HOME (lo mismo que esperar 15 segundos), hasta que empieza la pausa después de los rechazos. También indica hasta cuándo se abre sin el PIN, cuándo termina una pausa (*en 12 min*), el tiempo de juego que queda hoy (con *Cerrado al agotarse el tiempo de juego de hoy*) y el código de error cuando la pantalla del PIN no puede abrirse. El marco del ícono toma el color del resultado. Si la puerta llegara a detenerse camino al homebrew, termina a los 10 segundos y regresa al menú HOME, en lugar de dejar la consola en su pantalla.
  - **La pantalla de apertura** (*Pantalla de apertura*, en *Menú homebrew*): iniciada desde el menú HOME, la puerta abre primero el homebrew igual que PlayGuard se abre a sí mismo: centrado sobre el azul noche con la esfera de Cadran a la derecha, el ícono del homebrew sobre un suave halo turquesa, su nombre, *Protected by PlayGuard*, una barra que se llena y, abajo, la marca de PlayGuard con su anillo girando. Luego la pantalla del PIN de la consola, y el homebrew en cuanto el PIN es correcto (en su horario libre, justo después de la barra). Dura 2 segundos salvo que se configure en 1, 3 o 5 segundos, o en *Desactivada* (la pantalla del PIN de inmediato); Ⓐ o Ⓑ saltan el resto, y volver a intentarlo tras un rechazo no la muestra otra vez. Se lee de la tarjeta SD en cada inicio (`config/playguard/gate_wait.txt`), así que cambiarla no rehace los forwarders.
- **Su ícono desde el álbum** (Ⓐ › *Cambiar el icono*): las 30 capturas más recientes del álbum en una cuadrícula, cada una como el ícono que daría, primero el del `.nro` y marcado el que está en uso; al lado, el ícono que muestra ahora el menú HOME y el que mostrará, el juego en el que se tomó la captura y cuándo (*Hoy*, *Ayer*, al segundo cuando dos se tomaron en el mismo minuto). L y R mueven el cuadrado del ícono a lo largo de la captura (cinco posiciones, de su borde izquierdo al derecho), Y vuelve al del `.nro`, A elige el seleccionado: se convierte en el JPEG de 256 × 256 del menú HOME. Volver a instalarlo lo conserva, con su encuadre.
- **Íconos e idiomas:** un ícono de otro tamaño, un PNG, o un JPEG con datos EXIF o codificación progresiva (el menú HOME los muestra como un “?”) se convierte en un JPEG de 256 × 256 simple. Los datos de control del forwarder solo declaran el inglés americano, el idioma de su único ícono: así el ícono se muestra en una consola configurada en cualquier idioma.
- **Al iniciar** (Ⓐ en un forwarder, como las opciones de forwarder de sphaira): *Elegir un usuario al iniciar* (desactivado de forma predeterminada) pregunta qué usuario juega cuando se inicia, como lo hace un juego, para que su tiempo de juego sea el de ese usuario en *Actividad* (el propio perfil de un adulto deja las estadísticas del niño como están); el límite de tiempo de juego de la consola lo sigue contando, sin importar quién juegue, como cuenta cualquier juego. *Capturas de pantalla* y *Captura de video* (activados de forma predeterminada) permiten que el botón de captura las tome. Cada interruptor pide el PIN y vuelve a crear el forwarder (con el mismo ID de programa), y queda en el historial de cambios; *Detalles* los enumera.
- **Proteger todos los forwarders con el PIN** y **Reinstalar todos** (el ícono y la versión de un `.nro` nuevo, la puerta tal como la trae este PlayGuard, uno eliminado del menú HOME restablecido): con una pulsación cada uno.
- **Bloquear el menú homebrew** (*Menú homebrew*, al final de la pantalla): ni el álbum ni un juego iniciado manteniendo R vuelven a abrir el menú homebrew, así que un homebrew solo se inicia desde su forwarder, que puede pedir el PIN.
  - **Paso a paso:** mientras no se pueda bloquear, Ⓐ enumera los pasos, numerados, el siguiente en ámbar y a una pulsación (*Siguiente paso*): un PIN, los parches de firma, el forwarder de PlayGuard, un menú homebrew protegido con el PIN (sphaira si está en `/switch`, si no hbmenu, creado ya protegido de una vez), iniciado una vez desde el menú HOME; la lista vuelve después de cada paso que crea un forwarder.
  - Requiere dos caminos de vuelta a PlayGuard desde el menú HOME: su propio forwarder, y un forwarder de un menú homebrew (sphaira o hbmenu, según el nombre del `.nro`) protegido con el PIN, que mantiene todos los homebrew, incluida una tienda de homebrew, a un PIN de distancia. Y un PIN en la consola, y los parches de firma que necesitan los forwarders. Mientras tanto, Ⓐ indica lo que falta.
  - El camino de vuelta del menú cuenta una vez que se vio funcionar: iniciado una vez desde el menú HOME, con el PIN ingresado en su puerta (lo indica el registro de la puerta, que se lee cuando PlayGuard se abre o bloquea). Un menú protegido que nunca se inició así aparece como tal.
  - Al bloquear se muestran las comprobaciones como una lista (una marca de verificación turquesa para lo que está listo, un triángulo ámbar o coral para lo que no), y se pide mantener Ⓐ 2 segundos en *Bloquear* (o un dedo sobre el botón), mientras se llena una barra; si se suelta antes, nada cambia. Luego viene la **hoja de rescate** (también escrita en `config/playguard/rescue.html`, una página para abrir e imprimir en una computadora, en el idioma que se muestra, con el código QR): cómo deshacerlo desde una computadora, o con el sysmodule de recuperación cuando está instalado, y un código QR a la ayuda ([más abajo](troubleshooting.md#menú-homebrew-bloqueado)); *Comprobar el menú homebrew* también la ofrece.
  - En emuMMC (o en una sysMMC con una emuMMC configurada), advierte que el bloqueo está en la tarjeta SD, así que ambos sistemas lo leen, mientras que los forwarders son de cada sistema: el otro sistema no tiene ninguno de estos caminos de vuelta.
  - Mientras está bloqueado, se rechaza eliminar el forwarder de PlayGuard o el último menú protegido (o quitarle su PIN): desbloquee primero. Eliminar uno de varios menús protegidos pide confirmación, en coral, nombrando los que quedan.
  - Antes de bloquear, ofrece proteger de una vez los forwarders creados por PlayGuard que todavía no están protegidos, e indica cuáles seguirían iniciando homebrew sin el PIN (un menú sin proteger, o uno creado por otra app), cuáles escriben en la tarjeta SD (un administrador de archivos, un instalador como Cyberfoil o Goldleaf, un servidor FTP: podrían cambiar el archivo del bloqueo) y cuándo está instalado uLaunch (inicia los homebrew a su manera, y el bloqueo no llega hasta ahí).
  - El PIN rechazado tres veces o más en un mismo día en la puerta de un forwarder se indica en la campana (qué forwarder, el día, cuántas veces).
  - Cuando falla la escritura de un archivo, el error dice por qué cuando puede saberlo: la tarjeta SD está llena, o no se puede escribir nada en ella (su interruptor de bloqueo, o el bit de archivo que corrige *Tools* de hekate).
  - *Comprobar el menú homebrew* muestra lo que leerá Atmosphère (si el álbum y un juego iniciado con R abren el menú homebrew), los caminos de vuelta, los parches de firma, la versión de Atmosphère con la que se bloqueó y el sysmodule de recuperación; el informe de diagnóstico incluye lo mismo. Si no está bloqueado, lo indica primero y ofrece *Bloquear*, con los pasos que faltan (la hoja de rescate se queda); con un sysmodule de recuperación desactualizado, el PlayGuard con el que vino y cómo reemplazarlo (*Actualizar el módulo* cuando esta compilación lo incluye). En emuMMC, o en sysMMC con una emuMMC configurada, indica que el otro sistema también lee el bloqueo, sin los forwarders instalados aquí.
  - Son unas pocas líneas al final de `atmosphere/config/override_config.ini`, entre dos comentarios de PlayGuard; el resto del archivo se deja como está, y se guarda tal como estaba en `config/playguard/override_config.ini.backup`. Atmosphère lee el archivo cada vez que se inicia un programa: no hace falta reiniciar. Desbloquear quita esas líneas y vuelve a escribir los valores propios de Atmosphère para las claves que el resto del archivo no define (Atmosphère conserva lo que leyó hasta que se reinicia la consola).
  - Bloquear y desbloquear piden el PIN, diga lo que diga *Proteger PlayGuard con el PIN*, y quedan en el historial de cambios. Mientras está bloqueado, PlayGuard comprueba cada vez que se inicia que los dos caminos de vuelta siguen ahí; la campana indica cuando falta uno. Un bloqueo quitado del archivo fuera de PlayGuard (desde una computadora, un homebrew) se indica en la campana y en el historial de cambios; PlayGuard nunca lo vuelve a poner por sí solo, ya que eliminar el archivo es el camino de vuelta desde una computadora. Eliminar un forwarder (Configuración de la consola › Gestión de datos, donde no se pide ningún PIN) nunca quita el bloqueo: solo elimina un camino de vuelta.
  - Desde una computadora: elimine `atmosphere/config/override_config.ini` (o vuelva a poner el respaldo) y luego reinicie la consola.
  - Con el sysmodule de recuperación instalado (opcional, [sysmodule/README.md](../../sysmodule/README.md), en inglés), una solicitud `RESCUE` también lo quita al arrancar, igual que una actualización de Atmosphère desde que se bloqueó (sus parches de firma pueden ir con retraso, y ningún forwarder se inicia sin ellos): la siguiente vez que se inicia PlayGuard lo indica y ofrece volver a bloquearlo.
- **Un forwarder eliminado fuera de PlayGuard** (en la Configuración de la consola): se indica una vez en el historial de cambios, y en la campana mientras dure, con *Volver a instalar* u *Olvidar* en su fila.
- **Parches de firma:** la consola solo inicia un forwarder con parches de firma de fs y del loader (sys-patch, o archivos sigpatch), con todos los patrones que sys-patch tiene para este firmware aplicados. Cuando faltan, o sys-patch omitió uno (un sys-patch antiguo en un firmware más reciente), la pantalla lo indica arriba de la lista, nombrando lo que no está parcheado, y al agregar, crear el de PlayGuard o reinstalarlos todos advierte primero que el forwarder todavía no se iniciaría: agregarlo de todos modos (se inicia en cuanto los parches estén en su lugar) o cancelar. Bloquear el menú homebrew también los espera (*Consola › Parches de juegos* da los detalles). Se advierte de los archivos sigpatch que quedan junto a sys-patch, y *Consola › Parches de juegos* los aparta (los mueve, nunca los elimina). Cada forwarder conserva el cargador con el que se creó: se ofrece volver a crear los creados por un PlayGuard anterior (la campana, *Consola › Forwarders*). El `.nro` se queda donde está; eliminar un forwarder nunca lo toca.
- **Tiempo de juego:** iniciado desde su ícono, un homebrew funciona como un juego: el temporizador lo cuenta y lo suspende como a cualquier juego cuando se acaba el tiempo, y *Actividad* lo muestra. PlayGuard sigue a la mano desde el menú de homebrew.
- **En los respaldos de la configuración** (*Historial › Respaldos*): lo que creó PlayGuard y si el menú homebrew estaba bloqueado. Al restaurar un respaldo, después del resto, ofrece volver a poner los forwarders que ya no están en el menú HOME (protegidos como estaban, con su horario y su ícono); un bloqueo solo se indica, y hay que volver a hacerlo desde aquí.
- Cada cambio pide el PIN cuando *Proteger PlayGuard con el PIN* lo indica, se rechaza en modo de solo lectura y queda en el historial de cambios. Lo que creó PlayGuard se guarda por sistema (sysMMC / emuMMC), en `forwarders.json`.

### Las líneas de la consola

- **Firmware, Atmosphère, hekate** (cuando su `bootloader/update.bin` está en la tarjeta SD) **y sys-patch** (cuando está instalado; su versión, de su `config/sys-patch/log.ini`), cada uno un mosaico con la versión instalada y luego la versión más reciente que encontró la última búsqueda de actualizaciones en una píldora ámbar (*23.0.1 disponible*, el mosaico con borde ámbar, y debajo cuántas versiones de atraso cuando son más de una), o *Actualizado*; una advertencia en la esquina del mosaico del firmware cuando el firmware más reciente necesita primero un Atmosphère o un hekate más reciente, o todavía no es compatible con ellos (las notas de la versión del firmware explican por qué: las notas de versión de Atmosphère y hekate indican qué firmware admiten).
- **Notas de la versión:** Ⓐ en cada uno las abre: una pestaña para la versión instalada, una por cada versión más reciente (cinco como máximo) y versiones más antiguas en las pestañas restantes (seis en total; Ⓛ / Ⓡ pasan de una a otra), que se abre en la siguiente, con etiquetas (una marca de verificación para la versión instalada, un destello para una más reciente, el firmware más reciente que admite, correcciones de seguridad, control parental modificado), las notas tal como se publicaron (en inglés), *Ver en línea* (un código QR) y cuándo se obtuvieron. Las de Atmosphère, hekate y sys-patch vienen de sus versiones en GitHub (las de sys-patch, de `impeeza/sys-patch`), las del firmware de switchbrew; se guardan en la tarjeta SD para leerlas sin conexión y se vuelven a obtener una vez al día (Ⓧ ahora).
- **La compatibilidad** y luego el **almacenamiento** (emuMMC o sysMMC, con un ícono de información cuando el otro sistema tiene su propio control parental: Ⓐ lo explica).
- Si Atmosphère **oculta el número de serie** (*Sí (XAW00000000000)*, el número que ve entonces el sistema; en emuMMC, cuando no lo oculta, un ícono de advertencia, que Ⓐ explica), y luego el **número de serie real** de la consola, con su parte central oculta: cuando está oculto, se lee del respaldo de PRODINFO que Atmosphère guarda en `atmosphere/automatic_backups/` (el de esta consola, por su ID de dispositivo, cuando la tarjeta SD tiene varios). Ⓐ en su ícono de información lo muestra completo e indica de dónde viene.
- **Parches de juegos** (sys-patch o archivos sigpatch, con la recomendación de sys-patch cuando solo se usan archivos: un ícono de información, o uno de advertencia cuando hay algo que revisar, que Ⓐ explica).

## Preferencias

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=preferences)*

- Lo que configura al propio PlayGuard:
  - Primero, como mosaicos: idioma (todos los idiomas de la consola, o el de la propia consola), tema (oscuro de forma predeterminada, claro o el de la consola), tamaño del texto (automático / normal / más grande) y la pestaña con la que se abre.
  - **La intro** al iniciar: la pantalla de carga (su ícono con el anillo girando, la barra, la esfera de Cadran a la derecha) durante 2 segundos (activada de forma predeterminada; Ⓐ o Ⓑ la saltan; desactivada, PlayGuard se abre en cuanto está listo).
- Los mensajes que muestra:
  - Los tipos de notificaciones (vea [más arriba](#cómo-se-comportan-las-entradas)).
  - Un **recordatorio mensual para apoyar PlayGuard** (activado de forma predeterminada, nunca durante el primer mes ni justo después de una actualización; *No volver a mostrar* en el recordatorio o este interruptor lo desactivan para siempre, actualizaciones incluidas).
- Cómo se actualiza: *Buscar actualizaciones cada día* al iniciar, y *Actualizar con* (vea [Información](#versión-y-actualizaciones)).
- **Primeros pasos** otra vez (también en *Información › Ayuda y diagnóstico*).
- Una opción que afecta a una sola función está junto a ella: el tiempo extra en *Tiempo de juego*, la reactivación del control parental en *Seguridad*, la comprobación del reloj en *Consola › Reloj de red*. El historial de cambios y los respaldos tienen su pestaña, [Historial](#historial--lo-que-cambió-playguard-y-los-respaldos); buscar actualizaciones ahora y *Ayuda y diagnóstico* están en *Información*.

## Información

*[Probarlo en el simulador ›](https://playguard.link/simulator/?tab=about)*

### Versión y actualizaciones

- Arriba, su tarjeta: el logotipo, el nombre, la versión.
- El PlayGuard instalado: versión (en ámbar cuando hay una más reciente; Ⓐ abre sus **notas de la versión**).
- **Buscar actualizaciones** ahora; *Buscar actualizaciones cada día* al iniciar está en *Preferencias*.
- *Actualizar con* (en *Preferencias*): en PlayGuard, que descarga la versión, la verifica, se reemplaza a sí mismo y se reinicia, o con sphaira, Homebrew App Store o a mano (se pregunta en la primera actualización si no se eligió nada). Pasarle el relevo a una tienda pide el PIN como un cambio (*Proteger PlayGuard con el PIN › Antes de un cambio*), y siempre mientras el menú homebrew está bloqueado: una tienda inicia cualquier homebrew. Si se instaló desde el Homebrew App Store o el App Store de sphaira, la tienda es la vía recomendada ahí, para que lleve el control de la versión; cuando su catálogo todavía no tiene la nueva versión (la publica tras una revisión, a menudo unos días después), PlayGuard lo indica y ofrece actualizar en su lugar.
- Mientras se conoce una versión más reciente, PlayGuard se abre en **PlayGuard X.Y.Z está disponible** (su fecha y sus notas): *Actualizar*, o *Más tarde* por 1 día, 1 semana o 30 días como máximo.
- Las notas de la versión son las del propio PlayGuard, en la misma pantalla que las de Atmosphère y hekate en *Consola*: a partir de sus versiones en GitHub, una pestaña por versión (la instalada, las más recientes, las más antiguas), cada una con sus puntos destacados como tarjetas y todos sus cambios, guardadas en la tarjeta SD para leerlas sin conexión (las de la versión instalada también están en la aplicación, para cuando todavía no se obtuvo nada).
- Después de una actualización, PlayGuard se abre una vez en **Novedades de la X.Y.Z** (los mismos puntos destacados, y luego los códigos QR).

### Ayuda y diagnóstico

*Ayuda y diagnóstico ›*, una pantalla propia, debajo de *Buscar actualizaciones*:

- **Arriba**, en *Para empezar*, dos mosaicos: *Primeros pasos* y *Enviar un informe…*; luego *Diagnóstico*.
- **Primeros pasos** vuelve a abrir la guía (con un paso *Desvincular la app para celular* mientras está vinculada, un paso *Reactivar la alarma de “Se acabó el tiempo”* mientras está desactivada, un paso *Proteger PlayGuard* una vez definido el PIN —la opción *Proteger PlayGuard con el PIN*—, y un interruptor para que deje de aparecer al iniciar). Debajo de *Cerrar*, *Apoyar PlayGuard* muestra los códigos QR para apoyar el proyecto.
- Cómo se inició PlayGuard (una aplicación sobre un juego o desde su ícono de HOME, o un applet) y su carpeta de datos.
- **Exportar un informe de diagnóstico**, o **enviarlo en línea** (*Enviar un informe…*; consulte [Reportar un error](troubleshooting.md#reportar-un-error)); cuántos informes guardados, archivos de fallo, registros de sesión y enlaces de informes enviados en línea se conservan, por cantidad, antigüedad o tamaño.

### Créditos

*Créditos y licencia ›*: quién lo hizo, en qué se basa, su licencia y su código fuente como código QR, en un solo diálogo. Luego, cómo **apoyar PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), mostrados como códigos QR para escanear con un celular) y un pequeño *Made in France* 🇫🇷.

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
| **PlayGuard iniciado desde un forwarder** (su ícono del menú HOME, también una aplicación) | **Sí**, con el nombre del ícono | Deducido: una aplicación como cualquier otra; no medido por PlayGuard |
| PlayGuard abierto desde el Álbum (modo applet) | No | Deducido: no es una aplicación, y el tiempo fuera de las aplicaciones no parecía contar |
| Mientras el control parental está desbloqueado temporalmente (también la pausa de PlayGuard mientras está en él) | Quizás no | No medido: pregunta abierta |
| Después de cambiar el reloj | El día vuelve a empezar; tras un salto hacia atrás, el conteo se detiene hasta que el reloj pasa la hora anterior | Observado (22.0.0); *Hoy* indica cuando la consola no está contando |

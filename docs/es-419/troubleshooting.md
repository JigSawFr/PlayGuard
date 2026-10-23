<!-- translated from docs/troubleshooting.md, sha256 45158eaaf5a4 -->
# Solución de problemas

Recuperar el acceso a una consola bloqueada, los problemas frecuentes, reportar un error y el modo desarrollador. Volver al [README](../../README.md).

## ¿Consola bloqueada?

PlayGuard solo ve el control parental del sistema en el que se ejecuta: **la emuMMC y la sysMMC tienen cada una el suyo** (un PIN eliminado en una sigue estando en la otra). Ejecútelo en cada una que necesite corregir.

### Consola de segunda mano

Usted conoce el PIN, pero la app para celular del dueño anterior sigue vinculada (la desvinculación falla, o el restablecimiento de fábrica pide su cuenta).

*Seguridad › Desvincular la app para celular* y luego, si no quiere ningún control parental, *Eliminar todo el control parental*. Ambas opciones funcionan sin conexión, tanto en emuMMC como en sysMMC.

### PIN olvidado

*Mostrar el PIN* pide el propio PIN, así que no sirve aquí. Con el sysmodule de recuperación instalado (más abajo), coloque un archivo `RESCUE` y reinicie: la pantalla de recuperación define entonces uno nuevo (nunca muestra el anterior). Sin él, *Eliminar todo el control parental* permite empezar de cero (antes se guarda un respaldo de la configuración, que nunca contiene el PIN): con *Proteger PlayGuard con el PIN* activado (lo predeterminado, *Antes de un cambio*), primero ponga la tarjeta SD en una computadora y cambie `"pin_lock"` a `"off"` en `sd:/config/playguard/config.json`.

### El temporizador de juego lo bloquea todo (límite de 0 minutos) y se olvidó el PIN

En ese caso, PlayGuard ni siquiera puede iniciarse. Instale **de antemano** el sysmodule de recuperación opcional (`playguard-rescue.zip`); si queda bloqueado, coloque un archivo vacío `config/playguard/RESCUE` en la tarjeta SD y encienda la consola: el módulo la desbloquea para que PlayGuard pueda abrirse. Consulte [`sysmodule/README.md`](../../sysmodule/README.md) (en inglés).

El módulo debe ir a la par de la versión de PlayGuard: con cada actualización de PlayGuard, vuelva a extraer el `playguard-rescue.zip` de esa versión (o use *Actualizar* en *El sysmodule de recuperación no está actualizado*, en la campana de notificaciones) y luego reinicie la consola. Un módulo más antiguo deja un informe que PlayGuard no puede verificar: la pantalla de recuperación entonces no ofrece nada, indica que el módulo es más antiguo, y volver a usar `RESCUE` da el mismo resultado hasta que se actualice.

### Menú homebrew bloqueado

*Consola › Forwarders › Bloquear el menú homebrew* impide que el álbum y un juego iniciado con R abran el menú homebrew. Si el forwarder de PlayGuard y el menú protegido ya no están (eliminados en la Configuración de la consola, o en el otro sistema: el bloqueo está en la tarjeta SD, y lo leen tanto la sysMMC como la emuMMC):

- **Desde una computadora:** elimine `atmosphere/config/override_config.ini` de la tarjeta SD (o vuelva a poner `config/playguard/override_config.ini.backup`, el archivo como estaba) y luego reinicie la consola.
- **Con el sysmodule de recuperación instalado** (opcional): coloque un archivo vacío `config/playguard/RESCUE` en la tarjeta SD y reinicie; el sysmodule quita el bloqueo al arrancar, y PlayGuard lo indica la siguiente vez que se inicia. Una actualización de Atmosphère desde que se bloqueó lo quita de la misma manera.

### Consola sin modificar

PlayGuard no puede ayudar: necesita Atmosphère. El procedimiento de la clave maestra del soporte de Nintendo es la vía oficial.

## Problemas frecuentes

### El tiempo restante no baja

*Hoy* indica *La consola no está contando el tiempo de juego en este momento* cuando no se movió durante unos 90 s. Las causas probables:

- **El reloj de red nunca se ajustó.** *Consola › Reloj de red › Medir la diferencia horaria* y luego ajústelo ([reloj de red](features.md#reloj-de-red)).
- **El reloj se atrasó.** Tras un salto hacia atrás, el conteo se detiene hasta que el reloj pasa la hora anterior; no hay nada que hacer.
- **Se acaba de escribir un límite.** Inicie un juego un minuto y vuelva a revisar.
- **El control parental está desbloqueado temporalmente.** Es posible que entonces no se cuente el tiempo de juego: *Hoy › Reactivar* (o un toque en la píldora del encabezado).

### Mis límites cambiaron solos

- **La app para celular sigue vinculada.** Su próxima sincronización sobrescribe lo que configuró PlayGuard: *Seguridad › Desvincular la app para celular*.
- **Otra herramienta, o un perfil.** *Hoy* muestra una tarjeta ámbar para los límites distintos de los que PlayGuard vio la última vez, y *Historial › Cambios hechos con PlayGuard* enumera cada cambio visto fuera de PlayGuard. Ⓐ en uno restablece el valor anterior.
- **Un perfil programado** que se aplicó, o que se revirtió al terminar su período (*Tiempo de juego › Perfiles › Perfil programado*).

### El niño jugó más allá del límite

- **La consola solo avisa.** *Tiempo de juego › Cuando se acabe el tiempo*: elija *Suspender el programa*.
- **El control parental quedó desbloqueado.** Mientras lo está, el límite de hoy queda en pausa: la píldora del encabezado lo indica en todas las pestañas, y al tocarla lleva a *Hoy › Reactivar*. Un reinicio de la consola también lo bloquea.
- **El reloj se atrasó.** El conteo se detiene hasta que el reloj pasa la hora anterior ([qué cuenta](features.md#qué-cuenta-como-tiempo-de-juego)).
- **El otro sistema.** La emuMMC y la sysMMC tienen cada una su propio control parental: configúrelo en ambas, u oculte el menú de arranque de hekate ([compatibilidad](compatibility.md#emummc-y-sysmmc)).

### Todo el límite vuelve a estar disponible hoy

Cambiar el reloj (el reloj de red o el de la consola) pone a cero el tiempo de juego de hoy. Ajuste el reloj antes de que el niño juegue, no después.

### PlayGuard se abre en solo lectura

La consola ejecuta un firmware más reciente que el más reciente probado: PlayGuard comprueba si una versión más reciente es compatible con él y ofrece actualizar ([firmware más reciente](compatibility.md#firmware-más-reciente)). Un lápiz tachado a la derecha del título indica que la aplicación está en solo lectura; al tocarlo se abre la pantalla del firmware.

### PlayGuard pide el PIN todo el tiempo, o nunca

*Seguridad › Proteger PlayGuard con el PIN*: *Antes de un cambio* (lo predeterminado) lo pide una vez, antes del primer cambio, y el PIN vale hasta que se cierra PlayGuard. *Volver a pedir el PIN tras 5 minutos* lo vuelve a pedir pasado ese tiempo. *Mostrar el PIN* siempre lo pide.

### Un juego no se inicia, o muestra un “!” en el menú HOME

*Consola › Comprobar los juegos instalados* indica qué falla en cada uno y qué hacer; los datos de guardado siempre se conservan ([juegos instalados](features.md#juegos-instalados)).

### Un forwarder protegido muestra el error 2347-0100

Su puerta (`config/playguard/gate-….nro`) ya no es el archivo con el que se creó el forwarder: se modificó o se reemplazó en la tarjeta SD, así que el forwarder no la inicia. Abra PlayGuard desde su propio forwarder: vuelve a escribir la puerta al iniciarse; *Consola › Forwarders › Reinstalar todos* vuelve a crear todos los forwarders con ella.

### Un forwarder protegido se cierra de inmediato (“Se produjo un error”)

Antes de cualquier pantalla de PlayGuard, sin código de error (el informe de Atmosphère en `atmosphere/crash_reports/` nombra el módulo `forwarder`, resultado `2000-0000`): lo creó un PlayGuard cuya puerta se compiló sin sus recursos, que el cargador del forwarder rechazó. Actualice PlayGuard y luego vuelva a crearlo: la campana y *Consola › Forwarders* lo ofrecen para cada forwarder creado con un cargador anterior.

### Un forwarder protegido se queda en la pantalla de su puerta después del PIN

El PIN se acepta, la pantalla con el ícono del homebrew se queda y no pasa nada más: su puerta dibujó esa pantalla con un solo búfer y esperó indefinidamente para dibujar la siguiente. Actualice PlayGuard y luego vuelva a crearlo: la campana y *Consola › Forwarders* lo ofrecen para cada forwarder creado con una puerta anterior.

### Un forwarder muestra “No se pudo iniciar el software”

La consola se negó a iniciarlo: todavía hay una verificación de firma. En este orden, revise:

- **Archivos de sigpatches junto a sys-patch.** Archivos `.ips` en `atmosphere/kip_patches/fs_patches`, `loader_patches` o `atmosphere/exefs_patches/es_patches` mientras sys-patch está activo: dos herramientas que parchean las mismas verificaciones, la causa que más mencionan los hilos de sys-patch y de sigpatches en GBAtemp. *Consola › Parches de juegos* lo advierte, y Ⓐ ahí los aparta (los mueve a `config/playguard/sigpatches-aside/`, nunca los elimina; el `bootloader/patches.ini` de hekate se queda). Reinicie la consola.
- **Un patrón que sys-patch no aplicó.** Los forwarders necesitan sus parches de fs y de loader, todos los patrones que tiene para este firmware (`noncasigchk_17.0.0+`, `nocntchk_19.0.0+`, `noacidsigchk_10.0.0+`), no solo uno por módulo. *Información › Ayuda y diagnóstico › Exportar un informe de diagnóstico* indica, en su línea `game patches`, la versión de sys-patch y cada patrón omitido; `sd:/config/sys-patch/log.ini` contiene lo mismo. Verifique que `config/sys-patch/config.ini` parchee este sistema (`patch_emummc=1` en la emuMMC) y no desactive ninguno; el propio responsable de sys-patch dice que 23.0.x no necesita un sys-patch más reciente que el 1.6.2.3.
- **No se reinició desde que cambiaron los parches.** sys-patch parchea al arrancar.
- **Un forwarder creado por un PlayGuard anterior.** La campana y *Consola › Forwarders* ofrecen volver a crear los que se crearon con un cargador anterior.

### La campana o una pestaña muestra una insignia ámbar

Algo requiere su atención: Ⓨ abre la lista, y Ⓐ en una entrada aplica la solución o abre donde se hace ([notificaciones](features.md#notificaciones--lo-que-requiere-su-atención-desde-cualquier-pestaña)).

## Reportar un error

Un error muestra el código del sistema (por ejemplo, `0x0001088E`) con lo que se sabe de él: qué hacer, para los códigos cuyo significado está documentado; si no, un informe ayuda.

1. *Información › Ayuda y diagnóstico › Exportar un informe de diagnóstico* guarda un archivo de texto en `sd:/config/playguard/logs/reports/`: firmware, versión de Atmosphère, cuánto tardó el inicio y cuánto tardaron las pantallas lentas (el historial de cambios, *Todos los juegos*, la comprobación de los juegos instalados, las pestañas que se actualizan solas), relojes, almacenamiento, estado del ocultamiento del número de serie y de los parches de juegos, lo que hace la redirección DNS de Atmosphère con los servidores de control parental y de hora de Nintendo (leído de su archivo hosts en la tarjeta SD, no una prueba de red, con el final del propio `atmosphere/logs/dns_mitm_startup.log` de Atmosphère), el resultado sin procesar de cada consulta al control parental, y los registros: cómo terminó la última sesión, la memoria y el espacio libre en la tarjeta SD, los archivos de fallo más recientes y las últimas 80 líneas registradas. **Nunca contiene el PIN ni el número de serie.**
2. [Abra un issue](https://github.com/JigSawFr/PlayGuard/issues/new) y adjúntelo.

### Enviar un informe en línea

Con la consola conectada a Internet, *Información › Ayuda y diagnóstico › Enviar un informe en línea* (Ⓧ en el informe de diagnóstico, en modo desarrollador) lo envía a un gist secreto en su cuenta de GitHub, después de indicar exactamente qué se envía. Un informe guardado antes en `logs/reports/` se puede enviar de la misma forma.

- **Qué se envía:** el informe y cada archivo como archivos separados:
  - los informes de fallo más recientes: los 3 últimos de PlayGuard en `logs/crashes/`, y los 2 últimos de Atmosphère de un homebrew de la última semana, en `sd:/atmosphere/crash_reports/` (puede incluirse el fallo de otro homebrew: la lista que se muestra antes de enviar indica cuáles);
  - los registros de esta sesión y de la anterior (`logs/sessions/`; de uno largo, su comienzo y sus últimos 64 KiB);
  - los archivos de depuración: la referencia del bloque del temporizador de juego, el final del registro del temporizador mientras *Registrar el temporizador de juego* está activado, los últimos 20 cambios del historial de cambios (sin nombres de juegos, cuentas ni perfiles, y solo con su fecha), y la configuración de PlayGuard.
- **Inicio de sesión:** sin sesión iniciada, PlayGuard ofrece iniciarla primero; *Cuenta de GitHub*, justo debajo una vez iniciada la sesión, la cierra; si GitHub rechaza el gist, PlayGuard ofrece volver a iniciar sesión.
- **Después:** muestra dos códigos QR, con el enlace debajo del primero y su identificador en grupos de cuatro para leerlo en voz alta o escribirlo: el informe, para quien ya lo esté ayudando, y el formulario de reporte de error con el enlace, sus versiones, cómo se inició PlayGuard (sobre un juego o desde el Álbum; agregue el menú) y el sistema (sysMMC o emuMMC) ya completados, para escanearlo y reportar un error. El enlace también queda en las notificaciones, y cada uno se guarda en `logs/uploads/` como un acceso directo de Internet (`.url`) que una computadora abre en su navegador.

> [!WARNING]
> Cualquier persona con el enlace puede leer el informe hasta que usted elimine el gist en GitHub, y un reporte de error abierto con él es público: la confirmación lo indica.

### Lo que PlayGuard guarda en la tarjeta SD

Todo está en `sd:/config/playguard/logs/`, en texto plano que una computadora puede abrir:

- `sessions/session_<date>.txt` — **un archivo por sesión**: todo lo que PlayGuard registró (lo que antes solo llegaba a una PC por nxlink), cada línea con su hora y su nivel (`E` error, `W` advertencia, `I` información), y una última línea que indica cómo terminó la sesión: `=== end: clean exit ===` (salida normal), o `=== end: crash, see logs/crashes/… ===` (fallo). Un archivo deja de crecer al llegar a 1 MiB.
- `crashes/crash_<date>.txt` — **un archivo por fallo**, cuya primera línea dice qué pasó:
  - una **excepción de la CPU** (un puntero incorrecto, una instrucción no definida, un desbordamiento de pila): el tipo de falla y la dirección a la que accedió, la traza de llamadas (backtrace) y los registros del procesador. Las direcciones son desplazamientos dentro de `playguard.nro` (`playguard+0x1a2b3c`): el mapa del enlazador de la compilación (el artefacto `-linker-map` de su ejecución de CI) da la función. Luego PlayGuard termina como antes: la pantalla de error de la consola, y el propio informe de Atmosphère en `sd:/atmosphere/crash_reports/`;
  - una **excepción no capturada**: su tipo y su mensaje, y la traza de llamadas;
  - una **sesión que no terminó**: se detecta en el siguiente inicio, cuando el archivo de la sesión anterior no tiene línea de cierre (un fallo que PlayGuard no pudo capturar, un cierre desde el menú HOME mientras estaba colgado, o la consola apagada), con las últimas 60 líneas de esa sesión.

  Los archivos de fallo capturados mientras PlayGuard se ejecuta terminan con las últimas 80 líneas registradas antes del fallo.
- `reports/`, `uploads/`, `play_timer/` — los informes guardados, los enlaces de los informes enviados en línea, el registro del temporizador de juego.

*Información › Ayuda y diagnóstico* define cuántos informes, archivos de fallo, registros de sesión y enlaces se conservan (cada carpeta por separado; el más reciente siempre se conserva).

## Modo desarrollador (investigar un firmware nuevo)

Presione Ⓨ siete veces en *Información › Versión*; las herramientas de desarrollo aparecen al final de *Información › Ayuda y diagnóstico*. Este modo agrega:

- un interruptor de **solo lectura**: activado, la aplicación no puede cambiar nada; es la forma segura de investigar un firmware nuevo (la pantalla del firmware lo ofrece directamente);
- el informe de diagnóstico en pantalla (Ⓨ lo guarda, Ⓧ lo envía en línea), y un acceso directo para exportarlo desde la pestaña Tiempo de juego;
- **Autoprueba de la consola**: lo que PlayGuard comprueba por sí solo en esta consola antes de recorrer a mano [la lista de verificación en consola](../hardware-test.md) (en inglés): el PIN, los parches de firma, la carpeta de datos, cada forwarder creado por PlayGuard (en el menú HOME, protegido, visto funcionar a través de su puerta), la puerta en la tarjeta SD, lo que Atmosphère lee del bloqueo del menú homebrew, los caminos de vuelta, el sysmodule de recuperación. No cambia nada, y escribe su resultado en Markdown en `config/playguard/self-test.md`, para pegarlo en la pull request;
- **Instalar otra compilación** en su lugar, para probar una corrección antes de que se publique ([más abajo](#instalar-otra-compilación));
- **Comparar bloque del temporizador** (solo mientras la app para celular está vinculada y puede sincronizarse, no con un archivo hosts que bloquee a Nintendo; también en *Seguridad*), para descifrar ajustes que PlayGuard todavía no muestra: guarde el bloque sin procesar como referencia, cambie un ajuste en la app para celular, vuelva: PlayGuard enumera los valores que cambiaron (guardados en `logs/reports/` si lo pide). Así se podrían confirmar horas de dormir distintas para cada día, configuradas desde la app para celular. Adjunte ese archivo a un issue, o envíelo con el informe (*Enviar un informe en línea*).
- **Registrar el temporizador de juego**: cada 30 s mientras PlayGuard está abierto, una línea de lo que informa el temporizador de juego (tiempo restante, tiempo jugado, el bloque de configuración sin procesar…) en `logs/play_timer/log.csv`, un archivo listo para una hoja de cálculo, más una línea justo después de cada cambio que hace PlayGuard, que indica cuál (su última columna, `event`), y una nota cuando el reloj se movió más que el tiempo transcurrido. Si se deja abierto pasada la medianoche, o hasta que se acaba el tiempo, muestra lo que un solo informe no puede: cuándo se reinicia el tiempo jugado, qué dice la consola cerca del final. El interruptor se recuerda; solo registra en modo desarrollador. Lo que se sabe hasta ahora está en [parental-controls.md](../parental-controls.md) (en inglés).
- **Vaciar la caché** y **Eliminar los archivos de registro**, cada uno tras una confirmación que indica cuántos archivos y cuánto espacio: la caché (`cache/` y el `nand/<system>/cache/` de cada sistema: nombres e íconos de los juegos, los últimos datos de Actividad, la lista de compilaciones) se vuelve a leer de la consola cuando hace falta; los archivos de registro son los archivos de fallo, los registros de sesión (salvo el de esta sesión, que aún se está escribiendo), los informes guardados y el registro del temporizador de juego, mientras que `uploads/` (los enlaces para eliminar los informes enviados en línea) y la referencia del bloque se conservan. La configuración, el historial, los respaldos, los perfiles y las exportaciones nunca se tocan.
- **Volver a ejecutar las migraciones**: los traslados de las organizaciones anteriores — la carpeta de datos junto al `.nro`, los registros de antes de sus subcarpetas, lo que compartían todos los sistemas —, cada uno en una línea para marcar o desmarcar (todos marcados al principio; A cambia uno), luego *Ejecutar las N elegidas* y una confirmación que las enumera con lo que mueve cada una. Se ejecutan de inmediato, diga lo que diga `migrations.json` (si no, cada una se ejecuta una sola vez, en el primer inicio de una compilación que la necesite; el nivel que registra nunca baja); un archivo que ya está en su lugar nunca se reemplaza. Un diálogo indica cuántos elementos se movieron; cada paso queda en el registro de sesión.
- **Estadísticas de imagen**: una línea por segundo en `logs/frame_stats.txt`, con el tiempo que tardó la pantalla en dibujarse y las imágenes que llegaron tarde, para comparar una pantalla con otra en la consola. **Halo de fondo** apaga la luz suave detrás de las pantallas (el modo de la línea termina entonces en `-noglow`), para medir cuánto cuesta.

### Instalar otra compilación

*Instalar otra compilación* pone una compilación en lugar del propio `.nro` de PlayGuard, para probar una corrección antes de que se publique.

**Lo que enumera**, cada tipo en su propia pestaña (Ⓛ / Ⓡ, que se muestran a cada lado de las pestañas, pasan de una a otra; la pantalla se abre en la de la compilación instalada, y cada pestaña cuenta las compilaciones más recientes que la instalada):

- una de las 20 últimas versiones (incluidas las preliminares, etiquetadas);
- uno de los 20 últimos commits de `main`;
- la compilación más reciente de una pull request abierta desde una rama de este repositorio (nunca desde un fork: su código todavía no se revisó). A en la pestaña de las pull requests, cuando se muestra, las ordena al revés (compilación más reciente primero, o por número): la indicación de A dice cuál.

**Cada compilación** muestra su título (el asunto de un commit o de una pull request; el mensaje completo de un commit es su descripción) sobre etiquetas: el tipo de cambio (*Novedad*, *Corrección*, *Docs*…) y su alcance, *Incompatible*, la pull request de la que vino un commit, cómo terminaron sus comprobaciones (*CI OK*, *CI fallida*, *CI en curso*), preliminar, *instalada* o *nueva*; su antigüedad (en minutos, horas y luego días) y su commit a la derecha. Una compilación abre primero su página: etiquetas (su tipo, sus comprobaciones, instalada o nueva), una tarjeta de datos (compilación, tamaño, commit, suma de verificación, cuándo la borra GitHub), **Instalar**, **Ver en GitHub** (su página como código QR para un celular), una advertencia cuando sus comprobaciones todavía están en curso, y luego las notas de una versión o la descripción de una pull request, presentadas a partir del Markdown de GitHub (títulos, listas, casillas de tareas, código, citas y alertas, tablas).

**Lectura segura:** todo lo que viene de GitHub se muestra como texto simple: no se abre ningún enlace, no se ejecuta HTML ni scripts, y se eliminan los caracteres invisibles que podrían disfrazar un título (inversiones de derecha a izquierda, caracteres de ancho cero, caracteres de control, UTF-8 no válido).

**Inicio de sesión.** Las versiones publicadas no necesitan nada; las demás son los artefactos del workflow de compilación, que GitHub solo entrega a usuarios con sesión iniciada. **Cuenta de GitHub** inicia sesión con un código y un código QR para escanear con un celular. Los únicos permisos que se piden son leer los archivos del workflow de compilación (Actions) y crear gists, para *Enviar un informe en línea*; por lo demás, el token solo puede leer lo que es público. Se guarda en `github_token`, nunca se envía con un informe, y *Cuenta de GitHub* cierra la sesión. Con la sesión iniciada, Ⓨ (*Buscar*) acepta el número de una pull request (abierta o cerrada) o un commit (de 7 a 40 dígitos hexadecimales) y encuentra su compilación, solo desde una rama de este repositorio.

**Instalación:** PlayGuard descarga la compilación, la verifica (tamaño, el SHA-256 que registra GitHub, el encabezado NRO), la pone en lugar de su propio `.nro` y se reinicia con ella (antes pide el PIN si hay uno definido). Su ventana indica en qué paso está, el tamaño y la velocidad de la descarga, y qué compilación reemplaza; Cancelar desaparece una vez que la compilación se está poniendo en su lugar.

- La lista se conserva 10 minutos y se muestra de inmediato, con su antigüedad (Ⓧ la vuelve a obtener, y también la pantalla cada 3 minutos mientras las comprobaciones de una compilación están en curso); mientras se obtiene, una barra muestra el avance.
- Una descarga interrumpida (se perdió la red) continúa desde donde se detuvo la próxima vez que se instala la misma compilación.
- Los artefactos vencen a los 90 días.

**Volver atrás.** La compilación reemplazada se conserva (`playguard.nro.previous`): **Volver a la compilación anterior**, en la misma lista, las intercambia de nuevo, después del PIN; nombra esa compilación por su versión, su pull request y su commit. Si una compilación instalada desde aquí (o con *Actualizar* desde PlayGuard) falla antes de su primera pantalla, el siguiente inicio de PlayGuard ofrece volver a la compilación anterior en una pantalla de texto simple, antes de cargar cualquier otra cosa (A vuelve atrás, B la inicia de todos modos). Volver atrás pide el PIN en la propia pantalla del PIN de la consola cuando hay uno configurado (no en la pantalla de bloqueo de PlayGuard, que podría ser lo que falla); sin él, se inicia la compilación instalada. La misma lista permite volver a cualquier versión publicada en cualquier momento.

**Qué compilación se ejecuta:** *Información › Versión* muestra el commit en modo desarrollador. El encabezado de cada pantalla muestra la versión en ejecución junto al título: la de una versión publicada en una etiqueta gris (`1.2.0`), la de cualquier otra compilación en una etiqueta ámbar seguida de su commit, para que una compilación de prueba nunca se confunda con la versión publicada.

<!-- translated from docs/troubleshooting.md, sha256 498fbfed802c -->
# Solución de problemas

Recuperar el acceso a una consola bloqueada, los problemas habituales, informar de un error y el modo desarrollador. Volver al [README](../../README.md).

## ¿Consola bloqueada?

PlayGuard solo ve el control parental del sistema en el que se ejecuta: **la emuMMC y la sysMMC tienen cada una el suyo** (un PIN quitado en una sigue estando en la otra). Ejecútalo en cada una de las que haya que arreglar.

### Consola de segunda mano

Conoces el PIN, pero la aplicación móvil del anterior propietario sigue vinculada (la desvinculación falla, o el restablecimiento de fábrica pide su cuenta).

*PIN › Desvincular la app para móvil* y luego, si no quieres ningún control parental, *Eliminar todo el control parental*. Las dos funcionan sin conexión, tanto en la emuMMC como en la sysMMC.

### PIN olvidado

*Mostrar el PIN* pide el propio PIN, así que aquí no sirve. Con el sysmodule de recuperación instalado (más abajo), coloca un archivo `RESCUE` y reinicia: la pantalla de recuperación define entonces uno nuevo (nunca muestra el antiguo). Sin él, *Eliminar todo el control parental* permite empezar de cero (antes se guarda una copia de seguridad de los ajustes; nunca contiene el PIN): con *Proteger PlayGuard con el PIN* activado (lo predeterminado, *Antes de un cambio*), pon antes la tarjeta SD en un ordenador y cambia `"pin_lock"` a `"off"` en `sd:/config/playguard/config.json`.

### El temporizador lo bloquea todo (límite de 0 minutos) y has olvidado el PIN

En ese caso, PlayGuard ni siquiera puede iniciarse. Instala **de antemano** el sysmodule de recuperación opcional (`playguard-rescue.zip`); si te quedas bloqueado, coloca un archivo vacío `config/playguard/RESCUE` en la tarjeta SD y arranca: desbloquea la consola para que PlayGuard pueda abrirse. Consulta [`sysmodule/README.md`](../../sysmodule/README.md) (en inglés).

El módulo debe seguir la versión de PlayGuard: con cada actualización de PlayGuard, vuelve a extraer el `playguard-rescue.zip` de esa versión (o *Actualizar* en *El sysmodule de recuperación no está actualizado*, en la campana de notificaciones) y reinicia la consola. Un módulo más antiguo deja un informe que PlayGuard no puede verificar: la pantalla de recuperación no ofrece entonces nada, indica que el módulo es más antiguo, y volver a usar `RESCUE` da el mismo resultado hasta que se actualice.

### Menú homebrew bloqueado

*Consola › Forwarders › Bloquear el menú homebrew* impide que el álbum y un juego iniciado con R abran el menú homebrew. Si el forwarder de PlayGuard y el menú protegido han desaparecido los dos (eliminados en la configuración de la consola, o en el otro sistema: el bloqueo está en la tarjeta SD y lo leen tanto la sysMMC como la emuMMC):

- **Desde un ordenador:** elimina `atmosphere/config/override_config.ini` de la tarjeta SD (o vuelve a poner `config/playguard/override_config.ini.backup`, el archivo tal como estaba) y reinicia la consola.
- **Con el módulo de rescate instalado** (opcional): coloca un archivo vacío `config/playguard/RESCUE` en la tarjeta SD y reinicia; el módulo quita el bloqueo al arrancar, y PlayGuard lo indica la siguiente vez que se inicia. Una actualización de Atmosphère desde que se bloqueó lo quita de la misma forma.

### Consola sin modificar

PlayGuard no puede ayudar: necesita Atmosphère. La vía oficial es el procedimiento de la clave maestra del servicio de atención al cliente de Nintendo.

## Problemas habituales

### El tiempo restante no baja

*Hoy* indica *La consola no está contando el tiempo de juego ahora mismo* cuando no se ha movido durante unos 90 s. Las causas probables:

- **El reloj de red nunca se ajustó.** *Consola › Reloj de red › Medir el desfase del reloj*, y luego ajústalo ([reloj de red](features.md#reloj-de-red)).
- **El reloj se atrasó.** Tras un salto hacia atrás, la cuenta se detiene hasta que el reloj pasa la hora anterior; no hay nada que hacer.
- **Se acaba de escribir un límite.** Inicia un juego un minuto y vuelve a comprobarlo.
- **El control parental está desbloqueado temporalmente.** Es posible que entonces no se cuente el tiempo de juego: *PIN › Reactivar el control parental*.

### Mis límites han cambiado solos

- **La aplicación móvil sigue vinculada.** Su próxima sincronización sobrescribe lo que ha configurado PlayGuard: *PIN › Desvincular la app para móvil*.
- **Otra herramienta, o un perfil.** *Hoy* muestra una tarjeta ámbar cuando los límites difieren de los que PlayGuard vio por última vez, y *Preferencias › Historial de cambios* enumera cada cambio visto fuera de PlayGuard. Ⓐ en uno restablece el valor anterior.
- **Un perfil programado** que se aplicó, o que se retiró al terminar su periodo (*Tiempo de juego › Perfiles › Perfil programado*).

### El niño ha jugado más allá del límite

- **La consola solo avisa.** *Tiempo de juego › Cuando se acabe el tiempo*: elige *Suspender el programa*.
- **El control parental se quedó desbloqueado.** Mientras lo está, el límite de hoy queda en pausa: la píldora de la cabecera lo indica en todas las pestañas; *PIN › Reactivar el control parental*. Reiniciar la consola también lo bloquea.
- **El reloj se atrasó.** La cuenta se detiene hasta que el reloj pasa la hora anterior ([qué cuenta](features.md#qué-cuenta-como-tiempo-de-juego)).
- **El otro sistema.** La emuMMC y la sysMMC tienen cada una su propio control parental: configúralo en ambas, u oculta el menú de arranque de hekate ([compatibilidad](compatibility.md#emummc-y-sysmmc)).

### Hoy vuelve a estar disponible todo el límite

Cambiar el reloj (el reloj de red o el reloj de la consola) pone a cero el tiempo de juego de hoy. Ajusta el reloj antes de que juegue el niño, no después.

### PlayGuard se abre en solo lectura

La consola usa un firmware más reciente que el más reciente probado: PlayGuard comprueba si una versión más reciente lo admite y propone actualizar ([firmware más reciente](compatibility.md#firmware-más-reciente)). Un lápiz tachado a la derecha del título indica que la aplicación está en solo lectura; al tocarlo se abre la pantalla del firmware.

### PlayGuard pide el PIN todo el rato, o nunca

*PIN › Proteger PlayGuard con el PIN*: *Antes de un cambio* (lo predeterminado) lo pide una vez, antes del primer cambio, y el PIN vale hasta que se cierra PlayGuard. *Volver a pedir el PIN pasados 5 minutos* lo vuelve a pedir pasado ese tiempo. *Mostrar el PIN* lo pide siempre.

### Un juego no se inicia, o muestra un «!» en el menú HOME

*Consola › Comprobar los juegos instalados* indica qué le pasa a cada uno y qué hacer; los datos de guardado siempre se conservan ([juegos instalados](features.md#juegos-instalados)).

### La campana o una pestaña muestra una insignia ámbar

Hay algo que requiere tu atención: Ⓨ abre la lista, y Ⓐ en una entrada aplica la solución o abre el lugar donde se hace ([notificaciones](features.md#notificaciones--lo-que-requiere-tu-atención-desde-cualquier-pestaña)).

## Informar de un error

Un error muestra el código del sistema (por ejemplo, `0x0001088E`) con lo que se sabe de él: qué hacer con los códigos cuyo significado está documentado; si no, un informe ayuda.

1. *Acerca de › Ayuda y diagnóstico › Exportar un informe de diagnóstico* guarda un archivo de texto en `sd:/config/playguard/logs/reports/`: firmware, versión de Atmosphère, cuánto tardó el inicio y cuánto las pantallas lentas (el historial de cambios, *Todos los juegos*, la comprobación de los juegos instalados, las pestañas que se actualizan solas), relojes, almacenamiento, estado de la ocultación del número de serie y de los parches de juegos, el resultado en bruto de cada consulta al control parental, y los registros: cómo terminó la última ejecución, la memoria y el espacio libre en la tarjeta SD, los archivos de cierres más recientes y las últimas 80 líneas registradas. **Nunca contiene el PIN ni el número de serie.**
2. [Abre una incidencia](https://github.com/JigSawFr/PlayGuard/issues/new) y adjúntalo.

### Enviar un informe en línea

Con la consola conectada a Internet, *Acerca de › Ayuda y diagnóstico › Enviar un informe en línea* (Ⓧ en el informe de diagnóstico en modo desarrollador) lo envía a un gist secreto de tu cuenta de GitHub, después de indicar exactamente qué se envía. Un informe guardado antes en `logs/reports/` se puede enviar de la misma manera.

- **Lo que se envía:** el informe y cada archivo como archivos separados:
  - los informes de cierres más recientes: los 3 últimos de PlayGuard en `logs/crashes/`, y los 2 últimos de Atmosphère de un homebrew de la última semana, en `sd:/atmosphere/crash_reports/` (el cierre de otro homebrew puede ser uno de ellos: la lista que se muestra antes de enviar dice cuáles);
  - los registros de esta ejecución y de la anterior (`logs/sessions/`; si uno es largo: su principio y sus últimos 64 KiB);
  - los archivos de depuración: la referencia del bloque del temporizador, el final de la grabación del temporizador mientras *Registrar el temporizador de juego* esté activado, los 20 últimos cambios del historial (sin nombres de juegos, cuentas o perfiles, y solo con su fecha), y los ajustes de PlayGuard.
- **Iniciar sesión:** sin sesión iniciada, PlayGuard ofrece iniciarla antes; *Cuenta de GitHub*, justo debajo una vez iniciada la sesión, la cierra; si GitHub rechaza el gist, PlayGuard ofrece volver a iniciar sesión.
- **Después:** muestra el enlace y dos códigos QR: el informe, y el formulario de informe de error con el enlace, tus versiones, cómo se abrió PlayGuard (sobre un juego o desde el Álbum; añade el menú) y el sistema (sysMMC o emuMMC) ya rellenados. Cada enlace se guarda en `logs/uploads/` como acceso directo de Internet (`.url`), que un ordenador abre en su navegador.

> [!WARNING]
> Cualquiera que tenga el enlace puede leer el informe, hasta que borres el gist en GitHub, y un informe de error abierto con él es público: la confirmación lo dice.

### Lo que PlayGuard guarda en la tarjeta SD

Todo está en `sd:/config/playguard/logs/`, en texto plano que un ordenador puede abrir:

- `sessions/session_<date>.txt` — **un archivo por ejecución**: todo lo que PlayGuard registró (lo que antes solo llegaba a un PC por nxlink), cada línea con su hora y su nivel (`E` error, `W` aviso, `I` información), y una última línea que dice cómo terminó la ejecución: `=== end: clean exit ===` (salida normal), o `=== end: crash, see logs/crashes/… ===` (cierre inesperado). Un archivo deja de crecer al llegar a 1 MiB.
- `crashes/crash_<date>.txt` — **un archivo por cierre inesperado**, cuya primera línea dice qué pasó:
  - una **excepción de la CPU** (un puntero incorrecto, una instrucción no definida, un desbordamiento de pila): el tipo de fallo y la dirección afectada, la traza de llamadas y los registros. Las direcciones son desplazamientos dentro de `playguard.nro` (`playguard+0x1a2b3c`): el mapa del enlazador de la compilación (el artefacto `-linker-map` de su ejecución de CI) da la función. Después, PlayGuard termina como antes: la pantalla de error de la consola, y el informe de Atmosphère en `sd:/atmosphere/crash_reports/`;
  - una **excepción no capturada**: su tipo y su mensaje, y la traza de llamadas;
  - una **ejecución que no terminó**: se detecta en el siguiente inicio, cuando el archivo de sesión de la última ejecución no tiene línea final (un cierre que PlayGuard no pudo capturar, un cierre desde el menú HOME mientras estaba colgado, o la consola apagada), con las últimas 60 líneas de esa ejecución.

  Los archivos de cierres capturados mientras PlayGuard se ejecuta terminan con las últimas 80 líneas registradas antes del cierre.
- `reports/`, `uploads/`, `play_timer/` — los informes guardados, los enlaces de los informes enviados en línea, la grabación del temporizador de juego.

*Acerca de › Ayuda y diagnóstico* define cuántos informes, archivos de cierres, registros de sesión y enlaces se conservan (cada carpeta por separado; el más reciente siempre se queda).

## Modo desarrollador (investigar un firmware nuevo)

Pulsa Ⓨ siete veces en *Acerca de › Versión*; las herramientas de desarrollador aparecen al final de *Acerca de › Ayuda y diagnóstico*. Añade:

- un interruptor de **solo lectura**: activado, la aplicación no puede cambiar nada; es la forma segura de investigar un firmware nuevo (la pantalla del firmware lo ofrece directamente);
- el informe de diagnóstico en pantalla (Ⓨ lo guarda, Ⓧ lo envía en línea), y un acceso directo para exportarlo desde la pestaña Tiempo de juego;
- **Instalar otra compilación** en su lugar, para probar una corrección antes de que se publique ([más abajo](#instalar-otra-compilación));
- **Comparar bloque del temporizador** (solo mientras la aplicación móvil está vinculada y puede sincronizarse, no con un archivo hosts que bloquee a Nintendo; también en *PIN*), para descifrar ajustes que PlayGuard aún no muestra: guarda el bloque en bruto como referencia, cambia un ajuste en la aplicación móvil y vuelve: PlayGuard enumera los valores que han cambiado (guardados en `logs/reports/` si lo pides). Así se podrían confirmar horas de dormir distintas para cada día, configuradas desde la aplicación móvil. Adjunta ese archivo a una incidencia, o envíalo con el informe (*Enviar un informe en línea*).
- **Registrar el temporizador de juego**: cada 30 s mientras PlayGuard está abierto, una línea con lo que indica el temporizador (tiempo restante, tiempo jugado, el bloque de ajustes en bruto…) en `logs/play_timer/log.csv`, un archivo listo para una hoja de cálculo, más una línea justo después de cada cambio que hace PlayGuard, que dice cuál (su última columna, `event`), y una nota cuando el reloj se ha movido más que el tiempo transcurrido. Si se deja abierto pasada la medianoche, o hasta que se acabe el tiempo, muestra lo que un solo informe no puede: cuándo se pone a cero el tiempo jugado y qué dice la consola cerca del final. El interruptor se recuerda; solo registra en modo desarrollador. Lo que se sabe hasta ahora está en [parental-controls.md](../parental-controls.md) (en inglés).
- **Vaciar la caché** y **Eliminar los archivos de registro**, cada uno tras una confirmación que indica cuántos archivos y cuánto espacio: la caché (`cache/` y la `nand/<sistema>/cache/` de cada sistema: nombres e iconos de los juegos, las últimas cifras de Actividad, la lista de compilaciones) se vuelve a leer de la consola cuando hace falta; los archivos de registro son los archivos de cierres, los registros de sesión (menos el de esta ejecución, que aún se está escribiendo), los informes guardados y la grabación del temporizador de juego, mientras que `uploads/` (los enlaces para eliminar los informes enviados en línea) y la referencia del bloque se conservan. Los ajustes, el historial, las copias de seguridad, los perfiles y las exportaciones nunca se tocan.
- **Volver a ejecutar las migraciones**: los traslados de las organizaciones anteriores — la carpeta de datos junto al `.nro`, los registros de antes de sus subcarpetas, lo que compartían todos los sistemas —, cada uno en una línea para marcar o desmarcar (todos marcados al principio; A cambia uno), luego *Ejecutar las N elegidas* y una confirmación que las enumera con lo que mueve cada una. Se ejecutan al momento, diga lo que diga `migrations.json` (si no, cada una se ejecuta una sola vez, en el primer inicio de una compilación que la necesite; el nivel que registra nunca baja); un archivo que ya está en su sitio nunca se reemplaza. Un diálogo indica cuántos elementos se movieron; cada paso queda en el registro de sesión.
- **Estadísticas de imagen**: una línea por segundo en `logs/frame_stats.txt`, con el tiempo que tardó la pantalla en dibujarse y las imágenes que llegaron tarde, para comparar una pantalla con otra en la consola. **Halo de fondo** apaga la luz suave detrás de las pantallas (el modo de la línea termina entonces en `-noglow`), para medir lo que cuesta.

### Instalar otra compilación

*Instalar otra compilación* pone una compilación en lugar del propio `.nro` de PlayGuard, para probar una corrección antes de que se publique.

**Lo que enumera**, cada tipo en su propia pestaña (Ⓛ / Ⓡ, a ambos lados de las pestañas, pasan de una a otra; la pantalla se abre en la de la compilación instalada, y cada pestaña cuenta las compilaciones más recientes que la instalada):

- una de las 20 últimas versiones (versiones preliminares incluidas, etiquetadas);
- uno de los 20 últimos commits de `main`;
- la compilación más reciente de una pull request abierta desde una rama de este repositorio (nunca desde un fork: su código aún no se ha revisado). A en la pestaña de las pull requests, cuando se muestra, las ordena al revés (la compilación más reciente primero, o por número).

**Cada compilación** muestra su título (el asunto de un commit o de una pull request; el mensaje completo de un commit es su descripción) sobre etiquetas: el tipo de cambio (*Novedad*, *Corrección*, *Docs*…) y su ámbito, *Incompatible*, la pull request de la que viene un commit, cómo terminaron sus comprobaciones (*CI OK*, *CI fallida*, *CI en curso*), versión preliminar, *instalada* o *nueva*; su antigüedad y su commit a la derecha. Una compilación abre primero su página: etiquetas (su tipo, sus comprobaciones, instalada o nueva), una ficha de datos (fecha de compilación, tamaño, commit, suma de comprobación, cuándo la borra GitHub), **Instalar**, **Ver en GitHub** (su página como código QR para un móvil), y luego las notas de una versión o la descripción de una pull request, presentadas a partir del Markdown de GitHub (títulos, listas, casillas, código, citas y avisos, tablas).

**Sin riesgo al leer:** todo lo que viene de GitHub se muestra como texto plano: no se abre ningún enlace, no se ejecuta ningún HTML ni script, y se eliminan los caracteres invisibles que podrían disfrazar un título (inversiones de derecha a izquierda, caracteres de ancho cero, caracteres de control, UTF-8 no válido).

**Iniciar sesión.** Las versiones publicadas no necesitan nada; las demás son los artefactos del workflow de compilación, que GitHub solo entrega a usuarios con sesión iniciada. **Cuenta de GitHub** inicia sesión con un código y un código QR para escanear con un móvil. Los únicos permisos que se piden son leer los archivos del workflow de compilación (Actions) y crear gists, para *Enviar un informe en línea*; por lo demás, el token solo puede leer lo que es público. Se guarda en `github_token`, nunca se envía con un informe, y *Cuenta de GitHub* cierra la sesión. Con la sesión iniciada, Ⓨ (*Buscar*) acepta el número de una pull request (abierta o cerrada) o un commit (de 7 a 40 dígitos hexadecimales) y encuentra su compilación, solo desde una rama de este repositorio.

**Instalar:** PlayGuard descarga la compilación, la comprueba (tamaño, el SHA-256 que registra GitHub, la cabecera NRO), la pone en lugar de su propio `.nro` y se reinicia con ella (antes se pide el PIN siempre que haya uno definido).

- La lista se conserva 10 minutos y se muestra al instante, con su antigüedad (Ⓧ la vuelve a descargar); mientras se descarga, una barra muestra cuánto falta.
- Una descarga interrumpida (pérdida de red) continúa donde se quedó la próxima vez que se instala la misma compilación.
- Los artefactos caducan a los 90 días.

**Volver atrás.** La compilación sustituida se conserva (`playguard.nro.previous`): **Volver a la compilación anterior**, en la misma lista, las vuelve a intercambiar, tras el PIN. Si una compilación instalada desde aquí (o con *Actualizar* en PlayGuard) se cierra antes de su primera pantalla, el siguiente inicio de PlayGuard ofrece volver a la compilación anterior en una pantalla de texto simple, antes de cargar nada más (A vuelve atrás, B la inicia de todos modos), sin el PIN, ya que la propia pantalla del PIN puede ser lo que falla. La misma lista permite volver a cualquier versión publicada en cualquier momento.

**Qué compilación se ejecuta:** *Acerca de › Versión* muestra el commit en modo desarrollador. La cabecera de cada pantalla muestra la versión en uso junto al título: la de una versión publicada en una etiqueta gris (`1.2.0`), la de cualquier otra compilación en una etiqueta ámbar seguida de su commit, para que nunca se confunda una compilación de prueba con la versión publicada.

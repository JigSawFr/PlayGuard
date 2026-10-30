<!-- translated from docs/getting-started.md, sha256 6b46b5d26c07 -->
# Guía de inicio

De la tarjeta SD a una consola configurada para tu hijo o tu hija, en unos diez minutos, y después las tareas de cada día. Volver al [README](../../README.md).

## Antes de empezar

- **Una consola con Atmosphère.** PlayGuard maneja el control parental de la propia consola a través de un servicio restringido: necesita un firmware personalizado. ¿Consola sin modificar? No puede ayudar ([solución de problemas](troubleshooting.md#consola-sin-modificar)).
- **Un firmware compatible.** De 21.0.0 a 23.0.1 para todas las funciones; con los anteriores, todo menos el temporizador de juego; uno más reciente se abre en solo lectura hasta que una versión lo admita ([compatibilidad](compatibility.md)).
- **Un solo límite para toda la consola.** El temporizador de juego es el de la consola: es el mismo para todos los usuarios. Para tener límites distintos para cada niño hace falta otra herramienta ([por qué](compatibility.md#playguard-o-un-sysmodule-de-sustitución)).

¿Quieres echar un vistazo antes? [Prueba PlayGuard en el navegador](https://guard.boreale.dev/simulator/?not_set_up=1&utm_source=github&utm_medium=docs), con una consola simulada en su primer inicio.

## 1. Instálalo

Busca *PlayGuard* en la Homebrew App Store o en la App Store de sphaira e instálalo. O a mano: extrae `playguard.zip` de la [última versión](https://github.com/JigSawFr/PlayGuard/releases/latest) en la raíz de la tarjeta SD ([instalación](install.md)).

> [!TIP]
> Extrae también `playguard-rescue.zip`, el sysmodule de recuperación opcional. No hace nada hasta que lo necesitas, y es la única forma de volver a entrar si un límite de 0 minutos lo bloquea todo y has olvidado el PIN ([¿Consola bloqueada?](troubleshooting.md#consola-bloqueada)).

## 2. Ábrelo de la forma correcta

Abre PlayGuard **sobre un juego** (mantén pulsado R al iniciar un juego, con hbmenu, sphaira o la Homebrew App Store), con **el usuario de un padre o una madre**:

- sobre un juego, PlayGuard se ejecuta como aplicación, con toda su memoria; desde el álbum se ejecuta como applet, con menos;
- ese tiempo cuenta como tiempo de juego del niño, salvo que *Seguridad › Pausar el control parental mientras estoy en PlayGuard* esté activado (desactivado por defecto): entonces, una vez introducido el PIN, PlayGuard pone en pausa el control parental mientras estás en él, y se reactiva al salir.
- una vez introducido el PIN, PlayGuard pone en pausa el control parental mientras estás en él, para que ese tiempo no se descuente del del niño; se reactiva al salir.

## 3. Sigue *Primeros pasos*

En el primer inicio aparece la guía *Primeros pasos* (más adelante: *Preferencias › Primeros pasos*). *Hoy* muestra los mismos pasos en una tarjeta hasta que se completan (*1 de 3*):

1. **El PIN.** El PIN de la propia consola, que el menú HOME pide para iniciar un juego pasado el límite o para cambiar los ajustes. Evita 0000, 1234 o 2580: PlayGuard señala un PIN que cualquiera probaría primero.
2. **Un límite diario.** El mismo todos los días, o uno para entre semana y otro para el fin de semana. Cambia cualquier día suelto más adelante en *Tiempo de juego*.
3. **El reloj de red.** El temporizador de juego cuenta con él. Una consola que nunca llega a los servidores de Nintendo lo mantiene impreciso: consulta el paso 5.

## 4. Desvincula la aplicación móvil

¿Sigue vinculada a la aplicación *Control parental* de Nintendo (o, en una consola de segunda mano, a la de otra persona)? *Seguridad › Desvincular la app para móvil*. Si no, la próxima sincronización de la aplicación sobrescribe lo que hayas configurado en PlayGuard. Funciona sin conexión.

## 5. Ajusta el reloj de red

*Consola › Reloj de red › Medir el desfase del reloj* consulta tres servidores de hora públicos, y luego *Ajustar el reloj de red*. Ajustar el reloj pone a cero el tiempo de juego de hoy, así que hazlo antes de que juegue el niño.

Una consola sin Internet: cuando ningún servidor responde, PlayGuard ofrece ajustar el reloj de red al reloj de la consola. Comprueba antes el reloj de la consola con un móvil o un reloj, con *Configuración de la consola › Consola › Fecha y hora › Sincronizar el reloj por Internet* desactivado ([detalles](features.md#reloj-de-red)).

## 6. Elige qué pasa cuando se acaba el tiempo

*Tiempo de juego › Cuando se acabe el tiempo*: la consola muestra «Se acabó el tiempo» y luego **suspende el juego** o **solo avisa**. Suspender detiene el juego sin pelea, pero se puede perder el progreso sin guardar; avisar se lo deja al niño. Añade una **alarma de hora de dormir** en la misma pestaña si quieres.

## 7. Protege el propio PlayGuard

*Seguridad › Proteger PlayGuard con el PIN*:

- **Antes de un cambio** (lo predeterminado) — cualquiera puede mirar, solo el padre o la madre cambia algo;
- **Para abrir PlayGuard** — una pantalla de bloqueo al iniciar;
- **Nunca** — PlayGuard lo señala en la campana mientras haya un PIN definido.

## 8. Entrega la consola

*Hoy › Mostrar al niño* muestra solo el tiempo restante y cuándo termina el juego, lo bastante grande para leerlo desde el otro lado de la habitación. Ⓐ y el PIN devuelven las pantallas del padre o la madre.

> [!IMPORTANT]
> **Un juego que se deja abierto en el menú HOME sigue contando.** Ciérralo o pon la consola en modo de reposo ([qué cuenta](features.md#qué-cuenta-como-tiempo-de-juego)).

## Tareas de cada día

| Para… | Dónde |
|---|---|
| Dar más tiempo hoy | *Hoy › Tiempo extra* (+15 min, +30 min, +1 h). Se retira solo al día siguiente. |
| Dar más tiempo otro día | *Hoy › Tiempo extra › Otro día…* |
| Terminar por hoy | *Hoy › Terminar por hoy*: ahora, o dentro de 5 minutos para guardar. |
| Cambiar el límite de un día | *Tiempo de juego*: Ⓐ en el día, ↑/↓, y luego **+** para guardar. |
| Pasar del curso escolar a las vacaciones | *Tiempo de juego › Perfiles*. |
| Ver quién ha jugado a qué | *Actividad*: por juego, por cuenta, hoy, 7 días, desde siempre; exportación. |
| Bloquear todos los juegos por ahora | *Tiempo de juego › Bloquear todos los juegos*. Al desactivarlo, vuelven los límites anteriores. |
| Deshacer un cambio | **ZR** *Deshacer* justo después, o *Historial › Cambios hechos con PlayGuard* en cualquier momento. |
| Guardar una copia de los ajustes | *Historial › Hacer una copia de los ajustes* (nunca el PIN). |
| Saber qué requiere tu atención | La campana (Ⓨ): lo que hay que hacer en ámbar, lo que conviene saber con una «i». |

## Y después

- Cada pestaña y cada ajuste: [funciones](features.md).
- Algo no funciona como se espera: [solución de problemas](troubleshooting.md#problemas-habituales).
- Lo que hace la consola, y con qué certeza lo sabemos: [cómo funciona el control parental](../parental-controls.md) (en inglés).

<!-- translated from docs/getting-started.md, sha256 150621760428 -->
# Primeros pasos

De la tarjeta SD a una consola configurada para su hijo, en unos diez minutos, y luego las tareas de todos los días. Volver al [README](../../README.md).

## Antes de empezar

- **Una consola con Atmosphère.** PlayGuard maneja el control parental de la propia consola a través de un servicio restringido: necesita un firmware personalizado. ¿La consola no está modificada? No puede ayudar ([solución de problemas](troubleshooting.md#consola-sin-modificar)).
- **Un firmware compatible.** De 21.0.0 a 23.0.1 para todas las funciones; los anteriores, todo menos el temporizador de juego; uno más reciente se abre en solo lectura hasta que una versión sea compatible con él ([compatibilidad](compatibility.md)).
- **Un solo límite para toda la consola.** El temporizador de juego es el de la consola: es el mismo para todos los usuarios. Para tener límites distintos para cada niño se necesita otra herramienta ([por qué](compatibility.md#playguard-o-un-sysmodule-de-reemplazo)).

¿Quiere echar un vistazo primero? [Pruebe PlayGuard en su navegador](https://playguard.link/simulator/?not_set_up=1), con una consola simulada en su primer inicio.

## 1. Instalarlo

Extraiga `playguard.zip` de la [última versión](https://github.com/JigSawFr/PlayGuard/releases/latest) en la raíz de la tarjeta SD ([instalación](install.md)). PlayGuard todavía no está en el Homebrew App Store ni en sphaira: su ficha sigue pendiente de aprobación.

> [!TIP]
> Extraiga también `playguard-rescue.zip`, el sysmodule de recuperación opcional. No hace nada hasta que lo necesite, y es la única forma de volver a entrar si un límite de 0 minutos lo bloquea todo y se olvidó el PIN ([¿Consola bloqueada?](troubleshooting.md#consola-bloqueada)).

## 2. Abrirlo de la forma correcta

Abra PlayGuard **sobre un juego** (mantenga presionado R al iniciar un juego, con hbmenu, sphaira o el Homebrew App Store), con **el usuario de un adulto**:

- sobre un juego, PlayGuard se ejecuta como aplicación, con toda su memoria; desde el Álbum se ejecuta como applet, con menos;
- la consola cuenta el tiempo que se pasa en PlayGuard como tiempo de ese juego, y se lo asigna al usuario elegido al iniciar: el de un adulto, no el de un niño;
- una vez ingresado el PIN, PlayGuard pone en pausa el control parental mientras está en él, para que ese tiempo no se descuente del tiempo del niño; se reactiva al salir.

## 3. Siga *Primeros pasos*

En el primer inicio aparece la guía *Primeros pasos* (más adelante: *Preferencias › Primeros pasos*). *Hoy* muestra los mismos pasos como una tarjeta hasta que se completan (*1 de 3*):

1. **El PIN.** El propio PIN de la consola, que el menú HOME pide para iniciar un juego después del límite o para cambiar la configuración. Evite 0000, 1234 o 2580: PlayGuard señala un PIN que cualquiera probaría primero.
2. **Un límite diario.** El mismo todos los días, o uno entre semana y otro para el fin de semana. Cualquier día se puede cambiar después en *Tiempo de juego*.
3. **El reloj de red.** El temporizador de juego cuenta con él. Una consola que nunca llega a los servidores de Nintendo lo mantiene impreciso: consulte el paso 5.

## 4. Desvincular la app para celular

¿Sigue vinculada a la app *Control parental* de Nintendo (o es una consola de segunda mano vinculada a la de otra persona)? *Seguridad › Desvincular la app para celular*. Si no, la próxima sincronización de la app sobrescribe lo que configure en PlayGuard. Funciona sin conexión.

## 5. Ajustar el reloj de red

*Consola › Reloj de red › Medir la diferencia horaria* consulta tres servidores de hora públicos, y luego *Ajustar el reloj de red*. Ajustar el reloj pone a cero el tiempo de juego de hoy, así que hágalo antes de que el niño juegue.

Una consola sin Internet: cuando ningún servidor responde, PlayGuard ofrece ajustar el reloj de red al reloj de la consola. Antes, compare el reloj de la consola con un celular o un reloj, con *Configuración de la consola › Consola › Fecha y hora › Sincronizar el reloj por Internet* desactivado ([detalles](features.md#ningún-servidor-responde)).

## 6. Elegir qué pasa cuando se acaba el tiempo

*Tiempo de juego › Cuando se acabe el tiempo*: la consola muestra “Se acabó el tiempo” y luego **suspende el juego** o **solo avisa**. Suspender detiene el juego sin pelear, pero el progreso no guardado puede perderse; avisar deja la decisión al niño. Si quiere, agregue una **alarma de hora de dormir** en la misma pestaña.

## 7. Proteger el propio PlayGuard

*Seguridad › Proteger PlayGuard con el PIN*:

- **Antes de un cambio** (lo predeterminado): cualquiera puede mirar, solo el adulto cambia algo;
- **Para abrir PlayGuard**: una pantalla de bloqueo al iniciar;
- **Nunca**: PlayGuard lo señala en la campana mientras haya un PIN definido.

## 8. Entregar la consola

*Hoy › Mostrar al niño* muestra solo el tiempo restante y cuándo termina el juego, lo bastante grande para leerse desde el otro lado de la habitación. Ⓐ y el PIN devuelven las pantallas del adulto.

> [!IMPORTANT]
> **Un juego que se deja abierto en el menú HOME sigue contando.** Ciérrelo, o ponga la consola en modo de suspensión ([qué cuenta](features.md#qué-cuenta-como-tiempo-de-juego)).

## Tareas de todos los días

| Para… | Dónde |
|---|---|
| Dar más tiempo hoy | *Hoy › Tiempo extra* (+15 min, +30 min, +1 h). Se restablece solo al día siguiente. |
| Dar más tiempo otro día | *Hoy › Tiempo extra › Otro día…* |
| Terminar por hoy | *Hoy › Terminar por hoy*: ahora, o en 5 minutos para guardar. |
| Cambiar el límite de un día | *Tiempo de juego*: Ⓐ en el día, ↑/↓, y luego **+** para guardar. |
| Pasar del período escolar a las vacaciones | *Tiempo de juego › Perfiles*; *Perfil programado* para un período elegido de antemano. |
| Ver quién jugó a qué | *Actividad*: por juego, por cuenta, hoy, 7 días, en total; exportación o un código QR. |
| Bloquear todos los juegos por ahora | *Tiempo de juego › Bloquear todos los juegos*. Al desactivarlo, vuelven los límites anteriores. |
| Deshacer un cambio | **ZR** *Deshacer* justo después, o *Historial › Cambios hechos con PlayGuard* en cualquier momento. |
| Guardar una copia de la configuración | *Historial › Respaldar la configuración* (nunca el PIN). |
| Saber qué requiere su atención | La campana (Ⓨ): lo que hay que hacer en ámbar, lo que conviene saber con una “i”. |

## Siguiente

- Cada pestaña y cada opción: [funciones](features.md).
- Algo no funciona como se esperaba: [solución de problemas](troubleshooting.md#problemas-frecuentes).
- Qué hace la consola, y con qué certeza lo sabemos: [cómo funciona el control parental](../parental-controls.md) (en inglés).

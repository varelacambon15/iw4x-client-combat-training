# Advanced Combat Training para IW4x

El multijugador online de Modern Warfare 2 (2009), sin conexión y contra bots: listas de juego como las del online,
experiencia, rangos, desbloqueos, retos y prestigio, salas que se llenan de jugadores (bots) que entran y salen,
y una dificultad que se adapta a cómo juegas.

## Qué necesitas

- **Call of Duty: Modern Warfare 2 (2009) de Steam**, con el multijugador instalado.
- En SteamOS, el juego configurado con Proton (lo hace Steam solo, al ser un juego de Windows).

## Instalar en la KONKR (o en cualquier SteamOS / Linux)

1. Descomprime `AdvancedCombatTraining.zip` en modo escritorio.
2. Abre una terminal (Konsole) en la carpeta `AdvancedCombatTraining` y ejecuta:
   ```
   bash install_konkr.sh
   ```
   El script encuentra el juego, instala IW4x si no lo tienes y copia Combat Training.
3. En Steam, en **Call of Duty: Modern Warfare 2 - Multiplayer → Propiedades → General → Opciones de lanzamiento**, pega
   la línea que te muestra el script:
   ```
   bash -c 'exec "${@/iw4mp.exe/iw4x.exe}"' -- %command% +set fs_game mods/advanced_combat_training
   ```
   Si usas Konkr Tweaks, pon `~/.local/bin/kt-run ` delante.

## Instalar en Windows

Copia `iw4x.dll` y la carpeta `mods` en la carpeta del juego (con IW4x ya instalado) y arranca
`iw4x.exe +set fs_game mods/advanced_combat_training`. No uses el lanzador de IW4x: al actualizar, sustituiría `iw4x.dll`.

## Jugar

En el menú principal, **ADVANCED COMBAT TRAINING** → elige una lista de juego. La partida empieza sola, y al terminar verás
cuál es la siguiente: el juego pasa de una a otra como en el online. Para dejarlo, en la pausa elige **End Game**
(Terminar partida) y vuelves al menú principal.

- Tu rango, clases, retos y prestigio son los de tu perfil de IW4x.
- Los bots usan un nivel parecido al tuyo. Si ganas con facilidad mejoran, si lo pasas mal aflojan.
  Esa valoración se guarda en `mods/advanced_combat_training/scriptdata/ct_mmr.txt` (se conserva al actualizar).
- Mapas: los 16 originales, más los de los DLC Stimulus y Resurgence si los tienes instalados (si faltan, no salen).

## Supply Drops y variantes de armas

Lo nuevo frente a Combat Training, como en Advanced Warfare. Todo está en el menú principal, en **SUPPLY DROPS** (lleva
el icono de "nuevo" cuando tienes cajas sin abrir):

- **Abrir:** la caja se abre con su animación y salen tres variantes, la mejor al final, cada una en su tarjeta con la
  foto del arma, la rareza (**Enlisted** verde, **Professional** azul, **Elite** dorada) y sus estadísticas: las barras
  del juego con lo que la variante sube (verde) o baja (rojo), y la lista de cambios.
- **Qué cambian:** daño (general, a corta o a larga distancia), daño en la cabeza, movilidad y munición de reserva. Más
  rareza, más ventajas; a veces con una desventaja. Las Advanced traen siempre una Professional o mejor.
- **Armory:** tus armas con variantes; en cada una eliges cuál equipar o la desguazas (**Salvage**) por Credits. También
  puedes desguazar de golpe todas las Enlisted o Professional que no tengas equipadas.
- **Armas bloqueadas:** si te toca una variante de un arma que aún no has desbloqueado por nivel, esa arma queda
  disponible para ti desde ya, como en Advanced Warfare.
- **Crear una clase:** al elegir un arma de la que tienes variantes, sale un paso más para escoger la variante (o
  ninguna) antes de los accesorios. Va una variante por arma y vale para todas las clases.
- **Store y contratos:** en la misma pantalla compras Supply Drops con Credits y ves tus contratos (3 diarios y 1
  semanal) con su progreso.
- **Cómo se consiguen:** una por cada 30 minutos jugados, una por nivel, dos Advanced por prestigio, una por la primera
  victoria del día y por los contratos. Credits al final de cada partida, al abrir cajas y al desguazar. Empiezas con una.
- **Tu progreso** se guarda en `players/supply_drops/profile.txt`, en la carpeta del juego.
- **Ajustes:** el ritmo, las probabilidades y los precios están en `mods/advanced_combat_training/supply_drops/config.txt`.

## Ajustes

`mods/advanced_combat_training/combat_training/user.cfg` se conserva al actualizar. Por ejemplo, `set scr_xpscale 2` para doble
experiencia, o `set bots_main_chat 0` para que los bots no escriban en el chat.

Las listas de juego están en `mods/advanced_combat_training/combat_training/playlists.txt` y se pueden editar.

## Créditos

- [IW4x](https://github.com/iw4x/iw4x-client): el cliente sobre el que está hecho.
- [Bot Warfare](https://github.com/ineedbots/iw4_bot_warfare), de INeedBots: los bots.

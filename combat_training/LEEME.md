# Combat Training para IW4x

El multijugador online de Modern Warfare 2 (2009), sin conexión y contra bots: listas de juego como las del online,
experiencia, rangos, desbloqueos, retos y prestigio, salas que se llenan de jugadores (bots) que entran y salen,
y una dificultad que se adapta a cómo juegas.

## Qué necesitas

- **Call of Duty: Modern Warfare 2 (2009) de Steam**, con el multijugador instalado.
- En SteamOS, el juego configurado con Proton (lo hace Steam solo, al ser un juego de Windows).

## Instalar en la KONKR (o en cualquier SteamOS / Linux)

1. Descomprime `CombatTraining.zip` en modo escritorio.
2. Abre una terminal (Konsole) en la carpeta `CombatTraining` y ejecuta:
   ```
   bash install_konkr.sh
   ```
   El script encuentra el juego, instala IW4x si no lo tienes y copia Combat Training.
3. En Steam, en **Call of Duty: Modern Warfare 2 - Multiplayer → Propiedades → General → Opciones de lanzamiento**, pega
   la línea que te muestra el script:
   ```
   bash -c 'exec "${@/iw4mp.exe/iw4x.exe}"' -- %command% +set fs_game mods/combat_training
   ```
   Si usas Konkr Tweaks, pon `~/.local/bin/kt-run ` delante.

## Instalar en Windows

Copia `iw4x.dll` y la carpeta `mods` en la carpeta del juego (con IW4x ya instalado) y arranca
`iw4x.exe +set fs_game mods/combat_training`. No uses el lanzador de IW4x: al actualizar, sustituiría `iw4x.dll`.

## Jugar

En el menú principal, **COMBAT TRAINING** → elige una lista de juego. La partida empieza sola, y al terminar verás
cuál es la siguiente: el juego pasa de una a otra como en el online. Para dejarlo, en la pausa elige **End Game**
(Terminar partida) y vuelves al menú principal.

- Tu rango, clases, retos y prestigio son los de tu perfil de IW4x.
- Los bots usan un nivel parecido al tuyo. Si ganas con facilidad mejoran, si lo pasas mal aflojan.
  Esa valoración se guarda en `mods/combat_training/scriptdata/ct_mmr.txt` (se conserva al actualizar).
- Mapas: los 16 originales, más los de los DLC Stimulus y Resurgence si los tienes instalados (si faltan, no salen).

## Ajustes

`mods/combat_training/combat_training/user.cfg` se conserva al actualizar. Por ejemplo, `set scr_xpscale 2` para doble
experiencia, o `set bots_main_chat 0` para que los bots no escriban en el chat.

Las listas de juego están en `mods/combat_training/combat_training/playlists.txt` y se pueden editar.

## Créditos

- [IW4x](https://github.com/iw4x/iw4x-client): el cliente sobre el que está hecho.
- [Bot Warfare](https://github.com/ineedbots/iw4_bot_warfare), de INeedBots: los bots.

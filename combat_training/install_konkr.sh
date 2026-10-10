#!/usr/bin/env bash
# Combat Training: instala IW4x con Combat Training en el MW2 (2009) de Steam, en SteamOS (KONKR Pocket FIT,
# Steam Deck o cualquier Linux con Steam). Ejecutar desde la carpeta descomprimida:
#     bash install_konkr.sh
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
GAME_DIR_NAME="Call of Duty Modern Warfare 2"
RAWFILES_URL="https://github.com/iw4x/iw4x-rawfiles/releases/latest/download/release.zip"
LAUNCH_OPTIONS="bash -c 'exec \"\${@/iw4mp.exe/iw4x.exe}\"' -- %command% +set fs_game mods/combat_training"

say() { printf '\033[1;33m==>\033[0m %s\n' "$*"; }
fail() { printf '\033[1;31mError:\033[0m %s\n' "$*" >&2; exit 1; }

[ -f "$HERE/iw4x.dll" ] && [ -d "$HERE/mods/combat_training" ] || fail "ejecuta este script desde la carpeta CombatTraining descomprimida"

# --- La carpeta del juego: en cualquiera de las bibliotecas de Steam ---------------------------------------------
say "Buscando Modern Warfare 2 en tus bibliotecas de Steam..."
libraries=()
for root in "$HOME/.local/share/Steam" "$HOME/.steam/steam" "$HOME/.steam/root"; do
    vdf="$root/steamapps/libraryfolders.vdf"
    [ -f "$vdf" ] || continue
    libraries+=("$root")
    while IFS= read -r path; do
        libraries+=("$path")
    done < <(grep -oP '"path"\s+"\K[^"]+' "$vdf")
done

GAME=""
for lib in "${libraries[@]}"; do
    candidate="$lib/steamapps/common/$GAME_DIR_NAME"
    if [ -f "$candidate/iw4mp.exe" ]; then
        GAME="$candidate"
        break
    fi
done
[ -n "$GAME" ] || fail "no encuentro '$GAME_DIR_NAME' con iw4mp.exe. Instala el MW2 (2009) en Steam (incluido el multijugador) y vuelve a ejecutar el script."
say "Juego encontrado en: $GAME"

# --- IW4x: los archivos de la comunidad, si aún no están -----------------------------------------------------------
if [ ! -f "$GAME/iw4x.exe" ] || [ ! -d "$GAME/iw4x" ]; then
    say "Descargando IW4x (archivos del juego de la comunidad)..."
    tmp="$(mktemp -d)"
    trap 'rm -rf "$tmp"' EXIT
    curl -fL --progress-bar -o "$tmp/release.zip" "$RAWFILES_URL" || fail "no se pudo descargar IW4x desde $RAWFILES_URL"
    unzip -oq "$tmp/release.zip" -d "$GAME"
else
    say "IW4x ya está instalado"
fi

# --- Combat Training ----------------------------------------------------------------------------------------------
if [ -f "$GAME/iw4x.dll" ] && [ ! -f "$GAME/iw4x.dll.original" ]; then
    cp "$GAME/iw4x.dll" "$GAME/iw4x.dll.original"
    say "Copia de seguridad del iw4x.dll original: iw4x.dll.original"
fi
cp "$HERE/iw4x.dll" "$GAME/iw4x.dll"

mkdir -p "$GAME/mods"
# tus ajustes y tu valoración se conservan entre actualizaciones
keep=("combat_training/user.cfg" "scriptdata/ct_mmr.txt")
saved="$(mktemp -d)"
for f in "${keep[@]}"; do
    if [ -f "$GAME/mods/combat_training/$f" ]; then
        mkdir -p "$saved/$(dirname "$f")"
        cp "$GAME/mods/combat_training/$f" "$saved/$f"
    fi
done
rm -rf "$GAME/mods/combat_training"
cp -r "$HERE/mods/combat_training" "$GAME/mods/"
cp -r "$saved/." "$GAME/mods/combat_training/"
rm -rf "$saved"
say "Combat Training instalado"

# --- Opciones de lanzamiento ---------------------------------------------------------------------------------------
echo
say "Último paso, en Steam:"
echo "   1. Biblioteca -> Call of Duty: Modern Warfare 2 - Multiplayer -> Propiedades -> General."
echo "   2. En 'Opciones de lanzamiento' pega esta línea:"
echo
echo "      $LAUNCH_OPTIONS"
echo
echo "   (Si usas Konkr Tweaks, pon ~/.local/bin/kt-run delante de bash.)"
if command -v wl-copy >/dev/null 2>&1; then
    printf '%s' "$LAUNCH_OPTIONS" | wl-copy && echo "   La línea ya está copiada en el portapapeles."
elif command -v xclip >/dev/null 2>&1; then
    printf '%s' "$LAUNCH_OPTIONS" | xclip -selection clipboard && echo "   La línea ya está copiada en el portapapeles."
fi
echo
echo "   Al abrir el multijugador verás COMBAT TRAINING arriba del menú principal."

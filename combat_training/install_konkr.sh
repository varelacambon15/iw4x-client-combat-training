#!/usr/bin/env bash
# Combat Training: instala IW4x con Combat Training en el MW2 (2009) de Steam, en SteamOS (KONKR Pocket FIT,
# Steam Deck o cualquier Linux con Steam). Ejecutar desde la carpeta descomprimida:
#     bash install_konkr.sh                      (busca el juego en tus bibliotecas de Steam)
#     bash install_konkr.sh "/ruta/al/juego"     (el juego está en otra carpeta)
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
GAME_DIR_NAME="Call of Duty Modern Warfare 2"
# Which mod this package holds: combat_training or advanced_combat_training (the file MOD next to this script)
MOD="$(cat "$HERE/MOD" 2>/dev/null | tr -d '[:space:]')"
MOD="${MOD:-combat_training}"
RAWFILES_URL="https://github.com/iw4x/iw4x-rawfiles/releases/latest/download/release.zip"
LAUNCH_OPTIONS="bash -c 'exec \"\${@/iw4mp.exe/iw4x.exe}\"' -- %command% +set fs_game mods/$MOD"

say() { printf '\033[1;33m==>\033[0m %s\n' "$*"; }
fail() { printf '\033[1;31mError:\033[0m %s\n' "$*" >&2; exit 1; }

[ -f "$HERE/iw4x.dll" ] && [ -d "$HERE/mods/$MOD" ] || fail "ejecuta este script desde la carpeta del paquete descomprimida"

# --- La carpeta del juego: la indicada, o en cualquiera de las bibliotecas de Steam -------------------------------
GAME=""
if [ $# -ge 1 ]; then
    GAME="${1%/}"
    [ -d "$GAME" ] || fail "no existe la carpeta '$GAME'"
    [ -f "$GAME/iw4mp.exe" ] || [ -f "$GAME/iw4x.exe" ] || fail "en '$GAME' no están ni iw4mp.exe ni iw4x.exe: tiene que ser la carpeta del MW2 (2009)"
    [ -d "$GAME/main" ] || fail "en '$GAME' falta la carpeta 'main' con los archivos del juego: IW4x necesita el MW2 completo de Steam, no solo los archivos de IW4x"
fi

if [ -z "$GAME" ]; then
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
[ -n "$GAME" ] || fail "no encuentro '$GAME_DIR_NAME' con iw4mp.exe. Instala el MW2 (2009) en Steam (incluido el multijugador), o indica la carpeta: bash install_konkr.sh \"/ruta/al/juego\""
fi
say "Juego en: $GAME"
STEAM_GAME=0
case "$GAME" in */steamapps/common/*) STEAM_GAME=1 ;; esac

# --- IW4x: los archivos de la comunidad, si aún no están -----------------------------------------------------------
if ! ls "$GAME" | grep -qix "iw4x.exe"; then
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
    if [ -f "$GAME/mods/$MOD/$f" ]; then
        mkdir -p "$saved/$(dirname "$f")"
        cp "$GAME/mods/$MOD/$f" "$saved/$f"
    fi
done
rm -rf "$GAME/mods/$MOD"
cp -r "$HERE/mods/$MOD" "$GAME/mods/"
cp -r "$saved/." "$GAME/mods/$MOD/"
rm -rf "$saved"
say "Instalado: mods/$MOD"

# --- Opciones de lanzamiento ---------------------------------------------------------------------------------------
echo
if [ "$STEAM_GAME" = 1 ]; then
    say "Último paso, en Steam:"
    echo "   1. Biblioteca -> Call of Duty: Modern Warfare 2 - Multiplayer -> Propiedades -> General."
    echo "   2. En 'Opciones de lanzamiento' pega esta línea:"
    echo
    echo "      $LAUNCH_OPTIONS"
    echo
    echo "   (Si usas Konkr Tweaks, pon ~/.local/bin/kt-run delante de bash.)"
    copy="$LAUNCH_OPTIONS"
else
    say "Último paso, en Steam (el juego no está en una biblioteca de Steam, así que se añade aparte):"
    echo "   1. Juegos -> Añadir un juego que no es de Steam -> Explorar, y elige:"
    echo "      $GAME/iw4x.exe"
    echo "   2. En el juego añadido: Propiedades -> Compatibilidad -> Forzar una herramienta de compatibilidad -> Proton."
    echo "   3. En Propiedades -> General -> 'Opciones de lanzamiento' pega:"
    echo
    echo "      +set fs_game mods/$MOD"
    echo
    echo "   Steam tiene que estar abierto y con el MW2 en tu cuenta: IW4x lo comprueba al arrancar."
    copy="+set fs_game mods/$MOD"
fi
if command -v wl-copy >/dev/null 2>&1; then
    printf '%s' "$copy" | wl-copy && echo "   Las opciones de lanzamiento ya están copiadas en el portapapeles."
elif command -v xclip >/dev/null 2>&1; then
    printf '%s' "$copy" | xclip -selection clipboard && echo "   Las opciones de lanzamiento ya están copiadas en el portapapeles."
fi
echo
echo "   Al abrir el juego verás COMBAT TRAINING arriba del menú principal."

#!/usr/bin/env bash
#                _ _
# __      ____ _| | |_ __   __ _ _ __   ___ _ __
# \ \ /\ / / _` | | | '_ \ / _` | '_ \ / _ \ '__|
#  \ V  V / (_| | | | |_) | (_| | |_) |  __/ |
#   \_/\_/ \__,_|_|_| .__/ \__,_| .__/ \___|_|
#                   |_|         |_|
#
# -----------------------------------------------------
# Restore last wallpaper
# -----------------------------------------------------

# -----------------------------------------------------
# Set defaults
# -----------------------------------------------------

ml4w_cache_folder="$HOME/.cache/ml4w/hyprland-dotfiles"

defaultwallpaper="$HOME/.config/ml4w/wallpapers/default.jpg"

cachefile="$ml4w_cache_folder/current_wallpaper"

# -----------------------------------------------------
# Get current wallpaper
# -----------------------------------------------------

if [ -f "$cachefile" ]; then
    sed -i "s|~|$HOME|g" "$cachefile"
    wallpaper=$(cat $cachefile)
    if [ -f $wallpaper ]; then
        echo ":: Wallpaper $wallpaper exists"
    else
        echo ":: Wallpaper $wallpaper does not exist. Using default."
        wallpaper=$defaultwallpaper
    fi
else
    echo ":: $cachefile does not exist. Using default wallpaper."
    wallpaper=$defaultwallpaper
fi

# -----------------------------------------------------
# Set wallpaper
# -----------------------------------------------------

echo ":: Setting wallpaper with source image $wallpaper"
if [ -f ~/.local/bin/waypaper ]; then
    export PATH=$PATH:~/.local/bin/
fi

backend=""
waypaper_config="$HOME/.config/waypaper/config.ini"
if [ -f "$waypaper_config" ]; then
    backend=$(awk -F'=' '/^[[:space:]]*backend[[:space:]]*=/{gsub(/[[:space:]]/, "", $2); print tolower($2); exit}' "$waypaper_config")
fi

if [ -n "${XDG_RUNTIME_DIR:-}" ] && [ -n "${HYPRLAND_INSTANCE_SIGNATURE:-}" ]; then
    hypr_socket="$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket.sock"
    socket_wait=20
    while [ ! -S "$hypr_socket" ] && [ "$socket_wait" -gt 0 ]; do
        sleep 0.25
        socket_wait=$((socket_wait - 1))
    done
fi

if [ "$backend" = "swww" ] && ! pgrep -x swww-daemon >/dev/null 2>&1; then
    echo ":: Starting swww-daemon"
    swww-daemon >/dev/null 2>&1 &
    sleep 0.5
fi

if [ "$backend" = "awww" ] && ! pgrep -x awww-daemon >/dev/null 2>&1; then
    echo ":: Starting awww-daemon"
    awww-daemon >/dev/null 2>&1 &
    sleep 0.5
fi

max_attempts=5
attempt=1
while [ "$attempt" -le "$max_attempts" ]; do
    if waypaper --wallpaper "$wallpaper"; then
        echo ":: Wallpaper restored (attempt $attempt/$max_attempts)"
        exit 0
    fi
    echo ":: Wallpaper restore failed (attempt $attempt/$max_attempts)"
    if [ "$backend" = "swww" ] && ! pgrep -x swww-daemon >/dev/null 2>&1; then
        echo ":: Restarting swww-daemon"
        swww-daemon >/dev/null 2>&1 &
    fi
    if [ "$backend" = "awww" ] && ! pgrep -x awww-daemon >/dev/null 2>&1; then
        echo ":: Restarting awww-daemon"
        awww-daemon >/dev/null 2>&1 &
    fi
    sleep 1
    attempt=$((attempt + 1))
done

echo ":: ERROR: Unable to restore wallpaper after $max_attempts attempts" >&2
exit 1

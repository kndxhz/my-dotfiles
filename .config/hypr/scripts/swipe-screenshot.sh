#!/usr/bin/env bash
set -eu

state_dir="$HOME/.cache/hypr"
lock_file="$state_dir/swipe-screenshot.lock"

mkdir -p "$state_dir"

# Debounce duplicate swipe events.
now_ms="$(date +%s%3N)"
if [ -f "$lock_file" ]; then
    prev_ms="$(cat "$lock_file" 2>/dev/null || echo 0)"
    if [ $((now_ms - prev_ms)) -lt 700 ]; then
        exit 0
    fi
fi
echo "$now_ms" > "$lock_file"

action="save"
mode="active"

for arg in "$@"; do
    case "$arg" in
        copy|save|copysave|edit) action="$arg" ;;
        active|screen|output|area) mode="$arg" ;;
    esac
done

case "$mode" in
    active|screen|output|area) ;;
    *) mode="active" ;;
esac

if [ "$action" = "copy" ]; then
    /usr/bin/grimblast --notify copy "$mode"
    exit 0
fi

save_dir_raw="$(cat "$HOME/.config/ml4w/settings/screenshot-folder")"
save_name_raw="$(cat "$HOME/.config/ml4w/settings/screenshot-filename")"

eval "save_dir=$save_dir_raw"
eval "save_name=$save_name_raw"

mkdir -p "$save_dir"

/usr/bin/grimblast --notify "$action" "$mode" "$save_dir/$save_name"

#!/usr/bin/env bash

set -euo pipefail

trackpoint_node="/sys/devices/platform/i8042/serio1/drvctl"
state_file="${XDG_RUNTIME_DIR:-/tmp}/trackpoint-disabled"

if [[ ! -w "$trackpoint_node" ]]; then
    notify-send "TrackPoint" "No writable TrackPoint control found"
    exit 1
fi

if [[ -e "$state_file" ]]; then
    pkexec /bin/sh -c "printf 'none' > '$trackpoint_node'"
    rm -f "$state_file"
    notify-send "TrackPoint" "Enabled"
else
    pkexec /bin/sh -c "printf 'psmouse' > '$trackpoint_node'"
    : > "$state_file"
    notify-send "TrackPoint" "Disabled"
fi

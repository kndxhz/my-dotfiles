#!/usr/bin/env bash
set -euo pipefail
directory=$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")
source_file="$directory/workspaces.c"
library="$directory/workspaces.so"
if [[ -f "$library" && "$library" -nt "$source_file" && "$library" -nt "$directory/responsive-media.h" ]]; then
    exit 0
fi
temporary=$(mktemp "$directory/.workspaces.XXXXXX.so")
trap 'rm -f -- "$temporary"' EXIT
cc -shared -fPIC -O2 -Wall -Wextra -Werror "$source_file" -o "$temporary" \
    $(pkg-config --cflags --libs gtk+-3.0 json-glib-1.0)
mv -f -- "$temporary" "$library"

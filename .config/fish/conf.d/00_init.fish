# -----------------------------------------------------
# INIT
# -----------------------------------------------------

set -U fish_greeting ""

# -----------------------------------------------------
# Exports
# -----------------------------------------------------
# Wayland compatibility: don't force a GTK input method module.
# Let toolkits pick the native frontend instead.
if set -q GTK_IM_MODULE
    set -e GTK_IM_MODULE
end
if set -q -U GTK_IM_MODULE
    set -eU GTK_IM_MODULE
end

export EDITOR=nvim

set -U fish_user_paths /usr/lib/ccache/bin/
set -U fish_user_paths $fish_user_paths $HOME/.cargo/bin/
set -U fish_user_paths $fish_user_paths $HOME/.local/bin/
# -----------------------------------------------------
# AUTOSTART
# -----------------------------------------------------

# -----------------------------------------------------
# Fastfetch
# -----------------------------------------------------
# Run fastfetch on interactive shells, but don't auto-run inside kitty.
if status is-interactive
    set -l in_kitty 0

    if set -q KITTY_WINDOW_ID
        set in_kitty 1
    else if set -q TERM; and test "$TERM" = "xterm-kitty"
        set in_kitty 1
    end

    if test $in_kitty -eq 0
        # fastfetch
    end
end
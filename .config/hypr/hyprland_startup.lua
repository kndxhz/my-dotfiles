local home = os.getenv("HOME") or ""

local startupCommands = {
    "hyprctl setcursor Bibata-Modern-Ice 24",
    home .. "/.config/ml4w/listeners.sh --startall",
    "/usr/lib/polkit-gnome/polkit-gnome-authentication-agent-1",
    home .. "/.config/hypr/scripts/wallpaper-restore.sh",
    "swaync",
    home .. "/.config/hypr/scripts/gtk.sh",
    "hypridle",
    "wl-paste --watch cliphist store",
    home .. "/.config/ml4w/scripts/ml4w-autostart",
    home .. "/.config/hypr/scripts/cleanup.sh",
    "dbus-update-activation-environment --systemd WAYLAND_DISPLAY XDG_CURRENT_DESKTOP",
}

hl.on("hyprland.start", function()
    for _, command in ipairs(startupCommands) do
        hl.exec_cmd(command)
    end
    hl.exec_cmd("waylyrics", { workspace = "special:silent" })
end)

if os.getenv("HYPRLAND_CONFIG_VERIFY") ~= "1" then
    hl.on("config.reloaded", function()
        hl.exec_cmd(home .. "/.config/com.ml4w.hyprlandsettings/hyprctl.sh")
    end)
end

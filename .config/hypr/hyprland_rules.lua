local layerRules = {
    { match = { namespace = "swaync-control-center" }, blur = true },
    { match = { namespace = "swaync-notification-window" }, blur = true },
    { match = { namespace = "swaync-control-center" }, ignore_alpha = 0.5 },
    { match = { namespace = "swaync-notification-window" }, ignore_alpha = 0.5 },
    { match = { namespace = "waybar" }, blur = true },
    { match = { namespace = "waybar" }, ignore_alpha = 0.01 },
}

for _, rule in ipairs(layerRules) do
    hl.layer_rule(rule)
end

local windowRules = {
    {
        name = "pavucontrol",
        match = { class = ".*org.pulseaudio.pavucontrol.*" },
        float = true,
        center = true,
        pin = true,
        size = { 700, 600 },
    },
    {
        name = "waypaper",
        match = { class = ".*waypaper.*" },
        float = true,
        center = true,
        pin = true,
        size = { 900, 700 },
    },
    {
        name = "newelle",
        match = { class = "io.github.qwersyk.Newelle" },
        float = true,
        center = true,
        pin = true,
        size = { 1000, 700 },
    },
    {
        name = "ml4w-calendar",
        match = { class = "com.ml4w.calendar" },
        float = true,
        move = { "monitor_w-window_w-21", 76 },
        pin = true,
        size = { 400, 400 },
    },
    {
        name = "ml4w-sidebar",
        match = { class = "com.ml4w.sidebar" },
        float = true,
        move = { "monitor_w-window_w-21", 76 },
        pin = true,
        size = { 400, 660 },
    },
    {
        name = "ml4w-welcome",
        match = { class = "com.ml4w.welcome" },
        float = true,
        center = true,
        pin = true,
        size = { 700, 600 },
    },
    {
        name = "ml4w-settings",
        match = { class = "com.ml4w.settings" },
        float = true,
        move = { "monitor_w*0.5-window_w*0.5", 86 },
        pin = true,
        size = { 800, 600 },
    },
    {
        name = "blueman-manager",
        match = { class = "blueman-manager" },
        float = true,
        center = true,
        size = { 800, 600 },
    },
    {
        name = "nwg-look",
        match = { class = "nwg-look" },
        float = true,
        center = true,
        size = { 700, 600 },
    },
    {
        name = "nwg-displays",
        match = { class = "nwg-displays" },
        float = true,
        center = true,
        size = { 900, 600 },
    },
    {
        name = "missioncenter",
        match = { class = "io.missioncenter.MissionCenter" },
        float = true,
        center = true,
        pin = true,
        size = { 900, 600 },
    },
    {
        name = "gnome-calculator",
        match = { class = "org.gnome.Calculator" },
        float = true,
        center = true,
        size = { 700, 600 },
    },
    {
        name = "hyprland-share-picker",
        match = { class = "hyprland-share-picker" },
        float = true,
        pin = true,
        center = true,
        size = { 600, 400 },
    },
    {
        name = "nm-connection-editor",
        match = { class = "nm-connection-editor" },
        float = true,
        center = true,
        size = { 800, 700 },
    },
    {
        name = "picture-in-picture",
        match = { title = [=[^([Pp]icture[-\s]?[Ii]n[-\s]?[Pp]icture)(.*)$]=] },
        float = true,
        pin = true,
        center = true,
    },
    {
        name = "dotfiles-floating",
        match = { class = "dotfiles-floating" },
        float = true,
        center = true,
        size = { 1000, 700 },
    },
    {
        name = "dotfiles-sidepad",
        match = { class = "dotfiles-sidepad" },
        float = true,
        pin = true,
        center = true,
        size = { 1000, 700 },
    },
}

for _, rule in ipairs(windowRules) do
    hl.window_rule(rule)
end

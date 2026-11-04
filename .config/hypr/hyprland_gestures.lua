local scripts = (os.getenv("HOME") or "") .. "/.config/hypr/scripts"

-- Native workspace gestures track finger movement throughout the swipe.
hl.config({
    gestures = {
        workspace_swipe_use_r = false,
        workspace_swipe_create_new = true,
        workspace_swipe_forever = false,
    },
})

hl.gesture({ fingers = 3, direction = "horizontal", action = "workspace" })
hl.gesture({ fingers = 3, direction = "up", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh copy active") end })
hl.gesture({ fingers = 3, direction = "down", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh copy screen") end })
hl.gesture({ fingers = 4, direction = "up", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh save active") end })
hl.gesture({ fingers = 4, direction = "down", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh save screen") end })

local scripts = (os.getenv("HOME") or "") .. "/.config/hypr/scripts"

hl.gesture({ fingers = 3, direction = "horizontal", action = "workspace" })
hl.gesture({ fingers = 3, direction = "up", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh copy active") end })
hl.gesture({ fingers = 3, direction = "down", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh copy screen") end })
hl.gesture({ fingers = 4, direction = "up", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh save active") end })
hl.gesture({ fingers = 4, direction = "down", action = function() hl.exec_cmd(scripts .. "/swipe-screenshot.sh save screen") end })

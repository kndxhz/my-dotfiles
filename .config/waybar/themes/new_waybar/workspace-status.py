#!/usr/bin/env python3
"""Display a workspace without making empty workspaces persistent."""
import json
import os
import socket
import subprocess
import sys
import time

workspace_id = int(sys.argv[1])
previous = None


def query(command):
    return json.loads(subprocess.check_output(
        ["hyprctl", "-j", command], text=True, timeout=2
    ))


def update():
    global previous
    active = query("activeworkspace").get("id")
    workspace = next((w for w in query("workspaces")
                      if w.get("id") == workspace_id), {})
    windows = workspace.get("windows", 0)
    state = "active" if active == workspace_id else ("occupied" if windows else "empty")
    result = json.dumps({
        "text": str(workspace_id),
        "class": state,
        "tooltip": f"工作区 {workspace_id} · {windows} 个窗口",
    }, ensure_ascii=False)
    if result != previous:
        print(result, flush=True)
        previous = result


events = {"workspace", "workspacev2", "focusedmon", "focusedmonv2",
          "createworkspace", "destroyworkspace", "openwindow",
          "closewindow", "movewindow", "movewindowv2",
          "moveworkspace", "renameworkspace", "configreloaded"}

while True:
    try:
        path = os.path.join(os.environ["XDG_RUNTIME_DIR"], "hypr",
                            os.environ["HYPRLAND_INSTANCE_SIGNATURE"], ".socket2.sock")
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as connection:
            connection.connect(path)
            update()
            with connection.makefile() as stream:
                for line in stream:
                    if line.partition(">>")[0] in events:
                        update()
    except BrokenPipeError:
        break
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f"workspace-status: {error}", file=sys.stderr)
    time.sleep(1)

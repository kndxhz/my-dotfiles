#!/usr/bin/env python3
"""Stream workspace buttons without keeping empty workspaces alive."""
import json
import os
import socket
import subprocess
import sys
import time

previous = None


def query(command):
    return json.loads(subprocess.check_output(
        ["hyprctl", "-j", command], text=True, timeout=2
    ))


def visible_workspaces(workspaces, active=None):
    by_id = {w["id"]: w for w in workspaces if w["id"] > 0}
    ids = set(range(1, 6)) | {i for i, w in by_id.items() if w.get("windows", 0) > 0}
    # Keep an empty extra workspace visible until the user leaves it.
    if active is not None and active > 0:
        ids.add(active)
    return [{"id": i, "windows": by_id.get(i, {}).get("windows", 0)}
            for i in sorted(ids)]


def update():
    global previous
    active = query("activeworkspace").get("id")
    result = json.dumps({
        "active": active,
        "workspaces": visible_workspaces(query("workspaces"), active),
    }, ensure_ascii=False)
    if result != previous:
        print(result, flush=True)
        previous = result


events = {"workspace", "workspacev2", "focusedmon", "focusedmonv2",
          "createworkspace", "destroyworkspace", "openwindow",
          "closewindow", "movewindow", "movewindowv2",
          "moveworkspace", "renameworkspace", "configreloaded"}

def main():
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


if __name__ == "__main__":
    main()

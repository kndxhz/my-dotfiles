#!/usr/bin/env python3
"""Stream workspace buttons without keeping empty workspaces alive."""
import json
import os
import socket
import subprocess
import sys
import time

previous = None
pending_attention = set()
known_titles = {}
BROWSER_CLASSES = ("chrome", "chromium", "firefox", "brave", "vivaldi", "microsoft-edge", "zen-browser")


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


def attention_workspaces(clients, active, event="", payload=""):
    # IPC addresses omit the 0x prefix used in the clients response.
    by_address = {c["address"].removeprefix("0x"): c for c in clients}
    pending_attention.intersection_update(by_address)
    address = payload.partition(",")[0].removeprefix("0x")
    client = by_address.get(address)
    if client is not None:
        # Use the event payload: a clients snapshot may already contain a
        # later title while older events are still queued.
        title = payload.partition(",")[2] if event == "windowtitlev2" else client.get("title", "")
        browser = any(name in client.get("class", "").lower() for name in BROWSER_CLASSES)
        changed_title = (event == "windowtitlev2" and browser
                         and address in known_titles and title != known_titles[address])
        if event in {"urgent", "bell"} or changed_title or (event == "openwindow" and browser):
            pending_attention.add(address)
        if event == "windowtitlev2":
            known_titles[address] = title
    # Visiting a workspace acknowledges all its pending notifications.
    for address, client in by_address.items():
        if client.get("workspace", {}).get("id") == active:
            pending_attention.discard(address)
    for stale in known_titles.keys() - by_address.keys():
        del known_titles[stale]
    # Other IPC events must not overwrite the last processed title.
    for address, client in by_address.items():
        known_titles.setdefault(address, client.get("title", ""))
    return {by_address[address].get("workspace", {}).get("id")
            for address in pending_attention}


def update(event="", payload=""):
    global previous
    active = query("activeworkspace").get("id")
    attention = attention_workspaces(query("clients"), active, event, payload)
    existing = query("workspaces")
    workspaces = visible_workspaces(existing, active)
    for workspace in workspaces:
        workspace["urgent"] = workspace["id"] in attention
    result = json.dumps({
        "active": active,
        # Fixed 1-5 buttons are placeholders, not additional workspaces.
        "workspace-count": sum(w["id"] > 0 for w in existing),
        "workspaces": workspaces,
    }, ensure_ascii=False)
    if result != previous:
        print(result, flush=True)
        previous = result


events = {"workspace", "workspacev2", "focusedmon", "focusedmonv2",
          "createworkspace", "destroyworkspace", "openwindow",
          "closewindow", "movewindow", "movewindowv2",
          "moveworkspace", "renameworkspace", "configreloaded",
          "urgent", "bell", "windowtitlev2", "activewindowv2"}

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
                        event, _, payload = line.rstrip("\n").partition(">>")
                        if event in events:
                            update(event, payload)
        except BrokenPipeError:
            break
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            print(f"workspace-status: {error}", file=sys.stderr)
        time.sleep(1)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Media key: one click toggles playback; two within 350 ms skip."""
import fcntl
import json
import os
from pathlib import Path
import subprocess
import time
import uuid

DOUBLE_CLICK_SECONDS = 0.350


def media(action):
    subprocess.run(["playerctl", action], check=False, timeout=5)


def click():
    runtime = Path(os.environ["XDG_RUNTIME_DIR"]) / "hypr-headset-click"
    runtime.mkdir(mode=0o700, exist_ok=True)
    token = uuid.uuid4().hex
    # Keep the lock inode in place: unlinking it would allow competing locks.
    with (runtime / "state").open("a+") as state:
        def load():
            state.seek(0)
            try:
                return json.load(state)
            except (ValueError, OSError):
                return {}

        def save(value):
            state.seek(0)
            state.truncate()
            json.dump(value, state)
            state.flush()

        fcntl.flock(state, fcntl.LOCK_EX)
        previous = load()
        now = time.monotonic()
        if previous and 0 <= now - previous["time"] < DOUBLE_CLICK_SECONDS:
            save({})
            media("next")
            return
        # Resolve an expired click even if its sleeping process was delayed.
        if previous:
            save({})
            media("play-pause")
        save({"time": now, "token": token})
        fcntl.flock(state, fcntl.LOCK_UN)
        time.sleep(DOUBLE_CLICK_SECONDS)
        fcntl.flock(state, fcntl.LOCK_EX)
        if load().get("token") == token:
            save({})
            media("play-pause")


if __name__ == "__main__":
    click()

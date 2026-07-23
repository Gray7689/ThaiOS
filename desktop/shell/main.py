#!/usr/bin/env python3
"""
ThaiOS Shell — Session manager for ThaiDesktop
Launches and manages all desktop components
"""

import json
import os
import signal
import subprocess
import sys
import time

CONFIG_PATH = "/usr/share/ThaiOS/session.conf"


class ThaiShell:
    def __init__(self):
        self.processes = {}
        self.running = True
        self.config = self.load_config()

    def load_config(self):
        config = {}
        if os.path.exists(CONFIG_PATH):
            with open(CONFIG_PATH) as f:
                for line in f:
                    line = line.strip()
                    if not line or line.startswith("[") or line.startswith("#"):
                        continue
                    if "=" in line:
                        key, val = line.split("=", 1)
                        config[key.strip()] = val.strip()
        return config

    def launch_component(self, name, path):
        if not path or not os.path.exists(path):
            print(f"[ThaiShell] Component {name} not found: {path}")
            return

        try:
            proc = subprocess.Popen(
                [path],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                preexec_fn=os.setsid if hasattr(os, "setsid") else None
            )
            self.processes[name] = proc
            print(f"[ThaiShell] Started {name} (PID: {proc.pid})")
        except Exception as e:
            print(f"[ThaiShell] Failed to start {name}: {e}")

    def start(self):
        print("[ThaiShell] ThaiOS Desktop Session starting...")

        components = [
            ("compositor", self.config.get("Compositor", "/usr/bin/thai-desktop")),
            ("panel", self.config.get("Panel", "/usr/bin/thai-panel")),
            ("dock", self.config.get("Dock", "/usr/bin/thai-dock")),
            ("app-menu", self.config.get("AppMenu", "/usr/bin/thai-app-menu")),
            ("notifications", self.config.get("NotificationCenter", "/usr/bin/thai-notifications")),
            ("control-center", self.config.get("ControlCenter", "/usr/bin/thai-control-center")),
        ]

        for name, path in components:
            self.launch_component(name, path)
            time.sleep(0.5)

        print("[ThaiShell] ThaiOS Desktop session ready")

    def stop(self):
        print("[ThaiShell] Shutting down ThaiOS Desktop...")
        for name, proc in reversed(list(self.processes.items())):
            try:
                proc.terminate()
                proc.wait(timeout=5)
                print(f"[ThaiShell] Stopped {name}")
            except Exception:
                try:
                    proc.kill()
                except Exception:
                    pass
        self.running = False

    def handle_signal(self, signum, frame):
        self.stop()
        sys.exit(0)

    def run(self):
        signal.signal(signal.SIGINT, self.handle_signal)
        signal.signal(signal.SIGTERM, self.handle_signal)
        self.start()

        try:
            while self.running:
                for name, proc in list(self.processes.items()):
                    if proc.poll() is not None:
                        print(f"[ThaiShell] {name} exited, restarting...")
                        self.launch_component(name, self.config.get(
                            name.capitalize().replace("-", ""),
                            f"/usr/bin/thai-{name}"
                        ))
                time.sleep(2)
        except KeyboardInterrupt:
            self.stop()


if __name__ == "__main__":
    shell = ThaiShell()
    shell.run()

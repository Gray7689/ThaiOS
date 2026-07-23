#!/usr/bin/env python3
"""
ThaiOS Panel — Top bar with application menu, system tray, and clock
"""

import json
import os
import subprocess
import sys

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango
except ImportError:
    print("[ThaiPanel] GTK3 not available. Install GTK3 and PyGObject.")
    sys.exit(1)

CONFIG_DIR = os.path.expanduser("~/.config/ThaiOS")
THEME_DIR = "/usr/share/ThaiOS/themes"

COLORS = {
    "bg": "#0F1A2E",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "surface": "#1A2744",
    "border": "#2D4066",
    "text_secondary": "#8899B3",
}


class ThaiPanel(Gtk.Window):
    def __init__(self):
        super().__init__(type=Gtk.WindowType.TOPLEVEL)
        self.set_title("ThaiOS Panel")
        self.set_default_size(0, 36)
        self.set_position(Gtk.WindowPosition.TOP)
        self.stick()
        self.set_decorated(False)
        self.set_keep_above(True)

        screen = Gdk.Screen.get_default()
        monitor = screen.get_monitor_geometry(0)
        self.set_size_request(monitor.width, 36)
        self.move(0, 0)

        self.set_type_hint(Gdk.WindowTypeHint.DOCK)

        css = (
            f"#panel {{ background-color: {COLORS['bg']}; border-bottom: 1px solid {COLORS['border']}; }}"
            f"#panel button {{ background: transparent; border: none; color: {COLORS['fg']}; "
            f"padding: 0 12px; font-family: Sarabun; font-size: 12px; }}"
            f"#panel button:hover {{ background: {COLORS['surface']}; }}"
            f"#clock {{ color: {COLORS['fg']}; padding: 0 16px; font-family: Sarabun; font-size: 12px; }}"
            f"#menu-btn {{ color: {COLORS['accent']}; font-weight: bold; }}"
            f".system-tray {{ background: transparent; }}"
        )
        style_provider = Gtk.CssProvider()
        style_provider.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(
            screen, style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

        self.box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        self.box.set_name("panel")
        self.add(self.box)

        # Application Menu button
        self.menu_btn = Gtk.Button(label="\u25b6 ThaiOS")
        self.menu_btn.set_name("menu-btn")
        self.menu_btn.connect("clicked", self.on_menu_click)
        self.box.pack_start(self.menu_btn, False, False, 0)

        # Workspace indicator
        self.workspace_label = Gtk.Label(label="  1 ")
        self.workspace_label.set_name("clock")
        self.box.pack_start(self.workspace_label, False, False, 0)

        # Spacer
        self.box.pack_start(Gtk.Label(), True, True, 0)

        # System Tray
        self.tray_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)
        self.tray_box.get_style_context().add_class("system-tray")
        self.box.pack_end(self.tray_box, False, False, 8)

        # Quick toggles
        self.wifi_btn = Gtk.Button(label="\u2388")
        self.wifi_btn.connect("clicked", lambda b: self.toggle_control_center())
        self.box.pack_end(self.wifi_btn, False, False, 0)

        self.vol_btn = Gtk.Button(label="\u266a")
        self.box.pack_end(self.vol_btn, False, False, 0)

        self.battery_btn = Gtk.Button(label="\u26a1")
        self.box.pack_end(self.battery_btn, False, False, 0)

        # Clock
        self.clock = Gtk.Label(label="")
        self.clock.set_name("clock")
        self.box.pack_end(self.clock, False, False, 0)

        # Notification button
        self.notif_btn = Gtk.Button(label="\u26a0")
        self.notif_btn.connect("clicked", self.on_notifications_click)
        self.box.pack_end(self.notif_btn, False, False, 0)

        # Update clock
        GLib.timeout_add_seconds(1, self.update_clock)
        self.update_clock()

        self.connect("destroy", Gtk.main_quit)
        self.show_all()

    def update_clock(self):
        from datetime import datetime
        now = datetime.now()
        self.clock.set_text(now.strftime("%a %b %d  %H:%M"))
        return True

    def on_menu_click(self, btn):
        subprocess.Popen(["/usr/bin/thai-app-menu"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def on_notifications_click(self, btn):
        subprocess.Popen(["/usr/bin/thai-notifications"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def toggle_control_center(self):
        subprocess.Popen(["/usr/bin/thai-control-center"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


if __name__ == "__main__":
    Gtk.init(sys.argv)
    panel = ThaiPanel()
    Gtk.main()

#!/usr/bin/env python3
"""
ThaiOS Dock — Modern application dock with launchers and running apps
"""

import os
import subprocess
import sys
import json

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango, GdkPixbuf
except ImportError:
    print("[ThaiDock] GTK3 not available.")
    sys.exit(1)

DOCK_APPS = [
    {"name": "ThaiBrowser", "icon": "ThaiBrowser", "cmd": "/usr/bin/thai-browser"},
    {"name": "ThaiFiles", "icon": "ThaiFiles", "cmd": "/usr/bin/thai-files"},
    {"name": "ThaiStore", "icon": "ThaiStore", "cmd": "/usr/bin/thai-store"},
    {"name": "ThaiTerminal", "icon": "ThaiTerminal", "cmd": "/usr/bin/thai-terminal"},
    {"name": "ThaiSettings", "icon": "ThaiSettings", "cmd": "/usr/bin/thai-settings"},
    {"name": "ThaiMedia", "icon": "ThaiMedia", "cmd": "/usr/bin/thai-media"},
]

COLORS = {
    "bg": "rgba(15, 26, 46, 0.85)",
    "accent": "#FF6B35",
    "fg": "#E8EDF5",
    "surface": "rgba(26, 39, 68, 0.9)",
    "border": "#2D4066",
    "running": "#FF6B35",
}


class ThaiDock(Gtk.Window):
    def __init__(self):
        super().__init__(type=Gtk.WindowType.TOPLEVEL)
        self.set_title("ThaiOS Dock")
        self.set_decorated(False)
        self.set_keep_above(True)
        self.set_type_hint(Gdk.WindowTypeHint.DOCK)
        self.set_accept_focus(False)

        screen = Gdk.Screen.get_default()
        monitor = screen.get_monitor_geometry(0)

        dock_size = 56
        margin = 12
        spacing = 8
        total_w = len(DOCK_APPS) * (dock_size + spacing) + margin * 2 - spacing
        dock_y = monitor.height - dock_size - margin - 36

        self.set_size_request(total_w, dock_size + 12)
        self.move((monitor.width - total_w) // 2, dock_y)

        css = (
            f"#dock {{ background: {COLORS['bg']}; border-radius: 16px; border: 1px solid {COLORS['border']}; }}"
            f"#dock button {{ background: transparent; border: 2px solid transparent; "
            f"border-radius: 12px; padding: 6px; margin: 2px; }}"
            f"#dock button:hover {{ background: {COLORS['surface']}; border-color: {COLORS['accent']}; }}"
            f"#dock button.running {{ border-bottom: 3px solid {COLORS['running']}; }}"
            f"#dock label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 10px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.fixed = Gtk.Fixed()
        self.add(self.fixed)

        self.event_box = Gtk.EventBox()
        self.event_box.set_name("dock")
        self.fixed.put(self.event_box, 0, 0)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=spacing)
        self.main_box.set_margin_start(margin)
        self.main_box.set_margin_end(margin)
        self.main_box.set_margin_top(6)
        self.main_box.set_margin_bottom(6)
        self.event_box.add(self.main_box)

        self.buttons = {}
        for app in DOCK_APPS:
            btn_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=2)
            btn = Gtk.Button()
            btn.set_tooltip_text(app["name"])

            icon_theme = Gtk.IconTheme.get_default()
            icon_path = f"/usr/share/ThaiOS/icons/scalable/apps/{app['icon']}.svg"
            if os.path.exists(icon_path):
                pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size(icon_path, 36, 36)
                img = Gtk.Image.new_from_pixbuf(pixbuf)
            else:
                img = Gtk.Image.new_from_icon_name("application-x-executable",
                                                   Gtk.IconSize.LARGE_TOOLBAR)
            btn.set_image(img)
            btn.set_size_request(44, 44)
            btn.connect("clicked", self.launch_app, app)

            self.buttons[app["name"]] = btn
            btn_box.pack_start(btn, False, False, 0)
            self.main_box.pack_start(btn_box, False, False, 0)

        self.connect("destroy", Gtk.main_quit)
        self.show_all()

    def launch_app(self, btn, app):
        subprocess.Popen([app["cmd"]], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


if __name__ == "__main__":
    Gtk.init(sys.argv)
    dock = ThaiDock()
    Gtk.main()

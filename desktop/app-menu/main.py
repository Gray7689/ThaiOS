#!/usr/bin/env python3
"""
ThaiOS App Menu — Application launcher with categorized grid
"""

import os
import subprocess
import sys

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    gi.require_version("Gdk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, GdkPixbuf, Pango
except ImportError:
    print("[ThaiAppMenu] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_secondary": "#8899B3",
}

APPS = [
    ("ThaiBrowser", "ThaiBrowser", "/usr/bin/thai-browser", "Internet"),
    ("ThaiFiles", "ThaiFiles", "/usr/bin/thai-files", "System"),
    ("ThaiStore", "ThaiStore", "/usr/bin/thai-store", "System"),
    ("ThaiTerminal", "ThaiTerminal", "/usr/bin/thai-terminal", "System"),
    ("ThaiSettings", "ThaiSettings", "/usr/bin/thai-settings", "System"),
    ("ThaiMedia", "ThaiMedia", "/usr/bin/thai-media", "Media"),
    ("ThaiEditor", "ThaiEditor", "/usr/bin/thai-editor", "Office"),
    ("ThaiBackup", "ThaiBackup", "/usr/bin/thai-backup", "System"),
]


class ThaiAppMenu(Gtk.Window):
    def __init__(self):
        super().__init__(type=Gtk.WindowType.POPUP)
        self.set_title("ThaiOS Applications")
        self.set_decorated(False)
        self.set_keep_above(True)
        self.set_accept_focus(True)
        self.set_size_request(700, 500)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_type_hint(Gdk.WindowTypeHint.MENU)

        screen = Gdk.Screen.get_default()
        css = (
            f"#menu-window {{ background: {COLORS['bg']}; border: 1px solid {COLORS['border']}; "
            f"border-radius: 16px; }}"
            f"#search-entry {{ background: {COLORS['surface']}; color: {COLORS['fg']}; "
            f"border: 1px solid {COLORS['border']}; border-radius: 24px; padding: 8px 16px; "
            f"font-family: Sarabun; font-size: 14px; margin: 16px; }}"
            f"#search-entry:focus {{ border-color: {COLORS['accent']}; }}"
            f"#category-label {{ color: {COLORS['text_secondary']}; font-family: Sarabun; "
            f"font-size: 11px; text-transform: uppercase; padding: 8px 16px; }}"
            f".app-btn {{ background: transparent; border: none; border-radius: 12px; "
            f"padding: 12px; }}"
            f".app-btn:hover {{ background: {COLORS['surface']}; }}"
            f".app-btn label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 12px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("menu-window")
        self.add(self.main_box)

        # Search bar
        self.search = Gtk.SearchEntry()
        self.search.set_name("search-entry")
        self.search.set_placeholder_text("Cerca applicazioni...")
        self.search.connect("search-changed", self.filter_apps)
        self.main_box.pack_start(self.search, False, False, 0)

        # Categories + Apps
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        scroll.set_size_request(-1, 400)
        self.main_box.pack_start(scroll, True, True, 0)

        self.app_grid = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        scroll.add(self.app_grid)

        self.build_app_grid("")
        self.connect("key-press-event", self.on_key_press)
        self.connect("focus-out-event", self.on_focus_out)

    def build_app_grid(self, filter_text):
        for child in self.app_grid.get_children():
            self.app_grid.remove(child)

        categories = {}
        for name, icon, cmd, cat in APPS:
            if filter_text and filter_text.lower() not in name.lower():
                continue
            if cat not in categories:
                categories[cat] = []
            categories[cat].append((name, icon, cmd))

        for cat, apps in categories.items():
            cat_label = Gtk.Label(label=cat)
            cat_label.set_name("category-label")
            cat_label.set_xalign(0)
            self.app_grid.pack_start(cat_label, False, False, 0)

            flow = Gtk.FlowBox()
            flow.set_max_children_per_line(4)
            flow.set_selection_mode(Gtk.SelectionMode.NONE)
            flow.set_homogeneous(True)

            for name, icon, cmd in apps:
                btn = Gtk.Button()
                btn.get_style_context().add_class("app-btn")
                btn.set_size_request(140, 100)

                vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)

                icon_path = f"/usr/share/ThaiOS/icons/scalable/apps/{icon}.svg"
                if os.path.exists(icon_path):
                    pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size(icon_path, 40, 40)
                    img = Gtk.Image.new_from_pixbuf(pixbuf)
                else:
                    img = Gtk.Image.new_from_icon_name("application-x-executable", Gtk.IconSize.DIALOG)
                vbox.pack_start(img, False, False, 0)

                label = Gtk.Label(label=name)
                vbox.pack_start(label, False, False, 0)

                btn.add(vbox)
                btn.connect("clicked", lambda b, c=cmd: self.launch(c))
                flow.add(btn)

            self.app_grid.pack_start(flow, False, False, 8)

    def filter_apps(self, entry):
        self.build_app_grid(entry.get_text())

    def launch(self, cmd):
        subprocess.Popen([cmd], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.close()

    def on_key_press(self, widget, event):
        if event.keyval == Gdk.KEY_Escape:
            self.close()
        return False

    def on_focus_out(self, widget, event):
        self.close()


def main():
    Gtk.init(sys.argv)
    menu = ThaiAppMenu()
    menu.show_all()
    menu.search.grab_focus()
    Gtk.main()


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
ThaiOS Control Center — Quick settings panel
"""

import os
import subprocess
import sys

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango
except ImportError:
    print("[ThaiControlCenter] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
    "green": "#4CAF50",
}


class ThaiControlCenter(Gtk.Window):
    def __init__(self):
        super().__init__(type=Gtk.WindowType.POPUP)
        self.set_title("ThaiOS Control Center")
        self.set_decorated(False)
        self.set_keep_above(True)
        self.set_size_request(320, 420)
        self.set_type_hint(Gdk.WindowTypeHint.DROPDOWN_MENU)

        screen = Gdk.Screen.get_default()
        monitor = screen.get_monitor_geometry(0)
        self.move(monitor.width - 340, 38)

        css = (
            f"#cc-window {{ background: {COLORS['bg']}; border-left: 1px solid {COLORS['border']}; }}"
            f"#cc-header {{ background: {COLORS['surface']}; padding: 16px; }}"
            f"#cc-header label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 16px; font-weight: bold; }}"
            f".toggle-btn {{ background: {COLORS['surface']}; border: none; border-radius: 12px; "
            f"padding: 12px; margin: 4px; }}"
            f".toggle-btn:hover {{ background: {COLORS['border']}; }}"
            f".toggle-btn label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 11px; }}"
            f".toggle-btn.active {{ background: {COLORS['accent']}; }}"
            f"#brightness-slider {{ background: {COLORS['border']}; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("cc-window")
        self.add(self.main_box)

        # Header
        header = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        header.set_name("cc-header")
        hlabel = Gtk.Label(label="Controllo Rapido")
        hlabel.set_xalign(0)
        header.pack_start(hlabel, True, True, 0)

        settings_btn = Gtk.Button(label="\u2699")
        settings_btn.connect("clicked", lambda b: self.open_settings())
        header.pack_end(settings_btn, False, False, 0)
        self.main_box.pack_start(header, False, False, 0)

        content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        content.set_margin_start(12)
        content.set_margin_end(12)
        content.set_margin_top(12)
        content.set_margin_bottom(12)

        # Toggle grid
        toggle_grid = Gtk.Grid()
        toggle_grid.set_column_spacing(8)
        toggle_grid.set_row_spacing(8)
        toggle_grid.set_column_homogeneous(True)
        toggle_grid.set_row_homogeneous(True)

        toggles = [
            ("Wi-Fi", "\u2388", True),
            ("Bluetooth", "\u2261", False),
            ("Non disturbare", "\u26a0", False),
            ("Modalit\u00e0 scura", "\u263d", False),
            ("Schermo", "\u25a3", False),
            ("Audio", "\u266b", False),
        ]

        for i, (label, icon, active) in enumerate(toggles):
            btn = Gtk.Button(label=f"{icon}\n{label}")
            btn.get_style_context().add_class("toggle-btn")
            if active:
                btn.get_style_context().add_class("active")
            btn.connect("clicked", self.toggle_btn)
            toggle_grid.attach(btn, i % 3, i // 3, 1, 1)

        content.pack_start(toggle_grid, False, False, 0)

        # Brightness
        bright_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        bright_box.set_margin_top(8)
        bright_label = Gtk.Label(label="\u2600")
        bright_label.set_name("cc-label")
        self.bright_slider = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL)
        self.bright_slider.set_range(0, 100)
        self.bright_slider.set_value(80)
        self.bright_slider.set_size_request(200, 20)
        bright_box.pack_start(bright_label, False, False, 0)
        bright_box.pack_start(self.bright_slider, True, True, 0)

        content.pack_start(bright_box, False, False, 0)

        # Volume
        vol_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        vol_label = Gtk.Label(label="\u266a")
        self.vol_slider = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL)
        self.vol_slider.set_range(0, 100)
        self.vol_slider.set_value(60)
        self.vol_slider.set_size_request(200, 20)
        vol_box.pack_start(vol_label, False, False, 0)
        vol_box.pack_start(self.vol_slider, True, True, 0)

        content.pack_start(vol_box, False, False, 0)

        # Media player placeholder
        media_frame = Gtk.Frame()
        media_frame.get_style_context().add_class("toggle-btn")
        media_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        media_box.set_margin(12)
        media_title = Gtk.Label(label="Nessun media in riproduzione")
        media_title.get_style_context().add_class("toggle-btn")
        media_box.pack_start(media_title, False, False, 0)

        media_controls = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=12)
        for icon in ["\u23ee", "\u25b6", "\u23ed"]:
            b = Gtk.Button(label=icon)
            b.get_style_context().add_class("toggle-btn")
            media_controls.pack_start(b, False, False, 0)
        media_box.pack_start(media_controls, False, False, 0)
        media_frame.add(media_box)
        content.pack_start(media_frame, False, False, 0)

        self.main_box.pack_start(content, True, True, 0)

        self.connect("focus-out-event", lambda w, e: self.close())
        self.connect("key-press-event", self.on_key_press)

    def toggle_btn(self, btn):
        ctx = btn.get_style_context()
        if ctx.has_class("active"):
            ctx.remove_class("active")
        else:
            ctx.add_class("active")

    def open_settings(self):
        subprocess.Popen(["/usr/bin/thai-settings"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.close()

    def on_key_press(self, widget, event):
        if event.keyval == Gdk.KEY_Escape:
            self.close()
        return False


if __name__ == "__main__":
    Gtk.init(sys.argv)
    cc = ThaiControlCenter()
    cc.show_all()
    Gtk.main()

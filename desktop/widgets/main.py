#!/usr/bin/env python3
"""
ThaiOS Widgets — Desktop widgets engine
"""

import json
import os
import subprocess
import sys
from datetime import datetime

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango
except ImportError:
    print("[ThaiWidgets] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "rgba(26, 39, 68, 0.8)",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "text_sec": "#8899B3",
}


class BaseWidget(Gtk.Window):
    def __init__(self, title, width=300, height=200, x=100, y=100):
        super().__init__(type=Gtk.WindowType.POPUP)
        self.set_title(f"ThaiOS Widget - {title}")
        self.set_decorated(False)
        self.set_keep_above(False)
        self.set_size_request(width, height)
        self.move(x, y)
        self.set_type_hint(Gdk.WindowTypeHint.DOCK)
        self.set_accept_focus(False)

        screen = Gdk.Screen.get_default()
        css = (
            f"#widget {{ background: {COLORS['bg']}; border: 1px solid rgba(45, 64, 102, 0.6); "
            f"border-radius: 16px; }}"
            f"#widget label {{ color: {COLORS['fg']}; font-family: Sarabun; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.box.set_name("widget")
        self.add(self.box)

    def add_content(self, content):
        self.box.pack_start(content, True, True, 0)


class ClockWidget(BaseWidget):
    def __init__(self):
        super().__init__("Clock", 250, 120, 40, 80)
        container = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        container.set_margin(16)

        self.time_label = Gtk.Label(label="")
        self.time_label.set_markup("<span font='Sarabun 42' color='#E8EDF5'>00:00</span>")
        container.pack_start(self.time_label, True, True, 0)

        self.date_label = Gtk.Label(label="")
        self.date_label.set_markup("<span font='Sarabun 14' color='#8899B3'>Loading...</span>")
        container.pack_start(self.date_label, False, False, 0)

        self.add_content(container)
        self.update_time()
        GLib.timeout_add_seconds(30, self.update_time)

    def update_time(self):
        now = datetime.now()
        self.time_label.set_markup(f"<span font='Sarabun 42' color='#E8EDF5'>{now.strftime('%H:%M')}</span>")
        self.date_label.set_markup(
            f"<span font='Sarabun 14' color='#8899B3'>{now.strftime('%A %d %B %Y')}</span>"
        )
        return True


class WeatherWidget(BaseWidget):
    def __init__(self):
        super().__init__("Weather", 250, 160, 40, 220)
        container = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        container.set_margin(16)

        self.city_label = Gtk.Label()
        self.city_label.set_markup("<span font='Sarabun 13' color='#8899B3'>Bangkok</span>")
        container.pack_start(self.city_label, False, False, 0)

        self.temp_label = Gtk.Label()
        self.temp_label.set_markup("<span font='Sarabun 36' color='#E8EDF5'>32\u00b0</span>")
        container.pack_start(self.temp_label, False, False, 0)

        self.desc_label = Gtk.Label()
        self.desc_label.set_markup("<span font='Sarabun 12' color='#8899B3'>Sereno</span>")
        container.pack_start(self.desc_label, False, False, 0)

        self.add_content(container)


class WidgetManager:
    def __init__(self):
        self.widgets = []

    def load_widgets(self):
        self.widgets.append(ClockWidget())
        self.widgets.append(WeatherWidget())
        for w in self.widgets:
            w.show_all()

    def run(self):
        self.load_widgets()
        Gtk.main()


if __name__ == "__main__":
    Gtk.init(sys.argv)
    wm = WidgetManager()
    wm.run()

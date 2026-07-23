#!/usr/bin/env python3
"""
ThaiOS Notification Center — Slide-out notification panel
"""

import json
import os
import subprocess
import sys
import time
from datetime import datetime

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango
except ImportError:
    print("[ThaiNotifications] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
}


class ThaiNotificationCenter(Gtk.Window):
    def __init__(self):
        super().__init__(type=Gtk.WindowType.POPUP)
        self.set_title("ThaiOS Notifications")
        self.set_decorated(False)
        self.set_keep_above(True)
        self.set_size_request(380, 500)
        self.set_type_hint(Gdk.WindowTypeHint.DROPDOWN_MENU)

        screen = Gdk.Screen.get_default()
        monitor = screen.get_monitor_geometry(0)
        self.move(monitor.width - 400, 38)

        css = (
            f"#notif-window {{ background: {COLORS['bg']}; border-left: 1px solid {COLORS['border']}; }}"
            f"#header {{ background: {COLORS['surface']}; padding: 16px; }}"
            f"#header label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 16px; font-weight: bold; }}"
            f"#clear-btn {{ background: transparent; border: 1px solid {COLORS['border']}; "
            f"border-radius: 8px; color: {COLORS['text_sec']}; padding: 4px 12px; font-family: Sarabun; }}"
            f"#clear-btn:hover {{ background: {COLORS['accent']}; color: white; }}"
            f".notif-card {{ background: {COLORS['surface']}; border-radius: 12px; margin: 4px 12px; padding: 12px; }}"
            f".notif-card label.title {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 13px; font-weight: bold; }}"
            f".notif-card label.body {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 11px; }}"
            f".notif-card label.time {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 10px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("notif-window")
        self.add(self.main_box)

        # Header
        header = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        header.set_name("header")
        hlabel = Gtk.Label(label="Notifiche")
        hlabel.set_xalign(0)
        header.pack_start(hlabel, True, True, 0)

        clear_btn = Gtk.Button(label="Cancella tutte")
        clear_btn.set_name("clear-btn")
        clear_btn.connect("clicked", self.clear_all)
        header.pack_end(clear_btn, False, False, 0)
        self.main_box.pack_start(header, False, False, 0)

        # Notification list
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        self.notif_list = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        scroll.add(self.notif_list)
        self.main_box.pack_start(scroll, True, True, 0)

        self.load_notifications()
        self.connect("focus-out-event", lambda w, e: self.close())
        self.connect("key-press-event", self.on_key_press)

    def load_notifications(self):
        notif_dir = os.path.expanduser("~/.local/share/ThaiOS/notifications")
        os.makedirs(notif_dir, exist_ok=True)

        if not os.listdir(notif_dir):
            empty = Gtk.Label(label="Nessuna notifica")
            empty.get_style_context().add_class("body")
            self.notif_list.pack_start(empty, False, False, 12)
            return

        for fname in sorted(os.listdir(notif_dir), reverse=True)[:20]:
            fpath = os.path.join(notif_dir, fname)
            try:
                with open(fpath) as f:
                    data = json.load(f)
                self.add_notification_card(data.get("app", "System"),
                                           data.get("title", ""),
                                           data.get("body", ""),
                                           data.get("time", ""))
            except Exception:
                pass

    def add_notification_card(self, app, title, body, timestamp):
        card = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        card.get_style_context().add_class("notif-card")

        app_label = Gtk.Label(label=app)
        app_label.get_style_context().add_class("body")

        title_label = Gtk.Label(label=title)
        title_label.get_style_context().add_class("title")
        title_label.set_xalign(0)
        title_label.set_line_wrap(True)

        body_label = Gtk.Label(label=body)
        body_label.get_style_context().add_class("body")
        body_label.set_xalign(0)
        body_label.set_line_wrap(True)

        time_label = Gtk.Label(label=timestamp)
        time_label.get_style_context().add_class("time")
        time_label.set_xalign(1)

        card.pack_start(app_label, False, False, 0)
        card.pack_start(title_label, False, False, 0)
        card.pack_start(body_label, False, False, 0)
        card.pack_end(time_label, False, False, 0)

        self.notif_list.pack_start(card, False, False, 0)

    def clear_all(self, btn):
        notif_dir = os.path.expanduser("~/.local/share/ThaiOS/notifications")
        for f in os.listdir(notif_dir):
            os.remove(os.path.join(notif_dir, f))
        for child in self.notif_list.get_children():
            self.notif_list.remove(child)
        empty = Gtk.Label(label="Nessuna notifica")
        empty.get_style_context().add_class("body")
        self.notif_list.pack_start(empty, False, False, 12)
        self.show_all()

    def on_key_press(self, widget, event):
        if event.keyval == Gdk.KEY_Escape:
            self.close()
        return False


if __name__ == "__main__":
    Gtk.init(sys.argv)
    nc = ThaiNotificationCenter()
    nc.show_all()
    Gtk.main()

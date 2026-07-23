#!/usr/bin/env python3
"""
ThaiBackup — ThaiOS backup and restore tool
"""

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
    print("[ThaiBackup] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
    "green": "#4CAF50",
    "red": "#E63946",
}


class ThaiBackup(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiBackup")
        self.set_default_size(800, 550)
        self.set_position(Gtk.WindowPosition.CENTER)

        screen = Gdk.Screen.get_default()
        css = (
            f"#backup-window {{ background: {COLORS['bg']}; }}"
            f"#header {{ background: {COLORS['surface']}; border-bottom: 1px solid {COLORS['border']}; padding: 16px; }}"
            f"#header label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 20px; font-weight: bold; }}"
            f".card {{ background: {COLORS['surface']}; border: 1px solid {COLORS['border']}; "
            f"border-radius: 16px; padding: 20px; margin: 8px 0; }}"
            f".card label {{ color: {COLORS['fg']}; font-family: Sarabun; }}"
            f".card label.desc {{ color: {COLORS['text_sec']}; font-size: 12px; }}"
            f".btn-primary {{ background: {COLORS['accent']}; border: none; border-radius: 10px; "
            f"color: white; padding: 8px 20px; font-family: Sarabun; font-size: 13px; }}"
            f".btn-primary:hover {{ background: #E63946; }}"
            f".btn-secondary {{ background: transparent; border: 1px solid {COLORS['border']}; "
            f"border-radius: 10px; color: {COLORS['fg']}; padding: 8px 20px; font-family: Sarabun; }}"
            f".btn-secondary:hover {{ background: {COLORS['border']}; }}"
            f"#progress {{ background: {COLORS['border']}; border-radius: 6px; height: 8px; }}"
            f"#progress-fill {{ background: {COLORS['accent']}; border-radius: 6px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("backup-window")
        self.add(self.main_box)

        # Header
        header = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        header.set_name("header")
        title = Gtk.Label(label="ThaiBackup")
        header.pack_start(title, False, False, 0)

        status_badge = Gtk.Label(label="  \u25cf  Protetto  ")
        status_badge.set_markup(f"<span font='Sarabun 11' color='{COLORS['green']}'>\u25cf Protetto</span>")
        header.pack_end(status_badge, False, False, 0)
        self.main_box.pack_start(header, False, False, 0)

        # Content
        content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        content.set_margin(16)

        # Backup overview
        overview = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        overview.get_style_context().add_class("card")

        stats = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=16)
        stat_items = [
            ("Ultimo backup", "Oggi, 03:00"),
            ("Dimensione", "24.6 GB"),
            ("Destinazione", "Disco Locale"),
            ("Stato", "Completato"),
        ]
        for label, value in stat_items:
            sbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=2)
            lbl = Gtk.Label(label=label)
            lbl.get_style_context().add_class("desc")
            lbl.set_xalign(0)
            val = Gtk.Label(label=value)
            val.set_xalign(0)
            sbox.pack_start(lbl, False, False, 0)
            sbox.pack_start(val, False, False, 0)
            stats.pack_start(sbox, True, True, 0)

        overview.pack_start(stats, False, False, 0)
        content.pack_start(overview, False, False, 0)

        # Quick actions
        actions = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)

        for label, icon in [("\u25b6 Esegui Backup", "btn-primary"),
                            ("\u21ba Ripristina", "btn-secondary"),
                            ("\u2699 Impostazioni", "btn-secondary")]:
            btn = Gtk.Button(label=f"{label}")
            btn.get_style_context().add_class(icon)
            actions.pack_start(btn, False, False, 0)

        content.pack_start(actions, False, False, 0)

        # Scheduled backups
        schedule = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        schedule.get_style_context().add_class("card")

        sched_title = Gtk.Label(label="Backup Pianificati")
        sched_title.set_markup("<span font='Sarabun 14' color='#E8EDF5'><b>Backup Pianificati</b></span>")
        sched_title.set_xalign(0)
        schedule.pack_start(sched_title, False, False, 0)

        sched_items = [
            ("Backup Sistema", "Ogni giorno alle 03:00", True),
            ("Backup Documenti", "Ogni 6 ore", True),
            ("Backup Database", "Ogni settimana (Dom 02:00)", False),
        ]
        for name, time, active in sched_items:
            row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
            row.set_margin_top(4)
            nlabel = Gtk.Label(label=name)
            nlabel.set_xalign(0)
            nlabel.set_size_request(150, -1)
            tlabel = Gtk.Label(label=time)
            tlabel.get_style_context().add_class("desc")
            tlabel.set_xalign(0)
            switch = Gtk.Switch()
            switch.set_active(active)
            row.pack_start(nlabel, False, False, 0)
            row.pack_start(tlabel, True, True, 0)
            row.pack_end(switch, False, False, 0)
            schedule.pack_start(row, False, False, 0)

        content.pack_start(schedule, False, False, 0)

        self.main_box.pack_start(content, True, True, 0)
        self.connect("destroy", Gtk.main_quit)


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiBackup()
    app.show_all()
    Gtk.main()

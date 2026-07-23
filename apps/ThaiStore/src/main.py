#!/usr/bin/env python3
"""
ThaiStore — ThaiOS software center
"""

import json
import os
import subprocess
import sys
import threading

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango, GdkPixbuf
except ImportError:
    print("[ThaiStore] GTK3 not available.")
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

STORE_APPS = [
    {"name": "ThaiBrowser", "icon": "ThaiBrowser", "desc": "Browser web veloce e sicuro", "size": "4.2 MB", "installed": True},
    {"name": "ThaiFiles", "icon": "ThaiFiles", "desc": "Gestore di file elegante", "size": "2.8 MB", "installed": True},
    {"name": "ThaiTerminal", "icon": "ThaiTerminal", "desc": "Terminale potente con multipiattaforma", "size": "3.1 MB", "installed": True},
    {"name": "ThaiMedia", "icon": "ThaiMedia", "desc": "Riproduttore multimediale", "size": "6.7 MB", "installed": True},
    {"name": "ThaiEditor", "icon": "ThaiEditor", "desc": "Editor di testo professionale", "size": "2.1 MB", "installed": True},
    {"name": "ThaiBackup", "icon": "ThaiBackup", "desc": "Backup e ripristino del sistema", "size": "1.9 MB", "installed": True},
    {"name": "GIMP", "icon": "gimp", "desc": "Editor di immagini avanzato", "size": "24.5 MB", "installed": False},
    {"name": "LibreOffice", "icon": "libreoffice", "desc": "Suite per ufficio completa", "size": "185 MB", "installed": False},
    {"name": "Firefox", "icon": "firefox", "desc": "Browser web alternativo", "size": "52 MB", "installed": False},
]


class ThaiStore(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiStore")
        self.set_default_size(900, 620)
        self.set_position(Gtk.WindowPosition.CENTER)

        screen = Gdk.Screen.get_default()
        css = (
            f"#store-window {{ background: {COLORS['bg']}; }}"
            f"#header {{ background: {COLORS['surface']}; border-bottom: 1px solid {COLORS['border']}; padding: 16px; }}"
            f"#header label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 20px; font-weight: bold; }}"
            f"#search {{ background: {COLORS['bg']}; color: {COLORS['fg']}; "
            f"border: 1px solid {COLORS['border']}; border-radius: 24px; padding: 8px 16px; "
            f"font-family: Sarabun; font-size: 14px; }}"
            f"#search:focus {{ border-color: {COLORS['accent']}; }}"
            f".category {{ background: transparent; border: none; color: {COLORS['text_sec']}; "
            f"font-family: Sarabun; font-size: 13px; padding: 8px 16px; }}"
            f".category:hover, .category.active {{ color: {COLORS['accent']}; }}"
            f".app-card {{ background: {COLORS['surface']}; border: 1px solid {COLORS['border']}; "
            f"border-radius: 16px; padding: 16px; }}"
            f".app-card:hover {{ border-color: {COLORS['accent']}; }}"
            f".app-card label.name {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 14px; font-weight: bold; }}"
            f".app-card label.desc {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 11px; }}"
            f".install-btn {{ background: {COLORS['accent']}; border: none; border-radius: 8px; "
            f"color: white; padding: 4px 16px; font-family: Sarabun; font-size: 12px; }}"
            f".install-btn:hover {{ background: #E63946; }}"
            f".install-btn.installed {{ background: {COLORS['green']}; }}"
            f".featured {{ background: linear-gradient(135deg, {COLORS['surface']}, {COLORS['bg']}); "
            f"border: 1px solid {COLORS['accent']}; border-radius: 20px; padding: 24px; margin: 16px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("store-window")
        self.add(self.main_box)

        # Header
        header = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        header.set_name("header")
        title = Gtk.Label(label="ThaiStore")
        title.set_xalign(0)
        header.pack_start(title, False, False, 0)

        self.search_entry = Gtk.SearchEntry()
        self.search_entry.set_name("search")
        self.search_entry.set_placeholder_text("Cerca app...")
        self.search_entry.connect("search-changed", self.filter_apps)
        header.pack_start(self.search_entry, False, False, 0)
        self.main_box.pack_start(header, False, False, 0)

        # Categories
        cats = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)
        cats.set_margin_start(16)
        cats.set_margin_top(8)
        for cat in ["Tutte", "Installate", "Popolari", "Giochi", "Sviluppo", "Ufficio"]:
            btn = Gtk.Button(label=cat)
            btn.get_style_context().add_class("category")
            cats.pack_start(btn, False, False, 0)
        self.main_box.pack_start(cats, False, False, 0)

        # App list
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)

        self.app_list = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        self.app_list.set_margin(16)
        scroll.add(self.app_list)
        self.main_box.pack_start(scroll, True, True, 0)

        self.render_apps(STORE_APPS)
        self.connect("destroy", Gtk.main_quit)

    def render_apps(self, apps):
        for child in self.app_list.get_children():
            self.app_list.remove(child)

        # Featured section
        featured = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=16)
        featured.get_style_context().add_class("featured")
        f_text = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        f_title = Gtk.Label(label="Scopri ThaiBrowser")
        f_title.set_markup("<span font='Sarabun 18' color='#E8EDF5'><b>Scopri ThaiBrowser</b></span>")
        f_title.set_xalign(0)
        f_text.pack_start(f_title, False, False, 0)
        f_desc = Gtk.Label(label="Il browser pi\u00f9 veloce su ThaiOS. Naviga in sicurezza.")
        f_desc.set_xalign(0)
        f_desc.get_style_context().add_class("desc")
        f_text.pack_start(f_desc, False, False, 0)
        featured.pack_start(f_text, True, True, 0)
        self.app_list.pack_start(featured, False, False, 0)

        for app in apps:
            card = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=12)
            card.get_style_context().add_class("app-card")

            icon_path = f"/usr/share/ThaiOS/icons/scalable/apps/{app['icon']}.svg"
            if os.path.exists(icon_path):
                pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_size(icon_path, 48, 48)
                img = Gtk.Image.new_from_pixbuf(pixbuf)
            else:
                img = Gtk.Image.new_from_icon_name("application-x-executable", Gtk.IconSize.DIALOG)
            card.pack_start(img, False, False, 0)

            info = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=2)
            name_lbl = Gtk.Label(label=app["name"])
            name_lbl.get_style_context().add_class("name")
            name_lbl.set_xalign(0)
            info.pack_start(name_lbl, False, False, 0)

            desc_lbl = Gtk.Label(label=app["desc"])
            desc_lbl.get_style_context().add_class("desc")
            desc_lbl.set_xalign(0)
            info.pack_start(desc_lbl, False, False, 0)

            size_lbl = Gtk.Label(label=app["size"])
            size_lbl.get_style_context().add_class("desc")
            size_lbl.set_xalign(0)
            info.pack_start(size_lbl, False, False, 0)

            card.pack_start(info, True, True, 0)

            btn = Gtk.Button(label="Installato" if app["installed"] else "Installa")
            btn.get_style_context().add_class("install-btn")
            if app["installed"]:
                btn.get_style_context().add_class("installed")
            card.pack_end(btn, False, False, 0)

            self.app_list.pack_start(card, False, False, 0)

        self.show_all()

    def filter_apps(self, entry):
        text = entry.get_text().lower()
        if not text:
            self.render_apps(STORE_APPS)
            return
        filtered = [a for a in STORE_APPS if text in a["name"].lower() or text in a["desc"].lower()]
        self.render_apps(filtered)


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiStore()
    app.show_all()
    Gtk.main()

#!/usr/bin/env python3
"""
ThaiSettings — ThaiOS system settings panel
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
    print("[ThaiSettings] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
    "sidebar": "#0F1A2E",
}

SECTIONS = [
    ("\u26a1 Generale", [
        ("Informazioni sistema", "system-info"),
        ("Lingua e Regione", "locale"),
        ("Data e Ora", "datetime"),
        ("Account", "accounts"),
    ]),
    ("\u25a3 Schermo", [
        ("Display", "display"),
        ("Luminosit\u00e0", "brightness"),
        ("Fondo schermo", "wallpaper"),
        ("Blocco schermo", "lockscreen"),
    ]),
    ("\u266b Audio", [
        ("Suono", "sound"),
        ("Microfono", "microphone"),
        ("Suoni di sistema", "system-sounds"),
    ]),
    ("\u2699 Sistema", [
        ("Rete", "network"),
        ("Bluetooth", "bluetooth"),
        ("Stampanti", "printers"),
        ("Alimentazione", "power"),
        ("Memoria", "storage"),
    ]),
    ("\u2600 Personalizzazione", [
        ("Temi", "themes"),
        ("Scrivania", "desktop"),
        ("Dock", "dock"),
        ("Font", "fonts"),
        ("Icone", "icons"),
    ]),
    ("\u2b06 Aggiornamenti", [
        ("ThaiUpdater", "updater"),
    ]),
]


class ThaiSettings(Gtk.Window):
    def __init__(self):
        super().__init__(title="Thai Settings")
        self.set_default_size(950, 650)
        self.set_position(Gtk.WindowPosition.CENTER)

        screen = Gdk.Screen.get_default()
        css = (
            f"#settings-window {{ background: {COLORS['bg']}; }}"
            f"#sidebar {{ background: {COLORS['sidebar']}; border-right: 1px solid {COLORS['border']}; }}"
            f"#sidebar label.section {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 11px; "
            f"font-weight: bold; padding: 12px 16px 4px; text-transform: uppercase; }}"
            f"#sidebar button {{ background: transparent; border: none; color: {COLORS['text_sec']}; "
            f"font-family: Sarabun; font-size: 13px; padding: 8px 16px; text-align: left; }}"
            f"#sidebar button:hover {{ background: {COLORS['surface']}; color: {COLORS['fg']}; }}"
            f"#sidebar button.active {{ background: {COLORS['surface']}; color: {COLORS['accent']}; "
            f"border-left: 3px solid {COLORS['accent']}; }}"
            f"#content-area {{ background: {COLORS['bg']}; }}"
            f"#content-area label.title {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 24px; font-weight: bold; }}"
            f"#content-area label.desc {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 12px; }}"
            f".setting-row {{ background: {COLORS['surface']}; border: 1px solid {COLORS['border']}; "
            f"border-radius: 12px; padding: 16px; margin: 4px 0; }}"
            f".setting-row label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 13px; }}"
            f".switch {{ background: {COLORS['border']}; border-radius: 12px; }}"
            f".switch.active {{ background: {COLORS['accent']}; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        self.main_box.set_name("settings-window")
        self.add(self.main_box)

        # Sidebar
        sidebar = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        sidebar.set_name("sidebar")
        sidebar.set_size_request(220, -1)

        header = Gtk.Label(label="Thai Settings")
        header.set_markup("<span font='Sarabun 16' color='#E8EDF5'><b>Thai Settings</b></span>")
        header.set_margin(16)
        sidebar.pack_start(header, False, False, 0)

        scroll_s = Gtk.ScrolledWindow()
        scroll_s.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        sidebar_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)

        for section_name, items in SECTIONS:
            sec_label = Gtk.Label(label=section_name)
            sec_label.get_style_context().add_class("section")
            sec_label.set_xalign(0)
            sidebar_box.pack_start(sec_label, False, False, 0)

            for item_name, item_id in items:
                btn = Gtk.Button(label=item_name)
                btn.set_xalign(0)
                btn.connect("clicked", self.show_section, item_id)
                sidebar_box.pack_start(btn, False, False, 0)

        scroll_s.add(sidebar_box)
        sidebar.pack_start(scroll_s, True, True, 0)

        self.main_box.pack_start(sidebar, False, False, 0)

        # Content area
        self.content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.content.set_name("content-area")
        self.content.set_margin(24)

        self.content_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        self.content.pack_start(self.content_box, True, True, 0)
        self.main_box.pack_start(self.content, True, True, 0)

        self.show_section(None, "system-info")
        self.connect("destroy", Gtk.main_quit)

    def show_section(self, btn, section_id):
        for child in self.content_box.get_children():
            self.content_box.remove(child)

        title_map = {
            "system-info": "Informazioni Sistema",
            "display": "Display",
            "sound": "Audio",
            "themes": "Temi",
            "network": "Rete",
            "updater": "Aggiornamenti Sistema",
            "wallpaper": "Sfondo Schermo",
            "fonts": "Caratteri",
        }

        title = Gtk.Label(label=title_map.get(section_id, section_id))
        title.get_style_context().add_class("title")
        title.set_xalign(0)
        self.content_box.pack_start(title, False, False, 0)

        desc = Gtk.Label(label=f"Configura le impostazioni di {title_map.get(section_id, section_id).lower()}")
        desc.get_style_context().add_class("desc")
        desc.set_xalign(0)
        self.content_box.pack_start(desc, False, False, 0)

        self.content_box.pack_start(Gtk.Label(), False, False, 8)

        if section_id == "system-info":
            self.add_info_row("Sistema Operativo", "ThaiOS 1.0 (Songkran)")
            self.add_info_row("Kernel", "6.6.x (ThaiOS)")
            self.add_info_row("Architettura", "x86_64")
            self.add_info_row("Processore", "12th Gen Intel Core i7")
            self.add_info_row("Memoria", "15.4 GB")
            self.add_info_row("ThaiDesktop", "1.0")
        elif section_id == "display":
            self.add_toggle_row("Luminosit\u00e0 Automatica", True)
            self.add_toggle_row("Modalit\u00e0 Scura", True)
            self.add_toggle_row("Effetti di trasparenza", True)
            self.add_toggle_row("Animazioni", True)
        elif section_id == "themes":
            self.add_select_row("Tema", "ThaiOS Dark", ["ThaiOS Light", "ThaiOS Dark", "ThaiOS Classic"])
            self.add_select_row("Accento", "Arancione", ["Rosso", "Arancione", "Blu", "Verde", "Viola"])
            self.add_select_row("Icone", "ThaiOS Icons", ["ThaiOS Icons", "Classic"])
        elif section_id == "sound":
            self.add_slider_row("Volume Sistema", 60)
            self.add_toggle_row("Suoni di sistema", True)
            self.add_select_row("Dispositivo di uscita", "Altoparlanti", ["Altoparlanti", "Cuffie", "HDMI"])
        elif section_id == "network":
            self.add_toggle_row("Wi-Fi", True)
            self.add_info_row("Rete", "Connected - Home Network")
            self.add_info_row("IP", "192.168.1.42")
        elif section_id == "updater":
            self.add_info_row("Stato sistema", "Aggiornato")
            self.add_info_row("Ultimo aggiornamento", "23 Luglio 2025")
            update_btn = Gtk.Button(label="Verifica aggiornamenti")
            update_btn.get_style_context().add_class("install-btn")
            self.content_box.pack_start(update_btn, False, False, 0)

        self.show_all()

    def add_info_row(self, label, value):
        row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        row.get_style_context().add_class("setting-row")
        lbl = Gtk.Label(label=label)
        lbl.set_xalign(0)
        val = Gtk.Label(label=value)
        val.set_xalign(1)
        row.pack_start(lbl, True, True, 0)
        row.pack_end(val, False, False, 0)
        self.content_box.pack_start(row, False, False, 0)

    def add_toggle_row(self, label, active):
        row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        row.get_style_context().add_class("setting-row")
        lbl = Gtk.Label(label=label)
        lbl.set_xalign(0)
        switch = Gtk.Switch()
        switch.set_active(active)
        row.pack_start(lbl, True, True, 0)
        row.pack_end(switch, False, False, 0)
        self.content_box.pack_start(row, False, False, 0)

    def add_slider_row(self, label, value):
        row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        row.get_style_context().add_class("setting-row")
        lbl = Gtk.Label(label=label)
        lbl.set_xalign(0)
        slider = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL)
        slider.set_range(0, 100)
        slider.set_value(value)
        slider.set_size_request(200, 20)
        row.pack_start(lbl, True, True, 0)
        row.pack_end(slider, False, False, 0)
        self.content_box.pack_start(row, False, False, 0)

    def add_select_row(self, label, current, options):
        row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        row.get_style_context().add_class("setting-row")
        lbl = Gtk.Label(label=label)
        lbl.set_xalign(0)
        combo = Gtk.ComboBoxText()
        for opt in options:
            combo.append_text(opt)
        combo.set_active(options.index(current))
        row.pack_start(lbl, True, True, 0)
        row.pack_end(combo, False, False, 0)
        self.content_box.pack_start(row, False, False, 0)


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiSettings()
    app.show_all()
    Gtk.main()

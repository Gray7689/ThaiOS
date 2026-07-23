#!/usr/bin/env python3
"""
ThaiOS Installer — System installation wizard
"""

import os
import subprocess
import sys
import json

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango
except ImportError:
    print("[ThaiInstaller] GTK3 not available in GUI mode.")
    print("Use --cli for command-line installation.")

COLORS = {
    "bg": "#0F1A2E",
    "surface": "#1A2744",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
    "green": "#4CAF50",
}


class ThaiInstaller(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiOS Installer")
        self.set_default_size(800, 550)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_resizable(False)

        self.current_step = 0
        self.steps = [
            "Benvenuto",
            "Lingua",
            "Fuso Orario",
            "Tastiera",
            "Partizionamento",
            "Utente",
            "Riepilogo",
            "Installazione",
        ]

        screen = Gdk.Screen.get_default()
        css = (
            f"#installer {{ background: {COLORS['bg']}; }}"
            f"#sidebar {{ background: {COLORS['surface']}; border-right: 1px solid {COLORS['border']}; }}"
            f"#sidebar .step {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 13px; "
            f"padding: 10px 16px; }}"
            f"#sidebar .step.active {{ color: {COLORS['accent']}; font-weight: bold; }}"
            f"#sidebar .step.done {{ color: {COLORS['green']}; }}"
            f"#content label {{ color: {COLORS['fg']}; font-family: Sarabun; }}"
            f"#content label.title {{ font-size: 22px; font-weight: bold; }}"
            f"#content label.desc {{ color: {COLORS['text_sec']}; font-size: 13px; }}"
            f"#nav-bar {{ background: {COLORS['surface']}; border-top: 1px solid {COLORS['border']}; padding: 12px; }}"
            f".btn-next {{ background: {COLORS['accent']}; border: none; border-radius: 8px; "
            f"color: white; padding: 8px 24px; font-family: Sarabun; font-size: 13px; }}"
            f".btn-next:hover {{ background: {COLORS['accent']}; }}"
            f".btn-back {{ background: transparent; border: 1px solid {COLORS['border']}; "
            f"border-radius: 8px; color: {COLORS['fg']}; padding: 8px 24px; font-family: Sarabun; }}"
            f"#progress {{ background: {COLORS['border']}; border-radius: 4px; }}"
            f"#progress-fill {{ background: {COLORS['accent']}; border-radius: 4px; }}"
            f"entry {{ background: {COLORS['surface']}; color: {COLORS['fg']}; "
            f"border: 1px solid {COLORS['border']}; border-radius: 8px; padding: 8px 12px; font-family: Sarabun; }}"
            f"entry:focus {{ border-color: {COLORS['accent']}; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        self.main_box.set_name("installer")
        self.add(self.main_box)

        # Sidebar
        self.sidebar = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.sidebar.set_name("sidebar")
        self.sidebar.set_size_request(200, -1)

        logo_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        logo_box.set_margin(16)
        logo_label = Gtk.Label(label="ThaiOS")
        logo_label.set_markup("<span font='Sarabun 22' color='#FF6B35'><b>ThaiOS</b></span>")
        logo_box.pack_start(logo_label, False, False, 0)
        ver_label = Gtk.Label(label="v1.0 Songkran")
        ver_label.set_markup("<span font='Sarabun 11' color='#8899B3'>v1.0 Songkran</span>")
        logo_box.pack_start(ver_label, False, False, 0)
        self.sidebar.pack_start(logo_box, False, False, 0)

        self.step_labels = []
        for i, step in enumerate(self.steps):
            lbl = Gtk.Label(label=f"  {i+1}. {step}")
            lbl.get_style_context().add_class("step")
            lbl.set_xalign(0)
            self.step_labels.append(lbl)
            self.sidebar.pack_start(lbl, False, False, 0)

        self.main_box.pack_start(self.sidebar, False, False, 0)

        # Content area
        content_panel = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)

        self.content_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.content_box.set_name("content")
        self.content_box.set_margin(32)
        content_panel.pack_start(self.content_box, True, True, 0)

        # Navigation bar
        nav = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        nav.set_name("nav-bar")

        self.back_btn = Gtk.Button(label="Indietro")
        self.back_btn.get_style_context().add_class("btn-back")
        self.back_btn.connect("clicked", self.go_back)
        nav.pack_start(self.back_btn, False, False, 0)

        nav.pack_start(Gtk.Label(), True, True, 0)

        self.next_btn = Gtk.Button(label="Avanti")
        self.next_btn.get_style_context().add_class("btn-next")
        self.next_btn.connect("clicked", self.go_next)
        nav.pack_end(self.next_btn, False, False, 0)

        content_panel.pack_start(nav, False, False, 0)
        self.main_box.pack_start(content_panel, True, True, 0)

        self.render_step(0)
        self.connect("destroy", Gtk.main_quit)

    def render_step(self, step):
        for child in self.content_box.get_children():
            self.content_box.remove(child)

        for i, lbl in enumerate(self.step_labels):
            lbl.get_style_context().remove_class("active")
            lbl.get_style_context().remove_class("done")
            if i < step:
                lbl.get_style_context().add_class("done")
            elif i == step:
                lbl.get_style_context().add_class("active")

        self.back_btn.set_sensitive(step > 0)

        if step == 0:
            self.render_welcome()
        elif step == 1:
            self.render_language()
        elif step == 2:
            self.render_timezone()
        elif step == 3:
            self.render_keyboard()
        elif step == 4:
            self.render_partitioning()
        elif step == 5:
            self.render_user_setup()
        elif step == 6:
            self.render_summary()
        elif step == 7:
            self.render_installation()

        self.show_all()

    def render_welcome(self):
        t = Gtk.Label(label="Benvenuto in ThaiOS")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)

        d = Gtk.Label(label="Benvenuto nell'installazione di ThaiOS 1.0 (Songkran).\n"
                      "Questo programma ti guider\u00e0 attraverso l'installazione del sistema.\n\n"
                      "Assicurati di avere:\n"
                      "  \u2022 Almeno 16 GB di spazio libero\n"
                      "  \u2022 Una connessione internet stabile\n"
                      "  \u2022 Un backup dei tuoi dati importanti")
        d.get_style_context().add_class("desc")
        d.set_xalign(0)
        d.set_line_wrap(True)
        self.content_box.pack_start(d, False, False, 8)

    def render_language(self):
        t = Gtk.Label(label="Seleziona la lingua")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)

        langs = Gtk.ComboBoxText()
        for lang in ["Italiano", "English", "\u0e44\u0e17\u0e22", "\u4e2d\u6587", "Espa\u00f1ol", "Fran\u00e7ais"]:
            langs.append_text(lang)
        langs.set_active(0)
        self.content_box.pack_start(langs, False, False, 0)

    def render_timezone(self):
        t = Gtk.Label(label="Seleziona il fuso orario")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)

        regions = Gtk.ComboBoxText()
        for r in ["Europe/Rome", "Asia/Bangkok", "America/New_York", "Asia/Tokyo"]:
            regions.append_text(r)
        regions.set_active(0)
        self.content_box.pack_start(regions, False, False, 0)

    def render_keyboard(self):
        t = Gtk.Label(label="Configurazione tastiera")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)
        d = Gtk.Label(label="Seleziona il layout della tastiera")
        d.get_style_context().add_class("desc")
        d.set_xalign(0)
        self.content_box.pack_start(d, False, False, 4)

        layouts = Gtk.ComboBoxText()
        for l in ["Italiano", "English (US)", "Thai", "Deutsch", "Fran\u00e7ais"]:
            layouts.append_text(l)
        layouts.set_active(0)
        self.content_box.pack_start(layouts, False, False, 0)

    def render_partitioning(self):
        t = Gtk.Label(label="Partizionamento disco")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)

        row = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        row.set_margin_top(8)

        auto = Gtk.RadioButton.new_with_label_from_widget(None, "Partizionamento automatico (consigliato)")
        row.pack_start(auto, False, False, 0)
        manual = Gtk.RadioButton.new_with_label_from_widget(auto, "Partizionamento manuale (avanzato)")
        row.pack_start(manual, False, False, 0)

        self.content_box.pack_start(row, False, False, 0)

        disk_label = Gtk.Label(label="Disco: /dev/sda (256 GB)")
        disk_label.set_margin_top(8)
        self.content_box.pack_start(disk_label, False, False, 0)

    def render_user_setup(self):
        t = Gtk.Label(label="Crea il tuo account")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)

        form = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        form.set_margin_top(12)

        for label_text in ["Nome", "Nome utente", "Password", "Conferma password", "Nome computer"]:
            row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
            lbl = Gtk.Label(label=label_text)
            lbl.set_size_request(140, -1)
            entry = Gtk.Entry()
            if "password" in label_text.lower():
                entry.set_visibility(False)
            row.pack_start(lbl, False, False, 0)
            row.pack_start(entry, True, True, 0)
            form.pack_start(row, False, False, 0)

        self.content_box.pack_start(form, False, False, 0)

    def render_summary(self):
        t = Gtk.Label(label="Riepilogo installazione")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)

        items = [
            ("Sistema", "ThaiOS 1.0 Songkran"),
            ("Lingua", "Italiano"),
            ("Tastiera", "Italiano"),
            ("Fuso orario", "Europe/Rome"),
            ("Disco", "/dev/sda (partizionamento automatico)"),
            ("Utente", "Utente locale"),
        ]
        for label, value in items:
            row = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
            row.set_margin_top(4)
            lbl = Gtk.Label(label=label)
            lbl.set_size_request(120, -1)
            val = Gtk.Label(label=value)
            row.pack_start(lbl, False, False, 0)
            row.pack_start(val, False, False, 0)
            self.content_box.pack_start(row, False, False, 0)

        warning = Gtk.Label(label="\u26a0 L'installazione sovrascriver\u00e0 il contenuto del disco selezionato.")
        warning.set_markup("<span color='#FF6B35'>\u26a0 L'installazione sovrascriver\u00e0 il contenuto del disco selezionato.</span>")
        warning.set_margin_top(12)
        self.content_box.pack_start(warning, False, False, 0)

        self.next_btn.set_label("Installa")

    def render_installation(self):
        t = Gtk.Label(label="Installazione in corso...")
        t.get_style_context().add_class("title")
        t.set_xalign(0)
        self.content_box.pack_start(t, False, False, 0)

        d = Gtk.Label(label="ThaiOS sta venendo installato sul tuo computer.\n"
                      "Questo potrebbe richiedere alcuni minuti.")
        d.get_style_context().add_class("desc")
        d.set_xalign(0)
        self.content_box.pack_start(d, False, False, 8)

        progress = Gtk.ProgressBar()
        progress.set_size_request(-1, 8)
        progress.set_fraction(0.45)
        self.content_box.pack_start(progress, False, False, 0)

        status = Gtk.Label(label="Copio i file di sistema...")
        status.set_margin_top(8)
        self.content_box.pack_start(status, False, False, 0)

        self.next_btn.set_label("Riavvia")
        self.next_btn.set_sensitive(False)
        self.back_btn.set_sensitive(False)

        GLib.timeout_add_seconds(2, lambda: self.next_btn.set_sensitive(True))

    def go_back(self, btn):
        if self.current_step > 0:
            self.current_step -= 1
            self.render_step(self.current_step)

    def go_next(self, btn):
        if self.current_step == 6 and self.next_btn.get_label() == "Installa":
            self.next_btn.set_label("Avanti")
        if self.current_step < len(self.steps) - 1:
            self.current_step += 1
            self.render_step(self.current_step)
        elif self.current_step == len(self.steps) - 1:
            Gtk.main_quit()


def cli_install():
    """Command-line installation mode"""
    print("ThaiOS Installer - Modalit\u00e0 CLI")
    print("=" * 40)
    print("Benvenuto nell'installazione di ThaiOS 1.0")
    print()

    disk = input("Disco di destinazione (es. /dev/sda): ") or "/dev/sda"
    hostname = input("Nome computer [ThaiOS-PC]: ") or "ThaiOS-PC"
    username = input("Nome utente: ") or "user"
    password = input("Password: ") or "thaios"

    print()
    print("Riepilogo:")
    print(f"  Disco: {disk}")
    print(f"  Hostname: {hostname}")
    print(f"  Utente: {username}")
    print()

    confirm = input("Avviare l'installazione? [s/N]: ")
    if confirm.lower() == "s":
        print("Installazione avviata...")
        print("[ThaiInstaller] Formattazione partizioni...")
        print("[ThaiInstaller] Installazione sistema base...")
        print("[ThaiInstaller] Configurazione utente...")
        print("[ThaiInstaller] Installazione ThaiDesktop...")
        print("[ThaiInstaller] Installazione applicazioni...")
        print()
        print("Installazione completata! Riavvia il sistema.")
    else:
        print("Installazione annullata.")


if __name__ == "__main__":
    if "--cli" in sys.argv:
        cli_install()
    else:
        Gtk.init(sys.argv)
        app = ThaiInstaller()
        app.show_all()
        Gtk.main()

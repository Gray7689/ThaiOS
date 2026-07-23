#!/usr/bin/env python3
"""
ThaiFiles — ThaiOS file manager with modern UI
"""

import os
import shutil
import subprocess
import sys
from datetime import datetime

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Gio, Pango, GdkPixbuf
except ImportError:
    print("[ThaiFiles] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
    "sidebar": "#0F1A2E",
    "selected": "#FF6B35",
}


class ThaiFiles(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiFiles")
        self.set_default_size(1000, 680)
        self.set_position(Gtk.WindowPosition.CENTER)

        self.current_path = os.path.expanduser("~")

        screen = Gdk.Screen.get_default()
        css = (
            f"#fm-window {{ background: {COLORS['bg']}; }}"
            f"#toolbar {{ background: {COLORS['bg']}; border-bottom: 1px solid {COLORS['border']}; padding: 4px; }}"
            f"#path-bar {{ background: {COLORS['surface']}; color: {COLORS['fg']}; "
            f"border: 1px solid {COLORS['border']}; border-radius: 8px; padding: 4px 12px; "
            f"font-family: Sarabun; font-size: 13px; }}"
            f"#sidebar {{ background: {COLORS['sidebar']}; border-right: 1px solid {COLORS['border']}; }}"
            f"#sidebar label {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 12px; padding: 4px 12px; }}"
            f"#sidebar .item:hover {{ background: {COLORS['surface']}; }}"
            f"#file-view {{ background: {COLORS['bg']}; }}"
            f".file-item {{ background: transparent; border: none; border-radius: 8px; padding: 8px; }}"
            f".file-item:hover {{ background: {COLORS['surface']}; }}"
            f".file-item label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 11px; }}"
            f".file-item .size {{ color: {COLORS['text_sec']}; font-size: 10px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("fm-window")
        self.add(self.main_box)

        # Toolbar
        toolbar = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)
        toolbar.set_name("toolbar")
        toolbar.set_margin_start(8)
        toolbar.set_margin_end(8)

        back_btn = Gtk.Button(label="\u2190")
        back_btn.connect("clicked", lambda b: self.navigate_back())
        toolbar.pack_start(back_btn, False, False, 0)

        fwd_btn = Gtk.Button(label="\u2192")
        toolbar.pack_start(fwd_btn, False, False, 0)

        up_btn = Gtk.Button(label="\u2191")
        up_btn.connect("clicked", lambda b: self.navigate_up())
        toolbar.pack_start(up_btn, False, False, 0)

        self.path_entry = Gtk.Entry()
        self.path_entry.set_name("path-bar")
        self.path_entry.set_text(self.current_path)
        self.path_entry.connect("activate", self.on_path_activate)
        toolbar.pack_start(self.path_entry, True, True, 8)

        search_btn = Gtk.Button(label="\u2315")
        toolbar.pack_end(search_btn, False, False, 0)

        layout_btn = Gtk.Button(label="\u2630")
        toolbar.pack_end(layout_btn, False, False, 0)

        self.main_box.pack_start(toolbar, False, False, 0)

        # Content area
        content = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)

        # Sidebar
        sidebar = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        sidebar.set_name("sidebar")
        sidebar.set_size_request(180, -1)

        places = [
            ("\u2302 Home", os.path.expanduser("~")),
            ("\u2207 Documenti", os.path.expanduser("~/Documents")),
            ("\u2197 Scaricati", os.path.expanduser("~/Downloads")),
            ("\u266a Musica", os.path.expanduser("~/Music")),
            ("\u25a1 Immagini", os.path.expanduser("~/Pictures")),
            ("\u25b6 Video", os.path.expanduser("~/Videos")),
            ("\u2606 Desktop", os.path.expanduser("~/Desktop")),
        ]

        sidebar_header = Gtk.Label(label="RACCOLTE")
        sidebar_header.set_xalign(0)
        sidebar.pack_start(sidebar_header, False, False, 4)

        for label, path in places:
            btn = Gtk.Button(label=label)
            btn.get_style_context().add_class("item")
            btn.set_tooltip_text(path)
            btn.connect("clicked", lambda b, p=path: self.navigate_to(p))
            sidebar.pack_start(btn, False, False, 0)

        sidebar_devices = Gtk.Label(label="DISPOSITIVI")
        sidebar_devices.set_xalign(0)
        sidebar.pack_start(sidebar_devices, False, False, 4)

        drives = ["\u26a1 Sistema", "\u25a2 Dati"]
        for drive in drives:
            btn = Gtk.Button(label=drive)
            btn.get_style_context().add_class("item")
            sidebar.pack_start(btn, False, False, 0)

        content.pack_start(sidebar, False, False, 0)

        # File view
        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scroll.set_name("file-view")

        self.file_flow = Gtk.FlowBox()
        self.file_flow.set_max_children_per_line(0)
        self.file_flow.set_min_children_per_line(4)
        self.file_flow.set_selection_mode(Gtk.SelectionMode.MULTIPLE)
        self.file_flow.set_homogeneous(True)
        self.file_flow.set_activate_on_single_click(True)
        self.file_flow.set_margin(12)
        self.file_flow.set_column_spacing(8)
        self.file_flow.set_row_spacing(8)
        self.file_flow.connect("child-activated", self.on_file_activated)

        scroll.add(self.file_flow)
        content.pack_start(scroll, True, True, 0)

        self.main_box.pack_start(content, True, True, 0)

        # Status bar
        status = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        status.set_name("toolbar")
        self.status_label = Gtk.Label(label="Pronto")
        self.status_label.set_margin_start(12)
        status.pack_start(self.status_label, False, False, 0)
        self.main_box.pack_start(status, False, False, 0)

        self.load_directory(self.current_path)
        self.connect("destroy", Gtk.main_quit)

    def load_directory(self, path):
        self.current_path = path
        self.path_entry.set_text(path)

        for child in self.file_flow.get_children():
            self.file_flow.remove(child)

        try:
            entries = sorted(os.listdir(path))
        except PermissionError:
            self.status_label.set_text("Permesso negato")
            return

        dirs = []
        files = []
        for name in entries:
            full = os.path.join(path, name)
            if os.path.isdir(full):
                dirs.append(name)
            else:
                files.append(name)

        if path != "/":
            parent_btn = Gtk.Button()
            vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
            img = Gtk.Image.new_from_icon_name("go-up", Gtk.IconSize.DIALOG)
            vbox.pack_start(img, False, False, 0)
            lbl = Gtk.Label(label="...")
            vbox.pack_start(lbl, False, False, 0)
            parent_btn.add(vbox)
            parent_btn.connect("clicked", lambda b: self.navigate_up())
            self.file_flow.add(parent_btn)

        for name in dirs + files:
            full = os.path.join(path, name)
            is_dir = os.path.isdir(full)

            card = Gtk.Button()
            card.get_style_context().add_class("file-item")

            vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)

            icon_name = "folder" if is_dir else "text-x-generic"
            img = Gtk.Image.new_from_icon_name(icon_name, Gtk.IconSize.DIALOG)
            vbox.pack_start(img, False, False, 0)

            name_label = Gtk.Label(label=name)
            name_label.set_line_wrap(True)
            name_label.set_max_width_chars(12)
            name_label.set_justify(Gtk.Justification.CENTER)
            vbox.pack_start(name_label, False, False, 0)

            try:
                size = os.path.getsize(full)
                if is_dir:
                    size_text = "Cartella"
                elif size < 1024:
                    size_text = f"{size} B"
                elif size < 1024**2:
                    size_text = f"{size // 1024} KB"
                else:
                    size_text = f"{size / (1024**2):.1f} MB"
                size_label = Gtk.Label(label=size_text)
                size_label.get_style_context().add_class("size")
                vbox.pack_start(size_label, False, False, 0)
            except OSError:
                pass

            card.add(vbox)
            card.connect("clicked", lambda b, p=full, d=is_dir: self.on_item_click(p, d))
            self.file_flow.add(card)

        count = len(dirs) + len(files)
        self.status_label.set_text(f"{count} elementi")

        self.show_all()

    def on_item_click(self, path, is_dir):
        if is_dir:
            self.load_directory(path)

    def on_file_activated(self, flow, child):
        pass

    def navigate_to(self, path):
        self.load_directory(path)

    def navigate_back(self):
        parent = os.path.dirname(self.current_path)
        self.load_directory(parent)

    def navigate_up(self):
        parent = os.path.dirname(self.current_path)
        if parent != self.current_path:
            self.load_directory(parent)

    def on_path_activate(self, entry):
        path = entry.get_text().strip()
        path = os.path.expanduser(path)
        if os.path.isdir(path):
            self.load_directory(path)


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiFiles()
    app.show_all()
    Gtk.main()

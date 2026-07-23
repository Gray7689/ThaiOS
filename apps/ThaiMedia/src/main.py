#!/usr/bin/env python3
"""
ThaiMedia — ThaiOS media player
"""

import os
import subprocess
import sys

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango, GdkPixbuf
except ImportError:
    print("[ThaiMedia] GTK3 not available.")
    sys.exit(1)

COLORS = {
    "bg": "#0F1A2E",
    "surface": "#1A2744",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
}


class ThaiMedia(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiMedia")
        self.set_default_size(900, 600)
        self.set_position(Gtk.WindowPosition.CENTER)

        screen = Gdk.Screen.get_default()
        css = (
            f"#media-window {{ background: {COLORS['bg']}; }}"
            f"#header {{ background: {COLORS['surface']}; border-bottom: 1px solid {COLORS['border']}; padding: 12px; }}"
            f"#header label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 16px; }}"
            f"#sidebar {{ background: rgba(15, 26, 46, 0.5); border-right: 1px solid {COLORS['border']}; }}"
            f"#sidebar button {{ background: transparent; border: none; color: {COLORS['text_sec']}; "
            f"padding: 8px 16px; font-family: Sarabun; font-size: 12px; }}"
            f"#sidebar button:hover {{ color: {COLORS['fg']}; }}"
            f"#player-area {{ background: {COLORS['bg']}; }}"
            f"#controls {{ background: {COLORS['surface']}; border-top: 1px solid {COLORS['border']}; padding: 12px; }}"
            f"#controls button {{ background: transparent; border: none; color: {COLORS['fg']}; "
            f"font-size: 20px; padding: 4px 16px; }}"
            f"#controls button:hover {{ color: {COLORS['accent']}; }}"
            f".track {{ background: transparent; border: none; border-radius: 8px; padding: 8px 16px; }}"
            f".track:hover {{ background: {COLORS['surface']}; }}"
            f".track label {{ color: {COLORS['fg']}; font-family: Sarabun; font-size: 12px; }}"
            f".track .artist {{ color: {COLORS['text_sec']}; font-size: 11px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("media-window")
        self.add(self.main_box)

        # Header
        header = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        header.set_name("header")
        title = Gtk.Label(label="ThaiMedia")
        header.pack_start(title, False, False, 0)
        self.main_box.pack_start(header, False, False, 0)

        # Content area
        content = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)

        # Sidebar
        sidebar = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        sidebar.set_name("sidebar")
        sidebar.set_size_request(180, -1)
        for item in ["\u25b6 In Riproduzione", "\u266b Brani", "\u25a3 Album", "\u2692 Playlist"]:
            btn = Gtk.Button(label=item)
            btn.set_xalign(0)
            sidebar.pack_start(btn, False, False, 0)
        content.pack_start(sidebar, False, False, 0)

        # Player area
        player = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        player.set_name("player-area")

        # Album art + info
        center = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        center.set_valign(Gtk.Align.CENTER)
        center.set_halign(Gtk.Align.CENTER)
        center.set_margin(40)

        album_art = Gtk.Image.new_from_icon_name("audio-x-generic", Gtk.IconSize.DIALOG)
        album_art.set_size_request(200, 200)
        center.pack_start(album_art, False, False, 0)

        song_title = Gtk.Label(label="Nessun brano in riproduzione")
        song_title.set_markup("<span font='Sarabun 18' color='#E8EDF5'>Nessun brano in riproduzione</span>")
        center.pack_start(song_title, False, False, 4)

        artist = Gtk.Label(label="Apri un file per iniziare")
        artist.get_style_context().add_class("artist")
        center.pack_start(artist, False, False, 0)

        player.pack_start(center, True, True, 0)

        # Controls
        controls = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        controls.set_name("controls")
        controls.set_valign(Gtk.Align.END)

        controls.pack_start(Gtk.Label(), True, True, 0)

        for icon in ["\u23ee", "\u25b6", "\u23ed"]:
            btn = Gtk.Button(label=icon)
            btn.get_style_context().add_class("controls")
            controls.pack_start(btn, False, False, 0)

        self.vol_btn = Gtk.Button(label="\u266a")
        controls.pack_start(self.vol_btn, False, False, 0)

        controls.pack_start(Gtk.Label(), True, True, 0)
        player.pack_start(controls, False, False, 0)

        content.pack_start(player, True, True, 0)
        self.main_box.pack_start(content, True, True, 0)

        self.connect("destroy", Gtk.main_quit)


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiMedia()
    app.show_all()
    Gtk.main()

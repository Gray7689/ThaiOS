#!/usr/bin/env python3
"""
ThaiTerminal — ThaiOS terminal emulator
"""

import os
import subprocess
import sys

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    gi.require_version("Vte", "2.91")
    from gi.repository import GLib, Gtk, Gdk, Vte, Pango
except ImportError:
    try:
        import gi
        gi.require_version("GLib", "2.0")
        gi.require_version("Gtk", "3.0")
        from gi.repository import GLib, Gtk, Gdk, Pango
        FALLBACK = True
    except ImportError:
        print("[ThaiTerminal] VTE terminal widget not available. Install libvte.")
        sys.exit(1)
    else:
        FALLBACK = True
else:
    FALLBACK = False

COLORS = {
    "bg": "#0F1A2E",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "surface": "#1A2744",
    "border": "#2D4066",
    "black": "#0F1A2E",
    "red": "#E63946",
    "green": "#4CAF50",
    "yellow": "#F7C59F",
    "blue": "#457B9D",
    "magenta": "#E63946",
    "cyan": "#A8DADC",
    "white": "#E8EDF5",
}


class ThaiTerminal(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiTerminal")
        self.set_default_size(800, 500)
        self.set_position(Gtk.WindowPosition.CENTER)

        screen = Gdk.Screen.get_default()
        css = (
            f"#term-window {{ background: {COLORS['bg']}; }}"
            f"#term-toolbar {{ background: {COLORS['surface']}; border-bottom: 1px solid {COLORS['border']}; padding: 4px; }}"
            f"#term-toolbar button {{ background: transparent; border: none; color: {COLORS['text_sec']}; "
            f"padding: 4px 10px; font-family: Sarabun; font-size: 12px; }}"
            f"#term-toolbar button:hover {{ color: {COLORS['fg']}; }}"
            f"#term-toolbar label {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 11px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("term-window")
        self.add(self.main_box)

        # Toolbar
        toolbar = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)
        toolbar.set_name("term-toolbar")
        toolbar.set_margin_start(8)
        toolbar.set_margin_end(8)

        title_lbl = Gtk.Label(label="ThaiTerminal")
        toolbar.pack_start(title_lbl, False, False, 0)

        toolbar.pack_start(Gtk.Label(), True, True, 0)

        new_tab_btn = Gtk.Button(label="+")
        new_tab_btn.connect("clicked", self.new_terminal)
        toolbar.pack_end(new_tab_btn, False, False, 0)

        self.main_box.pack_start(toolbar, False, False, 0)

        # Terminal area
        if not FALLBACK:
            self.notebook = Gtk.Notebook()
            self.notebook.set_show_tabs(True)
            self.notebook.set_show_border(False)
            self.main_box.pack_start(self.notebook, True, True, 0)
            self.new_terminal()
        else:
            # Fallback: use a text view as simple terminal
            self.textview = Gtk.TextView()
            self.textview.set_editable(True)
            self.textview.set_cursor_visible(True)
            self.textview.modify_font(Pango.FontDescription("monospace 12"))
            self.textview.modify_bg(Gtk.StateType.NORMAL, Gdk.color_parse(COLORS['bg']))
            self.textview.modify_fg(Gtk.StateType.NORMAL, Gdk.color_parse(COLORS['fg']))
            scroll = Gtk.ScrolledWindow()
            scroll.add(self.textview)
            self.main_box.pack_start(scroll, True, True, 0)

        self.connect("destroy", Gtk.main_quit)

    def new_terminal(self, btn=None):
        if FALLBACK:
            return
        term = Vte.Terminal()
        term.set_font(Pango.FontDescription("Sarabun-ExtraLight 13"))
        term.set_colors(
            Gdk.RGBA(*[int(COLORS['fg'][i:i+2], 16)/255 for i in (1, 3, 5)]),
            Gdk.RGBA(*[int(COLORS['bg'][i:i+2], 16)/255 for i in (1, 3, 5)]),
            []
        )
        term.set_cursor_blink_mode(Vte.CursorBlinkMode.ON)
        term.set_cursor_shape(Vte.CursorShape.BLOCK)
        term.set_scrollback_lines(10000)

        shell = os.environ.get("SHELL", "/bin/bash")
        term.spawn_sync(
            Vte.PtyFlags.DEFAULT,
            os.path.expanduser("~"),
            [shell],
            [],
            GLib.SpawnFlags.DO_NOT_REAP_CHILD,
            None,
            None,
            None
        )

        page_num = self.notebook.append_page(term)
        tab_label = Gtk.Label(label=f"Terminale {page_num + 1}")
        self.notebook.set_tab_label(term, tab_label)
        self.notebook.set_current_page(page_num)
        self.show_all()


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiTerminal()
    app.show_all()
    Gtk.main()

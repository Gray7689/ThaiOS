#!/usr/bin/env python3
"""
ThaiEditor — ThaiOS text editor
"""

import os
import sys

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    from gi.repository import GLib, Gtk, Gdk, Pango, GtkSource
except ImportError:
    try:
        import gi
        gi.require_version("GLib", "2.0")
        gi.require_version("Gtk", "3.0")
        from gi.repository import GLib, Gtk, Gdk, Pango
        HAS_GTKSOURCE = False
    except ImportError:
        print("[ThaiEditor] GTK3 not available.")
        sys.exit(1)
    else:
        HAS_GTKSOURCE = False
else:
    HAS_GTKSOURCE = True

COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
    "line_numbers": "#3D5A80",
    "cursor": "#FF6B35",
}


class ThaiEditor(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiEditor")
        self.set_default_size(900, 600)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.current_file = None
        self.modified = False

        screen = Gdk.Screen.get_default()
        css = (
            f"#editor-window {{ background: {COLORS['bg']}; }}"
            f"#toolbar {{ background: {COLORS['surface']}; border-bottom: 1px solid {COLORS['border']}; padding: 4px; }}"
            f"#toolbar button {{ background: transparent; border: none; color: {COLORS['text_sec']}; "
            f"padding: 4px 10px; font-family: Sarabun; font-size: 12px; }}"
            f"#toolbar button:hover {{ background: {COLORS['border']}; color: {COLORS['fg']}; }}"
            f"#statusbar {{ background: {COLORS['surface']}; border-top: 1px solid {COLORS['border']}; "
            f"padding: 4px 12px; }}"
            f"#statusbar label {{ color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 11px; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("editor-window")
        self.add(self.main_box)

        # Toolbar
        toolbar = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=2)
        toolbar.set_name("toolbar")
        toolbar.set_margin_start(8)
        toolbar.set_margin_end(8)

        actions = [
            ("\u2714 Nuovo", self.new_file),
            ("\u21e7 Apri", self.open_file),
            ("\u21e9 Salva", self.save_file),
            ("\u2699 Esporta", None),
        ]
        for label, cb in actions:
            btn = Gtk.Button(label=label)
            if cb:
                btn.connect("clicked", cb)
            toolbar.pack_start(btn, False, False, 0)

        self.main_box.pack_start(toolbar, False, False, 0)

        # Editor area
        if HAS_GTKSOURCE:
            self.buffer = GtkSource.Buffer()
            lang_manager = GtkSource.LanguageManager.get_default()
            self.buffer.set_highlight_syntax(True)
            self.sourceview = GtkSource.View.new_with_buffer(self.buffer)
            self.sourceview.set_show_line_numbers(True)
            self.sourceview.set_tab_width(4)
            self.sourceview.set_auto_indent(True)
            self.sourceview.set_insert_spaces_instead_of_tabs(True)
        else:
            self.buffer = Gtk.TextBuffer()
            self.sourceview = Gtk.TextView.new_with_buffer(self.buffer)

        self.sourceview.modify_font(Pango.FontDescription("Sarabun-ExtraLight 13"))

        # Colors
        self.sourceview.modify_bg(Gtk.StateType.NORMAL, Gdk.color_parse(COLORS['bg']))
        self.sourceview.modify_fg(Gtk.StateType.NORMAL, Gdk.color_parse(COLORS['fg']))
        self.sourceview.modify_bg(Gtk.StateType.SELECTED, Gdk.color_parse(COLORS['accent']))
        self.sourceview.modify_fg(Gtk.StateType.SELECTED, Gdk.color_parse("#FFFFFF"))

        self.sourceview.set_wrap_mode(Gtk.WrapMode.WORD_CHAR)
        self.sourceview.set_cursor_visible(True)

        scroll = Gtk.ScrolledWindow()
        scroll.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scroll.add(self.sourceview)
        self.main_box.pack_start(scroll, True, True, 0)

        # Status bar
        status = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        status.set_name("statusbar")
        self.status_label = Gtk.Label(label="Pronto")
        status.pack_start(self.status_label, False, False, 0)

        self.cursor_pos = Gtk.Label(label="Ln 1, Col 1")
        status.pack_end(self.cursor_pos, False, False, 0)
        self.main_box.pack_start(status, False, False, 0)

        self.buffer.connect("changed", self.on_buffer_changed)
        self.sourceview.connect("move-cursor", self.update_cursor_pos)

        self.connect("destroy", Gtk.main_quit)

    def new_file(self, btn):
        self.buffer.set_text("")
        self.current_file = None
        self.modified = False
        self.set_title("ThaiEditor - Nuovo file")
        self.status_label.set_text("Nuovo file")

    def open_file(self, btn):
        dialog = Gtk.FileChooserDialog("Apri file", self, Gtk.FileChooserAction.OPEN,
                                       (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                        Gtk.STOCK_OPEN, Gtk.ResponseType.ACCEPT))
        if dialog.run() == Gtk.ResponseType.ACCEPT:
            self.current_file = dialog.get_filename()
            try:
                with open(self.current_file, "r") as f:
                    content = f.read()
                self.buffer.set_text(content)
                self.set_title(f"ThaiEditor - {os.path.basename(self.current_file)}")
                self.status_label.set_text(f"Aperto: {self.current_file}")
                self.modified = False
            except Exception as e:
                self.status_label.set_text(f"Errore: {e}")
        dialog.destroy()

    def save_file(self, btn):
        if not self.current_file:
            dialog = Gtk.FileChooserDialog("Salva file", self, Gtk.FileChooserAction.SAVE,
                                           (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                            Gtk.STOCK_SAVE, Gtk.ResponseType.ACCEPT))
            dialog.set_do_overwrite_confirmation(True)
            if dialog.run() == Gtk.ResponseType.ACCEPT:
                self.current_file = dialog.get_filename()
            else:
                dialog.destroy()
                return
            dialog.destroy()

        try:
            start = self.buffer.get_start_iter()
            end = self.buffer.get_end_iter()
            content = self.buffer.get_text(start, end, True)
            with open(self.current_file, "w") as f:
                f.write(content)
            self.modified = False
            self.set_title(f"ThaiEditor - {os.path.basename(self.current_file)}")
            self.status_label.set_text(f"Salvato: {self.current_file}")
        except Exception as e:
            self.status_label.set_text(f"Errore: {e}")

    def on_buffer_changed(self, buffer):
        if not self.modified:
            self.modified = True
            title = self.get_title()
            if not title.endswith("*"):
                self.set_title(title + " *")

    def update_cursor_pos(self, view, step, count, extend):
        cursor = self.buffer.get_insert()
        mark = self.buffer.get_iter_at_mark(cursor)
        line = mark.get_line() + 1
        col = mark.get_line_offset() + 1
        self.cursor_pos.set_text(f"Ln {line}, Col {col}")


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiEditor()
    app.show_all()
    Gtk.main()

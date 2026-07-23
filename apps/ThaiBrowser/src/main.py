#!/usr/bin/env python3
"""
ThaiBrowser — ThaiOS web browser
Built on WebKitGTK with ThaiOS design language
"""

import os
import sys

try:
    import gi
    gi.require_version("GLib", "2.0")
    gi.require_version("Gtk", "3.0")
    gi.require_version("WebKit2", "4.0")
    from gi.repository import GLib, Gtk, Gdk, WebKit2, Pango
except ImportError:
    print("[ThaiBrowser] WebKitGTK not available.")
    sys.exit(1)


COLORS = {
    "bg": "#1A2744",
    "surface": "#243456",
    "fg": "#E8EDF5",
    "accent": "#FF6B35",
    "border": "#2D4066",
    "text_sec": "#8899B3",
    "tab_bg": "#0F1A2E",
}


class ThaiBrowser(Gtk.Window):
    def __init__(self):
        super().__init__(title="ThaiBrowser")
        self.set_default_size(1100, 720)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.maximize()

        screen = Gdk.Screen.get_default()
        css = (
            f"#browser-window {{ background: {COLORS['tab_bg']}; }}"
            f"#nav-bar {{ background: {COLORS['tab_bg']}; border-bottom: 1px solid {COLORS['border']}; padding: 4px; }}"
            f"#url-entry {{ background: {COLORS['surface']}; color: {COLORS['fg']}; "
            f"border: 1px solid {COLORS['border']}; border-radius: 20px; padding: 6px 16px; "
            f"font-family: Sarabun; font-size: 13px; }}"
            f"#url-entry:focus {{ border-color: {COLORS['accent']}; }}"
            f".nav-btn {{ background: transparent; border: none; border-radius: 8px; "
            f"padding: 6px 12px; color: {COLORS['fg']}; font-size: 16px; }}"
            f".nav-btn:hover {{ background: {COLORS['surface']}; }}"
            f"#tab-bar {{ background: {COLORS['tab_bg']}}};"
            f".tab {{ background: {COLORS['surface']}; border: none; border-radius: 8px 8px 0 0; "
            f"padding: 6px 16px; color: {COLORS['text_sec']}; font-family: Sarabun; font-size: 12px; }}"
            f".tab.active {{ background: {COLORS['bg']}; color: {COLORS['fg']}; }}"
        )
        style = Gtk.CssProvider()
        style.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(screen, style, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.main_box.set_name("browser-window")
        self.add(self.main_box)

        # Tab bar
        self.tab_bar = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=2)
        self.tab_bar.set_name("tab-bar")
        self.tab_bar.set_margin_start(8)
        self.tab_bar.set_margin_top(4)

        self.tab_list = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=1)
        self.tab_bar.pack_start(self.tab_list, True, True, 0)

        new_tab_btn = Gtk.Button(label="+")
        new_tab_btn.get_style_context().add_class("nav-btn")
        new_tab_btn.connect("clicked", self.add_new_tab)
        self.tab_bar.pack_end(new_tab_btn, False, False, 4)

        self.main_box.pack_start(self.tab_bar, False, False, 0)

        # Navigation bar
        nav_bar = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=4)
        nav_bar.set_name("nav-bar")
        nav_bar.set_margin_start(8)
        nav_bar.set_margin_end(8)

        self.back_btn = Gtk.Button(label="\u2190")
        self.back_btn.get_style_context().add_class("nav-btn")
        nav_bar.pack_start(self.back_btn, False, False, 0)

        self.fwd_btn = Gtk.Button(label="\u2192")
        self.fwd_btn.get_style_context().add_class("nav-btn")
        nav_bar.pack_start(self.fwd_btn, False, False, 0)

        self.refresh_btn = Gtk.Button(label="\u21bb")
        self.refresh_btn.get_style_context().add_class("nav-btn")
        nav_bar.pack_start(self.refresh_btn, False, False, 0)

        self.url_entry = Gtk.Entry()
        self.url_entry.set_name("url-entry")
        self.url_entry.set_placeholder_text("Cerca o inserisci URL...")
        self.url_entry.connect("activate", self.navigate_to_url)
        nav_bar.pack_start(self.url_entry, True, True, 8)

        self.bookmark_btn = Gtk.Button(label="\u2606")
        self.bookmark_btn.get_style_context().add_class("nav-btn")
        nav_bar.pack_end(self.bookmark_btn, False, False, 0)

        self.main_box.pack_start(nav_bar, False, False, 0)

        # WebView stack
        self.notebook = Gtk.Notebook()
        self.notebook.set_show_tabs(False)
        self.notebook.set_show_border(False)
        self.main_box.pack_start(self.notebook, True, True, 0)

        self.add_new_tab()

        self.back_btn.connect("clicked", lambda b: self.get_current_webview().go_back())
        self.fwd_btn.connect("clicked", lambda b: self.get_current_webview().go_forward())
        self.refresh_btn.connect("clicked", lambda b: self.get_current_webview().reload())

        self.connect("destroy", Gtk.main_quit)

    def add_new_tab(self, btn=None, url="https://thaios.dev"):
        webview = WebKit2.WebView()
        webview.set_hexpand(True)
        webview.set_vexpand(True)
        webview.load_uri(url)
        webview.connect("notify::uri", self.on_uri_changed)
        webview.connect("notify::title", self.on_title_changed)

        page_num = self.notebook.append_page(webview, None)

        tab_btn = Gtk.Button(label="Nuova scheda")
        tab_btn.get_style_context().add_class("tab")
        tab_btn.connect("clicked", self.switch_tab, page_num)
        self.tab_list.pack_start(tab_btn, False, False, 0)
        tab_btn.show()

        self.notebook.set_current_page(page_num)
        self.update_tab_styles()

    def get_current_webview(self):
        page = self.notebook.get_current_page()
        if page >= 0:
            return self.notebook.get_nth_page(page)
        return None

    def switch_tab(self, btn, page_num):
        self.notebook.set_current_page(page_num)
        self.update_tab_styles()
        wv = self.notebook.get_nth_page(page_num)
        if wv and wv.get_uri():
            self.url_entry.set_text(wv.get_uri())

    def navigate_to_url(self, entry):
        url = entry.get_text().strip()
        if not url.startswith(("http://", "https://")):
            url = "https://" + url
        wv = self.get_current_webview()
        if wv:
            wv.load_uri(url)

    def on_uri_changed(self, webview, prop):
        uri = webview.get_uri()
        if uri:
            self.url_entry.set_text(uri)

    def on_title_changed(self, webview, prop):
        title = webview.get_title()
        if title:
            self.set_title(f"ThaiBrowser - {title}")

    def update_tab_styles(self):
        current = self.notebook.get_current_page()
        for i, child in enumerate(self.tab_list.get_children()):
            if i == current:
                child.get_style_context().add_class("active")
            else:
                child.get_style_context().remove_class("active")


if __name__ == "__main__":
    Gtk.init(sys.argv)
    app = ThaiBrowser()
    app.show_all()
    Gtk.main()

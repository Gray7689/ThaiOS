#ifndef THAIDESKTOP_CONFIG_H
#define THAIDESKTOP_CONFIG_H

struct thai_config {
    char *theme_path;
    char *icon_theme;
    char *cursor_theme;
    char *font_family;
    int font_size;
    int animation_speed;
    // Layout
    int panel_height;
    int dock_size;
    int dock_margin;
    // Behavior
    int focus_follows_mouse;
    int tap_to_click;
    int natural_scroll;
    int corner_delay;
    // Keyboard
    char *keyboard_layout;
    char *keyboard_variant;
    // Wallpaper
    char *wallpaper_path;
    char *wallpaper_mode;
};

struct thai_config *thai_config_load(const char *path);
void thai_config_destroy(struct thai_config *config);

#endif

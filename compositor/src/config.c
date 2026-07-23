#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

struct thai_config *thai_config_load(const char *path) {
    struct thai_config *config = calloc(1, sizeof(*config));
    if (!config) return NULL;

    config->theme_path = strdup("/usr/share/ThaiOS/themes/ThaiOS-Dark/index.theme");
    config->icon_theme = strdup("ThaiOS-Icons");
    config->cursor_theme = strdup("ThaiOS-Cursors");
    config->font_family = strdup("Sarabun");
    config->font_size = 12;
    config->animation_speed = 200;
    config->panel_height = 36;
    config->dock_size = 48;
    config->dock_margin = 8;
    config->focus_follows_mouse = 0;
    config->tap_to_click = 1;
    config->natural_scroll = 0;
    config->keyboard_layout = strdup("us");
    config->wallpaper_path = strdup("/usr/share/ThaiOS/wallpapers/ThaiOS-day.png");
    config->wallpaper_mode = strdup("fill");

    FILE *f = fopen(path, "r");
    if (!f) {
        return config;
    }

    char line[512];
    char section[64] = "";
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p && isspace(*p)) p++;
        if (!*p || *p == '#') continue;

        if (*p == '[') {
            char *end = strchr(p, ']');
            if (end) {
                *end = '\0';
                strncpy(section, p + 1, sizeof(section) - 1);
            }
            continue;
        }

        char *eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = p;
        char *value = eq + 1;

        while (key < eq && isspace(*key)) key++;
        char *ke = eq - 1;
        while (ke > key && isspace(*ke)) *ke-- = '\0';

        while (*value && isspace(*value)) value++;
        char *ve = value + strlen(value) - 1;
        while (ve > value && isspace(*ve)) *ve-- = '\0';

        if (strcmp(section, "Theme") == 0) {
            if (strcmp(key, "name") == 0) {
                free(config->theme_path);
                config->theme_path = strdup(value);
            }
        } else if (strcmp(section, "Display") == 0) {
            if (strcmp(key, "DefaultFont") == 0) {
                free(config->font_family);
                config->font_family = strdup(value);
            }
            if (strcmp(key, "DefaultIconTheme") == 0) {
                free(config->icon_theme);
                config->icon_theme = strdup(value);
            }
            if (strcmp(key, "DefaultCursorTheme") == 0) {
                free(config->cursor_theme);
                config->cursor_theme = strdup(value);
            }
        } else if (strcmp(section, "Input") == 0) {
            if (strcmp(key, "focus_follows_mouse") == 0) config->focus_follows_mouse = atoi(value);
            if (strcmp(key, "tap_to_click") == 0) config->tap_to_click = atoi(value);
            if (strcmp(key, "natural_scroll") == 0) config->natural_scroll = atoi(value);
        } else if (strcmp(section, "Keyboard") == 0) {
            if (strcmp(key, "layout") == 0) {
                free(config->keyboard_layout);
                config->keyboard_layout = strdup(value);
            }
        }
    }

    fclose(f);
    return config;
}

void thai_config_destroy(struct thai_config *config) {
    if (!config) return;
    free(config->theme_path);
    free(config->icon_theme);
    free(config->cursor_theme);
    free(config->font_family);
    free(config->keyboard_layout);
    free(config->wallpaper_path);
    free(config->wallpaper_mode);
    free(config);
}

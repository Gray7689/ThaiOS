#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theme.h"

static uint32_t parse_color(const char *str) {
    if (!str || !*str) return 0x000000;
    if (*str == '#') str++;
    unsigned long val = strtoul(str, NULL, 16);
    return (uint32_t)val;
}

struct thai_theme *thai_theme_load(const char *path) {
    struct thai_theme *theme = calloc(1, sizeof(*theme));
    if (!theme) return NULL;

    theme->name = strdup("ThaiOS-Dark");
    theme->background = 0x0F1A2E;
    theme->foreground = 0xE8EDF5;
    theme->accent = 0xFF6B35;
    theme->surface = 0x1A2744;
    theme->surface_alt = 0x243456;
    theme->border = 0x2D4066;
    theme->text = 0xE8EDF5;
    theme->text_secondary = 0x8899B3;
    theme->selection = 0xFF6B35;
    theme->selection_text = 0xFFFFFF;
    theme->corner_radius = 12;
    theme->border_width = 1;
    theme->animation_duration = 200;
    theme->font_family = strdup("Sarabun");
    theme->font_size = 12;

    FILE *f = fopen(path, "r");
    if (!f) {
        return theme;
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

        if (strcmp(section, "Colors") == 0) {
            if (strcmp(key, "Background") == 0) theme->background = parse_color(value);
            if (strcmp(key, "Foreground") == 0) theme->foreground = parse_color(value);
            if (strcmp(key, "Accent") == 0) theme->accent = parse_color(value);
            if (strcmp(key, "Surface") == 0) theme->surface = parse_color(value);
            if (strcmp(key, "SurfaceAlt") == 0) theme->surface_alt = parse_color(value);
            if (strcmp(key, "Border") == 0) theme->border = parse_color(value);
            if (strcmp(key, "Text") == 0) theme->text = parse_color(value);
            if (strcmp(key, "TextSecondary") == 0) theme->text_secondary = parse_color(value);
            if (strcmp(key, "Selection") == 0) theme->selection = parse_color(value);
            if (strcmp(key, "SelectionText") == 0) theme->selection_text = parse_color(value);
        } else if (strcmp(section, "Desktop") == 0) {
            if (strcmp(key, "DefaultFont") == 0) {
                free(theme->font_family);
                theme->font_family = strdup(value);
            }
        }
    }

    fclose(f);
    return theme;
}

void thai_theme_destroy(struct thai_theme *theme) {
    if (!theme) return;
    free(theme->name);
    free(theme->font_family);
    free(theme);
}

#ifndef THAIDESKTOP_THEME_H
#define THAIDESKTOP_THEME_H

#include <pixman.h>
#include <stdint.h>

struct thai_theme {
    char *name;
    // Colors
    uint32_t background;
    uint32_t foreground;
    uint32_t accent;
    uint32_t surface;
    uint32_t surface_alt;
    uint32_t border;
    uint32_t text;
    uint32_t text_secondary;
    uint32_t selection;
    uint32_t selection_text;
    // Geometry
    int corner_radius;
    int border_width;
    // Animations
    int animation_duration;
    // Fonts
    char *font_family;
    int font_size;
};

struct thai_theme *thai_theme_load(const char *path);
void thai_theme_destroy(struct thai_theme *theme);

// Color helpers
static inline uint32_t rgba_to_hex(float r, float g, float b, float a) {
    (void)a;
    uint8_t ri = (uint8_t)(r * 255.0f);
    uint8_t gi = (uint8_t)(g * 255.0f);
    uint8_t bi = (uint8_t)(b * 255.0f);
    return (ri << 16) | (gi << 8) | bi;
}

static inline void hex_to_rgba(uint32_t hex, float *r, float *g, float *b, float *a) {
    *r = ((hex >> 16) & 0xFF) / 255.0f;
    *g = ((hex >> 8) & 0xFF) / 255.0f;
    *b = (hex & 0xFF) / 255.0f;
    *a = 1.0f;
}

#endif

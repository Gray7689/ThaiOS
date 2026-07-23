// ThaiOS Wiri Theme Engine
// ==========================
// Sistema di temi con supporto per modalità chiara/scura,
// palette colori configurabile e animazioni fluide.

#include <thaios.h>

typedef struct wiri_theme {
    char name[64];
    bool is_dark;

    // Core colors
    u32 bg_primary;
    u32 bg_secondary;
    u32 bg_tertiary;
    u32 bg_surface;
    u32 bg_elevated;

    // Text colors
    u32 text_primary;
    u32 text_secondary;
    u32 text_disabled;
    u32 text_link;

    // Accent colors
    u32 accent;
    u32 accent_hover;
    u32 accent_pressed;

    // Widget colors
    u32 button_bg;
    u32 button_text;
    u32 input_bg;
    u32 input_border;
    u32 input_text;
    u32 selection_bg;

    // Panel & dock
    u32 panel_bg;
    u32 panel_text;
    u32 dock_bg;
    u32 dock_icon_bg;
    u32 dock_icon_active;

    // Notification
    u32 notif_bg;
    u32 notif_border;
    u32 notif_text;

    // Shadow & effects
    u32 shadow_color;
    u8 shadow_opacity;
    u32 highlight_color;

    // Animation
    u32 anim_duration_ms;
    u8 anim_easing;  // 0=linear, 1=ease-in, 2=ease-out, 3=ease-in-out

    // Window decorations
    u32 titlebar_bg;
    u32 titlebar_text;
    u32 titlebar_button_bg;
    u32 titlebar_button_hover;
    u32 border_color;
    u32 border_width;

    // Scrollbar
    u32 scrollbar_bg;
    u32 scrollbar_thumb;
    u32 scrollbar_thumb_hover;

    // Misc
    u32 success_color;
    u32 warning_color;
    u32 error_color;
    u32 info_color;
    u32 link_color;
    u32 separator_color;
} wiri_theme_t;

static wiri_theme_t g_active_theme;
static wiri_theme_t g_light_theme;
static wiri_theme_t g_dark_theme;
static bool g_theme_is_dark = true;

static void theme_set_light(wiri_theme_t *t) {
    strcpy(t->name, "Thai Light");
    t->is_dark = false;

    t->bg_primary     = 0xFFF5F5F5;
    t->bg_secondary   = 0xFFFFFFFF;
    t->bg_tertiary    = 0xFFE8E8E8;
    t->bg_surface     = 0xFFFFFFFF;
    t->bg_elevated    = 0xFFFFFFFF;

    t->text_primary   = 0xFF1A1A2E;
    t->text_secondary = 0xFF6B7280;
    t->text_disabled  = 0xFF9CA3AF;
    t->text_link      = 0xFF2563EB;

    t->accent         = 0xFF6366F1;
    t->accent_hover   = 0xFF4F46E5;
    t->accent_pressed = 0xFF4338CA;

    t->button_bg      = 0xFF6366F1;
    t->button_text    = 0xFFFFFFFF;
    t->input_bg       = 0xFFFFFFFF;
    t->input_border   = 0xFFD1D5DB;
    t->input_text     = 0xFF1A1A2E;

    t->panel_bg       = 0xFFE2E8F0;
    t->panel_text     = 0xFF1A1A2E;
    t->dock_bg        = 0xFFE2E8F0;
    t->dock_icon_bg   = 0xFFD1D5DB;

    t->notif_bg       = 0xFFFFFFFF;
    t->notif_border   = 0xFFE5E7EB;
    t->notif_text     = 0xFF1A1A2E;

    t->shadow_color   = 0xFF000000;
    t->shadow_opacity = 30;
    t->highlight_color = 0xFF6366F1;

    t->anim_duration_ms = 150;
    t->anim_easing = 3;

    t->titlebar_bg    = 0xFFFFFFFF;
    t->titlebar_text  = 0xFF1A1A2E;
    t->border_color   = 0xFFE5E7EB;
    t->border_width   = 1;

    t->success_color  = 0xFF10B981;
    t->warning_color  = 0xFFF59E0B;
    t->error_color    = 0xFFEF4444;
    t->info_color     = 0xFF3B82F6;
}

static void theme_set_dark(wiri_theme_t *t) {
    strcpy(t->name, "Thai Dark");
    t->is_dark = true;

    t->bg_primary     = 0xFF0F0F23;
    t->bg_secondary   = 0xFF1A1A2E;
    t->bg_tertiary    = 0xFF25253D;
    t->bg_surface     = 0xFF1A1A2E;
    t->bg_elevated    = 0xFF25253D;

    t->text_primary   = 0xFFE2E8F0;
    t->text_secondary = 0xFF94A3B8;
    t->text_disabled  = 0xFF64748B;
    t->text_link      = 0xFF818CF8;

    t->accent         = 0xFF818CF8;
    t->accent_hover   = 0xFF6366F1;
    t->accent_pressed = 0xFF4F46E5;

    t->button_bg      = 0xFF818CF8;
    t->button_text    = 0xFFFFFFFF;
    t->input_bg       = 0xFF1E1E32;
    t->input_border   = 0xFF3B3B5C;
    t->input_text     = 0xFFE2E8F0;

    t->panel_bg       = 0xFF1A1A2E;
    t->panel_text     = 0xFFE2E8F0;
    t->dock_bg        = 0xFF1A1A2E;
    t->dock_icon_bg   = 0xFF25253D;

    t->notif_bg       = 0xFF1E1E32;
    t->notif_border   = 0xFF3B3B5C;
    t->notif_text     = 0xFFE2E8F0;

    t->shadow_color   = 0xFF000000;
    t->shadow_opacity = 60;
    t->highlight_color = 0xFF818CF8;

    t->anim_duration_ms = 150;
    t->anim_easing = 3;

    t->titlebar_bg    = 0xFF1A1A2E;
    t->titlebar_text  = 0xFFE2E8F0;
    t->border_color   = 0xFF3B3B5C;
    t->border_width   = 1;

    t->success_color  = 0xFF34D399;
    t->warning_color  = 0xFFFBBF24;
    t->error_color    = 0xFFF87171;
    t->info_color     = 0xFF60A5FA;
}

void theme_init(void) {
    theme_set_light(&g_light_theme);
    theme_set_dark(&g_dark_theme);
    g_active_theme = g_dark_theme;
    g_theme_is_dark = true;
    kprintf("[THEME] Theme engine initialized (dark mode)\n");
}

void theme_set_dark_mode(bool dark) {
    g_theme_is_dark = dark;
    g_active_theme = dark ? g_dark_theme : g_light_theme;
}

bool theme_is_dark(void) {
    return g_theme_is_dark;
}

wiri_theme_t *theme_get_active(void) {
    return &g_active_theme;
}

u32 theme_blend(u32 color_a, u32 color_b, u8 t) {
    u8 r = ((color_a >> 16) & 0xFF) * t / 255 + ((color_b >> 16) & 0xFF) * (255 - t) / 255;
    u8 g = ((color_a >> 8) & 0xFF) * t / 255 + ((color_b >> 8) & 0xFF) * (255 - t) / 255;
    u8 b = (color_a & 0xFF) * t / 255 + (color_b & 0xFF) * (255 - t) / 255;
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

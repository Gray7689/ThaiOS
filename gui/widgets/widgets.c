// ThaiOS Wiri Widget Library
// ============================
// Toolkit per widget: button, label, input, slider, list, window.

#include <thaios.h>
#include <gui/themes/themes.c>

#define WIDGET_MAX_CHILDREN 64

typedef enum widget_type {
    WIDGET_NONE,
    WIDGET_WINDOW,
    WIDGET_BUTTON,
    WIDGET_LABEL,
    WIDGET_INPUT,
    WIDGET_SLIDER,
    WIDGET_LIST,
    WIDGET_DROPDOWN,
    WIDGET_CHECKBOX,
    WIDGET_IMAGE
} widget_type_t;

typedef struct widget {
    widget_type_t type;
    i32 x, y, width, height;
    u32 bg_color;
    u32 text_color;
    char text[256];
    bool visible;
    bool enabled;
    bool focused;
    bool hovered;

    // Event callbacks
    void (*on_click)(struct widget *self);
    void (*on_key)(struct widget *self, u32 key);
    void (*on_mouse_enter)(struct widget *self);
    void (*on_mouse_leave)(struct widget *self);

    // Children (for containers)
    struct widget *children[WIDGET_MAX_CHILDREN];
    int child_count;

    // Specific widget data
    union {
        struct { int min, max, value; } slider;
        struct { char placeholder[128]; } input;
        struct { int selected; char items[32][128]; int item_count; } list;
    } data;

    void *priv;
} widget_t;

typedef struct window {
    widget_t base;
    char title[128];
    bool resizable;
    bool maximizable;
    bool minimizable;
    bool maximized;
    bool modal;
    u32 titlebar_color;
    u32 border_color;
    int border_size;
    struct window *parent;
} window_t;

// Widget creation
widget_t *widget_create(widget_type_t type, i32 x, i32 y, i32 w, i32 h) {
    widget_t *wid = (widget_t*)kmalloc(sizeof(widget_t));
    if (!wid) return NULL;

    memset(wid, 0, sizeof(widget_t));
    wid->type = type;
    wid->x = x;
    wid->y = y;
    wid->width = w;
    wid->height = h;
    wid->visible = true;
    wid->enabled = true;
    wid->focused = false;
    wid->hovered = false;
    wid->bg_color = theme_get_active()->bg_surface;
    wid->text_color = theme_get_active()->text_primary;

    return wid;
}

window_t *window_create(const char *title, i32 x, i32 y, i32 w, i32 h) {
    window_t *win = (window_t*)kmalloc(sizeof(window_t));
    if (!win) return NULL;

    memset(win, 0, sizeof(window_t));
    win->base.type = WIDGET_WINDOW;
    win->base.x = x;
    win->base.y = y;
    win->base.width = w;
    win->base.height = h;
    win->base.visible = true;
    win->base.bg_color = theme_get_active()->bg_surface;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->resizable = true;
    win->maximizable = true;
    win->minimizable = true;
    win->border_size = 4;
    win->titlebar_color = theme_get_active()->titlebar_bg;
    win->border_color = theme_get_active()->border_color;

    return win;
}

void widget_add_child(widget_t *parent, widget_t *child) {
    if (!parent || !child || parent->child_count >= WIDGET_MAX_CHILDREN) return;
    parent->children[parent->child_count++] = child;
}

widget_t *button_create(i32 x, i32 y, i32 w, i32 h, const char *text,
                        void (*on_click)(widget_t*)) {
    widget_t *btn = widget_create(WIDGET_BUTTON, x, y, w, h);
    if (!btn) return NULL;

    strncpy(btn->text, text, sizeof(btn->text) - 1);
    btn->on_click = on_click;
    btn->bg_color = theme_get_active()->button_bg;
    btn->text_color = theme_get_active()->button_text;

    return btn;
}

widget_t *label_create(i32 x, i32 y, const char *text) {
    widget_t *lbl = widget_create(WIDGET_LABEL, x, y, 100, 24);
    if (!lbl) return NULL;

    strncpy(lbl->text, text, sizeof(lbl->text) - 1);
    lbl->bg_color = 0;  // Transparent
    lbl->text_color = theme_get_active()->text_primary;

    return lbl;
}

// Rendering (simplified)
void widget_render(widget_t *wid, u32 *buf, int buf_w, int buf_h) {
    if (!wid || !wid->visible) return;

    switch (wid->type) {
        case WIDGET_BUTTON: {
            // Draw background
            for (int y = wid->y; y < wid->y + wid->height && y < buf_h; y++) {
                for (int x = wid->x; x < wid->x + wid->width && x < buf_w; x++) {
                    if (x >= 0 && y >= 0) {
                        buf[y * buf_w + x] = wid->bg_color;
                    }
                }
            }
            // TODO: render text centered
            break;
        }
        case WIDGET_LABEL: {
            // TODO: render text
            break;
        }
        case WIDGET_INPUT: {
            // Draw input box
            for (int y = wid->y; y < wid->y + wid->height && y < buf_h; y++) {
                for (int x = wid->x; x < wid->x + wid->width && x < buf_w; x++) {
                    if (x >= 0 && y >= 0) {
                        buf[y * buf_w + x] = wid->focused ?
                            theme_get_active()->input_bg :
                            theme_get_active()->input_bg;
                    }
                }
            }
            break;
        }
        case WIDGET_WINDOW: {
            window_t *win = (window_t*)wid;
            // Titlebar
            for (int y = wid->y; y < wid->y + win->border_size + 32 && y < buf_h; y++) {
                for (int x = wid->x; x < wid->x + wid->width && x < buf_w; x++) {
                    if (x >= 0 && y >= 0) {
                        buf[y * buf_w + x] = win->titlebar_color;
                    }
                }
            }
            // Client area
            int title_h = win->border_size + 32;
            for (int y = wid->y + title_h; y < wid->y + wid->height && y < buf_h; y++) {
                for (int x = wid->x; x < wid->x + wid->width && x < buf_w; x++) {
                    if (x >= 0 && y >= 0) {
                        buf[y * buf_w + x] = wid->bg_color;
                    }
                }
            }
            break;
        }
        default: break;
    }

    // Render children recursively
    for (int i = 0; i < wid->child_count; i++) {
        widget_render(wid->children[i], buf, buf_w, buf_h);
    }
}

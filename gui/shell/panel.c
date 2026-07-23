// ThaiOS Wiri Desktop Shell
// ===========================
// Barra delle applicazioni, dock, menu start, centro notifiche.
// Componenti UI del Desktop Environment.

#include <thaios.h>
#include <mm.h>

#define PANEL_HEIGHT 48
#define DOCK_ICON_SIZE 48
#define START_MENU_WIDTH 320
#define START_MENU_HEIGHT 480
#define NOTIFICATION_WIDTH 360

typedef struct wiri_icon {
    char name[64];
    char exec[256];
    u32 *icon_data;     // PNG decoded
    u32 icon_w, icon_h;
    bool pinned;
} wiri_icon_t;

typedef struct wiri_dock {
    wiri_icon_t icons[16];
    int icon_count;
    i32 x, y;            // Position
    bool autohide;
} wiri_dock_t;

typedef struct wiri_panel {
    i32 x, y, width, height;
    u32 bg_color;
    wiri_dock_t dock;
    bool show_start;
    bool show_notifications;
    u32 *framebuffer;    // Panel framebuffer
} wiri_panel_t;

typedef struct wiri_notification {
    char title[64];
    char message[256];
    char app[32];
    u64 timestamp;
    u32 color;
    bool read;
    bool persistent;
} wiri_notification_t;

#define MAX_NOTIFICATIONS 64
static wiri_notification_t g_notifications[MAX_NOTIFICATIONS];
static int g_notification_count = 0;

static wiri_panel_t g_panel;

void panel_init(u32 screen_width, u32 screen_height) {
    g_panel.x = 0;
    g_panel.y = screen_height - PANEL_HEIGHT;
    g_panel.width = screen_width;
    g_panel.height = PANEL_HEIGHT;
    g_panel.bg_color = 0xFF2d2d44;
    g_panel.show_start = false;
    g_panel.show_notifications = false;

    g_panel.framebuffer = (u32*)kcalloc(screen_width * PANEL_HEIGHT, sizeof(u32));
    if (!g_panel.framebuffer) return;

    kprintf("[PANEL] Desktop panel initialized (%dx%d)\n", screen_width, PANEL_HEIGHT);
}

void panel_add_dock_icon(const char *name, const char *exec, bool pinned) {
    wiri_dock_t *dock = &g_panel.dock;
    if (dock->icon_count >= 16) return;

    wiri_icon_t *icon = &dock->icons[dock->icon_count];
    strncpy(icon->name, name, sizeof(icon->name) - 1);
    strncpy(icon->exec, exec, sizeof(icon->exec) - 1);
    icon->pinned = pinned;
    icon->icon_data = NULL;
    dock->icon_count++;

    kprintf("[PANEL] Dock icon added: %s\n", name);
}

void panel_add_notification(const char *title, const char *message,
                            const char *app, u32 color) {
    if (g_notification_count >= MAX_NOTIFICATIONS) {
        // Remove oldest
        memmove(&g_notifications[0], &g_notifications[1],
                (MAX_NOTIFICATIONS - 1) * sizeof(wiri_notification_t));
        g_notification_count--;
    }

    wiri_notification_t *n = &g_notifications[g_notification_count++];
    strncpy(n->title, title, sizeof(n->title) - 1);
    strncpy(n->message, message, sizeof(n->message) - 1);
    strncpy(n->app, app, sizeof(n->app) - 1);
    n->timestamp = 0; // TODO: get system time
    n->color = color;
    n->read = false;
    n->persistent = false;

    kprintf("[PANEL] Notification: %s - %s\n", title, message);
}

void panel_render(u32 *framebuffer, u32 fb_width, u32 fb_height) {
    u32 *buf = g_panel.framebuffer;
    int w = g_panel.width;
    int h = g_panel.height;

    // Background
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            buf[y * w + x] = g_panel.bg_color;
        }
    }

    // Draw dock icons
    wiri_dock_t *dock = &g_panel.dock;
    int icon_start = 10;
    for (int i = 0; i < dock->icon_count; i++) {
        int ix = icon_start + i * (DOCK_ICON_SIZE + 8);
        int iy = (PANEL_HEIGHT - DOCK_ICON_SIZE) / 2;

        // Draw icon background
        for (int dy = 0; dy < DOCK_ICON_SIZE; dy++) {
            for (int dx = 0; dx < DOCK_ICON_SIZE; dx++) {
                int bx = ix + dx;
                int by = iy + dy;
                if (bx >= 0 && bx < w && by >= 0 && by < h) {
                    buf[by * w + bx] = 0xFF4a4a6a;  // Placeholder
                }
            }
        }
    }

    // System tray area (right side)
    int tray_x = w - 200;
    // Clock
    const char *clock_str = "12:00";
    // TODO: render text

    // Copy panel to framebuffer
    int fb_w = w < (int)fb_width ? w : fb_width;
    int fb_h = h < (int)fb_height ? h : fb_height;
    int offset_y = g_panel.y;

    for (int y = 0; y < fb_h && (offset_y + y) < (int)fb_height; y++) {
        for (int x = 0; x < fb_w; x++) {
            framebuffer[(offset_y + y) * fb_width + x] = buf[y * w + x];
        }
    }
}

// Start menu
typedef struct start_menu_entry {
    char name[64];
    char exec[256];
    char category[32];
    u32 icon_color;
} start_menu_entry_t;

#define START_MENU_ENTRIES 128
static start_menu_entry_t g_start_entries[START_MENU_ENTRIES];
static int g_start_entry_count = 0;

void start_menu_add(const char *name, const char *exec,
                    const char *category, u32 icon_color) {
    if (g_start_entry_count >= START_MENU_ENTRIES) return;

    start_menu_entry_t *e = &g_start_entries[g_start_entry_count++];
    strncpy(e->name, name, sizeof(e->name) - 1);
    strncpy(e->exec, exec, sizeof(e->exec) - 1);
    strncpy(e->category, category, sizeof(e->category) - 1);
    e->icon_color = icon_color;
}

void start_menu_render(u32 *buf, int buf_w, int buf_h, int menu_x, int menu_y) {
    // Background
    for (int y = 0; y < START_MENU_HEIGHT && y < buf_h; y++) {
        for (int x = 0; x < START_MENU_WIDTH && x < buf_w; x++) {
            int bx = menu_x + x;
            int by = menu_y + y;
            if (bx >= 0 && bx < buf_w && by >= 0 && by < buf_h) {
                buf[by * buf_w + bx] = 0xFF1e1e32;
            }
        }
    }
    // Entries would be rendered here
}

// ThaiOS Wiri Compositor
// ========================
// Compositor Wayland-based con rendering GPU-accelerato.
// Gestisce surface, wl_surface, damage, e output.

#include <thaios.h>
#include <mm.h>

#define MAX_SURFACES 256
#define MAX_OUTPUTS  8
#define VIRTUAL_WIDTH  1920
#define VIRTUAL_HEIGHT 1080

typedef struct wiri_color {
    u8 r, g, b, a;
} wiri_color_t;

typedef struct wiri_rect {
    i32 x, y;
    i32 width, height;
} wiri_rect_t;

typedef struct wiri_surface {
    u64 id;
    wiri_rect_t geometry;
    u32 *buffer;             // Framebuffer ARGB
    bool damaged;
    bool visible;
    bool mapped;
    u64 window_id;
    struct wiri_surface *next;
} wiri_surface_t;

typedef struct wiri_output {
    char name[32];
    wiri_rect_t geometry;
    u32 *framebuffer;        // Physical framebuffer
    usize framebuffer_size;
    u32 width;
    u32 height;
    u32 stride;
    u8 bpp;
    bool active;
} wiri_output_t;

typedef struct wiri_compositor {
    wiri_surface_t *surfaces;
    wiri_output_t outputs[MAX_OUTPUTS];
    int output_count;
    u64 next_surface_id;
    u32 bg_color;
    bool running;
} wiri_compositor_t;

static wiri_compositor_t g_compositor;

void compositor_init(void) {
    memset(&g_compositor, 0, sizeof(g_compositor));
    g_compositor.bg_color = 0xFF1a1a2e;  // Dark theme default
    g_compositor.running = true;
    g_compositor.next_surface_id = 1;

    kprintf("[WIRI] Compositor initialized\n");
}

int compositor_add_output(const char *name, u32 width, u32 height,
                          u32 *framebuffer, u32 stride, u8 bpp) {
    if (g_compositor.output_count >= MAX_OUTPUTS) return -ENOMEM;

    wiri_output_t *output = &g_compositor.outputs[g_compositor.output_count];
    strncpy(output->name, name, sizeof(output->name) - 1);
    output->width = width;
    output->height = height;
    output->framebuffer = framebuffer;
    output->stride = stride;
    output->bpp = bpp;
    output->framebuffer_size = stride * height;
    output->geometry.x = 0;
    output->geometry.y = 0;
    output->geometry.width = width;
    output->geometry.height = height;
    output->active = true;

    g_compositor.output_count++;
    kprintf("[WIRI] Output '%s' added: %dx%d @ %dbpp\n", name, width, height, bpp);
    return g_compositor.output_count - 1;
}

wiri_surface_t *compositor_create_surface(u32 width, u32 height) {
    wiri_surface_t *surface = (wiri_surface_t*)kmalloc(sizeof(wiri_surface_t));
    if (!surface) return NULL;

    surface->id = g_compositor.next_surface_id++;
    surface->buffer = (u32*)kcalloc(width * height, sizeof(u32));
    if (!surface->buffer) { kfree(surface); return NULL; }

    surface->geometry.x = 0;
    surface->geometry.y = 0;
    surface->geometry.width = width;
    surface->geometry.height = height;
    surface->damaged = true;
    surface->visible = true;
    surface->mapped = false;
    surface->window_id = 0;
    surface->next = g_compositor.surfaces;
    g_compositor.surfaces = surface;

    kprintf("[WIRI] Surface %llu created: %dx%d\n", surface->id, width, height);
    return surface;
}

void compositor_destroy_surface(wiri_surface_t *surface) {
    if (!surface) return;

    // Remove from list
    wiri_surface_t **prev = &g_compositor.surfaces;
    while (*prev) {
        if (*prev == surface) {
            *prev = surface->next;
            break;
        }
        prev = &(*prev)->next;
    }

    kfree(surface->buffer);
    kfree(surface);
}

void compositor_render(void) {
    for (int o = 0; o < g_compositor.output_count; o++) {
        wiri_output_t *output = &g_compositor.outputs[o];
        if (!output->active) continue;

        u32 *fb = output->framebuffer;
        u32 w = output->width;
        u32 h = output->height;

        // Clear to background color
        for (u32 y = 0; y < h; y++) {
            for (u32 x = 0; x < w; x++) {
                fb[y * output->stride / 4 + x] = g_compositor.bg_color;
            }
        }

        // Render surfaces (back to front)
        wiri_surface_t *surf = g_compositor.surfaces;
        while (surf) {
            if (!surf->visible || !surf->mapped) {
                surf = surf->next;
                continue;
            }

            wiri_rect_t *geo = &surf->geometry;

            // Blit surface buffer to framebuffer with clipping
            for (i32 y = 0; y < geo->height; y++) {
                i32 fb_y = geo->y + y;
                if (fb_y < 0 || fb_y >= (i32)h) continue;

                for (i32 x = 0; x < geo->width; x++) {
                    i32 fb_x = geo->x + x;
                    if (fb_x < 0 || fb_x >= (i32)w) continue;

                    u32 pixel = surf->buffer[y * geo->width + x];
                    u8 alpha = (pixel >> 24) & 0xFF;

                    if (alpha == 255) {
                        fb[fb_y * output->stride / 4 + fb_x] = pixel;
                    } else if (alpha > 0) {
                        // Alpha blending
                        u32 bg = fb[fb_y * output->stride / 4 + fb_x];
                        u8 r = ((pixel >> 16) & 0xFF) * alpha / 255 + ((bg >> 16) & 0xFF) * (255 - alpha) / 255;
                        u8 g = ((pixel >> 8) & 0xFF) * alpha / 255 + ((bg >> 8) & 0xFF) * (255 - alpha) / 255;
                        u8 b = (pixel & 0xFF) * alpha / 255 + (bg & 0xFF) * (255 - alpha) / 255;
                        fb[fb_y * output->stride / 4 + fb_x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                    }
                }
            }

            surf->damaged = false;
            surf = surf->next;
        }
    }
}

void composizer_run(void) {
    while (g_compositor.running) {
        compositor_render();
        // TODO: vsync, event loop
        // sched_yield();
    }
}

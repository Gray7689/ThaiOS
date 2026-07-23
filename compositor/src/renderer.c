#include <stdlib.h>
#include <string.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>
#include "server.h"
#include "theme.h"

extern struct thai_server server;

void thai_render_background(struct wlr_output *output, struct timespec *when) {
    (void)when;
    struct wlr_renderer *renderer = server.renderer;
    if (!renderer) return;

    int width = output->width;
    int height = output->height;

    wlr_renderer_begin(renderer, width, height);

    float bg_color[4];
    hex_to_rgba(server.theme->background, &bg_color[0], &bg_color[1], &bg_color[2], &bg_color[3]);

    wlr_renderer_clear(renderer, bg_color);

    wlr_renderer_end(renderer);
}

void thai_render_fullscreen(struct wlr_output *output, struct timespec *when,
                             pixman_region32_t *damage) {
    (void)damage;
    wlr_output_attach_render(output, NULL);
    thai_render_background(output, when);
    wlr_output_commit(output);
}

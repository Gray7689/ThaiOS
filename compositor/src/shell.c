#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include "server.h"

extern struct thai_server server;

void handle_new_layer_surface(struct wl_listener *listener, void *data) {
    struct wlr_layer_surface_v1 *layer_surface = data;
    struct thai_layer_surface *tsurf = calloc(1, sizeof(*tsurf));
    if (!tsurf) return;

    tsurf->server = &server;
    tsurf->layer_surface = layer_surface;
    tsurf->mapped = false;

    tsurf->scene_tree = wlr_scene_tree_create(server.scene->tree);
    if (!tsurf->scene_tree) {
        free(tsurf);
        return;
    }

    struct wlr_scene_layer_surface_v1 *scene_layer =
        wlr_scene_layer_surface_v1_create(tsurf->scene_tree, layer_surface);
    if (!scene_layer) {
        wlr_scene_node_destroy(&tsurf->scene_tree->node);
        free(tsurf);
        return;
    }

    tsurf->map.notify = handle_layer_surface_map;
    wl_signal_add(&layer_surface->events.map, &tsurf->map);

    tsurf->unmap.notify = handle_layer_surface_unmap;
    wl_signal_add(&layer_surface->events.unmap, &tsurf->unmap);

    tsurf->destroy.notify = handle_layer_surface_destroy;
    wl_signal_add(&layer_surface->events.destroy, &tsurf->destroy);

    tsurf->commit.notify = handle_layer_surface_commit;
    wl_signal_add(&layer_surface->surface->events.commit, &tsurf->commit);

    wl_list_insert(&server.views, &tsurf->link);

    wlr_layer_surface_v1_configure(layer_surface, 0, 0);
}

void handle_layer_surface_map(struct wl_listener *listener, void *data) {
    struct thai_layer_surface *tsurf = wl_container_of(listener, tsurf, map);
    tsurf->mapped = true;
}

void handle_layer_surface_unmap(struct wl_listener *listener, void *data) {
    struct thai_layer_surface *tsurf = wl_container_of(listener, tsurf, unmap);
    tsurf->mapped = false;
}

void handle_layer_surface_destroy(struct wl_listener *listener, void *data) {
    struct thai_layer_surface *tsurf = wl_container_of(listener, tsurf, destroy);
    wl_list_remove(&tsurf->link);
    wl_list_remove(&tsurf->map.link);
    wl_list_remove(&tsurf->unmap.link);
    wl_list_remove(&tsurf->destroy.link);
    wl_list_remove(&tsurf->commit.link);
    free(tsurf);
}

void handle_layer_surface_commit(struct wl_listener *listener, void *data) {
    (void)data;
    struct thai_layer_surface *tsurf = wl_container_of(listener, tsurf, commit);
    if (!tsurf->mapped) return;

    struct wlr_layer_surface_v1 *layer = tsurf->layer_surface;
    struct wlr_output *wlr_output = wlr_layer_surface_v1_get_output(layer);
    if (!wlr_output) return;

    struct wlr_box full_area = {0};
    wlr_output_layout_get_box(server.output_layout, wlr_output, &full_area);

    uint32_t width = full_area.width;
    uint32_t height = full_area.height;

    struct wlr_box box = {0};
    uint32_t exclusive_zone = 0;

    switch (layer->current.anchor) {
        case ZWLR_LAYER_SHELL_V1_ANCHOR_TOP:
        case ZWLR_LAYER_SHELL_V1_ANCHOR_LEFT | ZWLR_LAYER_SHELL_V1_ANCHOR_RIGHT:
            box.width = width;
            box.height = layer->current.desired_height;
            exclusive_zone = box.height;
            break;
        case ZWLR_LAYER_SHELL_V1_ANCHOR_BOTTOM:
        case ZWLR_LAYER_SHELL_V1_ANCHOR_LEFT | ZWLR_LAYER_SHELL_V1_ANCHOR_RIGHT | ZWLR_LAYER_SHELL_V1_ANCHOR_BOTTOM:
            box.width = width;
            box.height = layer->current.desired_height;
            box.y = height - box.height;
            exclusive_zone = box.height;
            break;
        default:
            box.width = layer->current.desired_width;
            box.height = layer->current.desired_height;
            break;
    }

    wlr_layer_surface_v1_configure(layer, box.width, box.height);
}

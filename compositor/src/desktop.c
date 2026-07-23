#include <stdlib.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include "server.h"

extern struct thai_server server;

void handle_new_xdg_surface(struct wl_listener *listener, void *data) {
    struct wlr_xdg_surface *xdg_surface = data;

    if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
        wlr_log(WLR_DEBUG, "ThaiDesktop: new popup surface");
        return;
    }

    struct thai_view *view = calloc(1, sizeof(*view));
    if (!view) return;

    view->server = &server;
    view->xdg_surface = xdg_surface;
    view->mapped = false;
    view->x = 100;
    view->y = 100;

    view->scene_tree = wlr_scene_tree_create(server.scene->tree);
    if (!view->scene_tree) {
        free(view);
        return;
    }

    struct wlr_scene_surface *scene_surface =
        wlr_scene_surface_create(view->scene_tree, xdg_surface->surface);
    if (!scene_surface) {
        wlr_scene_node_destroy(&view->scene_tree->node);
        free(view);
        return;
    }

    view->map.notify = handle_xdg_surface_map;
    wl_signal_add(&xdg_surface->events.map, &view->map);

    view->unmap.notify = handle_xdg_surface_unmap;
    wl_signal_add(&xdg_surface->events.unmap, &view->unmap);

    view->destroy.notify = handle_xdg_surface_destroy;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    view->request_move.notify = handle_xdg_surface_request_move;
    wl_signal_add(&xdg_surface->toplevel->events.request_move, &view->request_move);

    view->request_resize.notify = handle_xdg_surface_request_resize;
    wl_signal_add(&xdg_surface->toplevel->events.request_resize, &view->request_resize);

    view->request_maximize.notify = handle_xdg_surface_request_maximize;
    wl_signal_add(&xdg_surface->toplevel->events.request_maximize, &view->request_maximize);

    view->request_fullscreen.notify = handle_xdg_surface_request_fullscreen;
    wl_signal_add(&xdg_surface->toplevel->events.request_fullscreen, &view->request_fullscreen);

    wl_list_insert(&server.views, &view->link);
}

void handle_xdg_surface_map(struct wl_listener *listener, void *data) {
    (void)data;
    struct thai_view *view = wl_container_of(listener, view, map);
    view->mapped = true;
    focus_view(view, view->xdg_surface->surface);

    struct wlr_scene_node *node = &view->scene_tree->node;
    node->position.x = view->x;
    node->position.y = view->y;

    wlr_scene_node_set_enabled(node, true);
}

void handle_xdg_surface_unmap(struct wl_listener *listener, void *data) {
    (void)data;
    struct thai_view *view = wl_container_of(listener, view, unmap);
    view->mapped = false;
    wlr_scene_node_set_enabled(&view->scene_tree->node, false);
}

void handle_xdg_surface_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct thai_view *view = wl_container_of(listener, view, destroy);
    wl_list_remove(&view->link);
    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_fullscreen.link);
    free(view);
}

void handle_xdg_surface_request_move(struct wl_listener *listener, void *data) {
    struct thai_view *view = wl_container_of(listener, view, request_move);
    struct wlr_xdg_toplevel_move_event *event = data;
    begin_move(view, event->seat, event->serial);
}

void handle_xdg_surface_request_resize(struct wl_listener *listener, void *data) {
    struct thai_view *view = wl_container_of(listener, view, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    begin_resize(view, event->seat, event->serial, event->edges);
}

void handle_xdg_surface_request_maximize(struct wl_listener *listener, void *data) {
    struct thai_view *view = wl_container_of(listener, view, request_maximize);
    (void)data;
    struct wlr_output *output = wlr_output_layout_output_at(server.output_layout,
        server.cursor->cursor->x, server.cursor->cursor->y);
    if (!output) return;

    struct wlr_box *area = wlr_output_layout_get_box(server.output_layout, output);
    view->x = 0;
    view->y = 0;
    view->scene_tree->node.position.x = 0;
    view->scene_tree->node.position.y = 0;
    wlr_xdg_toplevel_set_size(view->xdg_surface, area->width, area->height);
}

void handle_xdg_surface_request_fullscreen(struct wl_listener *listener, void *data) {
    struct thai_view *view = wl_container_of(listener, view, request_fullscreen);
    struct wlr_xdg_toplevel_set_fullscreen_event *event = data;
    (void)event;
    wlr_xdg_toplevel_set_fullscreen(view->xdg_surface, true);
}

void focus_view(struct thai_view *view, struct wlr_surface *surface) {
    if (!view || !surface) return;

    struct thai_view *prev_focus = NULL;
    struct wlr_surface *prev_surface = NULL;

    struct wlr_seat *seat = server.cursor->cursor->seat;
    prev_surface = seat->keyboard_state.focused_surface;

    if (prev_surface) {
        struct wlr_keyboard_group *keyboard = wlr_seat_get_keyboard(seat);
        if (keyboard) {
            wlr_seat_keyboard_notify_clear_focus(seat);
        }
    }

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    if (keyboard) {
        wlr_seat_keyboard_notify_enter(seat, surface,
            keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }

    wlr_scene_node_raise_to_top(&view->scene_tree->node);
}

void begin_move(struct thai_view *view, struct wlr_seat *seat, uint32_t serial) {
    server.cursor->grabbed_view = view;
    server.cursor->grab_x = server.cursor->cursor->x - view->scene_tree->node.position.x;
    server.cursor->grab_y = server.cursor->cursor->y - view->scene_tree->node.position.y;
}

void begin_resize(struct thai_view *view, struct wlr_seat *seat, uint32_t serial, uint32_t edges) {
    server.cursor->grabbed_view = view;
    server.cursor->grab_x = server.cursor->cursor->x;
    server.cursor->grab_y = server.cursor->cursor->y;
    server.cursor->grab_sx = view->scene_tree->node.position.x;
    server.cursor->grab_sy = view->scene_tree->node.position.y;
    server.cursor->resize_edges = edges;
}

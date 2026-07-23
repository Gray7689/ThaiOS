#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>
#include "cursor.h"
#include "server.h"

extern struct thai_server server;

void handle_new_input(struct wl_listener *listener, void *data) {
    struct wlr_input_device *device = data;
    struct thai_cursor *cursor = server.cursor;

    if (!cursor) {
        cursor = thai_cursor_create(&server);
        server.cursor = cursor;
    }

    thai_cursor_attach_input(cursor, device);

    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            wlr_log(WLR_INFO, "ThaiDesktop: keyboard '%s' connected", device->name);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            wlr_log(WLR_INFO, "ThaiDesktop: pointer '%s' connected", device->name);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            wlr_log(WLR_INFO, "ThaiDesktop: touch device '%s' connected", device->name);
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            wlr_log(WLR_INFO, "ThaiDesktop: tablet '%s' connected", device->name);
            break;
        default:
            break;
    }
}

void handle_cursor_motion(struct wl_listener *listener, void *data) {
    struct thai_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
    struct wlr_event_pointer_motion *event = data;

    if (cursor->grabbed_view) {
        double new_x = cursor->cursor->x + event->delta_x;
        double new_y = cursor->cursor->y + event->delta_y;

        if (cursor->resize_edges) {
            process_cursor_resize(cursor, new_x, new_y);
        } else {
            cursor->grabbed_view->scene_tree->node.position.x = new_x - cursor->grab_x;
            cursor->grabbed_view->scene_tree->node.position.y = new_y - cursor->grab_y;
            cursor->grabbed_view->x = cursor->grabbed_view->scene_tree->node.position.x;
            cursor->grabbed_view->y = cursor->grabbed_view->scene_tree->node.position.y;
        }
    }

    wlr_cursor_move(cursor->cursor, event->device, event->delta_x, event->delta_y);
    process_cursor_moved(cursor);
}

void handle_cursor_motion_absolute(struct wl_listener *listener, void *data) {
    struct thai_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_absolute);
    struct wlr_event_pointer_motion_absolute *event = data;

    wlr_cursor_warp_absolute(cursor->cursor, event->device, event->x, event->y);
    process_cursor_moved(cursor);
}

void handle_cursor_button(struct wl_listener *listener, void *data) {
    struct thai_cursor *cursor = wl_container_of(listener, cursor, cursor_button);
    struct wlr_event_pointer_button *event = data;

    wlr_seat_pointer_notify_button(cursor->cursor->seat,
        event->time_msec, event->button, event->state);

    struct wlr_surface *surface = NULL;
    double sx, sy;
    struct thai_view *view = desktop_view_at(cursor, &surface, &sx, &sy);

    if (event->state == WLR_BUTTON_PRESSED && view) {
        focus_view(view, surface);
    }

    if (!cursor->grabbed_view && event->state == WLR_BUTTON_RELEASED) {
        cursor->grabbed_view = NULL;
        cursor->resize_edges = 0;
    }
}

void handle_cursor_axis(struct wl_listener *listener, void *data) {
    struct thai_cursor *cursor = wl_container_of(listener, cursor, cursor_axis);
    struct wlr_event_pointer_axis *event = data;

    wlr_seat_pointer_notify_axis(cursor->cursor->seat,
        event->time_msec, event->orientation, event->delta,
        event->delta_discrete, event->source);
}

void handle_cursor_frame(struct wl_listener *listener, void *data) {
    struct thai_cursor *cursor = wl_container_of(listener, cursor, cursor_frame);
    (void)data;
    wlr_seat_pointer_notify_frame(cursor->cursor->seat);
}

void process_cursor_moved(struct thai_cursor *cursor) {
    struct wlr_surface *surface = NULL;
    double sx, sy;
    struct thai_view *view = desktop_view_at(cursor, &surface, &sx, &sy);

    if (surface) {
        wlr_seat_pointer_notify_enter(cursor->cursor->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(cursor->cursor->seat, 0, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(cursor->cursor->seat);
        wlr_xcursor_manager_set_cursor_image(cursor->xcursor_manager, "default", cursor->cursor);
    }
}

struct thai_view *desktop_view_at(struct thai_cursor *cursor, struct wlr_surface **surface, double *sx, double *sy) {
    struct wlr_scene_node *node = wlr_scene_node_at(
        &server.scene->tree->node,
        cursor->cursor->x, cursor->cursor->y, sx, sy);

    if (!node || node->type != WLR_SCENE_NODE_SURFACE) {
        return NULL;
    }

    struct wlr_scene_surface *scene_surface = wlr_scene_surface_from_node(node);
    *surface = scene_surface->surface;

    struct thai_view *view;
    wl_list_for_each(view, &server.views, link) {
        if (view->scene_tree == node->parent || view->scene_tree == node) {
            return view;
        }
    }

    return NULL;
}

void process_cursor_resize(struct thai_cursor *cursor, double new_x, double new_y) {
    struct thai_view *view = cursor->grabbed_view;
    if (!view) return;

    double dx = new_x - cursor->grab_x;
    double dy = new_y - cursor->grab_y;

    struct wlr_box geo_box = {0};
    wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);

    int new_width = geo_box.width;
    int new_height = geo_box.height;
    int new_x_pos = view->x;
    int new_y_pos = view->y;

    if (cursor->resize_edges & WLR_EDGE_TOP) {
        new_y_pos = cursor->grab_sy + dy;
        new_height = geo_box.height - dy;
    } else if (cursor->resize_edges & WLR_EDGE_BOTTOM) {
        new_height = geo_box.height + dy;
    }

    if (cursor->resize_edges & WLR_EDGE_LEFT) {
        new_x_pos = cursor->grab_sx + dx;
        new_width = geo_box.width - dx;
    } else if (cursor->resize_edges & WLR_EDGE_RIGHT) {
        new_width = geo_box.width + dx;
    }

    if (new_width < 100 || new_height < 50) return;

    view->x = new_x_pos;
    view->y = new_y_pos;
    view->scene_tree->node.position.x = new_x_pos;
    view->scene_tree->node.position.y = new_y_pos;
    wlr_xdg_toplevel_set_size(view->xdg_surface, new_width, new_height);
}

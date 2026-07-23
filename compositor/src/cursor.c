#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>
#include "cursor.h"
#include "server.h"

extern struct thai_server server;

struct thai_cursor *thai_cursor_create(struct thai_server *server) {
    struct thai_cursor *cursor = calloc(1, sizeof(*cursor));
    if (!cursor) return NULL;

    cursor->server = server;
    cursor->cursor = wlr_cursor_create();
    if (!cursor->cursor) {
        free(cursor);
        return NULL;
    }

    wlr_cursor_attach_output_layout(cursor->cursor, server->output_layout);

    cursor->xcursor_manager = wlr_xcursor_manager_create("ThaiOS-Cursors", 24);
    if (!cursor->xcursor_manager) {
        wlr_log(WLR_ERROR, "ThaiDesktop: failed to create cursor manager");
    }

    cursor->cursor_motion.notify = handle_cursor_motion;
    wl_signal_add(&cursor->cursor->events.motion, &cursor->cursor_motion);

    cursor->cursor_motion_absolute.notify = handle_cursor_motion_absolute;
    wl_signal_add(&cursor->cursor->events.motion_absolute, &cursor->cursor_motion_absolute);

    cursor->cursor_button.notify = handle_cursor_button;
    wl_signal_add(&cursor->cursor->events.button, &cursor->cursor_button);

    cursor->cursor_axis.notify = handle_cursor_axis;
    wl_signal_add(&cursor->cursor->events.axis, &cursor->cursor_axis);

    cursor->cursor_frame.notify = handle_cursor_frame;
    wl_signal_add(&cursor->cursor->events.frame, &cursor->cursor_frame);

    struct wlr_seat *seat = wlr_seat_create(server->wl_display, "thai-seat");
    if (!seat) {
        wlr_log(WLR_ERROR, "ThaiDesktop: failed to create seat");
        thai_cursor_destroy(cursor);
        return NULL;
    }

    cursor->cursor->seat = seat;

    return cursor;
}

void thai_cursor_destroy(struct thai_cursor *cursor) {
    if (!cursor) return;

    wl_list_remove(&cursor->cursor_motion.link);
    wl_list_remove(&cursor->cursor_motion_absolute.link);
    wl_list_remove(&cursor->cursor_button.link);
    wl_list_remove(&cursor->cursor_axis.link);
    wl_list_remove(&cursor->cursor_frame.link);

    if (cursor->xcursor_manager) {
        wlr_xcursor_manager_destroy(cursor->xcursor_manager);
    }
    if (cursor->cursor) {
        wlr_cursor_destroy(cursor->cursor);
    }
    free(cursor);
}

void thai_cursor_attach_input(struct thai_cursor *cursor, struct wlr_input_device *device) {
    switch (device->type) {
        case WLR_INPUT_DEVICE_POINTER:
            wlr_cursor_attach_input_device(cursor->cursor, device);
            break;
        case WLR_INPUT_DEVICE_TOUCH:
            wlr_cursor_attach_input_device(cursor->cursor, device);
            break;
        case WLR_INPUT_DEVICE_TABLET_TOOL:
            wlr_cursor_attach_input_device(cursor->cursor, device);
            break;
        case WLR_INPUT_DEVICE_KEYBOARD: {
            struct wlr_keyboard *keyboard = wlr_keyboard_from_input_device(device);
            struct wlr_seat *seat = cursor->cursor->seat;
            struct wlr_keyboard *current = wlr_seat_get_keyboard(seat);
            if (!current) {
                wlr_seat_set_keyboard(seat, device);
            }
            (void)keyboard;
            break;
        }
        default:
            break;
    }
}

#ifndef THAIDESKTOP_CURSOR_H
#define THAIDESKTOP_CURSOR_H

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include "server.h"

struct thai_cursor {
    struct thai_server *server;
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *xcursor_manager;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;
    struct thai_view *grabbed_view;
    double grab_x, grab_y;
    double grab_sx, grab_sy;
    int resize_edges;
};

struct thai_cursor *thai_cursor_create(struct thai_server *server);
void thai_cursor_destroy(struct thai_cursor *cursor);
void thai_cursor_attach_input(struct thai_cursor *cursor, struct wlr_input_device *device);

#endif

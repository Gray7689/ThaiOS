#include <stdlib.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include "server.h"
#include "cursor.h"
#include "theme.h"

extern struct thai_server server;

void handle_new_output(struct wl_listener *listener, void *data) {
    struct wlr_output *wlr_output = data;
    struct thai_output *output = calloc(1, sizeof(*output));
    if (!output) return;

    output->server = &server;
    output->wlr_output = wlr_output;
    wl_list_init(&output->views);
    wl_list_insert(&server.outputs, &output->link);

    output->scene_output = wlr_scene_output_create(server.scene, wlr_output);
    if (!output->scene_output) {
        wlr_log(WLR_ERROR, "Failed to create scene output");
        free(output);
        return;
    }

    output->frame.notify = handle_output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->destroy.notify = handle_output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    output->request_state.notify = handle_output_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    struct wlr_output_mode *preferred = wlr_output_preferred_mode(wlr_output);
    if (preferred) {
        wlr_output_state_set_mode(&state, preferred);
    }

    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    wlr_output_layout_add_auto(server.output_layout, wlr_output);

    wlr_log(WLR_INFO, "ThaiDesktop: output '%s' connected", wlr_output->name);
}

void handle_output_frame(struct wl_listener *listener, void *data) {
    struct thai_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene_output *scene_output = output->scene_output;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    wlr_scene_output_commit(scene_output, NULL);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

void handle_output_destroy(struct wl_listener *listener, void *data) {
    struct thai_output *output = wl_container_of(listener, output, destroy);
    wlr_log(WLR_INFO, "ThaiDesktop: output '%s' disconnected", output->wlr_output->name);
    wl_list_remove(&output->link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->request_state.link);
    free(output);
}

void handle_output_request_state(struct wl_listener *listener, void *data) {
    struct thai_output *output = wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;
    wlr_output_commit_state(output->wlr_output, event->state);
}

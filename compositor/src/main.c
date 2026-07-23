#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include "server.h"

static struct thai_server server;
static int exit_status = 0;

static void handle_signal(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        wl_display_terminate(server.wl_display);
    }
}

static void spawn_launcher(void) {
    if (fork() == 0) {
        execl("/usr/bin/thai-shell", "thai-shell", NULL);
        _exit(EXIT_FAILURE);
    }
}

static void print_version(void) {
    printf("ThaiDesktop 1.0\n");
    printf("ThaiOS Desktop Compositor\n");
}

static void print_usage(void) {
    printf("Usage: thai-desktop [options]\n");
    printf("  -c, --config <path>  Configuration file path\n");
    printf("  -v, --version        Show version information\n");
    printf("  -h, --help           Show this help message\n");
}

int main(int argc, char *argv[]) {
    char *config_path = NULL;

    static struct option long_options[] = {
        {"config", required_argument, 0, 'c'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "c:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                config_path = strdup(optarg);
                break;
            case 'v':
                print_version();
                return 0;
            case 'h':
                print_usage();
                return 0;
            default:
                print_usage();
                return 1;
        }
    }

    wlr_log_init(WLR_DEBUG, NULL);
    wlr_log(WLR_INFO, "ThaiDesktop 1.0 starting");

    if (!config_path) {
        config_path = strdup("/etc/ThaiOS/desktop.conf");
    }

    server.config_path = config_path;
    server.wl_display = wl_display_create();
    if (!server.wl_display) {
        wlr_log(WLR_ERROR, "Failed to create Wayland display");
        return 1;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    server.backend = wlr_backend_autocreate(server.wl_display);
    if (!server.backend) {
        wlr_log(WLR_ERROR, "Failed to create backend");
        return 1;
    }

    server.renderer = wlr_renderer_autocreate(server.backend);
    if (!server.renderer) {
        wlr_log(WLR_ERROR, "Failed to create renderer");
        return 1;
    }

    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (!server.allocator) {
        wlr_log(WLR_ERROR, "Failed to create allocator");
        return 1;
    }

    server.compositor = wlr_compositor_create(server.wl_display, 5, server.renderer);

    server.scene = wlr_scene_create();
    if (!server.scene) {
        wlr_log(WLR_ERROR, "Failed to create scene graph");
        return 1;
    }

    server.output_layout = wlr_output_layout_create();
    server.scene_layout = wlr_scene_attach_output_layout(server.scene, server.output_layout);

    wl_list_init(&server.outputs);
    wl_list_init(&server.inputs);
    wl_list_init(&server.views);

    server.new_output.notify = handle_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    wlr_data_device_manager_create(server.wl_display);

    server.xdg_shell = wlr_xdg_shell_create(server.wl_display, 5);
    server.new_xdg_surface.notify = handle_new_xdg_surface;
    wl_signal_add(&server.xdg_shell->events.new_surface, &server.new_xdg_surface);

    server.layer_shell = wlr_layer_shell_v1_create(server.wl_display, 4);
    server.new_layer_surface.notify = handle_new_layer_surface;
    wl_signal_add(&server.layer_shell->events.new_surface, &server.new_layer_surface);

    server.new_input.notify = handle_new_input;
    wl_signal_add(&server.backend->events.new_input, &server.new_input);

    server.theme = thai_theme_load("/usr/share/ThaiOS/themes/ThaiOS-Dark/index.theme");

    // Start session
    spawn_launcher();

    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) {
        wlr_log(WLR_ERROR, "Failed to add Wayland socket");
        return 1;
    }

    wlr_log(WLR_INFO, "ThaiDesktop running on Wayland socket: %s", socket);
    setenv("WAYLAND_DISPLAY", socket, 1);

    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return 1;
    }

    wl_display_run(server.wl_display);

    wl_display_destroy(server.wl_display);
    free(config_path);

    return exit_status;
}

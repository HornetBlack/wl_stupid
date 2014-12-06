
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include "simple.h"

static void shell_surface_ping(void *data,
                               struct wl_shell_surface *wl_shell_surface,
                               uint32_t serial);
static void shell_surface_configure(void *data,
                                    struct wl_shell_surface *wl_shell_surface,
                                    uint32_t edges,
                                    int32_t width,
                                    int32_t height);
static void shell_surface_popup_done(void *data,
                                     struct wl_shell_surface *wl_shell_surface);


const struct wl_shell_surface_listener shell_surface_listener = {
        .ping       = shell_surface_ping,
        .configure  = shell_surface_configure,
        .popup_done = shell_surface_popup_done,
};

struct my_window *create_window(struct my_display *display, int width, int height)
{
        struct my_window *window;

        window = calloc(1, sizeof *window);
        if (window == NULL) {
                perror(""); exit(1);
        }

        window->callback = NULL;
        window->display = display;
        window->width = width;
        window->height = height;
        window->surface = wl_compositor_create_surface(display->compositor);
        
        if (display->shell) {
                window->shell_surface = wl_shell_get_shell_surface(display->shell, window->surface);

                assert(window->shell_surface);

                wl_shell_surface_add_listener(window->shell_surface,
                                              &shell_surface_listener,
                                              window);
                wl_shell_surface_set_title(window->shell_surface,
                                           "Hello wayland!");
                wl_shell_surface_set_toplevel(window->shell_surface);
        } else {
                fprintf(stderr, "Incompatible shell\n");
                exit(1);
        }
        
        return window;
}

void destroy_window(struct my_window *window)
{
        int i;
        
        if (window->callback) {
                wl_callback_destroy(window->callback);
                window->callback = NULL;
        }

        
        for (i = 0; i < BUFFERS; i++) {
                if (window->buffers[i].buffer != NULL) {
                        wl_buffer_destroy(window->buffers[i].buffer);
                        window->buffers[i].buffer = NULL;
                }
        }

        if (window->shm_pool) {
                wl_shm_pool_destroy(window->shm_pool);
                munmap(window->shm_data,
                       window->display->width * window->display->height * 4 * BUFFERS);
                remove(window->shm_fname);
                window->shm_pool = NULL;
        }
        
        if (window->shell_surface) {
                wl_shell_surface_destroy(window->shell_surface);
                window->shell_surface = NULL;
        }
        
        wl_surface_destroy(window->surface);
        window->surface = NULL;
        free(window);
}

/* Window will be marked unresponsive if this is not performed. */
static void shell_surface_ping(void *data,
                               struct wl_shell_surface *shell_surface,
                               uint32_t serial)
{
        wl_shell_surface_pong(shell_surface, serial);
}

/* Accept configuration hints. (or not) */
static void shell_surface_configure(void *data,
                                    struct wl_shell_surface *wl_shell_surface,
                                    uint32_t edges,
                                    int32_t width,
                                    int32_t height)
{
        /* data param is unused remove comment for cast */
        struct my_window *window = data;
        printf("Config = (%u, %d, %d)\n", edges, width, height);

        window->width = width;
        window->height = height;
}

/* Some shit about popups. */
static void shell_surface_popup_done(void *data,
                                     struct wl_shell_surface *wl_shell_surface)
{
}

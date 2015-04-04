#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <string.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <assert.h>
#include "simple.h"

/* wl_display related */

static void registry_global(void *data,
                            struct wl_registry *registry,
                            uint32_t name,
                            const char *interface,
                            uint32_t version);
static void registry_global_remove(void *a,
                                   struct wl_registry *b,
                                   uint32_t name);
static void shm_format(void *data,
                       struct wl_shm *wl_shm,
                       uint32_t format);
static void output_geometry(void *data,
                            struct wl_output *output,
                            int32_t x,
                            int32_t y,
                            int32_t width_mm,
                            int32_t height_mm,
                            int32_t sub_pixel,
                            const char *make,
                            const char *model,
                            int32_t transform);
static void output_mode(void *data,
                        struct wl_output *wl_output,
                        uint32_t flags,
                        int32_t width,
                        int32_t height,
                        int32_t refresh);
static void output_done(void *data,
                        struct wl_output *wl_output);
static void output_scale(void *data,
                         struct wl_output *wl_output,
                         int32_t factor);
static void xdg_shell_ping(void *data,
                           struct xdg_shell *xdg_shell,
                           uint32_t serial);

/* Listen's to messages from the  */
const struct wl_registry_listener registry_listener = {
        .global        = registry_global,
        .global_remove = registry_global_remove
};

struct wl_shm_listener shm_listener = {
        .format = shm_format
};

struct wl_output_listener output_listener = {
        .geometry = output_geometry,
        .mode     = output_mode,
        .done     = output_done,
        .scale    = output_scale
};

struct xdg_shell_listener xdg_shell_listener = {
        .ping = xdg_shell_ping
};

/* Create a display object. Creating the nessasary wayland objects
 * and setting up the registry listener.
 */
struct my_display *create_display()
{
        struct my_display *display;

        display = calloc(1, sizeof(*display));
        if (display == NULL) {
                perror(""); exit(1);
        }

        display->display = wl_display_connect(NULL);
        if (display->display == NULL) {
                perror("Couldn't connect to default wayland display");
                exit(1);
        }

        display->formats = 0;
        display->registry = wl_display_get_registry(display->display);
        wl_registry_add_listener(display->registry,
                                 &registry_listener,
                                 display);
        wl_display_roundtrip(display->display);
        if (display->shm == NULL) {
                fprintf(stderr, "No global wl_shm\n");
                exit(1);
        }
        wl_display_roundtrip(display->display);
        
        if (!(display->formats & (1 << WL_SHM_FORMAT_ARGB8888))) {
                fprintf(stderr, "Pixel format xrgb32 not available\n");
                exit(1);
        }

        return display;
}


void destroy_display(struct my_display *display)
{
        if (display->shm != NULL) {
                wl_shm_destroy(display->shm);
                display->shm = NULL;
        }
        
        if (display->compositor != NULL) {
                wl_compositor_destroy(display->compositor);
                display->shm = NULL;
        }

        if (display->xdg_shell != NULL) {
                xdg_shell_destroy(display->xdg_shell);
                display->xdg_shell = NULL;
        }
        
	if (display->shell) {
                wl_shell_destroy(display->shell);
                display->shm = NULL;
        }

        if (display->output) {
                wl_output_destroy(display->output);
                display->output = NULL;
        }

        wl_registry_destroy(display->registry); display->registry = NULL;
        wl_display_flush(display->display); 
        wl_display_disconnect(display->display); display->display = NULL;
        free(display);
}

static void registry_global(void *data,
                            struct wl_registry *registry,
                            uint32_t name,
                            const char *interface,
                            uint32_t version)
{
        struct my_display *d = data;

        printf("%u %s\n", name, interface);

        /* Bind interfaces that we want to use */
        if (strcmp(interface, "wl_compositor") == 0) {
                /* Compositor for creating surface objects */
                d->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
                
        } else if (strcmp(interface, "wl_shm") == 0) {
                /* shm for exchange share memory objects */
                d->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
                wl_shm_add_listener(d->shm, &shm_listener, d);
                
        } else if (strcmp(interface, "xdg_shell") == 0) {
                /* xdg_shell for desktop window. */
                d->xdg_shell = wl_registry_bind(registry, name, &xdg_shell_interface, version);
                xdg_shell_add_listener(d->xdg_shell, &xdg_shell_listener, d);
                xdg_shell_use_unstable_version(d->xdg_shell, XDG_SHELL_VERSION_CURRENT);
        } else if (strcmp(interface, "wl_shell") == 0) {
                /* For fall back. Don't use if xdg_shell is avaliable. */
                d->shell = wl_registry_bind(registry, name, &wl_shell_interface, version);
        } else if (strcmp(interface, "wl_output") == 0) {
                /* output for screen intormation */
                if (d->output == NULL) {
                        d->output = wl_registry_bind(registry, name, &wl_output_interface, version);
                        assert(d->output);
                        wl_output_add_listener(d->output, &output_listener, d);
                } else {
                        printf("Observed new output but ignored it.\n");
                }
        }
        
}

static void registry_global_remove(void *a,
                                   struct wl_registry *b,
                                   uint32_t c)
{
}

static void shm_format(void *data,
                       struct wl_shm *wl_shm,
                       uint32_t format)
{
        struct my_display *d = data;

        d->formats |= (1 << format);
}

static void output_geometry(void *data,
                            struct wl_output *output,
                            int32_t x,
                            int32_t y,
                            int32_t width_mm,
                            int32_t height_mm,
                            int32_t sub_pixel,
                            const char *make,
                            const char *model,
                            int32_t transform)
{
        printf("OutputGeometry { x: %d, y: %d, height: %d mm, width %d mm,"
               " sub_pixel: %d, transform: %d, make: '%s', model: '%s' }\n",
               x, y, height_mm, width_mm, sub_pixel, transform, make, model);
}

static void output_mode(void *data,
                        struct wl_output *wl_output,
                        uint32_t flags,
                        int32_t width,
                        int32_t height,
                        int32_t refresh)
{
        struct my_display *display = data;
        
        display->width = width;
        display->height = height;
                
        printf("Mode { flags: ");

        if (flags && WL_OUTPUT_MODE_CURRENT) {
                printf("mode::current ");
                if (flags && ~WL_OUTPUT_MODE_CURRENT)
                        printf("and ");
        }
        if (flags && WL_OUTPUT_MODE_PREFERRED) {
                printf("mode::preferred");
        }
                
        printf(", width: %" PRId32 ", height: %" PRId32 ", "
               "refresh_rate: %" PRId32" }\n", width, height, refresh);
}

static void output_done(void *data,
                        struct wl_output *wl_output)
{
}

static void output_scale(void *data,
                         struct wl_output *wl_output,
                         int32_t factor)
{
        printf("ScalingFactor: %d\n", factor);
}

static void xdg_shell_ping(void *data,
                           struct xdg_shell *xdg_shell,
                           uint32_t serial)
{
        xdg_shell_pong(xdg_shell, serial);
}

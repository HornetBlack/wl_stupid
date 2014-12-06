#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <wayland-client.h>
#include "simple.h"

/* Marks the end of program running. */
static int running = 1;
static void signal_int(int signum) {
        running = 0;
}

int main(void)
{
        struct sigaction   sigint;
        struct my_display *display;
        struct my_window  *window;

        /* Connect to the display */
        printf("Connecting to display\n");
        display = create_display();
        printf("Connected!\n");

        /* Create a window */
        printf("Creating a window\n");
        window = create_window(display, MAX_WIDTH, MAX_HEIGHT);
        if (!window)
                return 1;
        printf("Window created\n");

        /* Set up singal handler. So SIGINT allows us to cleanly die. */
        sigint.sa_handler = signal_int;
        sigemptyset(&sigint.sa_mask);
        sigint.sa_flags = SA_RESETHAND;
        sigaction(SIGINT, &sigint, NULL);

        printf("Initialising buffers\n");
        /* Initialize */
        wl_surface_damage(window->surface, 0, 0, window->width, window->height);
        /* Draw first screen which allocates buffer(s). */
        draw(window, NULL, 0);

        printf("Starting loop\n");
        /* Main loop */
        while (running) {
                if (wl_display_dispatch(display->display) == -1) {
                        running = 0;
                }
        }
        printf("Loop exited\n");
        
        /* Destroy the display */
        printf("Destroying window\n");
        destroy_window(window); window = NULL;
        printf("Disconnecting display\n");
        destroy_display(display); display = NULL;
        printf("Done\n");

        return 0;
}


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/mman.h>

#include <wayland-client.h>
#include "simple.h"

/* Unlock buffer when wayland is done with it. */
static void buffer_release(void *data, struct wl_buffer *buffer) {
        struct my_buffer *my_buffer = data;
        my_buffer->busy = 0;
};

static const struct wl_buffer_listener buffer_listener = {
        buffer_release
};

static const struct wl_callback_listener frame_listener = {
        draw,
};

/**
 * Create a shared memory object and return the fd.
 */
static int create_shm(size_t size,
                      char **fname)
{
        const char template[] = "tmp-hello-wayland-XXXXXX";
        char *tmpname;
        int fd;

        tmpname = malloc(sizeof template);
        assert(tmpname != NULL);
        memcpy(tmpname, template, sizeof template);

        fd = mkstemp(tmpname);
        if (fd < 0) {
                perror("Failed making shm");
                exit(1);
        }

        if (ftruncate(fd, size) < 0) {
                close(fd);
                perror("Failed to resize shm");
                exit(1);
        }

        if (fname != NULL) {
                *fname = tmpname;
        }

        return fd;
}


struct my_buffer *select_buffer(struct my_window *window)
{
        struct my_display *display;
        struct my_buffer *buffer;
        int fd;
        void *data;
        int buf_num;

        int32_t stride;
        int32_t max_buffer_size;
        int32_t shm_pool_size;
        
        display = window->display;

        stride = display->width * 4;
        assert(stride > 0);
        max_buffer_size = stride * display->height;
        assert(max_buffer_size > 0);
        shm_pool_size = max_buffer_size * BUFFERS;
        assert(shm_pool_size > 0);
        
        for (buf_num = 0; buf_num < BUFFERS; buf_num++) {
                if (!window->buffers[buf_num].busy) {
                        buffer = &window->buffers[buf_num];
                        break;
                }
        }
        if (buf_num >= BUFFERS) {
                return NULL;
        }

        if (!window->shm_pool) {
                printf("Makeing new wl_shm_pool { size: %"PRId32" }\n", shm_pool_size);
                fd = create_shm(shm_pool_size, &window->shm_fname);
                data = mmap(NULL, shm_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                
                if (data == MAP_FAILED) {
                        close(fd);
                        perror("Mmap failed");
                        exit(1);
                }
                
                window->shm_data = data;
                window->shm_pool = wl_shm_create_pool(display->shm, fd, max_buffer_size * BUFFERS);
                close(fd);
        }

        /* Destroy wl_buffer object if window was resized.  */
        if (buffer->buffer
            && (buffer->width != window->width
                || buffer->height != window->height))
        {
                printf("Window resized, destroying buffer %d\n", buf_num);
                wl_buffer_destroy(buffer->buffer);
                buffer->buffer = NULL;
        }
                
        if (!buffer->buffer) {
                int32_t mem_offset = max_buffer_size * buf_num;
                buffer->stride = stride;
                buffer->width = window->width;
                buffer->height = window->height;
                buffer->buffer = wl_shm_pool_create_buffer(window->shm_pool,
                                                           mem_offset,
                                                           buffer->width,
                                                           buffer->height,
                                                           buffer->stride,
                                                           WL_SHM_FORMAT_ARGB8888);
                if (!buffer->buffer) {
                        fprintf(stderr, "Failed to create buffer\n");
                        return NULL;
                }

                wl_buffer_add_listener(buffer->buffer, &buffer_listener, buffer);

                buffer->data = (char*)window->shm_data + mem_offset;
        }

        return buffer;
}

/*
 * Draw the screen.
 */
void draw(void *data_, struct wl_callback *callback, uint32_t serial)
{
        struct my_window *window = data_;
        struct my_buffer *buffer;
        int32_t i;
        int32_t x, y;
        // double x, y;
        // int n_pixels;
        
        
        struct pixel {
                uint8_t b, g, r, a;
        } *buffer_data;
        
        time_t curr_time;

        /* Fps counter */
        static struct fps_counter {
                time_t last_start;
                int frames;
        } fps_counter = { 0, 0 };
        
        // n_pixels = window->width * window->height;

        buffer = select_buffer(window);
        if (!buffer) {
                fprintf(stderr,
                        "%s\n",
                        (callback == NULL) ? "Failed to create buffer.\n" :
                        "Both buffers are busy");
                abort();
        }

        buffer_data = buffer->data;
        assert(buffer_data != NULL);

        // printf("Drawing greyness\n");
        // printf("Buffer offset = %zd\n",  (char*)buffer_data - (char*)window->shm_data);
        
        for (y = 0; y < buffer->height; y++) {
                for (x = 0; x < buffer->width; x++) {
                        i = x + (y * buffer->stride)/4;
                        // buffer_data[i] = (struct pixel){ 128, 128, 128, 128 };

                        double rr, gg, bb;
                        rr = 255 * (double)x / (double)buffer->width;
                        gg = 255 * (double)y / (double)buffer->height;
                        bb = 255 * (double)(x+y)/2 / (double)((buffer->width + buffer->height)/2);

                        buffer_data[i] = (struct pixel){ bb, gg, rr, (0.7*255) };
                }
        }
        // printf("Done drawing\n");
        
        /* for (i = 0; i < n_pixels; i++) { */
        /*         x = (double)(i % window->width) / (double)window->width; */
        /*         y = (double)(i / window->width) / (double)window->height; */

        /*         buffer_data[i].a = 255; // blue */
        /*         buffer_data[i].r = 255 * x;  // green */
        /*         buffer_data[i].g = 255 * y; // red */
        /*         buffer_data[i].b = 0; // alpha */
        /* } */

        /* Tell compositor what to draw. */
        wl_surface_attach(window->surface, buffer->buffer, 0, 0);
        /* Tell compositor that it needs to draw */
        wl_surface_damage(window->surface, 0, 0, window->width, window->height);

        if (callback)
                wl_callback_destroy(callback);

        window->callback = wl_surface_frame(window->surface);
        wl_callback_add_listener(window->callback, &frame_listener, window);
        wl_surface_commit(window->surface);
        buffer->busy = 1;

        fps_counter.frames++;
        time(&curr_time);
        if (curr_time != fps_counter.last_start) {
                printf("fps = %d\n", fps_counter.frames);
                fps_counter.frames = 0;
                fps_counter.last_start = curr_time;
        }
}

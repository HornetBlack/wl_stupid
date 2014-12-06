#ifndef SIMPLE_H_
#define SIMPLE_H_

enum {
  MAX_WIDTH   = 640,            /**< Max width of window in pixels */
  MAX_HEIGHT  = 480,            /**< Max height of window in pixels */
  BUFFERS = 2,                  /**< Number of frame buffers. */
};

/**
 * \struct my_display
 * \brief  Contains objects relavent to the server.
 * 
 * With the exception of display and registry all of these objects are recieved via the
 * registry. During creation the registry will listen for events that bind these global object.
 * Other objections may be sent, but they won't be bound and ignored.
 */
struct my_display {
  struct wl_display    *display;
  struct wl_registry   *registry;
  struct wl_compositor *compositor;
  struct wl_shm        *shm;
  struct wl_shell      *shell;
  struct wl_output     *output;
  int formats;

  // Output size in pixels.
  int32_t width, height;
};

struct my_buffer {
  struct wl_buffer *buffer;
  int32_t width, height, stride;        /* The width and height on last render */
  void *data;
  int busy;
};

struct my_window {
  struct my_display *display;
  int width, height;
  struct wl_surface *surface;
  struct wl_callback *callback;
  struct wl_shell_surface *shell_surface;
  struct wl_shm_pool *shm_pool;
  char *shm_fname;
  void *shm_data;
  struct my_buffer buffers[BUFFERS];
};

/* Display */
struct my_display *create_display();
void destroy_display(struct my_display *display);

/* Window */
struct my_window *create_window(struct my_display *display, int width, int height);
void              destroy_window(struct my_window *window);

/* Buffers */
void draw(void *window, struct wl_callback *callback, uint32_t serial);

#endif /* SIMPLE_H_ */

# A Simple Wayland Client #
## CPU rendering for the soul ##

Requires:

* _Wayland libraries_,
  ensure `pkg-config --libs wayland-client`
  prints something like: `-lwayland-client`,
  and that `/usr/share/include` contains `wayland-client.h`

* A _Wayland compositor_.
  _Weston_ is the most reliable.
  It can by run in an X window (via `weston` command),
  or use a TTY and run `weston` or `weston-launch`.
  Gnome on wayland for GNOME >= 3.14 can be used.

* Both `wl_shell` and `xdg_shell` (v5) are supported. I have yet to
  update it to `zxdg_shell_v6`.

* A C compiler that supports gnu99.

* A copy of `xdg-shell.xml`. 
  I have included a copy from the `wayland-protocols` git.

## Compilation

    make

## Running

Ensure you have a running wayland compositor.
(`$WAYLAND_DISPLAY` and `$XDG_RUNTIME_DIR` are set,
and there is a unix socket at `$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY`)

Then run `./simple`.

If successful you should have semitransparent window
with a rendering of the mandelbrot set.

## Notes

This can be very CPU intense.
You can also modify the code to use metaballs demo instead of the mandelbrot.
But this will require per frame rendering.

It currenlty will ignore more than 1 screen. Which shouldn't be too
much of a problem. (We only use the screen info to set the maximum
window size)

## Todo ##

* Code layout. There is horrible mess everywhere.
* Some sort of commentary of what is going on.
* Allowing more intuitive switching to meta-balls example.
  (Currenlty remove the `#define BROT` from `buffer.c`)

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
  
* The compositor needs to support the `xdg_shell` interface.
  Check `weston-info | grep xdg_shell` from your compositor.
  Weston, Gnome and KDE support this.
  Other compositors should too, (as the built-in wl\_shell\_surface is broken).

* A C compiler that supports gnu99.

* A copy of `xdg-shell.xml`.
  You can fetch a copy from weston's source tree.

  http://cgit.freedesktop.org/wayland/weston/plain/protocol/xdg-shell.xml

## Compilation

    make

## Running

Ensure you are in a wayland compositor. (So $WAYLAND_DISPLAY is set).
Then run `./simple`. 

If successful you should have semitransparent window
with a rendering of the mandelbrot set.

## Notes

This can be very CPU intense.
You might want to modify the source code
to only re-render when the window is resized.
You can also modify the code to use metaballs demo instead of the mandelbrot.
But this will require per frame rendering.

As it doesn't dynamically accept wl_output objects correctly,
hot plugging screens may not work well.
Although I suspect that the only problem is that window cannot be bigger
than the first output is was told about. (Which should be hand

## Todo ##

* Code layout. There is horrible mess everywhere.
* Some sort of description of what is going on.

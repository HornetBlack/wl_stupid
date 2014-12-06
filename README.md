# Simple Wayland Client #
## CPU rendering for the soul ##

Requires:

* _Wayland libraries_,
  make sure `pkg-config --libs wayland-client`
  prints something like: `-lwayland-client`,
  and that `/usr/share/include` contains `wayland-client.h`

* A _Wayland compositor_.
  _Weston_ is the most reliable.
  It can by run in an X window (via `weston` command),
  or use ctrl-alt-fX log in and run `weston`.
  Gnome on wayland for GNOME >= 3.14 can be used.
  (xdg_surface is not used so any compositor that supports 1.6 interfaces should do)

* A compiler. _gcc_ and _clang_ both work.

To compile do this:

```bash
    gcc -Wall -Werror -std=gnu99 -O2 -o simple      \
        simple.c display.c window.c buffer.c -lm    \
        `pkg-config --libs wayland-client`
```


### Todo ###

* Makefile.
* Source Tree layout.
* Code layout. buffer.c is horrible mess.
* More comments
* Instructional article on how it works.

#include "input.h"
#include "render.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

void handle_input() {
  XEvent event;
  while (XPending(display)) {
    XNextEvent(display, &event);
    if (event.type == KeyPress) {
      char buf[128];
      KeySym keysym;
      int len = XLookupString(&event.xkey, buf, sizeof(buf), &keysym, NULL);
      if (len > 0) {
        printf("Key pressed: %s\n", buf);
      }
    }
  }
}

void input_cleanup() { XCloseDisplay(display); }

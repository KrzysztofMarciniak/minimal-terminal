#include "render.h"
#include <X11/Xlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handle_signal(int sig);

int running = 1;

void handle_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    running = 0;

    if (display) {
      if (window) {
        XDestroyWindow(display, window);
      }
      XCloseDisplay(display);
    }

    render_cleanup();
    exit(0);
  }
}

int main(void) {
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  init_rendering();
  while (running) {
    while (XPending(display)) {
      XEvent ev;
      XNextEvent(display, &ev);
      switch (ev.type) {
      case Expose:
        render_screen();
        break;
      case KeyPress:
        handle_key_event(&ev.xkey);
        render_screen();
        break;
      case ClientMessage:
        running = 0;
        break;
      case DestroyNotify:
        running = 0;
        break;
      }
    }
    struct timespec ts = {0, 10000000L};
    usleep(10000);
  }
  if (display) {
    if (window) {
      XDestroyWindow(display, window);
    }
    XCloseDisplay(display);
  }
  render_cleanup();
  exit(0);
}

#include "input.h"
#include "render.h"
#include <X11/Xlib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handle_signal(int sig);

int running = 1;

void handle_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    running = 0;
    XDestroyWindow(display, window);
  }
}

int main() {
  XEvent event;

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  init_rendering();
  while (running) {
    XNextEvent(display, &event);
    switch (event.type) {
    case Expose:
      render_screen();
      break;
    case KeyPress:
      handle_key_event(&event.xkey);
      render_screen();
      break;
    case DestroyNotify:
      running = 0;
      break;
    case ClientMessage:
      running = 0;
      break;
    }
  }
  render_cleanup();
  return 0;
}

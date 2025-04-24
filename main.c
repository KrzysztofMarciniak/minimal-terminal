#include "render.h"
#include "input.h"

int main() {
  XEvent event;
  init_rendering();
  while (1) {
    XNextEvent(display, &event);
    switch (event.type) {
    case Expose:
      render_screen();
      break;
    case KeyPress:
      handle_key_event(&event.xkey);
      render_screen(); 
      break;
    }
  }
  render_cleanup();
  return 0;
}

#include "input.h"
#include "terminal.h"
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <string.h>

extern Display *display;
extern Window window;
extern GC gc;

static char input_buffer[1024];
static int input_pos = 0;

void init_input() {
  input_pos = 0;
  memset(input_buffer, 0, sizeof(input_buffer));
}

void handle_input(XKeyEvent *event) {
  KeySym keysym;
  char buffer[20];
  int count;

  count = XLookupString(event, buffer, sizeof(buffer), &keysym, NULL);

  if (count > 0) {
    if (keysym == XK_Return) {
      terminal_write("\n");
      input_buffer[input_pos] = '\0';

      if (input_pos > 0) {
        terminal_execute_command(input_buffer);
      } else {
        terminal_write(terminal_get_prompt());
      }

      input_pos = 0;
    } else if (keysym == XK_BackSpace) {
      if (input_pos > 0) {
        input_pos--;
        int row = get_cursor_row();
        int col = get_cursor_col();
        if (col > 0) {
          terminal_move_cursor(row, col - 1);
          terminal_write(" ");
          terminal_move_cursor(row, col - 1);
        }
      }
    } else {
      if (input_pos < sizeof(input_buffer) - 1) {
        input_buffer[input_pos++] = buffer[0];
        char text[2] = {buffer[0], '\0'};
        terminal_write(text);
      }
    }
  }
}

void input_cleanup() {
  input_pos = 0;
  memset(input_buffer, 0, sizeof(input_buffer));
}

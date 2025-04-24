#include "render.h"
#include "ansi.h"
#include "terminal.h"
#include "input.h"
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Display *display;
Window window;
GC gc;
static XFontStruct *font;

static unsigned long x_colors[8] = {COLOR_BLACK,  COLOR_RED,  COLOR_GREEN,
                                    COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA,
                                    COLOR_CYAN,   COLOR_WHITE};

#define LETTERS_COLOR WhitePixel(display, DefaultScreen(display))
#define BACKGROUND_COLOR BlackPixel(display, DefaultScreen(display))

void init_rendering() {
  display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Cannot open X display\n");
    exit(1);
  }

  int screen = DefaultScreen(display);
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0,
                               TERM_COLS * CHAR_WIDTH, TERM_ROWS * CHAR_HEIGHT,
                               1, BACKGROUND_COLOR, BACKGROUND_COLOR);
  XSelectInput(display, window, ExposureMask | KeyPressMask);
  XMapWindow(display, window);

  gc = XCreateGC(display, window, 0, NULL);
  XSetForeground(display, gc, LETTERS_COLOR);

  font = XLoadQueryFont(display, "fixed");
  if (!font) {
    fprintf(stderr, "Cannot load font\n");
    exit(1);
  }
  XSetFont(display, gc, font->fid);

  init_terminal(TERM_ROWS, TERM_COLS);
  init_input();
}

void render_cleanup() {
  XFreeFont(display, font);
  XFreeGC(display, gc);
  XCloseDisplay(display);
  terminal_cleanup();
  input_cleanup();
}

void render_screen() {
  XClearWindow(display, window);

  const char **buffer = get_terminal_buffer();
  int rows = get_terminal_rows();
  int cols = get_terminal_cols();

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      char ch = buffer[i][j];
      if (ch != ' ') {
        char text[2] = {ch, '\0'};
        XDrawString(display, window, gc, j * CHAR_WIDTH,
                    (i + 1) * CHAR_HEIGHT - 4, text, 1);
      }
    }
  }

  int cursor_row = get_cursor_row();
  int cursor_col = get_cursor_col();
  XSetForeground(display, gc, x_colors[ANSI_COLOR_WHITE]);
  XFillRectangle(display, window, gc, cursor_col * CHAR_WIDTH,
                 cursor_row * CHAR_HEIGHT + CHAR_HEIGHT - 2, CHAR_WIDTH, 2);

  XFlush(display);
}

void handle_key_event(XKeyEvent *event) {
  handle_input(event);
}

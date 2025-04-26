#include "render.h"
#include "ansi.h"
#include "input.h"
#include "terminal.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define PADDING 5
#define GRID_COLOR 0x333333
#define BORDER_WIDTH 1
#define DEBUG_GRID false

Display *display;
Window window;
GC gc;
static Pixmap backBuffer;
static Atom wmDelete;

static XFontStruct *font;
static int charW, charH;
static int cols, rows;
static unsigned long colors[8] = {COLOR_BLACK,  COLOR_RED,  COLOR_GREEN,
                                  COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA,
                                  COLOR_CYAN,   COLOR_WHITE};

static inline int winW() {
  XWindowAttributes wa;
  XGetWindowAttributes(display, window, &wa);
  return wa.width;
}
static inline int winH() {
  XWindowAttributes wa;
  XGetWindowAttributes(display, window, &wa);
  return wa.height;
}

static void ensure_resize(int newW, int newH) {
  int newCols = (newW - 2 * PADDING) / charW;
  int newRows = (newH - 2 * PADDING) / charH;
  if (newCols < 1)
    newCols = 1;
  if (newRows < 1)
    newRows = 1;

  if (newCols != cols || newRows != rows) {
    cols = newCols;
    rows = newRows;
    resize_terminal(rows, cols);

    if (backBuffer)
      XFreePixmap(display, backBuffer);
    backBuffer = XCreatePixmap(display, window, newW, newH,
                               DefaultDepth(display, DefaultScreen(display)));
  }
}

static void handle_signal(int signum) {
  render_cleanup();
  exit(0);
}

void init_rendering() {
  display = XOpenDisplay(NULL);
  if (!display) {
    perror("XOpenDisplay");
    exit(1);
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  int screen = DefaultScreen(display);

  font = XLoadQueryFont(
      display, "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-1");
  if (!font && !(font = XLoadQueryFont(display, "fixed"))) {
    fprintf(stderr, "Unable to load any font\n");
    exit(1);
  }

  charW = font->max_bounds.width;
  charH = font->ascent + font->descent;

  cols = TERM_COLS;
  rows = TERM_ROWS;

  int w = cols * charW + 2 * PADDING;
  int h = rows * charH + 2 * PADDING;

  window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, w, h,
                               BORDER_WIDTH, BlackPixel(display, screen),
                               BlackPixel(display, screen));
  if (!window) {
    fprintf(stderr, "XCreateSimpleWindow failed\n");
    exit(1);
  }

  XStoreName(display, window, "Minimal Terminal");
  wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDelete, 1);

  gc = XCreateGC(display, window, 0, NULL);
  XSetFont(display, gc, font->fid);
  XSetForeground(display, gc, WhitePixel(display, screen));

  XSelectInput(display, window,
               ExposureMask | KeyPressMask | StructureNotifyMask);
  XMapWindow(display, window);
  XFlush(display);

  backBuffer =
      XCreatePixmap(display, window, w, h, DefaultDepth(display, screen));

  init_terminal(rows, cols);
  init_input();
}

void render_screen() {
  int w = winW(), h = winH();

  ensure_resize(w, h);

  XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
  XFillRectangle(display, backBuffer, gc, 0, 0, w, h);

  if (DEBUG_GRID) {
    XSetForeground(display, gc, GRID_COLOR);
    for (int i = 0; i <= rows; i++) {
      int y = PADDING + i * charH;
      XDrawLine(display, backBuffer, gc, PADDING, y, PADDING + cols * charW, y);
    }
  }
  for (int j = 0; j <= cols; j++) {
    int x = PADDING + j * charW;
    XDrawLine(display, backBuffer, gc, x, PADDING, x, PADDING + rows * charH);
  }

  XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
  const char **buf = get_terminal_buffer();
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      char ch = buf[r][c];
      if (ch != ' ') {
        int x = PADDING + c * charW;
        int y = PADDING + r * charH + font->ascent;
        char txt[2] = {ch, '\0'};
        XDrawString(display, backBuffer, gc, x, y, txt, 1);
      }
    }
  }

  int cr = get_cursor_row(), cc = get_cursor_col();
  XSetForeground(display, gc, colors[ANSI_COLOR_WHITE]);
  XFillRectangle(display, backBuffer, gc, PADDING + cc * charW,
                 PADDING + cr * charH + charH - 2, charW, 2);

  XCopyArea(display, backBuffer, window, gc, 0, 0, w, h, 0, 0);
  XFlush(display);
}

void handle_key_event(XKeyEvent *kev) { handle_input(kev); }

void process_events() {
  while (XPending(display)) {
    XEvent e;
    XNextEvent(display, &e);
    if (e.type == ClientMessage && (Atom)e.xclient.data.l[0] == wmDelete) {
      render_cleanup();
      exit(0);
    }
    switch (e.type) {
    case KeyPress:
      handle_key_event(&e.xkey);
      break;
    case Expose:
      render_screen();
      break;
    case ConfigureNotify:
      ensure_resize(e.xconfigure.width, e.xconfigure.height);
      render_screen();
      break;
    }
  }
}

void render_cleanup() {
  if (backBuffer)
    XFreePixmap(display, backBuffer);
  if (gc)
    XFreeGC(display, gc);
  if (font)
    XFreeFont(display, font);
  if (display)
    XCloseDisplay(display);
  terminal_cleanup();
  input_cleanup();
}

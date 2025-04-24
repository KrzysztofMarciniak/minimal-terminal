#ifndef RENDER_H
#define RENDER_H

#include <X11/Xlib.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

#define COLOR_BLACK   0x000000
#define COLOR_RED     0xFF0000
#define COLOR_GREEN   0x00FF00
#define COLOR_YELLOW  0xFFFF00
#define COLOR_BLUE    0x0000FF
#define COLOR_MAGENTA 0xFF00FF
#define COLOR_CYAN    0x00FFFF
#define COLOR_WHITE   0xFFFFFF

extern Display *display;
extern Window window;
extern GC gc;

void init_rendering();
void render_cleanup();
void render_screen();
void handle_key_event(XKeyEvent *event);

#endif // RENDER_H

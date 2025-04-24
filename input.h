#ifndef INPUT_H
#define INPUT_H

#include <X11/Xlib.h>

void init_input();
void handle_input(XKeyEvent *event);
void input_cleanup();

#endif

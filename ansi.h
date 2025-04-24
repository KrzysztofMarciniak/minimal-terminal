#ifndef ANSI_H
#define ANSI_H

#define ANSI_COLOR_BLACK   0
#define ANSI_COLOR_RED     1
#define ANSI_COLOR_GREEN   2
#define ANSI_COLOR_YELLOW  3
#define ANSI_COLOR_BLUE    4
#define ANSI_COLOR_MAGENTA 5
#define ANSI_COLOR_CYAN    6
#define ANSI_COLOR_WHITE   7

void parse_ansi(const char *seq, int *fg_color, int *bg_color, int *attr);

#endif // ANSI_H
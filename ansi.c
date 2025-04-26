#include "ansi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_ansi(const char *seq, int *fg_color, int *bg_color, int *attr) {
  if (seq[0] == '[') {
    char *end;
    int param = strtol(seq + 1, &end, 10);

    if (*end == 'm') {
      switch (param) {
      case 0:
        *attr = 0;
        *fg_color = ANSI_COLOR_WHITE;
        *bg_color = ANSI_COLOR_BLACK;
        break;
      case 1: // Bold
        *attr |= 1;
        break;
      case 30:
        *fg_color = ANSI_COLOR_BLACK;
        break;
      case 31:
        *fg_color = ANSI_COLOR_RED;
        break;
      case 32:
        *fg_color = ANSI_COLOR_GREEN;
        break;
      case 33:
        *fg_color = ANSI_COLOR_YELLOW;
        break;
      case 34:
        *fg_color = ANSI_COLOR_BLUE;
        break;
      case 35:
        *fg_color = ANSI_COLOR_MAGENTA;
        break;
      case 36:
        *fg_color = ANSI_COLOR_CYAN;
        break;
      case 37:
        *fg_color = ANSI_COLOR_WHITE;
        break;
      case 40:
        *bg_color = ANSI_COLOR_BLACK;
        break;
      case 41:
        *bg_color = ANSI_COLOR_RED;
        break;
      case 42:
        *bg_color = ANSI_COLOR_GREEN;
        break;
      case 43:
        *bg_color = ANSI_COLOR_YELLOW;
        break;
      case 44:
        *bg_color = ANSI_COLOR_BLUE;
        break;
      case 45:
        *bg_color = ANSI_COLOR_MAGENTA;
        break;
      case 46:
        *bg_color = ANSI_COLOR_CYAN;
        break;
      case 47:
        *bg_color = ANSI_COLOR_WHITE;
        break;
      }
    }
  }
}

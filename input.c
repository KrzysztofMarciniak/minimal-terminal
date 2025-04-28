#include "input.h"
#include "terminal.h"
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Display *display;
extern Window window;
extern GC gc;

#define MAX_INPUT_BUFFER_SIZE 1024
#define CTRL_L 12

static char input_buffer[MAX_INPUT_BUFFER_SIZE];
static int input_pos = 0;

typedef void (*CommandFunc)(char *args);

typedef struct {
  const char *name;
  CommandFunc func;
} Command;

static void cmd_clear(char *args);

static Command commands[] = {{"clear", cmd_clear}, {NULL, NULL}};

void init_input() {
  input_pos = 0;
  memset(input_buffer, 0, sizeof(input_buffer));
}

void input_cleanup() {
  input_pos = 0;
  memset(input_buffer, 0, sizeof(input_buffer));
}

static void cmd_clear(char *args) { terminal_clear(); }

static void process_command(const char *input) {
  if (!input || !*input)
    return;

  char temp[MAX_INPUT_BUFFER_SIZE];
  strncpy(temp, input, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';

  char *command = strtok(temp, " ");
  char *args = strtok(NULL, "");

  if (!command)
    return;

  for (Command *cmd = commands; cmd->name != NULL; cmd++) {
    if (strcmp(command, cmd->name) == 0) {
      cmd->func(args);
      return;
    }
  }

  terminal_execute_command(input);
}

static int is_printable(char c) { return (c >= 32 && c <= 126); }

void handle_input(XKeyEvent *event) {
  KeySym keysym;
  char buffer[32];
  int count = XLookupString(event, buffer, sizeof(buffer), &keysym, NULL);

  if (count <= 0)
    return;

  char c = buffer[0];

  if (c == CTRL_L) { // Ctrl+L
    terminal_clear();
    return;
  }
  if (keysym == XK_Return) { // Enter key
    input_buffer[input_pos] = '\0';

    if (input_pos > 0) { 
      terminal_write("\n");
      process_command(input_buffer);
    }

    input_pos = 0;
    memset(input_buffer, 0, sizeof(input_buffer));
    return;
  }

  if (keysym == XK_BackSpace) {
    if (input_pos > 0) {
      input_pos--;
      input_buffer[input_pos] = '\0';
      int row = get_cursor_row();
      int col = get_cursor_col();
      if (col > 0) {
        terminal_move_cursor(row, col - 1);
        terminal_write(" ");
        terminal_move_cursor(row, col - 1);
      }
    }
    return;
  }

  if (is_printable(c)) {
    if (input_pos < MAX_INPUT_BUFFER_SIZE - 1) {
      input_buffer[input_pos++] = c;
      char text[2] = {c, '\0'};
      terminal_write(text);
    } else {
      terminal_write("\a"); // Bell on overflow
    }
  }
}

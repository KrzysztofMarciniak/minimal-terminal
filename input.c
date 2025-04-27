#include "input.h"
#include "terminal.h"
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>

extern Display *display;
extern Window window;
extern GC gc;
int pty_fd = -1;
#define MAX_INPUT_BUFFER_SIZE 1024

static char input_buffer[MAX_INPUT_BUFFER_SIZE];
static int input_pos = 0;

typedef void (*CommandFunc)(char *args);

typedef struct {
  const char *name;
  CommandFunc func;
} Command;

static void cmd_exit(char *args);
static void cmd_clear(char *args);
static void cmd_unknown(char *input);

// Command registry
static Command commands[] = {
    {"clear", cmd_clear},
    {NULL, NULL}
};

void init_input() {
  input_pos = 0;
  memset(input_buffer, 0, sizeof(input_buffer));
}

void input_cleanup() {
  input_pos = 0;
  memset(input_buffer, 0, sizeof(input_buffer));
}

static void cmd_clear(char *args) {
  terminal_clear();
}

static void cmd_unknown(char *input) {
  terminal_execute_command(input);
}

static void process_command(const char *input) {
  char temp[MAX_INPUT_BUFFER_SIZE];
  strncpy(temp, input, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0'; 

  char *command = strtok(temp, " ");
  char *args = strtok(NULL, "");

  if (!command) {
    return;  
  }

  for (Command *cmd = commands; cmd->name != NULL; cmd++) {
    if (strcmp(command, cmd->name) == 0) {
      cmd->func(args);  
      return;
    }
  }

  cmd_unknown((char *)input);
}

void handle_input(XKeyEvent *event) {
  KeySym keysym;
  char buffer[20];
  int count = XLookupString(event, buffer, sizeof(buffer), &keysym, NULL);

  if (count <= 0) {
    return; // No valid input detected
  }

  if (buffer[0] == 12) { // Ctrl+L
    terminal_clear();
    return;
  }

  // Handle Enter key (Execute the command)
  if (keysym == XK_Return) {
    terminal_write("\n");
    input_buffer[input_pos] = '\0'; 

    if (input_pos > 0) {
      process_command(input_buffer);
    }

    input_pos = 0;
  }
  else if (keysym == XK_BackSpace) {
    if (input_pos > 0) {
        if (pty_fd >= 0) {
            input_buffer[input_pos] = '\0';
            write(pty_fd, input_buffer, strlen(input_buffer));
            write(pty_fd, "\n", 1);
            terminal_read_output();
        } else {
            process_command(input_buffer);  
        }
    }
    
  } else {
    if (input_pos < MAX_INPUT_BUFFER_SIZE - 1) {
      input_buffer[input_pos++] = buffer[0];
      char text[2] = {buffer[0], '\0'};
      terminal_write(text);
    } else {
      fprintf(stderr, "Input buffer overflow! Ignoring further input.\n");
    }
  }
}

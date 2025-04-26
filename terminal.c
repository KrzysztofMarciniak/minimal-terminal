#include "terminal.h"
#include "ansi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char **buffer = NULL;
static int cursor_row = 0;
static int cursor_col = 0;
static int term_rows = 0;
static int term_cols = 0;
static char *prompt = NULL;

void init_terminal(int rows, int cols) {
  term_rows = rows;
  term_cols = cols;

  buffer = (char **)malloc(term_rows * sizeof(char *));
  for (int i = 0; i < term_rows; i++) {
    buffer[i] = (char *)malloc(term_cols + 1);
    memset(buffer[i], ' ', term_cols);
    buffer[i][term_cols] = '\0';
  }
  cursor_row = 0;
  cursor_col = 0;

  const char *ps1 = getenv("PS1");
  if (ps1) {
    terminal_set_prompt(ps1);
  } else {
    terminal_set_prompt("$ ");
  }

  terminal_write(prompt);
}

void terminal_set_prompt(const char *new_prompt) {
  if (prompt) {
    free(prompt);
  }
  if (new_prompt) {
    prompt = malloc(strlen(new_prompt) + 1);
    if (prompt) {
      strcpy(prompt, new_prompt);
    }
  } else {
    prompt = malloc(1);
    if (prompt) {
      prompt[0] = '\0';
    }
  }
}

const char *terminal_get_prompt() { return prompt; }

const char **get_terminal_buffer() { return (const char **)buffer; }

int get_terminal_rows() { return term_rows; }

int get_terminal_cols() { return term_cols; }
void resize_terminal(int new_rows, int new_cols) {
  if (new_rows == term_rows && new_cols == term_cols)
    return;

  char **new_buffer = (char **)malloc(new_rows * sizeof(char *));
  for (int i = 0; i < new_rows; i++) {
    new_buffer[i] = (char *)malloc(new_cols + 1);
    memset(new_buffer[i], ' ', new_cols);
    new_buffer[i][new_cols] = '\0';
  }

  int min_rows = (new_rows < term_rows) ? new_rows : term_rows;
  int min_cols = (new_cols < term_cols) ? new_cols : term_cols;

  for (int i = 0; i < min_rows; i++) {
    memcpy(new_buffer[i], buffer[i], min_cols);
  }

  for (int i = 0; i < term_rows; i++) {
    free(buffer[i]);
  }
  free(buffer);

  buffer = new_buffer;
  term_rows = new_rows;
  term_cols = new_cols;

  if (cursor_row >= term_rows)
    cursor_row = term_rows - 1;
  if (cursor_col >= term_cols)
    cursor_col = term_cols - 1;
}

void terminal_write(const char *text) {
  int i = 0;
  while (text[i] != '\0') {
    if (text[i] == '\033') {
      char ansi_seq[32] = {0};
      int j = 0;
      int fg_color = 0, bg_color = 0, attr = 0;

      i++;
      while (text[i] != '\0' && j < 31 && !(text[i] >= 'A' && text[i] <= 'Z') &&
             !(text[i] >= 'a' && text[i] <= 'z')) {
        ansi_seq[j++] = text[i++];
      }

      if (text[i] != '\0' && j < 31) {
        ansi_seq[j++] = text[i++];
        ansi_seq[j] = '\0';
        parse_ansi(ansi_seq, &fg_color, &bg_color, &attr);
      }
    } else if (text[i] == '\n') {
      cursor_row++;
      cursor_col = 0;

      if (cursor_row >= term_rows) {
        for (int r = 0; r < term_rows - 1; r++) {
          memcpy(buffer[r], buffer[r + 1], term_cols);
        }
        memset(buffer[term_rows - 1], ' ', term_cols);
        cursor_row = term_rows - 1;
      }
      i++;
    } else if (text[i] == '\r') {
      cursor_col = 0;
      i++;
    } else {
      if (cursor_col < term_cols) {
        buffer[cursor_row][cursor_col] = text[i];
        cursor_col++;
      }
      i++;

      if (cursor_col >= term_cols) {
        cursor_row++;
        cursor_col = 0;

        if (cursor_row >= term_rows) {
          for (int r = 0; r < term_rows - 1; r++) {
            memcpy(buffer[r], buffer[r + 1], term_cols);
          }
          memset(buffer[term_rows - 1], ' ', term_cols);
          cursor_row = term_rows - 1;
        }
      }
    }
  }
}

void terminal_execute_command(const char *cmd) {
  FILE *fp;
  char buffer[1024];
  char command_buffer[1024];

  const char *shell = getenv("SHELL");
  if (!shell) {
    shell = "/bin/sh";
  }

  snprintf(command_buffer, sizeof(command_buffer), "%s -c \"%s\"", shell, cmd);

  fp = popen(command_buffer, "r");
  if (fp == NULL) {
    terminal_write("Error executing command\n");
    return;
  }

  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    terminal_write(buffer);
  }

  pclose(fp);
  terminal_write(prompt);
}

void terminal_move_cursor(int row, int col) {
  if (row >= 0 && row < term_rows && col >= 0 && col < term_cols) {
    cursor_row = row;
    cursor_col = col;
  }
}

int get_cursor_row() { return cursor_row; }

int get_cursor_col() { return cursor_col; }

void terminal_cleanup() {
  if (buffer) {
    for (int i = 0; i < term_rows; i++) {
      free(buffer[i]);
    }
    free(buffer);
    buffer = NULL;
  }

  if (prompt) {
    free(prompt);
    prompt = NULL;
  }
}

#include "terminal.h"
#include "ansi.h"
#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char **buffer = NULL;
static int cursor_row = 0;
static int cursor_col = 0;
static int term_rows = 0;
static int term_cols = 0;
static int current_fg = ANSI_COLOR_WHITE;
static int current_bg = ANSI_COLOR_BLACK;
static int current_attr = 0;

/**
 * Initializes the terminal buffer with specified dimensions.
 *
 * Allocates memory for the terminal buffer, sets each row to blank spaces,
 * resets the cursor position, and writes an initial prompt.
 *
 * @param rows Number of rows in the terminal buffer
 * @param cols Number of columns in the terminal buffer
 */
void init_terminal(int rows, int cols) {
  term_rows = rows;
  term_cols = cols;

  buffer = (char **)malloc(rows * sizeof(char *));
  for (int i = 0; i < rows; i++) {
    buffer[i] = (char *)malloc(cols + 1);
    memset(buffer[i], ' ', cols);
    buffer[i][cols] = '\0';
  }

  cursor_row = 0;
  cursor_col = 0;

  terminal_write("$ ");
}

/**
 * Retrieves the current terminal buffer as a constant array of strings.
 *
 * @return A pointer to the terminal buffer, which contains the current terminal content
 */
const char **get_terminal_buffer() { return (const char **)buffer; }

/**
 * Retrieves the number of rows in the terminal buffer.
 *
 * @return The total number of rows in the terminal buffer
 */
int get_terminal_rows() { return term_rows; }

int get_terminal_cols() { return term_cols; }

void terminal_write(const char *text) {
  int i = 0;
  while (text[i] != '\0') {
    if (text[i] == '\033') {
      char ansi_seq[32] = {0};
      int j = 0;

      i++;
      while (text[i] != '\0' && j < 31 && !(text[i] >= 'A' && text[i] <= 'Z') &&
             !(text[i] >= 'a' && text[i] <= 'z')) {
        ansi_seq[j++] = text[i++];
      }

      if (text[i] != '\0' && j < 31) {
        ansi_seq[j++] = text[i++];
        ansi_seq[j] = '\0';
        parse_ansi(ansi_seq, &current_fg, &current_bg, &current_attr);
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
  render_screen();
}

void terminal_execute_command(const char *cmd) {
  FILE *fp;
  char buffer[1024];
  fp = popen(cmd, "r");
  if (fp == NULL) {
    terminal_write("Error executing command\n");
    return;
  }
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    terminal_write(buffer);
  }
  pclose(fp);
  terminal_write("$ ");
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
}

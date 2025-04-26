#ifndef TERMINAL_H
#define TERMINAL_H

// Terminal dimensions
#define TERM_ROWS 24
#define TERM_COLS 80

void init_terminal(int rows, int cols);
const char** get_terminal_buffer();
int get_terminal_rows();
int get_terminal_cols();
void terminal_write(const char* text);
void terminal_move_cursor(int row, int col);
int get_cursor_row();
int get_cursor_col();
void terminal_execute_command(const char* cmd);
void terminal_cleanup();
void terminal_set_prompt(const char* prompt);
const char* terminal_get_prompt();
void resize_terminal(int new_rows, int new_cols);
void terminal_clear();

#endif // TERMINAL_H

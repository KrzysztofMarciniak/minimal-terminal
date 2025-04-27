#include "terminal.h"
#include <signal.h>
#include <sys/wait.h>

static int shell_pid = -1;
static int pty_fd = -1;
static char **buffer = NULL;
static int cursor_row = 0;
static int cursor_col = 0;
static int term_rows = 0;
static int term_cols = 0;
static char *prompt = NULL;

static void write_prompt(void) {
  if (prompt) {
    terminal_write(prompt);
  }
}
static void sigchld_handler(int signo) {
    int status;
    pid_t pid = waitpid(shell_pid, &status, WNOHANG);
    if (pid > 0) {
        terminal_cleanup();
        _exit(0);
    }
}

void init_terminal(int rows, int cols) {
  term_rows = rows;
  term_cols = cols;

  buffer = calloc(term_rows, sizeof(char *));
  if (!buffer) {
    perror("calloc buffer");
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < term_rows; ++i) {
    buffer[i] = calloc(term_cols + 1, sizeof(char));
    if (!buffer[i]) {
      perror("calloc buffer row");
      exit(EXIT_FAILURE);
    }
    memset(buffer[i], ' ', term_cols);
    buffer[i][term_cols] = '\0';
  }
  cursor_row = cursor_col = 0;

  const char *ps1 = getenv("PS1");
  terminal_set_prompt(ps1 ? ps1 : "$ ");
  write_prompt();
}

void terminal_set_prompt(const char *new_prompt) {
  free(prompt);
  if (new_prompt) {
    prompt = malloc(strlen(new_prompt) + 1);
    if (!prompt) {
      perror("malloc prompt");
      exit(EXIT_FAILURE);
    }
    strcpy(prompt, new_prompt);
  } else {
    prompt = malloc(1);
    if (!prompt) {
      perror("malloc empty prompt");
      exit(EXIT_FAILURE);
    }
    prompt[0] = '\0';
  }
}

const char *terminal_get_prompt(void) { return prompt; }

const char **get_terminal_buffer(void) { return (const char **)buffer; }

int get_terminal_rows(void) { return term_rows; }

int get_terminal_cols(void) { return term_cols; }

void resize_terminal(int new_rows, int new_cols) {
  if (new_rows == term_rows && new_cols == term_cols)
    return;

  char **new_buf = calloc(new_rows, sizeof(char *));
  if (!new_buf) {
    perror("calloc new_buf");
    return;
  }
  for (int i = 0; i < new_rows; ++i) {
    new_buf[i] = calloc(new_cols + 1, sizeof(char));
    if (!new_buf[i]) {
      perror("calloc new_buf row");
      for (int j = 0; j < i; ++j)
        free(new_buf[j]);
      free(new_buf);
      return;
    }
    memset(new_buf[i], ' ', new_cols);
    new_buf[i][new_cols] = '\0';
  }

  int min_rows = new_rows < term_rows ? new_rows : term_rows;
  int min_cols = new_cols < term_cols ? new_cols : term_cols;
  for (int i = 0; i < min_rows; ++i) {
    memcpy(new_buf[i], buffer[i], min_cols);
  }

  for (int i = 0; i < term_rows; ++i)
    free(buffer[i]);
  free(buffer);
  buffer = new_buf;
  term_rows = new_rows;
  term_cols = new_cols;
  if (cursor_row >= term_rows)
    cursor_row = term_rows - 1;
  if (cursor_col >= term_cols)
    cursor_col = term_cols - 1;

  if (pty_fd != -1) {
    struct winsize ws = {.ws_row = term_rows,
                         .ws_col = term_cols,
                         .ws_xpixel = 0,
                         .ws_ypixel = 0};
    ioctl(pty_fd, TIOCSWINSZ, &ws);
  }
}

void terminal_clear(void) {
  for (int i = 0; i < term_rows; ++i) {
    memset(buffer[i], ' ', term_cols);
  }
  cursor_row = cursor_col = 0;
  write_prompt();
}

void terminal_write(const char *text) {
  for (size_t i = 0; text[i]; ++i) {
    char c = text[i];
    if (c == '\033') {
      char seq[32] = {0};
      size_t j = 0;
      seq[j++] = c;
      while (text[i + 1] && j < sizeof(seq) - 1) {
        seq[j++] = text[++i];
        if ((seq[j - 1] >= 'A' && seq[j - 1] <= 'Z') ||
            (seq[j - 1] >= 'a' && seq[j - 1] <= 'z')) {
          break;
        }
      }
      seq[j] = '\0';
      // parse_ansi
    } else if (c == '\n') {
      cursor_row++;
      cursor_col = 0;
      if (cursor_row >= term_rows) {
        for (int r = 0; r < term_rows - 1; ++r)
          memcpy(buffer[r], buffer[r + 1], term_cols);
        memset(buffer[term_rows - 1], ' ', term_cols);
        cursor_row = term_rows - 1;
      }
    } else if (c == '\r') {
      cursor_col = 0;
    } else if (c == '\b' || c == 127) {
      if (cursor_col > 0) {
        cursor_col--;
        buffer[cursor_row][cursor_col] = ' ';
      }
    } else {
      if (cursor_col >= term_cols) {
        cursor_row++;
        cursor_col = 0;
      }
      if (cursor_row >= term_rows) {
        for (int r = 0; r < term_rows - 1; ++r)
          memcpy(buffer[r], buffer[r + 1], term_cols);
        memset(buffer[term_rows - 1], ' ', term_cols);
        cursor_row = term_rows - 1;
      }
      buffer[cursor_row][cursor_col++] = c;
    }
  }
}

void terminal_read_output(void) {
  if (pty_fd < 0)
    return;
  char buf[1024];
  struct pollfd pfd = {.fd = pty_fd, .events = POLLIN};
  while (poll(&pfd, 1, 100) > 0) {
    if (pfd.revents & POLLIN) {
      ssize_t n = read(pty_fd, buf, sizeof(buf) - 1);
      if (n > 0) {
        buf[n] = '\0';
        terminal_write(buf);
      } else if (n == 0) {
        close(pty_fd);
        pty_fd = -1;
        shell_pid = -1;
        terminal_write("\nShell terminated\n");
        terminal_cleanup();
        exit(0);
      } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("read shell");
        close(pty_fd);
        pty_fd = -1;
        shell_pid = -1;
        terminal_write("\nError reading shell\n");
        write_prompt();
        return;
      } else {
        break;
      }
    } else {
      break;
    }
  }
}

void terminal_start_shell(void) {
    if (pty_fd != -1) {
      terminal_write("Shell already running\n");
      write_prompt();
      return;
    }
    struct winsize ws = {
        .ws_row = term_rows, .ws_col = term_cols, .ws_xpixel = 0, .ws_ypixel = 0};
    shell_pid = forkpty(&pty_fd, NULL, NULL, &ws);
    if (shell_pid < 0) {
      perror("forkpty");
      terminal_write("Failed to start shell: ");
      terminal_write(strerror(errno));
      terminal_write("\n");
      write_prompt();
      return;
    }
    if (shell_pid == 0) {
      const char *shell = getenv("SHELL");
      if (!shell)
        shell = "/bin/sh";
      setenv("TERM", "xterm-256color", 1);
      execlp(shell, shell, "-i", NULL);
      perror("execlp");
      _exit(EXIT_FAILURE);
    }
  
    signal(SIGCHLD, sigchld_handler);
  
    int flags = fcntl(pty_fd, F_GETFL);
    fcntl(pty_fd, F_SETFL, flags | O_NONBLOCK);
    usleep(100000);
    terminal_read_output();
    write_prompt();
  }
  

void terminal_execute_command(const char *cmd) {
  if (pty_fd < 0) {
    terminal_write("Shell not started\n");
    write_prompt();
    return;
  }
  write(pty_fd, cmd, strlen(cmd));
  write(pty_fd, "\n", 1);
  terminal_read_output();
  write_prompt();
}

void terminal_move_cursor(int row, int col) {
  if (row >= 0 && row < term_rows)
    cursor_row = row;
  if (col >= 0 && col < term_cols)
    cursor_col = col;
}

int get_cursor_row(void) { return cursor_row; }
int get_cursor_col(void) { return cursor_col; }

void terminal_cleanup(void) {
  if (shell_pid > 0) {
    kill(shell_pid, SIGTERM);
    waitpid(shell_pid, NULL, 0);
  }
  if (pty_fd >= 0)
    close(pty_fd);
  for (int i = 0; i < term_rows; ++i)
    free(buffer[i]);
  free(buffer);
  free(prompt);
  buffer = NULL;
  prompt = NULL;
  pty_fd = -1;
  shell_pid = -1;
}
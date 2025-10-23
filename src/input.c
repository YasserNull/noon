// input.c
// This file manages the program's global input state (NoonInput), including
// command-line options. It also provides a portable `getline` implementation
// that supports advanced REPL features like history and raw terminal mode.

#include "input.h"
#include "config.h"
#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Windows headers for console input, Unix uses termios */
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// Global pointer to the input state.
NoonInput *ni = NULL;

/* initialize the global input state */
void init_input(void) {
  ni = safe_malloc(sizeof(NoonInput));
  // Input
  ni->program_name = "";
  ni->is_repl = 0;
  ni->file = NULL;
  ni->input = "<stdin>";
  // Options
  ni->debug = 0;
  ni->dump_tokens = 0;
  ni->dump_ast = 0;
  ni->check_syntax = 0;
  // Repl history
  ni->history = (History){NULL, 0};

  debug_func("");
}

/* protect repl columns for prompt text:
   - ">>> " for normal input
   - "... " for multiline input */
enum { PROTECT_COLS = 4 };

/* forward declarations for raw input mode setup */
static void enable_raw(void);
static void disable_raw(void);

/* forward declarations for history file operations */
static void history_load(void);
/* history */

/* add user input to REPL history */
static void history_add(const char *s) {
  // debug_func("s: %s",s);
  // The check for ni->is_repl is removed because this function is only
  // called from within the REPL-specific part of portable_getline.
  if (!s || *s == '\0')
    return;

  // check string length and validity
  size_t len = strnlen(s, 1024);
  if (len == 0 || len >= 1024)
    return;

  // duplicate string and store it
  char *dup = safe_strdup(s);
  char **tmp = safe_realloc(ni->history.items, (ni->history.count + 1) * sizeof(char *));
  ni->history.items = tmp;
  ni->history.items[ni->history.count++] = dup;
}

/* load REPL history from a file */
static void history_load(void) {
  const char *home = getenv("HOME");
  if (!home) {
    home = getenv("USERPROFILE"); // For Windows
  }
  if (!home) {
    return; // Cannot find home directory
  }

  char path[1024];
  snprintf(path, sizeof(path), "%s/.noon_history", home);

  FILE *fp = fopen(path, "r");
  if (!fp) {
    return; // File doesn't exist or cannot be opened, which is fine
  }

  char line_buf[2048]; // Max line length for history
  while (fgets(line_buf, sizeof(line_buf), fp)) {
    // Remove trailing newline character(s)
    line_buf[strcspn(line_buf, "\r\n")] = 0;
    history_add(line_buf);
  }

  fclose(fp);
}

/* get previous history entry */
static const char *history_up(size_t *idx) {
  debug_func("idx: %zu", *idx);
  if (!ni->is_repl)
    return ""; // skip if not in REPL mode
  if (ni->history.count == 0)
    return NULL;
  if (*idx < ni->history.count)
    (*idx)++;
  const char *result = ni->history.items[ni->history.count - *idx];
  return result ? result : "";
}

/* get next history entry */
static const char *history_down(size_t *idx) {
  // debug_func("idx: %zu",*idx);
  if (!ni->is_repl)
    return ""; // skip if not in REPL mode
  if (*idx == 0)
    return "";
  (*idx)--;
  if (*idx == 0)
    return "";
  const char *result = ni->history.items[ni->history.count - *idx];
  return result ? result : "";
}

/* platform raw + key read */
#ifdef _WIN32
static DWORD win_orig_mode;
static HANDLE win_hin = NULL;
static int win_raw = 0;

/* disable raw input mode on Windows */
static void disable_raw(void) {
  // debug_func("");
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  if (win_raw && win_hin) {
    SetConsoleMode(win_hin, win_orig_mode);
    win_raw = 0;
  }
}

/* enable raw input mode on Windows */
static void enable_raw(void) {
  // debug_func("");
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  if (win_raw)
    return;
  win_hin = GetStdHandle(STD_INPUT_HANDLE);
  if (GetConsoleMode(win_hin, &win_orig_mode)) {
    DWORD m = win_orig_mode;
    m &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(win_hin, m);
    atexit(disable_raw);
    win_raw = 1;
  }
}

/* read single key on Windows (supports arrows and control keys) */
static int read_key(void) {
  // debug_func("");
  if (!ni->is_repl)
    return 0; // skip if not in REPL mode
  INPUT_RECORD rec;
  DWORD read;
  while (1) {
    if (!ReadConsoleInput(win_hin, &rec, 1, &read))
      continue;
    if (rec.EventType != KEY_EVENT)
      continue;
    KEY_EVENT_RECORD k = rec.Event.KeyEvent;
    if (!k.bKeyDown)
      continue;
    switch (k.wVirtualKeyCode) {
    case VK_LEFT: return 'L';
    case VK_RIGHT: return 'R';
    case VK_UP: return 'U';
    case VK_DOWN: return 'D';
    case VK_HOME: return 'H';
    case VK_END: return 'E';
    case VK_RETURN: return '\n';
    case VK_BACK: return 127;
    case VK_DELETE: return 127;
    default:
      if (k.uChar.AsciiChar) {
        unsigned char ch = (unsigned char)k.uChar.AsciiChar;
        if (ch == 4)
          return 4; // Ctrl+D
        return (int)ch;
      }
    }
  }
}
#else
static struct termios orig_term;
static int unix_raw = 0;

/* disable raw input mode on Unix */
static void disable_raw(void) {
  // debug_func("");
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  if (unix_raw) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
    unix_raw = 0;
  }
}

/* enable raw input mode on Unix */
static void enable_raw(void) {
  // debug_func("");
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  if (unix_raw)
    return;
  if (tcgetattr(STDIN_FILENO, &orig_term) == 0) {
    struct termios raw = orig_term;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    atexit(disable_raw);
    unix_raw = 1;
  }
}

/* read a single key with escape decoding for arrows */
static int read_key(void) {
  // debug_func("");
  if (!ni->is_repl)
    return 0; // skip if not in REPL mode
  int c = getchar();
  if (c == 4)
    return 4;      /* Ctrl-D */
  if (c == 0x1b) { /* escape */
    int c2 = getchar();
    if (c2 == '[') {
      int c3 = getchar();
      if (c3 >= '0' && c3 <= '9') {
        int c4 = getchar(); /* expect ~ */
        if (c4 == '~') {
          switch (c3) {
          case '1':
          case '7': return 'H';
          case '4':
          case '8': return 'E';
          case '3': return 127;
          default: return 0;
          }
        }
        return 0;
      } else {
        switch (c3) {
        case 'A': return 'U';
        case 'B': return 'D';
        case 'C': return 'R';
        case 'D': return 'L';
        case 'H': return 'H';
        case 'F': return 'E';
        default: return 0;
        }
      }
    } else if (c2 == 'O') {
      int c3 = getchar();
      if (c3 == 'H')
        return 'H';
      if (c3 == 'F')
        return 'E';
      return 0;
    }
    return 0;
  }
  return c;
}
#endif

/* cursor movement and line redraw functions */

/* move cursor left n times */
static void move_left_stdout(size_t count) {
  // debug_func("count: %zu", count);
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  while (count--)
    putchar('\b');
}

/* redraw user line after modification */
static void render_replace(const char *buf, size_t len, size_t cursor, size_t *displayed_len, size_t *displayed_cursor) {
  // debug_func("buf: %s, len: %zu, cursor: %zu, displayed_len: %zu, displayed_cursor: %zu", buf, len, cursor, *displayed_len, *displayed_cursor);
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  if (*displayed_cursor > 0)
    move_left_stdout(*displayed_cursor);
  if (len > 0)
    fwrite(buf, 1, len, stdout);
  if (*displayed_len > len) {
    size_t extra = *displayed_len - len;
    for (size_t i = 0; i < extra; i++)
      putchar(' ');
    for (size_t i = 0; i < extra; i++)
      putchar('\b');
  }
  if (len > cursor) {
    size_t back = len - cursor;
    for (size_t i = 0; i < back; i++)
      putchar('\b');
  }
  fflush(stdout);
  *displayed_len = len;
  *displayed_cursor = cursor;
}

/* print new char and update display */
static void render_insert_char(char ch, const char *tail, size_t tail_len, size_t insert_pos, size_t *displayed_len, size_t *displayed_cursor) {
  // debug_func("ch: %c, tail: %s, tail_len: %zu, insert_pos: %zu, displayed_len: %zu, displayed_cursor: %zu", ch, tail , tail_len, insert_pos, *displayed_len, *displayed_cursor);
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  (void)insert_pos;
  putchar(ch);
  if (tail_len > 0)
    fwrite(tail, 1, tail_len, stdout);
  for (size_t i = 0; i < tail_len; i++)
    putchar('\b');
  fflush(stdout);
  (*displayed_len)++;
  (*displayed_cursor)++;
}

/* delete char before cursor and update display */
static void render_delete_before(const char *tail, size_t tail_len, size_t *displayed_len, size_t *displayed_cursor) {
  // debug_func("tail: %s, tail_len: %zu, displayed_len: %zu, displayed_cursor: %zu", tail , tail_len, *displayed_len, *displayed_cursor);
  if (!ni->is_repl)
    return; // skip if not in REPL mode
  putchar('\b');
  if (tail_len > 0)
    fwrite(tail, 1, tail_len, stdout);
  putchar(' ');
  for (size_t i = 0; i < tail_len + 1; i++)
    putchar('\b');
  fflush(stdout);
  if (*displayed_len > 0)
    (*displayed_len)--;
  if (*displayed_cursor > 0)
    (*displayed_cursor)--;
}

/* our custom getline version that works on all systems and supports REPL
 * history */
ssize_t portable_getline(char **lineptr, size_t *n, FILE *stream) {
  debug_func("lineptr: %p, n: %zu, stream: %p\n", (void *)lineptr, *n, (void *)stream);
  if (!lineptr || !n || !stream)
    return -1;

  /* normal file input */
  if (!ni->is_repl || stream != stdin) {
    char *buf = *lineptr;
    size_t cap = *n ? *n : 128;
    int allocated_here = 0;
    if (!buf) {
      buf = safe_malloc(cap);
      allocated_here = 1;
    }
    size_t len = 0;
    int c;
    while ((c = fgetc(stream)) != EOF) {
      if (len + 2 > cap) {
        size_t newcap = cap * 2;
        if (newcap < len + 2)
          newcap = len + 2;
        buf = safe_realloc(buf, newcap);
        cap = newcap;
      }
      buf[len++] = (char)c;
      if (c == '\n')
        break;
    }
    if (len == 0 && feof(stream)) {
      if (allocated_here) {
        free(buf);
        *lineptr = NULL;
        *n = 0;
      }
      return -1;
    }
    buf[len] = '\0';
    *lineptr = buf;
    *n = cap;
    return (ssize_t)len;
  }

  /* REPL mode with editing and history */
  static bool history_initialized = false;
  if (!history_initialized) {
    history_load();
    history_initialized = true;
  }

  enable_raw();
  fflush(stdout);

  size_t cap = (*n && *lineptr) ? *n : 64;
  int allocated_here = 0;
  char *buf = *lineptr ? *lineptr : safe_malloc(cap), *tmp;
  if (!*lineptr)
    allocated_here = 1;

  if (allocated_here)
    buf[0] = '\0';

  size_t len = 0;
  size_t cursor = 0;
  size_t hist_idx = 0;
  const char *hist_line = NULL;
  size_t displayed_len = 0;
  size_t displayed_cursor = 0;

  while (1) {
    int k = read_key();
    if (k == 4) { /* Ctrl-D -> EOF */
      disable_raw();
      if (allocated_here)
        free(buf);
      return -1;
    }
    if (k == '\n') {
      putchar('\n');
      fflush(stdout);
      if (len > 0)
        history_add(buf);
      hist_idx = 0;
      buf[len] = '\0';
      *lineptr = buf;
      *n = cap;
      disable_raw();
      return (ssize_t)len;
    }

    if (k == 127) { /* backspace */
      if (cursor > 0) {
        memmove(buf + cursor - 1, buf + cursor, len - cursor + 1);
        render_delete_before(buf + cursor - 1, len - cursor, &displayed_len, &displayed_cursor);
        cursor--;
        len--;
        buf[len] = '\0';
      }
      continue;
    }
    if (k == 'L') { /* left */
      if (cursor > 0) {
        if (cursor > 0) {
          putchar('\b');
          fflush(stdout);
          cursor--;
          if (displayed_cursor > 0)
            displayed_cursor--;
        }
      }
      continue;
    }
    if (k == 'R') { /* right */
      if (cursor < len) {
        putchar(buf[cursor]);
        cursor++;
        if (displayed_cursor < displayed_len)
          displayed_cursor++;
        fflush(stdout);
      }
      continue;
    }
    if (k == 'H') { /* Home key */
      if (displayed_cursor > 0)
        move_left_stdout(displayed_cursor);
      cursor = 0;
      displayed_cursor = 0;
      continue;
    }
    if (k == 'E') { /* End key */
      size_t toprint = len - cursor;
      if (toprint > 0) {
        fwrite(buf + cursor, 1, toprint, stdout);
        fflush(stdout);
      }
      cursor = len;
      displayed_cursor = displayed_len = len;
      continue;
    }
    if (k == 'U') { /* history up */
      if (ni->history.count == 0)
        continue;
      hist_line = history_up(&hist_idx);
      if (!hist_line)
        continue;
      size_t hlen = strlen(hist_line);
      if (cap < hlen + 1) {
        tmp = safe_realloc(buf, hlen + 1);
        buf = tmp;
        cap = hlen + 1;
        if (allocated_here == 0)
          allocated_here = 1;
      }
      memcpy(buf, hist_line, hlen + 1);
      len = hlen;
      cursor = len;
      buf[len] = '\0';
      render_replace(buf, len, cursor, &displayed_len, &displayed_cursor);
      continue;
    }
    if (k == 'D') { /* history down */
      hist_line = history_down(&hist_idx);
      size_t hlen = strlen(hist_line);
      if (cap < hlen + 1) {
        tmp = safe_realloc(buf, hlen + 1);
        buf = tmp;
        cap = hlen + 1;
        if (allocated_here == 0)
          allocated_here = 1;
      }
      memcpy(buf, hist_line, hlen + 1);
      len = hlen;
      cursor = len;
      buf[len] = '\0';
      render_replace(buf, len, cursor, &displayed_len, &displayed_cursor);
      continue;
    }
    if (k >= 32 && k <= 126) { /* printable */
      if (len + 2 > cap) {
        size_t newcap = cap * 2;
        if (newcap < len + 2)
          newcap = len + 2;
        tmp = safe_realloc(buf, newcap);
        buf = tmp;
        cap = newcap;
        if (allocated_here == 0)
          allocated_here = 1;
      }
      memmove(buf + cursor + 1, buf + cursor, len - cursor + 1);
      buf[cursor] = (char)k;
      cursor++;
      len++;
      buf[len] = '\0';
      render_insert_char((char)k, buf + cursor + 1, len - cursor, cursor, &displayed_len, &displayed_cursor);
      continue;
    }
  }

  return -1;
}

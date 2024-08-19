#include <pl_readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int pl_readline_add_history(_THIS, char *line) {
  list_prepend(this->history, strdup(line));
  return PL_READLINE_SUCCESS;
}

pl_readline_t pl_readline_init(int (*pl_readline_hal_getch)(),
                               void (*pl_readline_hal_putch)(int ch)) {
  pl_readline_t plreadln = malloc(sizeof(struct pl_readline));
  if (!plreadln)
    return NULL;
  // 设置回调函数
  plreadln->pl_readline_hal_getch = pl_readline_hal_getch;
  plreadln->pl_readline_hal_putch = pl_readline_hal_putch;
  // 设置history链表
  plreadln->history = NULL;
  return plreadln;
}
void pl_readline_uninit(_THIS) {
  list_free_with(this->history, free);
  free(this);
}
static void pl_readline_insert_char(char *str, char ch, int idx) {
  memmove(str + idx + 1, str + idx, strlen(str) - idx);
  str[idx] = ch;
}
static void pl_readline_delete_char(char *str, int idx) {
  int len = strlen(str);
  memmove(str + idx, str + idx + 1, len - idx);
  str[len] = '\0';
}

void pl_readline_print(_THIS, char *str) {
  while (*str) {
    this->pl_readline_hal_putch(*str++);
  }
}
int pl_readline(_THIS, char *buffer, size_t len) {
  int p = 0;
  while (true) {
    // if(p >= len) continue;
    int ch = this->pl_readline_hal_getch();
    switch (ch) {
    case PL_READLINE_KEY_DOWN:
      // printf("\e[B");
      break;
    case PL_READLINE_KEY_UP:
      // printf("\e[A");
      break;
    case PL_READLINE_KEY_LEFT:
      pl_readline_print(this, "\e[D");
      break;
    case PL_READLINE_KEY_RIGHT:
      pl_readline_print(this, "\e[C");
      break;
    case PL_READLINE_KEY_ENTER:
      this->pl_readline_hal_putch('\n');
      return PL_READLINE_SUCCESS;
    default:
      this->pl_readline_hal_putch(ch);
      break;
    }
  }
  return PL_READLINE_SUCCESS;
}

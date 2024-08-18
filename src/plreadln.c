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
int pl_readline(_THIS, char *buffer, size_t len) {
  int p = 0;
  while (true) {
    int ch = this->pl_readline_hal_getch();
    switch (ch) {
    case PL_READLINE_KEY_DOWN:
      printf("\e[B");
      break;
    case PL_READLINE_KEY_UP:
      printf("\e[A");
      break;
    case PL_READLINE_KEY_LEFT:
      printf("\e[D");
      break;
    case PL_READLINE_KEY_RIGHT:
      printf("\e[C");
      break;
    default:
      printf("%c",ch);
      break;
    }
  }
  return PL_READLINE_SUCCESS;
}

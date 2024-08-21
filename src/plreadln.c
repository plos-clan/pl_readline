#include <assert.h>
#include <pl_readline.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int pl_readline_add_history(_THIS, char *line) {
  if (strlen(line))
    list_prepend(this->history, strdup(line));
  return PL_READLINE_SUCCESS;
}

pl_readline_t pl_readline_init(int (*pl_readline_hal_getch)(),
                               void (*pl_readline_hal_putch)(int ch),
                               void (*pl_readline_hal_flush)(),
                               pl_readline_words_t words) {
  pl_readline_t plreadln = malloc(sizeof(struct pl_readline));
  if (!plreadln)
    return NULL;
  // 设置回调函数
  plreadln->pl_readline_hal_getch = pl_readline_hal_getch;
  plreadln->pl_readline_hal_putch = pl_readline_hal_putch;
  plreadln->pl_readline_hal_flush = pl_readline_hal_flush;
  // 设置history链表
  plreadln->history = NULL;
  // 设置words
  plreadln->words = words;
  return plreadln;
}
void pl_readline_uninit(_THIS) {
  list_free_with(this->history, free);
  free(this);
}
static void pl_readline_insert_char(char *str, char ch, int idx) {
  int len = strlen(str);
  memmove(str + idx + 1, str + idx, len - idx);
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
static void pl_readline_reset(_THIS, int p, int len) {
  char buf[255] = {0};
  if (p) {
    sprintf(buf, "\e[%dD", p);
    pl_readline_print(this, buf);
  }
  if (len) {
    for (int i = 0; i < len; i++) {
      this->pl_readline_hal_putch(' ');
    }
    sprintf(buf, "\e[%dD", len);
    pl_readline_print(this, buf);
  }
}
static void pl_readline_to_the_end(_THIS, int n) {
  char buf[255] = {0};
  sprintf(buf, "\e[%dC", n);
  pl_readline_print(this, buf);
}
// 处理向上向下键
static void pl_readline_handle_key_down_up(_THIS, int *history_idx,
                                           char *buffer, int *p, int *length,
                                           size_t len, int n) {
  list_t node = list_nth(this->history, *history_idx); // 获取历史记录
  if (!node) {
    (*history_idx) += n; // 超出历史记录的范围，回退到上一个记录
    return;
  }
  pl_readline_reset(this, *p, *length); // 重置光标和输入的字符
  this->pl_readline_hal_flush(); // 刷新输出缓冲区，在Linux下需要,否则会导致输入不显示
  *p = 0;                 // 光标移动到最左边
  *length = 0;            // 清空缓冲区长度
  memset(buffer, 0, len); // 清空缓冲区
  strcpy(buffer, node->data);
  pl_readline_print(this, buffer); // 打印历史记录
  *length = strlen(buffer);        // 更新缓冲区长度
  *p = *length;
}
// 处理输入的字符
static int pl_readline_handle_key(_THIS, int ch, char *buffer, int *p,
                                  int *length, int *history_idx, char *prompt,
                                  size_t len, char *input_buf,
                                  int *input_buf_ptr) {
  if (*length >= len) { // 输入的字符数超过最大长度
    pl_readline_to_the_end(this, *length - *p);
    this->pl_readline_hal_putch('\n');
    buffer[*length] = '\0';
    pl_readline_add_history(this, buffer);
    return PL_READLINE_SUCCESS;
  }
  switch (ch) {
  case PL_READLINE_KEY_DOWN:
    (*history_idx)--;
    pl_readline_handle_key_down_up(this, history_idx, buffer, p, length, len,
                                   1); // n = 1是为了的失败的时候还原
    break;
  case PL_READLINE_KEY_UP: {
    (*history_idx)++;
    pl_readline_handle_key_down_up(this, history_idx, buffer, p, length, len,
                                   -1); // n = -1是为了的失败的时候还原
    break;
  }
  case PL_READLINE_KEY_LEFT:
    if (!*p) // 光标在最左边
      return PL_READLINE_NOT_FINISHED;
    (*p)--;
    pl_readline_print(this, "\e[D");
    break;
  case PL_READLINE_KEY_RIGHT:
    if (*p == *length) // 光标在最右边
      return PL_READLINE_NOT_FINISHED;
    (*p)++;
    pl_readline_print(this, "\e[C");
    break;
  case PL_READLINE_KEY_BACKSPACE:
    if (!*p) // 光标在最左边
      return PL_READLINE_NOT_FINISHED;
    pl_readline_delete_char(buffer, --*p);
    (*length)--;
    int n = *length - *p;
    if (n) {
      char buf[255] = {0};
      sprintf(buf, "\e[%dC\e[D ", n);
      pl_readline_print(this, buf);

      sprintf(buf, "\e[%dD", n);
      pl_readline_print(this, buf);
      pl_readline_print(this, "\e[D");
      pl_readline_print(this, buffer + *p);
      pl_readline_print(this, buf);

    } else {
      pl_readline_print(this, "\e[D \e[D");
    }
    break;
  case PL_READLINE_KEY_ENTER:
    pl_readline_to_the_end(this, *length - *p);
    this->pl_readline_hal_putch('\n');
    buffer[*length] = '\0';
    pl_readline_add_history(this, buffer);
    return PL_READLINE_SUCCESS;
  case PL_READLINE_KEY_TAB: { // 自动补全
    pl_readline_print(this, "\n");
    pl_readline_print(this, prompt);
    pl_readline_print(this, buffer);
    int n = *length - *p;
    char buf[255] = {0};
    if (n) {
      sprintf(buf, "\e[%dD", n);
      pl_readline_print(this, buf);
    }
    this->pl_readline_hal_flush();
    break;
  }
  case ' ': {
  }
  default: {
    pl_readline_insert_char(buffer, ch, (*p)++);
    (*length)++;
    int n = *length - *p;
    if (n) {
      char buf[255] = {0};
      pl_readline_print(this, buffer + *p - 1);
      sprintf(buf, "\e[%dD", n);
      pl_readline_print(this, buf);

    } else {
      this->pl_readline_hal_putch(ch);
    }
    break;
  }
  }
  return PL_READLINE_NOT_FINISHED;
}

// 主体函数
int pl_readline(_THIS, char *prompt, char *buffer, size_t len) {
  int p = 0;
  int length = 0;       // 输入的字符数
  int history_idx = -1; // history的索引
  // 为了实现自动补全，需要将输入的字符保存到缓冲区中
  char *input_buf = malloc(len + 1);
  int input_buf_ptr = 0;
  assert(input_buf);
  // 清空缓冲区
  memset(input_buf, 0, len + 1);
  memset(buffer, 0, len);
  // 打印提示符
  pl_readline_print(this, prompt);
  this->pl_readline_hal_flush(); // 刷新输出缓冲区，在Linux下需要,否则会导致输入不显示
  // 循环读取输入
  while (true) {
    int ch = this->pl_readline_hal_getch(); // 读取输入
    int status =
        pl_readline_handle_key(this, ch, buffer, &p, &length, &history_idx,
                               prompt, len, input_buf, &input_buf_ptr);
    if (status == PL_READLINE_SUCCESS) {
      break;
    }
  }
  free(input_buf);
  return PL_READLINE_SUCCESS;
}

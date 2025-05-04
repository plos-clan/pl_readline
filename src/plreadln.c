//
// This file is part of pl_readline.
// pl_readline is free software: you can redistribute it and/or modify
// it under the terms of MIT license.
// See file LICENSE or https://opensource.org/licenses/MIT for full license
// details.
//
// Copyright (c) 2024 min0911_ https://github.com/min0911Y
//

// plreadln.c: 实现pl_readline的核心功能
#include <assert.h>
#include <pl_readline.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int pl_readline_add_history(_self, char *line) {
  list_prepend(self->history, strdup(line));
  return PL_READLINE_SUCCESS;
}

int pl_readline_modify_history(_self) {
  list_t node = list_nth(self->history, self->history_idx);
  // 当前历史记录肯定不为空，如果为空炸了算了
  free(node->data);
  node->data = strdup(self->buffer);
  return PL_READLINE_SUCCESS;
}

pl_readline_t pl_readline_init(
    int (*pl_readline_hal_getch)(void), int (*pl_readline_hal_putch)(int ch),
    void (*pl_readline_hal_flush)(void),
    void (*pl_readline_get_words)(char *buf, pl_readline_words_t words)) {
  pl_readline_t plreadln = malloc(sizeof(struct pl_readline));
  if (!plreadln)
    return NULL;
  // 设置回调函数
  plreadln->pl_readline_hal_getch = pl_readline_hal_getch;
  plreadln->pl_readline_hal_putch = pl_readline_hal_putch;
  plreadln->pl_readline_hal_flush = pl_readline_hal_flush;
  plreadln->pl_readline_get_words = pl_readline_get_words;
  // 设置history链表
  plreadln->history = NULL;
  plreadln->maxlen = PL_READLINE_DEFAULT_BUFFER_LEN;
  // 设置输入缓冲区
  plreadln->buffer = malloc(plreadln->maxlen);
  plreadln->input_buf = malloc(plreadln->maxlen);
  if (!plreadln->buffer || !plreadln->input_buf) {
    pl_readline_uninit(plreadln);
    return NULL;
  }
  pl_readline_add_history(plreadln, "");
  return plreadln;
}

void pl_readline_uninit(_self) {
  list_free_with(self->history, free);
  free(self->buffer);
  free(self->input_buf);
  free(self);
}

void pl_readline_insert_char(char *str, char ch, int idx) {
  int len = strlen(str) + 1; // 还要复制字符串结束符
  if (len)
    memmove(str + idx + 1, str + idx, len - idx);
  str[idx] = ch;
}

static void pl_readline_delete_char(char *str, int idx) {
  int len = strlen(str);
  if (len)
    memmove(str + idx, str + idx + 1, len - idx);
  str[len] = '\0';
}

void pl_readline_print(_self, char *str) {
  while (*str) {
    self->pl_readline_hal_putch(*str++);
  }
}

static void pl_readline_reset(_self, int p, int len) {
  char buf[255] = {0};
  if (p) {
    sprintf(buf, "\033[%dD", p);
    pl_readline_print(self, buf);
  }
  if (len) {
    for (int i = 0; i < len; i++) {
      self->pl_readline_hal_putch(' ');
    }
    sprintf(buf, "\033[%dD", len);
    pl_readline_print(self, buf);
  }
}

// Helper function to find the color of a command
static int get_command_color(_self, const char *word, int is_first_word) {
    // Initialize a temporary word list to hold commands
    pl_readline_words_t word_list = pl_readline_word_maker_init();

    // Get all defined commands
    self->pl_readline_get_words("", word_list);

    // Check if the word exactly matches any of our defined commands
    for (size_t i = 0; i < word_list->len; i++) {
        // Skip commands that require first position but aren't first
        if (word_list->words[i].first && !is_first_word) {
            continue;
        }

        // Check exact match
        if (strcmp(word_list->words[i].word, word) == 0) {
            int color = word_list->words[i].color;
            pl_readline_word_maker_destroy(word_list);
            return color;
        }
    }

    // Clean up
    pl_readline_word_maker_destroy(word_list);
    return PL_COLOR_RESET; // Default color
}

// Function to redisplay the buffer with colorized commands
static void redisplay_buffer_with_colors(_self, int show_prompt) {
    // Save original cursor position
    size_t original_ptr = self->ptr;
    size_t prompt_len = 0;

    // Calculate prompt length (only if shown)
    if (self->prompt) {
        // Count visible characters in the prompt (ignoring ANSI escape sequences)
        char *p = self->prompt;
        while (*p) {
            if (*p == '\033') {
                // Skip ANSI escape sequence
                while (*p && *p != 'm') p++;
                if (*p) p++; // Skip the 'm'
                continue;
            }
            prompt_len++;
            p++;
        }
    }

    // Move cursor back to start of line using absolute positioning
    pl_readline_print(self, "\r");

    // Display prompt only if requested
    if (show_prompt) {
        pl_readline_print(self, self->prompt);
    } else if (prompt_len > 0) {
        // If not showing prompt but there is a prompt, move cursor forward to skip prompt area
        char forward_buf[32];
        sprintf(forward_buf, "\033[%zuC", prompt_len);
        pl_readline_print(self, forward_buf);
    }

    // Track position considering the prompt if shown
    size_t display_position = prompt_len; // Start after prompt

    // Clear the line after cursor
    pl_readline_print(self, "\033[K");

    // Parse and colorize each word in the buffer
    char *buffer_copy = strdup(self->buffer);
    size_t buffer_len = strlen(buffer_copy);

    // Handle empty buffer
    if (buffer_len == 0) {
        free(buffer_copy);
        return;
    }

    // Get all tokens and preserve spaces
    char *words[256] = {0};
    int word_count = 0;
    char *p = buffer_copy;
    char *word_start = p;

    // Parse words and spaces
    while (*p) {
        if (*p == ' ') {
            // Found a space - terminate the current word
            *p = '\0';
            if (p > word_start) { // Non-empty word
                words[word_count++] = word_start;
            }
            // Mark this as a space position
            words[word_count++] = NULL; // NULL indicates a space
            word_start = p + 1;
        }
        p++;
    }

    // Don't forget the last word if it's not empty
    if (*word_start) {
        words[word_count++] = word_start;
    }

    // Render all words and spaces with correct coloring
    for (int i = 0; i < word_count; i++) {
        if (words[i] == NULL) {
            // This is a space
            pl_readline_print(self, " ");
            display_position++;
        } else {
            // This is a word
            // Check if this is the first word in the line
            int is_first = (i == 0 || (i > 0 && words[i-1] == NULL && i == 1));

            int color = get_command_color(self, words[i], is_first);

            if (color != PL_COLOR_RESET) {
                // Apply color
                char color_str[16];
                sprintf(color_str, "\033[%dm", color);
                pl_readline_print(self, color_str);
            }

            // Print the word
            pl_readline_print(self, words[i]);
            display_position += strlen(words[i]);

            if (color != PL_COLOR_RESET) {
                // Reset color
                pl_readline_print(self, "\033[0m");
            }
        }
    }

    free(buffer_copy);

    // If original buffer ends with space, print it
    if (self->length > 0 && self->buffer[self->length-1] == ' ') {
        pl_readline_print(self, " ");
        display_position++;
    }

    // Calculate cursor position (prompt + original_ptr)
    size_t target_position = prompt_len + original_ptr;

    // Position cursor correctly
    if (display_position != target_position) {
        // Use absolute positioning from the left edge
        pl_readline_print(self, "\r");

        if (target_position > 0) {
            char move_buf[32];
            sprintf(move_buf, "\033[%zuC", target_position);
            pl_readline_print(self, move_buf);
        }
    }
}

static void pl_readline_to_the_end(_self, int n) {
  char buf[255] = {0};
  sprintf(buf, "\033[%dC", n);
  pl_readline_print(self, buf);
}

// 处理向上向下键（移动到第n个历史）
static bool pl_readline_handle_history(_self, int n) {
  list_t node = list_nth(self->history, n); // 获取历史记录
  if (!node)
    return false;
  pl_readline_reset(self, self->ptr, self->length); // 重置光标和输入的字符
  self->pl_readline_hal_flush(); // 刷新输出缓冲区，在Linux下需要,否则会导致输入不显示
  self->ptr = 0;                         // 光标移动到最左边
  self->length = 0;                      // 清空缓冲区长度
  memset(self->buffer, 0, self->maxlen); // 清空缓冲区
  strcpy(self->buffer, node->data);
  pl_readline_print(self, self->buffer); // 打印历史记录
  self->length = strlen(self->buffer);   // 更新缓冲区长度
  self->ptr = self->length;

  memset(self->input_buf, 0, self->maxlen); // 清空输入缓冲区
  self->input_ptr = 0;                      // 输入缓冲区指针置0
  char *p = node->data;
  while (*p) {
    if (*p == ' ') {
      self->input_ptr = 0;
      p++;
      continue;
    }
    self->input_buf[self->input_ptr++] = *p++;
  }
  self->input_buf[self->input_ptr] = '\0';
  self->input_ptr = strlen(self->input_buf); // 更新输入缓冲区指针
  return true;
}

void pl_readline_insert_char_and_view(_self, char ch) {
  if (self->length >= self->maxlen) {
    self->maxlen *= 2;
    self->buffer = realloc(self->buffer, self->maxlen);
    self->input_buf = realloc(self->input_buf, self->maxlen);
    self->buffer[self->length] = '\0';
    if (!self->buffer) return; // 炸了算了
  }
  pl_readline_insert_char(self->buffer, ch, self->ptr++);
  self->length++;

  // Check if we have a complete word that needs coloring (no longer needed - always redraw)
  // Only checking word boundaries for completeness

  // Only check for keyword completion when not pressing space/newline
  if (ch != ' ' && ch != '\n') {
    // Create a temporary buffer for the current word
    char current_word[256] = {0};
    size_t word_start = self->ptr;
    size_t word_end = self->ptr;

    // Find the start of the current word (search backward until space)
    while (word_start > 0 && self->buffer[word_start-1] != ' ') {
      word_start--;
    }

    // Find the end of the current word (search forward until space or null)
    while (word_end < self->length && self->buffer[word_end] != ' ' && self->buffer[word_end] != '\0') {
      word_end++;
    }

    // Copy the current word
    if (word_start < word_end) {
      strncpy(current_word, self->buffer + word_start, word_end - word_start);
      current_word[word_end - word_start] = '\0';

      // Check if this is the first word in the line
      int is_first_word = (word_start == 0);

      // We no longer need to set should_colorize flag since we always redraw
      get_command_color(self, current_word, is_first_word);
    }
  }

  // Use colorized redisplay for all edits
  redisplay_buffer_with_colors(self, 0); // Don't show prompt during edit
}

void pl_readline_next_line(_self) {
  int n = self->length - self->ptr;
  char buf[255] = {0};
  if (!n) {
    pl_readline_print(self, "\n");
    return;
  }
  sprintf(buf, "\033[%dC", n); // 光标移动到最右边
  pl_readline_print(self, buf);
  pl_readline_print(self, "\n");
}

// 处理输入的字符
int pl_readline_handle_key(_self, int ch) {
  if (ch != PL_READLINE_KEY_TAB) {
    self->intellisense_mode = false;
    if (self->intellisense_word) {
      free(self->intellisense_word);
      self->intellisense_word = NULL;
    }
  }
  if (self->length >= self->maxlen) {
    self->maxlen *= 2;
    self->buffer = realloc(self->buffer, self->maxlen);
    self->input_buf = realloc(self->input_buf, self->maxlen);
    self->buffer[self->length] = '\0'; // input_buf在处理时会自动截断 不用我们加
    if (!self->buffer)
      return PL_READLINE_FAILED;
  }
  switch (ch) {
  case PL_READLINE_KEY_DOWN:
    pl_readline_modify_history(self);
    if (pl_readline_handle_history(self, self->history_idx - 1))
      self->history_idx--;
    break;
  case PL_READLINE_KEY_UP: {
    pl_readline_modify_history(self);
    if (pl_readline_handle_history(self, self->history_idx + 1))
      self->history_idx++;
    break;
  }
  case PL_READLINE_KEY_LEFT:
    if (!self->ptr) // 光标在最左边
      return PL_READLINE_NOT_FINISHED;
    self->ptr--;
    if (self->buffer[self->ptr] == ' ') {
      memset(self->input_buf, 0, self->maxlen);
      // 光标移动到前一个空格
      size_t i = self->ptr;
      while (i && self->buffer[i - 1] != ' ') {
        i--;
      }
      self->input_ptr = 0;
      // 从i开始复制到self->input_buf，直到遇到空格
      while (i < self->ptr && self->buffer[i] != ' ') {
        self->input_buf[self->input_ptr++] = self->buffer[i];
        i++;
      }
      // 字符串结束符号
      self->input_buf[self->input_ptr] = '\0';
    } else {
      self->input_ptr--;
    }

    // Redraw the line with updated coloring
    redisplay_buffer_with_colors(self, 0);
    break;
  case PL_READLINE_KEY_RIGHT:
    if (self->ptr == self->length) // 光标在最右边
      return PL_READLINE_NOT_FINISHED;
    self->ptr++;
    if (self->buffer[self->ptr - 1] == ' ') {
      memset(self->input_buf, 0, self->maxlen);
      // 光标移动到前一个空格
      size_t i = self->ptr;
      size_t j = i;
      while (i < self->length && self->buffer[i + 1] != ' ') {
        i++;
      }
      self->input_ptr = 0;
      // 从i开始复制到self->input_buf，直到遇到空格
      while (j <= i && self->buffer[i] != ' ') {
        self->input_buf[self->input_ptr++] = self->buffer[j];
        j++;
      }
      // 字符串结束符号
      self->input_buf[self->input_ptr] = '\0';
      self->input_ptr = 0;
    } else {
      self->input_ptr++;
    }

    // Redraw the line with updated coloring
    redisplay_buffer_with_colors(self, 0);
    break;
  case PL_READLINE_KEY_BACKSPACE:
    if (!self->ptr) // 光标在最左边
      return PL_READLINE_NOT_FINISHED;
    --self->ptr;
    if (self->buffer[self->ptr] == ' ') {
      memset(self->input_buf, 0, self->maxlen);
      // 光标移动到前一个空格
      size_t i = self->ptr;
      while (i && self->buffer[i - 1] != ' ') {
        i--;
      }
      self->input_ptr = 0;
      // 从i开始复制到self->input_buf，直到遇到空格
      while (i < self->ptr && self->buffer[i] != ' ') {
        self->input_buf[self->input_ptr++] = self->buffer[i];
        i++;
      }
      // 字符串结束符号
      self->input_buf[self->input_ptr] = '\0';
    } else {
      if (self->input_ptr)
        pl_readline_delete_char(self->input_buf, --self->input_ptr);
    }
    pl_readline_delete_char(self->buffer, self->ptr);

    self->length--;

    // Redraw the entire line with updated colors
    redisplay_buffer_with_colors(self, 0);
    break;
  case PL_READLINE_KEY_ENTER:
    pl_readline_to_the_end(self, self->length - self->ptr);
    self->pl_readline_hal_putch('\n');
    self->buffer[self->length] = '\0';
    self->history_idx = 0;
    pl_readline_modify_history(self);
    if (self->buffer[0] != '\0') {
      pl_readline_add_history(self, "");
    }
    return PL_READLINE_SUCCESS;
  case PL_READLINE_KEY_TAB: { // 自动补全
    pl_readline_words_t words = pl_readline_word_maker_init();
    pl_readline_word word_seletion = pl_readline_intellisense(self, words);
    if (word_seletion.word) {
      pl_readline_intellisense_insert(self, word_seletion);
      // Redisplay with colors after completion but don't show prompt
      redisplay_buffer_with_colors(self, 0);
      self->pl_readline_hal_flush();
    } else if (word_seletion.first) {
      pl_readline_print(self, "\n");
      pl_readline_print(self, self->prompt);
      self->buffer[self->length] = '\0';

      // Use colorized display without showing prompt since we printed it already
      redisplay_buffer_with_colors(self, 0);

      self->pl_readline_hal_flush();
    }
    break;
  }
  case PL_READLINE_KEY_CTRL_A:
  case PL_READLINE_KEY_HOME:
    pl_readline_reset(self, self->ptr, 0);
    self->ptr = 0;
    int i = 0;
    for (i = 0; self->buffer[i] != '\0' && self->buffer[i] != ' '; i++) {
        self->input_buf[i] = self->buffer[i];
    }
    self->input_buf[i] = '\0';
    self->input_ptr = 0;
    break;
  case PL_READLINE_KEY_END: {
    size_t diff = self->length - self->ptr;
    if (diff) {
      pl_readline_to_the_end(self, self->length - self->ptr);
      self->ptr = self->length;
    }
    break;
  }
  case PL_READLINE_KEY_PAGE_UP: {
    size_t len = list_length(self->history);
    pl_readline_modify_history(self);
    pl_readline_handle_history(self, len - 1);
    self->history_idx = len - 1;
    break;
  }
  case PL_READLINE_KEY_PAGE_DOWN:
    pl_readline_modify_history(self);
    pl_readline_handle_history(self, 0);
    self->history_idx = 0;
    break;
  case PL_READLINE_KEY_CTRL_C:
    self->buffer[0] = '\0';
    pl_readline_print(self, "^C\n");
    return PL_READLINE_SUCCESS;
  case ' ': {
    memset(self->input_buf, 0, self->maxlen);
    self->input_ptr = 0;
    goto handle;
  }
  default: {
    pl_readline_insert_char(self->input_buf, ch, self->input_ptr++);
  handle:
    pl_readline_insert_char_and_view(self, ch);
    break;
  }
  }
  return PL_READLINE_NOT_FINISHED;
}

// 主体函数
const char *pl_readline(_self, char *prompt) {
  // 清空运行时状态
  memset(self->buffer, 0, self->maxlen);
  memset(self->input_buf, 0, self->maxlen);
  self->input_ptr = 0;
  self->ptr = 0;
  self->length = 0;
  self->history_idx = 0;
  self->prompt = prompt;
  self->intellisense_mode = false;
  self->intellisense_word = NULL;

  // 打印提示符
  pl_readline_print(self, prompt);
  // 刷新输出缓冲区，在Linux下需要,否则会导致输入不显示
  self->pl_readline_hal_flush();

  // 循环读取输入
  while (true) {
    int ch = self->pl_readline_hal_getch(); // 读取输入
    int status = pl_readline_handle_key(self, ch);
    if (status == PL_READLINE_SUCCESS) {
      break;
    }
  }

  if (self->intellisense_word) {
    free(self->intellisense_word);
  }

  return self->buffer;
}

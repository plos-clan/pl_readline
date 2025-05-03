#pragma once

#include <pl_list.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Color definitions for terminal output
#define PL_COLOR_RESET    0
#define PL_COLOR_BLACK    30
#define PL_COLOR_RED      31
#define PL_COLOR_GREEN    32
#define PL_COLOR_YELLOW   33
#define PL_COLOR_BLUE     34
#define PL_COLOR_MAGENTA  35
#define PL_COLOR_CYAN     36
#define PL_COLOR_WHITE    37

#define PL_READLINE_KEY_UP 0xff00
#define PL_READLINE_KEY_DOWN 0xff01
#define PL_READLINE_KEY_LEFT 0xff02
#define PL_READLINE_KEY_RIGHT 0xff03
#define PL_READLINE_KEY_HOME 0xff04
#define PL_READLINE_KEY_END 0xff05
#define PL_READLINE_KEY_PAGE_UP 0xff06
#define PL_READLINE_KEY_PAGE_DOWN 0xff07
#define PL_READLINE_KEY_ENTER '\n'
#define PL_READLINE_KEY_TAB '\t'
#define PL_READLINE_KEY_CTRL_A 0x01
#define PL_READLINE_KEY_CTRL_C 0x03
#define PL_READLINE_KEY_BACKSPACE '\b'

#define _self pl_readline_t self
#define PL_READLINE_SUCCESS 0
#define PL_READLINE_FAILED -1
#define PL_READLINE_NOT_FINISHED 1
#define PL_READLINE_DEFAULT_BUFFER_LEN 32

typedef struct pl_readline_word {
    char *word; // u8bcdu7ec4
    /**
      u5982u679cfirstu4e3atrueuff0c
      u8fd9u4e2awordu5fc5u987bu5728u7b2cu4e00u4e2au53c2u6570u7684u4f4du7f6eu7684u65f6u5019u624du80fdu5f97u5230u8865u5168
      u5982abc u5219u5fc5u987bu8f93u5165"ab"u7136u540eu6309tabuff0cu624du4f1au6709u53efu80fdu6709"abc"
      u5982u679cu662f"qwe ab"u5219u4e0du4f1au8865u5168"qwe abc"uff0cu9664u975efirstu4e3afalse.
    */
    bool first;
    char sep; // u5206u9694u7b26
    int color; // ANSI color code (e.g., PL_COLOR_GREEN)
} pl_readline_word;

typedef struct pl_readline_words {
    size_t len;              // u8bcdu7ec4u6570u91cf
    size_t max_len;          // u8bcdu7ec4u6700u5927u6570u91cf
    pl_readline_word *words; // u8bcdu7ec4u5217u8868
} *pl_readline_words_t;

typedef struct pl_readline {
    int (*pl_readline_hal_getch)(void);    // u8f93u5165u51fdu6570
    void (*pl_readline_hal_putch)(int ch); // u8f93u51fau51fdu6570
    void (*pl_readline_hal_flush)(void);   // u5237u65b0u51fdu6570
    void (*pl_readline_get_words)(char *buf,
                                  pl_readline_words_t words); // u83b7u53d6u8bcdu7ec4u5217u8868
    char *buffer;         // u8f93u5165u7f13u51b2u533a
    char *input_buf;      // u8f93u5165u7f13u51b2u533auff08u8865u5168u7684u524du7f00uff09
    size_t ptr;           // u8f93u5165u7f13u51b2u533au6307u9488
    size_t input_ptr; // u8f93u5165u7f13u51b2u533auff08u8865u5168u7684u524du7f00uff09u6307u9488
    size_t maxlen;        // u7f13u51b2u533au6700u5927u957fu5ea6
    size_t length;        // u8f93u5165u7f13u51b2u533au957fu5ea6uff08u5df2u7ecfu8f93u5165u7684u5b57u7b26u6570uff09
    list_t history;       // u5386u53f2u8bb0u5f55u5217u8868
    int history_idx;      // u5386u53f2u8bb0u5f55u6307u9488
    char *prompt;         // u63d0u793au7b26
    bool intellisense_mode;  // u667au80fdu8865u5168u6a21u5f0f
    char *intellisense_word; // u667au80fdu8865u5168u8bcdu7ec4
} *pl_readline_t;

pl_readline_words_t pl_readline_word_maker_init(void);
pl_readline_t pl_readline_init(
    int (*pl_readline_hal_getch)(void), void (*pl_readline_hal_putch)(int ch),
    void (*pl_readline_hal_flush)(void),
    void (*pl_readline_get_words)(char *buf, pl_readline_words_t words));
const char *pl_readline(_self, char *prompt);
pl_readline_word pl_readline_intellisense(_self, pl_readline_words_t words);
void pl_readline_insert_char_and_view(_self, char ch);
void pl_readline_insert_char(char *str, char ch, int idx);
int pl_readline_word_maker_add(char *word, pl_readline_words_t words,
                               bool is_first, int color, char sep);
void pl_readline_print(_self, char *str);
void pl_readline_intellisense_insert(_self, pl_readline_word words);
void pl_readline_word_maker_destroy(pl_readline_words_t words);
void pl_readline_next_line(_self);
int pl_readline_handle_key(_self, int ch);
void pl_readline_uninit(_self);
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct pl_readline_word {
    char *word;
    /**
     如果first为true，
     这个word必须在第一个参数的位置的时候才能得到补全
     如abc 则必须输入"ab"然后按tab，才会有可能有"abc"
     如果是“qwe ab”则不会补全"qwe abc"，除非first为false.
    */
    bool first;
} pl_readline_word;


typedef struct pl_readline_words {
    int len;
    pl_readline_word *words;
} *pl_readline_words_t;
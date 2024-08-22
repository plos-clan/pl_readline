#include <pl_readline.h>
#include <stdio.h>
#include <string.h>

static void insert_char(_THIS, char ch, pl_readline_runtime *rt) {

  pl_readline_insert_char_and_view(this, ch, rt);
  pl_readline_insert_char(rt->input_buf, ch, rt->input_buf_ptr++);
}
static void insert_string(_THIS, char *str, pl_readline_runtime *rt) {
  while (*str) {
    insert_char(this, *str++, rt);
  }
}
pl_readline_word pl_readline_intellisense(_THIS, pl_readline_runtime *rt,
                                          pl_readline_words_t words) {
  char *buf = strdup(rt->input_buf);
  buf[rt->input_buf_ptr] = '\0';
  char *the_word = NULL;
  int times = 0;
  for (int i = 0; i < words->len; i++) {
    if (strncmp(buf, words->words[i].word, rt->input_buf_ptr) == 0) {
      if (!rt->intellisense_mode) {
        the_word = words->words[i].word;
        break;
      } else {
        times++;
        if (times > 2) {
          pl_readline_print(this, " ");
          pl_readline_print(this, words->words[i].word);
        } else if (times == 2) {
          pl_readline_print(this, "\n");
          //     pl_readline_print
          // FUCK TODO:
        }
        the_word = words->words[i].word;
      }
    }
  }
  pl_readline_word ret = {0};
  if (rt->intellisense_mode == false) {
    ret.word = the_word;
  }
  free(buf);

  return ret;
}
void pl_readline_intellisense_insert(_THIS, pl_readline_runtime *rt,
                                     pl_readline_word words) {
  //   char *buf = strdup(rt->input_buf);
  //   buf[rt->input_buf_ptr] = '\0';
  //   printf("Intellisense for: %s\n", buf);
  //   insert_string(this, "intellisense", rt);
  //   free(buf);
}
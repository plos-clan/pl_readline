#include "pl_readline.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>

int getch(void) {
    struct termios tm, tm_old;
    int            fd = 0;

    if (tcgetattr(fd, &tm) < 0) return -1;

    tm_old = tm;
    cfmakeraw(&tm);

    if (tcsetattr(fd, TCSANOW, &tm) < 0) return -1;
    int ch = getchar();

    if (tcsetattr(fd, TCSANOW, &tm_old) < 0) return -1;

    if (ch == 0x0d) { return PL_READLINE_KEY_ENTER; }
    if (ch == 0x7f) { return PL_READLINE_KEY_BACKSPACE; }
    if (ch == 0x9) { return PL_READLINE_KEY_TAB; }
    if (ch == 0x1b) {
        ch = getch();
        if (ch == '[') {
            ch = getch();
            switch (ch) {
            case 'A': return PL_READLINE_KEY_UP;
            case 'B': return PL_READLINE_KEY_DOWN;
            case 'C': return PL_READLINE_KEY_RIGHT;
            case 'D': return PL_READLINE_KEY_LEFT;
            case 'H': return PL_READLINE_KEY_HOME;
            case 'F': return PL_READLINE_KEY_END;
            case '5':
                if (getch() == '~') return PL_READLINE_KEY_PAGE_UP;
                break;
            case '6':
                if (getch() == '~') return PL_READLINE_KEY_PAGE_DOWN;
                break;
            default: return -1;
            }
        }
    }
    return ch;
}

void flush(void) {
    fflush(stdout);
}

// 统计`/`的个数
static int count_slash(const char *buf) {
    int count = 0;
    while (*buf) {
        if (*buf == '/') { count++; }
        buf++;
    }
    return count;
}

void handle_tab(char *buf, pl_readline_words_t words) {
    pl_readline_word_maker_add("ls", words, true, PL_COLOR_GREEN, ' ');
    pl_readline_word_maker_add("echo", words, true, PL_COLOR_BLUE, ' ');
    pl_readline_word_maker_add("cat", words, true, PL_COLOR_RED, ' ');
    pl_readline_word_maker_add("ps", words, true, PL_COLOR_CYAN, ' ');
    pl_readline_word_maker_add("exit", words, true, PL_COLOR_BLUE, ' ');
    pl_readline_word_maker_add("history", words, true, PL_COLOR_CYAN, ' ');
    pl_readline_word_maker_add("foo", words, false, PL_COLOR_YELLOW, ' ');
    pl_readline_word_maker_add("bar", words, false, PL_COLOR_MAGENTA, ' ');

    int   current     = words->len;
    char *fake_dirs[] = {"/home", "/usr", "/home/racaos", "/usr/local", NULL};
    // 匹配buf，看看有没有匹配的
    for (int i = 0; fake_dirs[i] != NULL; i++) {
        if (strncmp(buf, fake_dirs[i], strlen(buf)) == 0 && strlen(buf) != strlen(fake_dirs[i])) {
            if (count_slash(fake_dirs[i]) == count_slash(buf)) {
                pl_readline_word_maker_add(fake_dirs[i], words, false, PL_COLOR_YELLOW, '/');
            }
        }
    }
    if (strcmp(buf, "/") == 0) {
        // 只是为了让其着色
        pl_readline_word_maker_add("/", words, false, PL_COLOR_YELLOW, ' ');
    }
    if (words->len - current == 0) {
        for (int i = 0; fake_dirs[i] != NULL; i++) {
            if (strcmp(buf, fake_dirs[i]) == 0) {
                pl_readline_word_maker_add(fake_dirs[i], words, false, PL_COLOR_YELLOW, '/');
                break;
            }
        }
    }
}

int main(void) {
    pl_readline_t pl = pl_readline_init(getch, putchar, flush, handle_tab);
#if PL_ENABLE_HISTORY_FILE
    pl_readline_load_history(pl, ".pl_history");
#endif
    printf("Type 'exit' to quit!\n");
    while (1) {
        const char *buffer = pl_readline(pl, "\033[1;32m[user@localhost]$\033[0m ");

        // Check if it is a valid exit command
        int is_exit = 1;
        if (strncmp(buffer, "exit", 4) == 0) {
            // Check if there are only spaces after "exit"
            const char *p = buffer + 4;
            while (*p != '\0') {
                if (*p != ' ') {
                    is_exit = 0;
                    break;
                }
                p++;
            }
        } else {
            is_exit = 0;
        }
        if (strcmp(buffer, "history") == 0) {
            list_t node = pl->history;
            // Find the last node
            while (node && node->next)
                node = node->next;
            // Write from last to first, skipping the empty last entry
            int i = 1;
            while (node) {
                if (node->data && ((char *)node->data)[0] != '\0')
                    printf("%d: %s\n", i++, (char *)node->data);
                node = node->prev;
            }
            continue;
        }

        if (is_exit) break;
        printf("Your input: %s\n", buffer);
    }
#if PL_ENABLE_HISTORY_FILE
    pl_readline_save_history(pl, ".pl_history");
#endif
    pl_readline_uninit(pl);
}

#include <pl_readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>

int getch(void) {
    struct termios tm, tm_old;
    int fd = 0;

    if (tcgetattr(fd, &tm) < 0)
        return -1;

    tm_old = tm;
    cfmakeraw(&tm);

    if (tcsetattr(fd, TCSANOW, &tm) < 0)
        return -1;

    int ch = getchar();

    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
        return -1;

    switch (ch) {
    case 0x0d:
        return PL_READLINE_KEY_ENTER;
    case 0x7f:
        return PL_READLINE_KEY_BACKSPACE;
    case 0x09:
        return PL_READLINE_KEY_TAB;
    case 0x1b:
        ch = getchar();
        if (ch == '[') {
            ch = getchar();
            switch (ch) {
            case 'A':
                return PL_READLINE_KEY_UP;
            case 'B':
                return PL_READLINE_KEY_DOWN;
            case 'C':
                return PL_READLINE_KEY_RIGHT;
            case 'D':
                return PL_READLINE_KEY_LEFT;
            case 'H':
                return PL_READLINE_KEY_HOME;
            case 'F':
                return PL_READLINE_KEY_END;
            case '5':
                if (getchar() == '~')
                    return PL_READLINE_KEY_PAGE_UP;
                break;
            case '6':
                if (getchar() == '~')
                    return PL_READLINE_KEY_PAGE_DOWN;
                break;
            }
        }
        break;
    default:
        return ch;
    }
    return -1;
}

void flush() { fflush(stdout); }

void handle_tab(char *buf, pl_readline_words_t words) {
    pl_readline_word_maker_add("hello", words, true, ' ');
    pl_readline_word_maker_add("world", words, false, ' ');
    pl_readline_word_maker_add("foo", words, false, ' ');
    pl_readline_word_maker_add("bar", words, false, ' ');
    pl_readline_word_maker_add("baz", words, false, ' ');
    pl_readline_word_maker_add("qux", words, false, ' ');
    // pl_readline_word_maker_add("helloworld", words, false);
}

int main() {
    pl_readline_t n =
        pl_readline_init(getch, (void *)putchar, flush, handle_tab);
    char *buffer = malloc(255);
    while (1) {
        pl_readline(n, "input: ", buffer, 255);
        printf("you input: %s\n", buffer);
        if (strcmp(buffer, "exit") == 0)
            break;
    }
    pl_readline_uninit(n);
    free(buffer);
}
# pl_readline

一个简单的键盘输入库，计划支持上下左右方向键，tab 补全

## Example

**NOTE**: This example is for Linux only.

```c
#include <pl_readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>

int getch(void) {
    struct termios tm, tm_old;
    int fd = 0, ch;

    if (tcgetattr(fd, &tm) < 0) return -1; // 保存现在的终端设置

    tm_old = tm;
    cfmakeraw(&tm); // 更改终端设置为原始模式，让所有输入数据以字节为单位处理

    if (tcsetattr(fd, TCSANOW, &tm) < 0) { // 设置上更改之后的设置
        return -1;
    }

    ch = getchar();
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0) { // 更改设置为最初的样子
        return -1;
    }
    if (ch == 0x0d) {
        return PL_READLINE_KEY_ENTER;
    }
    if (ch == 0x7f) {
        return PL_READLINE_KEY_BACKSPACE;
    }
    if (ch == 0x9) {
        return PL_READLINE_KEY_TAB;
    }
    if (ch == 0x1b) {
        ch = getch();
        if (ch == '[') {
            ch = getch();
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
                ch = getch();
                if (ch == '~') {
                    return PL_READLINE_KEY_PAGE_UP;
                }
                return -1;
            case '6':
                ch = getch();
                if (ch == '~') {
                    return PL_READLINE_KEY_PAGE_DOWN;
                }
                return -1;
            }
        }
    }
    return ch;
}

void flush() { fflush(stdout); }

void handle_tab(char *buf, pl_readline_words_t words) {
    pl_readline_word_maker_add("hello", words, true, ' ');
    pl_readline_word_maker_add("world", words, false, ' ');
    pl_readline_word_maker_add("foo", words, false, ' ');
    pl_readline_word_maker_add("bar", words, false, ' ');
    pl_readline_word_maker_add("baz", words, false, ' ');
    pl_readline_word_maker_add("qux", words, false, ' ');
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
```

## Feature
- [x] 支持上下左右方向键
- [x] 支持 tab 补全

## Why to write pl_readline

因为我不想依赖于系统的 readline 库，而是自己实现一个简单的键盘输入库。在写一个裸机程序时，用这个库可以节省很多时间。当然，你也可以用这个库来为你的操作系统实现 shell，因为这个库是以 MIT 协议发布的。

## How to port to other system

### Basic support

终端需要支持 vt100 控制字符，能输出字符和读取输入字符，输出字符和输入字符需要没有缓冲，你可以在 getch 中刷新缓冲。

### Extended support

实现 Plant OS 的 vt100 扩展功能：`\x1b[C`向右到顶时会自动换行、`\x1b[D`向左到底时会自动换行
这样可以支持多行

### General explanation

如果你的终端没有 vt100 支持，可以搭配[os-terminal](https://github.com/plos-clan/libos-terminal)使用，效果也很不错。

## Contribution

本库支持的功能还不完整，欢迎 PR。

如果有任何 bug，请在 issues 中提出。

**不接受新 feature 的 issue，但接受 PR**

但如果是下面的问题，将不会答复：

- linux 上无法多行。

发送 issue 你可能需要知道的：遇到 bug 请讲明白复现步骤，否则很难帮你解决。

## Additional

**HAVE FUN! 祝你玩的开心！**

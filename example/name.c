#include <pl_readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

int getch(void) {
  struct termios tm, tm_old;
  int fd = 0, ch;

  if (tcgetattr(fd, &tm) < 0) { //保存现在的终端设置
    return -1;
  }

  tm_old = tm;
  cfmakeraw(
      &tm); //更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
  if (tcsetattr(fd, TCSANOW, &tm) < 0) { //设置上更改之后的设置
    return -1;
  }

  ch = getchar();
  if (tcsetattr(fd, TCSANOW, &tm_old) < 0) { //更改设置为最初的样子
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
    if (ch == 0x5b) {
      ch = getch();
      switch (ch) {
      case 0x41:
        return PL_READLINE_KEY_UP;
      case 0x42:
        return PL_READLINE_KEY_DOWN;
      case 0x43:
        return PL_READLINE_KEY_RIGHT;
      case 0x44:
        return PL_READLINE_KEY_LEFT;
      default:
        return -1;
      }
    }
  }
  return ch;
}
int main() {
  pl_readline_t n = pl_readline_init(getch, (void *)putchar);
  printf("Enter your name: ");
  fflush(stdout);
  char *buffer = malloc(255);
  pl_readline(n, buffer, 255);
  printf("Hello, %s!\n", buffer);
}
#include <stdio.h>
#include <signal.h>
#include <string.h>

// extern const char *const sys_siglist[];

int main(int argc, char const *argv[])
{
  // 这个的可移植性不好
  // printf("SIGNINT name is %s\n", sys_siglist[SIGINT]);
  // psignal 类似 perror，
  psignal(SIGINT, "Caught Singal");    // 是输出到 stderror 的，可以用 ./sntn 2>a.txt 尝试
  char *a = (SIGINT, "Caught Singal"); // 是输出到 stderror 的，可以用 ./sntn 2>a.txt 尝试
  // https://www.gnu.org/software/libc/manual/html_node/Signal-Messages.html
  // strsignal 不是线程安全的, psignal 是线程安全的，但两者都不能 signal handler 中可以重入
  printf("SIGTERM name is %s\n", strsignal(SIGTERM));
  printf("comma operator a is %s\n", a);
  return 0;
}

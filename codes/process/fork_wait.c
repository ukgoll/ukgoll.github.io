#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/

/**
 * 编译器在 Macos/FreeBSD 上优化的足够好， 1/0不会触发
 * 下面是在 Linux Ubuntu 运行的结果
 * 测试正常退出---------
parent process is 171750
child process(id: 171751) normal exit test
process invoke 171750 wait_result process id is 171751
process exited normally, and status is 64
测试 abort 退出---------
parent process is 171750
child process(id: 171752) abort exit test
process invoke 171750 wait_result process id is 171752
process accept signal and signal num is 6, signal name is Aborted:(core file generated)
测试 divide zero 退出---------
parent process is 171750
child process(id: 171754) divide zero test
调用 1 / 0
process invoke 171750 wait_result process id is 171754
process accept signal and signal num is 8, signal name is Floating point exception:(core file generated)
 *
 */

void test_fws(char *cemsg, void (*func)(void), int want_exit_status);

void fork_wait_status(int status);

void test_divide_zero()
{
  volatile int a = 1;
  volatile int b = 100;
  printf("调用 %d / %d\n", a, b - 100);
  int c = a / (b - 100); // 这里除 0
  printf("不会执行到这里: %d\n", c);
}

int main(int argc, char const *argv[])
{
  printf("测试正常退出---------\n");
  test_fws("normal exit", NULL, 0b00000000000000001000000001000000);
  printf("测试 abort 退出---------\n");
  test_fws("abort exit", abort, -1);
  printf("测试 divide zero 退出---------\n");
  test_fws("divide zero", test_divide_zero, -1);
}

void fork_wait_status(int status)
{
  // 这些宏是互斥的，用 else if
  if (WIFEXITED(status))
  {
    printf("process exited normally, and status is %d\n", WEXITSTATUS(status));
  }
  else if (WIFSIGNALED(status))
  {
    int signal_num = WTERMSIG(status);
    printf("process accept signal and signal num is %d, signal name is %s:%s\n", signal_num, strsignal(signal_num),
#ifdef WCOREDUMP
           WCOREDUMP(status) ? "(core file generated)" : "");
#else
           "");
#endif
  }
  else if (WIFSTOPPED(status))
  {
    printf("process stop with signal: %d\n", WSTOPSIG(status));
  }
  else if (WIFCONTINUED(status))
  {
    printf("process continue\n");
  }
}

void test_fws(char *cemsg, void (*func)(void), int want_exit_status)
{
  pid_t pid;
  int exit_status, wait_result;
  printf("parent process is %d\n", getpid());
  if ((pid = fork()) < 0)
  {
    printf("fork error: %s\n", strerror(errno));
  }
  else if (pid == 0)
  {
    printf("child process(id: %d) %s test\n", getpid(), cemsg);
    if (want_exit_status != -1)
    {
      exit(want_exit_status);
    }
    else
    {
      if (func != NULL)
      {
        func();
      }
      exit(0x11);
    }
  }
  if ((wait_result = wait(&exit_status)) == -1)
  {
    printf("parent process(pid: %d) wait error %s\n", getpid(), strerror(errno));
    // exit(EXIT_FAILURE);
  }
  printf("process invoke %d wait_result process id is %d\n", getpid(), wait_result);
  fork_wait_status(exit_status);
}

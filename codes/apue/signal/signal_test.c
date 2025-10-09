#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void u1(int signal_num);

int main(int argc, char const *argv[])
{
  int flag = 0;
  printf("[NSIG]: system signal max value: %d\n", NSIG);
  sigset_t c_sigset_t;
  if (sigprocmask(0, NULL, &c_sigset_t) == -1)
  {
    printf("invoke sigprocmask error: %s\n", strerror(errno));
    flag = 1;
  }
  printf("sigprocmask is %u\n", c_sigset_t);
  if (signal(SIGINT, SIG_IGN) == SIG_ERR)
  {
    printf("SIGINT ignore error\n");
    flag = 1;
  }
  // if (signal(SIGKILL, SIG_IGN) == SIG_ERR)
  // {
  //   printf("SIGKILL ignore error: %s\n", strerror(errno));
  //   flag = 1;
  // }
  // if (signal(SIGSTOP, SIG_IGN) == SIG_ERR)
  // {
  //   printf("SIGSTOP ignore error: %s\n", strerror(errno));
  //   flag = 1;
  // }
  if (signal(SIGQUIT, SIG_IGN) == SIG_ERR)
  {
    printf("SIGQUIT ignore error\n");
    flag = 1;
  }
  if (signal(SIGINFO, u1) == SIG_ERR)
  {
    printf("SIGINFO register error\n");
    flag = 1;
  }
  if (flag)
    return -1;
  printf("process id is %d\n", getpid());
  if (signal(SIGUSR1, u1) == SIG_ERR)
  {
    printf("SIGUSR1 register error\n");
    return -1;
  }
  if (signal(SIGUSR2, u1) == SIG_ERR)
  {
    printf("SIGUSR2 register error\n");
    return -1;
  }
  for (;;)
  {
    errno = 0;
    pause();
    printf("error is %s\n", strerror(errno));
  }
  return 0;
}

void u1(int signal_num)
{
  if (signal_num == SIGUSR1)
  {
    printf("SIGUSR1 received\n");
    // raise(SIGUSR2);
  }
  else if (signal_num == SIGUSR2)
  {
    printf("SIGUSR2 received\n");
    // raise(SIGUSR1);
  }
  else if (signal_num == SIGINFO)
  {
    printf("SIGINFO received\n");
  }
}

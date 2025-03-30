#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void u1(int signal_num);

int main(int argc, char const *argv[])
{
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
    pause();
  return 0;
}

void u1(int signal_num)
{
  if (signal_num == SIGUSR1)
  {
    printf("SIGUSR1 received\n");
  }
  else if (signal_num == SIGUSR2)
  {
    printf("SIGUSR2 received\n");
  }
}

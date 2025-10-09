#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void u1(int sig_num);

int main(int argc, char const *argv[])
{
  if (signal(SIGUSR1, u1) == SIG_ERR)
  {
    printf("register SIGUSR1 error\n");
    return -1;
  }
  printf("sleep test, process id is %d\n", getpid());
  unsigned int st = sleep(100);
  printf("st is %u\n", st);
  return 0;
}

void u1(int sig_num)
{
  printf("signal number is %d\n", sig_num);
  char *sig_name = strsignal(sig_num);
  printf("%d name is %s\n", sig_num, sig_name);
  psignal(sig_num, "Caught Singal"); // to stderror
}

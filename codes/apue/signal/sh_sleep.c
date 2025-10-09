#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

void signal_handler(int sig_num)
{
  // 這裡只是作為測試使用
  // printf 不是異步安全的。
  printf("sig num is %d\n", sig_num);
  // sleep 也不是異步安全的。  https://www.gnu.org/software/libc/manual/html_node/Sleeping.html
  sleep(10);
}

int main(int argc, char const *argv[])
{
  printf("process id is %d\n", getpid());
  printf("stdou buf size is %u", __fbufsize(stdout));
  struct sigaction ot;
  ot.sa_flags = 0;
  sigemptyset(&ot.sa_mask);
  ot.__sigaction_u.__sa_handler = signal_handler;
  if (sigaction(SIGUSR1, &ot, NULL) < 0)
  {
    printf("Register SIGUSR1 error\n");
  }
  int n;
  char buf[4] = {0};
  while (1)
  {
    n = read(STDIN_FILENO, buf, 3);
    if (n == -1)
    {
      if (EINTR == errno)
      {
        continue;
      }
      break;
    }
    else if (n == 0)
    {
      printf("read eof");
      break;
    }
    printf("read form stdin: [%d] byte, data is %s\n", n, buf);
  }
  return 0;
}

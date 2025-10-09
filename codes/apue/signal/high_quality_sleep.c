#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

void handle_signal(int sig_num);

int main(int argc, char const *argv[])
{
  printf("my process id is %d\n", getpid());
  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sa.sa_flags = 0; // 默认行为
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1)
  {
    printf("register SIGINT error: %s\n", strerror(errno));
    return -1;
  }
  struct timespec ss = {.tv_sec = 15}, rms = {0};
  printf("nano sleep start\n");
  ss.tv_nsec = 1000000000 - 1;
  // 在这里要判断 nanosleep 是否成功，如果参数不对，会失败，比如 tv_nsec 大于 1 秒，也就是 1000*1000*1000
  if (nanosleep(&ss, &rms) == -1)
  {
    printf("nano sleep error is %s\n", strerror(errno));
    if (EINVAL == errno)
    {
      printf("传递给 nanosleep 的参数不对\n");
    }
  }
  printf("nano sleep end, remained sec is %ld, remain nano sec is %ld\n", rms.tv_sec, rms.tv_nsec);
  return 0;
}

void handle_signal(int sig_num)
{
  if (sig_num == SIGINT)
  {
    printf("SIGINT is received and handled\n");
  }
}

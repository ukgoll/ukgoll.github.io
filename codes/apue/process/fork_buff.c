#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int global_val = 99;

int main(int argc, char const *argv[])
{
  pid_t pid;
  if (write(STDOUT_FILENO, "hello, world\n", 13) == -1)
  {
    printf("write to stdout error");
    exit(EXIT_FAILURE);
  }
  int val = 88;
  printf("try buff for fork\n");
  // 在这里强行 fflush 之后，没有数据在缓冲区，fork 之后的子进程没有了 buf 重定向到文件也看不到了(重点是重定向之后的到了文件 stdout 变成全缓冲了!!!!)
  if (argc == 2 && strcmp(argv[1], "flush") == 0)
  {
    if (fflush(stdout) != 0)
    {
      printf("flush stdout error %s", strerror(errno));
    }
  }
  if ((pid = fork()) < 0)
  {
    printf("fork error");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    global_val++;
    ++val;
  }
  else
  {
    sleep(2);
  }
  printf("process id is %ld, and parent id is %ld, global val is %d, val is %d\n", getpid(), getppid(), global_val, val);
  return 0;
}

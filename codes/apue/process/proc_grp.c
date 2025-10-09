#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include "../utils.h"

typedef enum
{
  P1,
  P2,
  P3
} ProcessEnum;
int main(int argc, char const *argv[])
{
  ProcessEnum pe = P1;
  if (argc != 2)
  {
    printf("usage: ./pgc <proc1 | proc2 | proc3>");
    return -1;
  }
  if (strncmp(argv[1], "proc1", 5) == 0)
  {
    printf("P1\n");
    pe = P1;
  }
  else if (strncmp(argv[1], "proc2", 5) == 0)
  {
    printf("P2\n");
    pe = P2;
  }
  else if (strncmp(argv[1], "proc3", 5) == 0)
  {
    printf("P3\n");
    pe = P3;
  }
  else
  {
    printf("Invalid argument: ./pgc <proc1 | proc2 | proc3>\n");
  }
  if (access("./prc_fifo", F_OK) == -1)
  {
    if (mkfifo("./prc_fifo", (S_IRWXU & ~S_IXUSR)) == -1)
    {
      eexit("mkfifo errro");
    }
  }
  int fd;
  if ((fd = open("./prc_fifo", O_RDWR)) == -1)
  {
    eexit("open prc_fifo error");
  }
  char read_buf[256] = {0};
  char write_buf[256] = {0};
  int n;
  if (pe == P1)
  {
    // proc1 读取 fifo，向 stdout 输出，忽略 fifo 的输入
    while ((n = read(fd, read_buf, 255)) > 0)
    {
      snprintf(write_buf, sizeof(write_buf), "[%s]: process ID is %d, process group id is %d, parent id is %d, session id is %d\n", argv[1], getpid(), getpgrp(), getppid(), getsid(0));
      write(STDOUT_FILENO, write_buf, strlen(write_buf));
      sleep(2);
    }
  }
  else if (pe == P2)
  {
    struct iovec iov[2];
    while ((n = read(STDIN_FILENO, read_buf, 255)) > 0)
    {
      iov[0].iov_base = read_buf;
      iov[0].iov_len = strlen(read_buf);
      snprintf(write_buf, sizeof(write_buf), "[%s]: process ID is %d, process group id is %d, parent id is %d, session id is %d\n", argv[1], getpid(), getpgrp(), getppid(), getsid(0));
      iov[1].iov_base = write_buf;
      iov[1].iov_len = strlen(write_buf);
      if (writev(STDOUT_FILENO, iov, 2) == -1)
      {
        write(STDOUT_FILENO, "proc2 error", 10);
      }
    }
  }
  else if (pe == P3)
  {
    // proc3 向 fifo 输入数据，start 开始
    write(fd, "start", 6);
    while ((n = read(STDIN_FILENO, read_buf, 255)) > 0)
    {
      printf("%s", read_buf);
      printf("[%s]: process ID is %d, process group id is %d, parent id is %d, session id is %d\n-------\n", argv[1], getpid(), getpgrp(), getppid(), getsid(0));
      write(fd, "continue", 8);
    }
  }
  return 0;
}

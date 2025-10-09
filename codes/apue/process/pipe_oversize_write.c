#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define WRITE_SIZE 8192
#define READ_SIZE 1024

void write_pattern(int fd, char c)
{
  char buf[WRITE_SIZE];
  memset(buf, c, WRITE_SIZE);
  write(fd, buf, WRITE_SIZE);
}

int main()
{
  int pipefd[2];
  if (pipe(pipefd) == -1)
  {
    perror("pipe");
    exit(1);
  }

  pid_t pid1 = fork();
  if (pid1 == 0)
  {
    // 子进程1写 'A'
    close(pipefd[0]); // 关闭读端
    write_pattern(pipefd[1], 'A');
    close(pipefd[1]);
    exit(0);
  }

  pid_t pid2 = fork();
  if (pid2 == 0)
  {
    // 子进程2写 'B'
    close(pipefd[0]); // 关闭读端
    write_pattern(pipefd[1], 'B');
    close(pipefd[1]);
    exit(0);
  }

  // 父进程：关闭写端，只读
  close(pipefd[1]);

  // 读取管道数据并写到 stdout
  char buf[READ_SIZE];
  ssize_t n;
  while ((n = read(pipefd[0], buf, READ_SIZE)) > 0)
  {
    write(STDOUT_FILENO, buf, n);
  }

  close(pipefd[0]);

  // 等两个子进程结束
  wait(NULL);
  wait(NULL);

  return 0;
}

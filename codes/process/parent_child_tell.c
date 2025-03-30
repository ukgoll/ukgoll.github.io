#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

void parent_first(pid_t pid, int fd[2]);
void child_wait(int fd[2]);

void child_first(int fd[2]);
void parent_wait(int fd[2], pid_t pid);

int main(int argc, char const *argv[])
{

  if (argc != 2)
  {
    printf("usage ./pct <pf | cf>\n pf mean parent first, cf mean child first\n");
    exit(EXIT_FAILURE);
  }
  int pf = 0;
  if (strncmp(argv[1], "pf", 2) == 0)
  {
    pf = 1;
  }
  else if (strncmp(argv[1], "cf", 2) == 0)
  {
    pf = 0;
  }
  else
  {
    printf("Invalid argument\n");
  }
  int fd[2];
  if (pipe(fd) == -1)
  {
    printf("create pipe error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  pid_t pid;
  if ((pid = fork()) < 0)
  {
    printf("fork error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    if (pf)
      child_wait(fd);
    else
      child_first(fd);
    exit(0);
  }
  if (pf)
    parent_first(pid, fd);
  else
    parent_wait(fd, pid);
  return 0;
}

void parent_first(pid_t pid, int fd[2])
{
  close(fd[0]);
  if (write(fd[1], &(pid), sizeof(pid)) != sizeof(pid))
  {
    printf("write fd to child error: %s\n", strerror(errno));
  }
  printf("parent process exec first, send pid %d to child\n", pid);
  close(fd[1]);
}

void child_wait(int fd[2])
{
  int newPid;
  close(fd[1]);
  if (read(fd[0], &newPid, sizeof(newPid)) != sizeof(newPid))
  {
    printf("read fd from parent error: %s", strerror(errno));
  }
  if (getpid() == newPid)
  {
    printf("child process exec second, read from parent %d, %ld\n", newPid, sizeof(newPid));
  }
  else
  {
    printf("Invalid fd read data\n");
  }
  close(fd[0]);
}

void parent_wait(int fd[2], pid_t pid)
{
  int newPid;
  close(fd[1]);
  if (read(fd[0], &newPid, sizeof(newPid)) != sizeof(newPid))
  {
    printf("read fd from child error: %s", strerror(errno));
  }
  if (pid == newPid)
  {
    printf("parent process exec second, read from child  %d, %ld\n", newPid, sizeof(newPid));
  }
  else
  {
    printf("Invalid fd read data\n");
  }
  close(fd[0]);
}

void child_first(int fd[2])
{
  close(fd[0]);
  int my_pid = getpid();
  if (write(fd[1], &(my_pid), sizeof(my_pid)) != sizeof(my_pid))
  {
    printf("write fd to parent error: %s\n", strerror(errno));
  }
  printf("child process exec first, send pid %d to parent\n", my_pid);
  close(fd[1]);
}

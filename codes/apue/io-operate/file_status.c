#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
  if (argc != 2)
  {
    printf("usage ./fstatus <file>");
    exit(EXIT_FAILURE);
  }
  int val;
  if ((val = fcntl(atoi(argv[1]), F_GETFL, 0)) < 0)
  {
    perror("error when open file descriptor");
    exit(EXIT_FAILURE);
  }
  printf("val is %d\n", val);
  switch (val & O_ACCMODE)
  {
  case O_RDONLY:
    printf("read only\n");
    break;
  case O_WRONLY:
    printf("write only\n");
    break;
  case O_SEARCH:
    printf("search only\n");
    break;
  case O_EXEC:
    printf("execute only\n");
    break;
  case O_RDWR:
    printf("read and write\n");
    break;
  default:
    break;
  }
  if (val & O_APPEND)
  {
    printf("can append\n");
  }
  if (val & O_NONBLOCK)
  {
    printf("No blocking\n");
  }
  if (val & O_SYNC)
  {
    printf("sync \n");
  }

  return 0;
}

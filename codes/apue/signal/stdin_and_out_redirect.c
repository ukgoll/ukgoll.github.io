#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
  struct stat std_input_stat;
  if (stat("/dev/fd/0", &std_input_stat) == -1)
  {
    printf("get standrad input fd stat error\n");
    exit(EXIT_FAILURE);
  }
  struct stat std_output_stat;
  if (stat("/dev/fd/1", &std_output_stat) == -1)
  {
    printf("get standrad output fd stat error\n");
    exit(EXIT_FAILURE);
  }
  printf("standrad input fd file inode is %llu, standrad output fd file inode is %llu\n", std_input_stat.st_ino, std_output_stat.st_ino);
  fflush(stdout);
  struct stat input_stat;
  if (fstat(STDIN_FILENO, &input_stat) == -1)
  {
    printf("get input fd stat error\n");
    exit(EXIT_FAILURE);
  }
  struct stat output_stat;
  if (fstat(STDOUT_FILENO, &output_stat) == -1)
  {
    printf("get out fd stat error\n");
    exit(EXIT_FAILURE);
  }
  printf("now input fd file inode is %llu, now output fd file inode is %llu\n", input_stat.st_ino, output_stat.st_ino);
  char buf[256];
  if (read(STDIN_FILENO, buf, 255) <= 0)
  {
    printf("read error");
  }
  else
  {
    printf("read data is %s\n", buf);
  }
  return 0;
}

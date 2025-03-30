#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

void copy_file_read_write(const char *f1, const char *f2);
void copy_file_splice(const char *f1, const char *f2);

int main(int argc, char const *argv[])
{
  if (argc != 4)
  {
    printf("usage ./spc <file1> <file2> <rw | sp>\n");
    exit(EXIT_FAILURE);
  }
  if (strcmp(argv[3], "rw") == 0)
  {
    copy_file_read_write(argv[1], argv[2]);
  }
  else if (strcmp(argv[3], "sp") == 0)
  {
    copy_file_splice(argv[1], argv[2]);
  }
  return 0;
}

void copy_file_read_write(const char *f1, const char *f2)
{
  int fd1 = open(f1, O_RDONLY);
  printf("read and write usage\n");
  if (fd1 == -1)
  {
    printf("errno when open %s, error: %s", f1, strerror(errno));
    exit(EXIT_FAILURE);
  }
  int fd2 = open(f2, O_WRONLY | O_CREAT | O_EXCL, 0664);
  if (fd2 == -1)
  {
    printf("errno when open %s, error: %s", f2, strerror(errno));
    exit(EXIT_FAILURE);
  }
  int n;
  char buf[256];
  while ((n = read(fd1, buf, 256)) > 0)
  {
    write(fd2, buf, n);
  }
  if (n == -1)
  {
    printf("copy file %s to file %s error: %s\n", f1, f2, strerror(errno));
  }
  printf("copy file %s to file %s successfully\n", f1, f2);
  close(fd1);
  close(fd2);
}

void copy_file_splice(const char *f1, const char *f2)
{
  int fd1 = open(f1, O_RDONLY);
  printf("splice usage\n");
  if (fd1 == -1)
  {
    printf("errno when open %s, error: %s\n", f1, strerror(errno));
    exit(EXIT_FAILURE);
  }
  int fd2 = open(f2, O_WRONLY | O_CREAT | O_EXCL, 0664);
  if (fd2 == -1)
  {
    printf("errno when open %s, error: %s", f2, strerror(errno));
    exit(EXIT_FAILURE);
  }
  int pipeline[2];
  if (pipe(pipeline) == -1)
  {
    printf("errno when create pipe, error: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  int n;
  while ((n = splice(fd1, NULL, pipeline[1], NULL, 256, SPLICE_F_MORE)) > 0)
  {
    splice(pipeline[0], NULL, fd2, NULL, n, SPLICE_F_MORE);
  }
  if (n == -1)
  {
    printf("copy file %s to file %s error: %s\n", f1, f2, strerror(errno));
  }
  printf("copy file %s to file %s successfully\n", f1, f2);
  close(fd1);
  close(fd2);
  close(pipeline[0]);
  close(pipeline[1]);
}

/*
time ./spc ~/a.txt ac.txt rw
read and write usage
copy file /home/vzgoll/a.txt to file ac.txt successfully

real    0m0.025s
user    0m0.007s
sys     0m0.017s


 time ./spc ~/a.txt acp.txt sp
splice usage
copy file /home/vzgoll/a.txt to file acp.txt successfully

real    0m0.039s
user    0m0.001s
sys     0m0.030s

0 is OK, -1 on error
*/

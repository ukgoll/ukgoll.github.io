#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
  if (argc != 2)
  {
    printf("./watch_file <file>\n");
    exit(EXIT_FAILURE);
  }
  const char *file_path = argv[1];
  printf("watch file %s\n", file_path);
  int kq;
  if ((kq = kqueue()) == -1)
  {
    perror("create kqueue error");
    exit(EXIT_FAILURE);
  }
  struct kevent sk;
  int fd;
  if ((fd = open(file_path, O_RDONLY)) == -1)
  {
    perror("open file error");
    exit(EXIT_FAILURE);
  }
  EV_SET(&sk, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_READ, 0, 0);
  if (kevent(kq, &sk, 1, NULL, 0, NULL) == -1)
  {
    perror("kevent add error");
    exit(EXIT_FAILURE);
  }
  struct kevent change_list[1];
  while (1)
  {
    int ret = kevent(kq, NULL, 0, change_list, 1, NULL);
    printf("ret is %d\n", ret);
    if (ret == -1)
    {
      perror("kqueue listen error");
      exit(EXIT_FAILURE);
    }
    else if (ret > 0)
    {
      if (change_list[0].filter != EVFILT_VNODE)
        continue;
      if (change_list[0].fflags & NOTE_WRITE)
      {
        printf("file_path be writed!!!\n");
      }
      if (change_list[0].fflags & NOTE_READ)
      {
        printf("file_path be read!!!\n");
      }
    }
  }

  close(kq);
  return 0;
}

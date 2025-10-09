#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int dir_count = 0;
int regular_count = 0;
int socket_count = 0;
int block_count = 0;
int character_count = 0;
int link_count = 0;
int fifo_count = 0;

typedef struct FD_LINK
{
  int fd;
  struct FD_LINK *next;
} FLK;

/*
  这个不行的，openat 打开的描述符太多了
*/
void caculate_dir(FLK *dlk);
int main(int argc, char const *argv[])
{
  if (argc != 2)
  {
    printf("usage: ./duc <directory>");
    exit(EXIT_FAILURE);
  }
  int fd = open(argv[1], O_DIRECTORY | O_RDONLY);
  if (fd == -1)
  {
    perror("Init directory error");
    exit(EXIT_FAILURE);
  }
  FLK *dlk = malloc(sizeof(FLK));
  if (!dlk)
  {
    perror("Memory allocation failed");
    close(fd);
    exit(EXIT_FAILURE);
  }
  dlk->fd = fd;
  dlk->next = NULL;
  caculate_dir(dlk);

  printf("Regular file count: %d;\nDirectory file count: %d;\nBlock file count: %d;\n"
         "character file count: %d;\nSocket file count: %d;\nLink file count: %d;\n"
         "FiFo file count: %d;\n",
         regular_count, dir_count, block_count, character_count, socket_count, link_count, fifo_count);
  return 0;
}

void caculate_dir(FLK *dlk)
{
  FLK *latest = dlk;
  do
  {
    // printf("%p\n", dlk);
    int fd = dlk->fd;
    DIR *c_dir = fdopendir(fd);
    if (c_dir == NULL)
    {
      // 按道理来说应该不会进入这里的
      printf("dlk fd %d\n", dlk->fd);
      perror("open dir error");
      close(fd);
      return;
    }
    struct dirent *dt;
    while ((dt = readdir(c_dir)) != NULL)
    {
      // printf("dt->d_name: %s\n", dt->d_name);
      if (strcmp(dt->d_name, ".") == 0 || strcmp(dt->d_name, "..") == 0)
        continue;
      // printf("address: %p, name: %.*s, record length: %d\n", dt, dt->d_namlen, dt->d_name, dt->d_reclen);
      switch (dt->d_type)
      {
      case DT_FIFO:
        ++fifo_count;
        break;
      case DT_DIR:
        ++dir_count;
        int newfd = openat(fd, dt->d_name, O_DIRECTORY | O_RDONLY);
        if (newfd == -1)
        {
          perror("openat failed");
          break;
        }
        // 动态分配新节点
        FLK *newdlk = malloc(sizeof(FLK));
        if (!newdlk)
        {
          perror("Memory allocation failed");
          close(newfd);
          break;
        }
        newdlk->fd = newfd;
        newdlk->next = NULL;
        latest->next = newdlk;
        latest = newdlk;
        break;
      case DT_REG:
        ++regular_count;
        break;
      case DT_BLK:
        ++block_count;
        break;
      case DT_SOCK:
        ++socket_count;
        break;
      case DT_CHR:
        ++character_count;
        break;
      case DT_LNK:
        ++link_count;
        break;
      default:
        break;
      }
    }
    closedir(c_dir);
    close(fd);
    FLK *tmp = dlk;
    dlk = dlk->next;
    if (dlk == NULL)
    {
      printf("dlk is NULL\n");
    }
    else
    {
      // printf("dlk is not NULL\n");
    }
    free(tmp);
  } while (dlk);
}

/*
time ./ducfd /
Regular file count: 185969;
Directory file count: 23916;
Block file count: 14;
character file count: 196;
Socket file count: 45;
Link file count: 25878;
FiFo file count: 15;

real    0m0.296s
user    0m0.079s
sys     0m0.210s
*/

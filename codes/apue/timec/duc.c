#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

int dir_count = 0;
int regular_count = 0;
int socket_count = 0;
int block_count = 0;
int character_count = 0;
int link_count = 0;
int fifo_count = 0;

void caculate_dir(char *file_path);
int main(int argc, char const *argv[])
{
  if (argc != 2)
  {
    printf("usage: ./duc <directory>");
    exit(EXIT_FAILURE);
  }
  char *file_path = strdup(argv[1]);
  caculate_dir(file_path);

  printf("Regular file count: %d;\nDirectory file count: %d;\nBlock file count: %d;\n"
         "character file count: %d;\nSocket file count: %d;\nLink file count: %d;\n"
         "FiFo file count: %d;\n",
         regular_count, dir_count, block_count, character_count, socket_count, link_count, fifo_count);
  return 0;
}

// POSIX POSIX.1 仅定义此结构中的 d_name 条目。所以 移植性不一定好
void caculate_dir(char *file_path)
{
  printf("file path %s\n", file_path);
  DIR *c_dir = opendir(file_path);
  if (c_dir == NULL)
  {
    printf("errno number refer https://man7.org/linux/man-pages/man3/opendir.3.html\n");
    if (errno == ENOTDIR)
    {
      printf("%s is not a directory.\n", file_path);
      return;
    }
    if (errno == ENOENT)
    {
      printf("Directory does not exist, or name is an empty string.\n");
      return;
    }
    perror("open dir error");
    return;
  }
  struct dirent *dt;
  while ((dt = readdir(c_dir)) != NULL)
  {
    // printf("address: %p, name: %.*s, record length: %d\n", dt, dt->d_namlen, dt->d_name, dt->d_reclen);
    switch (dt->d_type)
    {
    case DT_FIFO:
      ++fifo_count;
      break;
    case DT_DIR:
      ++dir_count;
      if (strcmp(dt->d_name, ".") == 0 || strcmp(dt->d_name, "..") == 0)
        break;
      if (sizeof(file_path) - strlen(file_path) < strlen(dt->d_name))
      {
        file_path = realloc(file_path, sizeof(file_path + strlen(dt->d_name)));
        if (file_path == NULL)
        {
          perror("realloc error");
          break;
        }
      }
      char *newp = strdup(file_path);
      strcat(newp, "/");
      strcat(newp, dt->d_name);
      caculate_dir(newp);
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
}

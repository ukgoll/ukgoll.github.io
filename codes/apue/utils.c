#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include "utils.h"

void eexit(char *str)
{
  perror(str);
  exit(EXIT_FAILURE);
}

void nexit(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args); // 使用 vprintf 处理可变参数
  va_end(args);

  exit(EXIT_FAILURE);
}

void print_ctime()
{
  time_t t;
  struct tm *tm_info;

  time(&t);                // 获取当前时间（时间戳）
  tm_info = localtime(&t); // 转换为本地时间

  printf("[%02d-%02d-%02d %02d:%02d:%02d]: ",
         tm_info->tm_year + 1900, // 年份是从 1900 开始的
         tm_info->tm_mon + 1,     // 月份是从 0 开始的
         tm_info->tm_mday,
         tm_info->tm_hour,
         tm_info->tm_min,
         tm_info->tm_sec);
}

int set_no_blocking(int fd)
{
  int val = fcntl(fd, F_GETFL);
  if (val == -1)
  {
    return -1;
  }
  return fcntl(fd, F_SETFL, val | O_NONBLOCK);
}

int set_blocking(int fd)
{
  int val = fcntl(fd, F_GETFL);
  if (val == -1)
  {
    return -1;
  }
  return fcntl(fd, F_SETFL, val & ~O_NONBLOCK);
}

char *gf_time(void)
{
  struct timeval tv;
  static char str[30]; // 存储格式化时间
  char *ptr;

  if (gettimeofday(&tv, NULL) < 0)
  {
    perror("gettimeofday error");
    exit(EXIT_FAILURE);
  }

  ptr = ctime(&tv.tv_sec);                                 // 获取时间字符串
  strcpy(str, &ptr[11]);                                   // 复制 "HH:MM:SS" 部分
  snprintf(str + 8, sizeof(str) - 8, ".%06d", tv.tv_usec); // 追加微秒

  return str;
}

void printf_error(int se, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args); // 使用 vprintf 处理可变参数
  va_end(args);
  printf(", [error] is %s\n", strerror(errno)); // 附加错误信息
  if (se)
  {
    exit(EXIT_FAILURE);
  }
}

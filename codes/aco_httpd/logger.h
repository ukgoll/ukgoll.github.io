#include <stdio.h>


enum ELoglevel{
  debug=0,
  info=1,
  error=2,
};

typedef struct logger_s
{
  enum ELoglevel log_level; // 日志等级
  int fd; // 打开的文件
} logger_t;


// 获取全局 logger
logger_t *get_logger();

// 设置日志等级 TODO 可以考虑多进程的时候 日志等级同步。
void set_log_level(enum ELoglevel el);

void log_Test(char *fmt, ...);

void log_warn();

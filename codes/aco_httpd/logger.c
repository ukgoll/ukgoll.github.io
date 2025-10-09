#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "logger.h"


static char *_get_logger_filename()
{
  time_t t = time(NULL);
  struct tm lt;
  localtime_r(&t, &lt);
}

logger_t *get_logger()
{
  static logger_t default_logger;
  return &default_logger;
}

void set_log_level(enum ELoglevel el)
{
  get_logger()->log_level = el;
}




void log_Test(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  // TODO
  vprintf(fmt, args);
  va_end(args);
}

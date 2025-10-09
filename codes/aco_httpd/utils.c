#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>


#include "utils.h"
#include "logger.h"

int set_non_blocking(int fd)
{
  int fg = fcntl(fd, F_GETFL);
  if(fcntl(fd, F_SETFL, fg | O_NONBLOCK) == -1){
    printf("fd set no blocing error: %d\n", errno);
    return errno;
  };
  return 0;
}

void log_with_error_msg(char *prefix)
{
  char buf[128];
  strerror_r(errno, buf, sizeof(buf));
  log_Test("%s: %s\n", prefix, buf);
}


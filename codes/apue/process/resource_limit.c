#include <stdio.h>
#include <sys/resource.h>

#include "../utils.h"

void print_rlimit(int name, char *prefix);

int main(int argc, char const *argv[])
{
  struct rlimit nofile_lr, fflie_lr;
  print_rlimit(-1, "process can opened file number");
  print_rlimit(RLIMIT_FSIZE, "process can created file number");
  return 0;
}

void print_rlimit(int name, char *prefix)
{
  struct rlimit s_rlim;
  if (name >= RLIM_NLIMITS)
  {
    printf("Invalid name\n");
    return;
  }
  if (getrlimit(name, &s_rlim) == -1)
  {
    printf_error(0, "limit resource is %s", strerror(errno));
  }
  if (s_rlim.rlim_cur == RLIM_INFINITY)
  {
    printf("%s soft limit is infinity\n", prefix);
  }
  else
  {
    printf("%s soft limit is %llu\n", prefix, s_rlim.rlim_cur);
  }
  if (s_rlim.rlim_max == RLIM_INFINITY)
  {
    printf("%s hard limit is infinity\n", prefix);
  }
  else
  {
    printf("%s hard limit is %llu\n", prefix, s_rlim.rlim_max);
  }
}

#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
  int thread_stack_size = sysconf(_SC_THREAD_ATTR_STACKSIZE);
  if (thread_stack_size == -1)
  {
    printf("get thread error\n");
  }
  else
  {
    printf("stack size is %d\n", thread_stack_size);
  }
  int iov_max = sysconf(_SC_IOV_MAX);
  if (iov_max == -1)
  {
    printf("get iov max error\n");
  }
  else
  {
    printf("iov max is %d\n", iov_max);
  }
  return 0;
}

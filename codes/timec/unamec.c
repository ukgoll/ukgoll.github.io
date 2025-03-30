#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/utsname.h>

/*
  uname shell 命令
  在 Linux 中和 Macos 中不一样，Linux 会打印所有，Macos 在 -s 和 -o 一起存在的时候就省略 -o
*/
int main(int argc, char const *argv[])
{
  long hostnameMax = sysconf(_SC_HOST_NAME_MAX);
  if (hostnameMax == -1)
  {
    perror("sysconf failed");
  }
  else
  {
    printf("Max hostname length: %ld\n", hostnameMax);
  }
  // char *a = "ab";
  char a[] = "ab";
  // sizeof 是一个运算符号，其实也可以不用加括号包裹变量。
  printf("sizeof(%s)=%lu, strlen(%s)=%lu\n", a, sizeof(*a), a, strlen(a));
  char hostname[hostnameMax];
  memset(hostname, 0, sizeof(hostname));
  if (gethostname(hostname, hostnameMax) < 0)
  {
    perror("host name");
  }
  printf("hostname is %s, hostname length is %lu\n", hostname, strlen(hostname));
  struct utsname utsn;
  uname(&utsn);
  printf("%s %s %s %s %s\n", utsn.sysname, utsn.nodename, utsn.release, utsn.version, utsn.machine);
  // char *cd = "cd";
  // printf("charcter is %c, %s, %p, %p\n", *cd, cd, cd, cd);
  return 0;
}

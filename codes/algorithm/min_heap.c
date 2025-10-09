#include <stdio.h>
#include <assert.h>

int main(int argc, char const *argv[])
{
  double a = 9.9;
  printf("a is %lf %lf\n", a, a << 9);
  assert(1 == 1);
  printf("void * pointer size is %lu\n", sizeof(void *));
  printf("char * pointer size is %lu\n", sizeof(char *));
#ifdef __aarch64__
  printf("MacOS M1 defined __aarch64__\n");
#else
  printf("MacOS M1 notttttt defined __aarch64__\n");
#endif
  return 0;
}

int tt()
{
  assert(1 == 1);
  printf("void * pointer size is %lu\n", sizeof(void *));
  printf("char * pointer size is %lu\n", sizeof(char *));
#ifdef __aarch64__
  printf("MacOS M1 defined __aarch64__\n");
#else
  printf("MacOS M1 notttttt defined __aarch64__\n");
#endif
  return 0;
}

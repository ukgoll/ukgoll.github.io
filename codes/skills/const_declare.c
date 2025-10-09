#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
  const char *p1 = "alfjal";
  printf("p1 is %s\n", p1);
  p1 = "hello, change";
  printf("p1 is %s\n", p1);
  // 字符串常量是 放在 只读段 .rodata ，所以如下的修改会有问题的。
  // char *const p2 = "heelowll";
  // printf("p2 is %s\n", p2);
  // p2[0] = 'a';
  // printf("p2 is %s\n", p2);
  //
  char *tmp = malloc(25);
  if (tmp == NULL)
  {
    printf("malloc error\n");
    return -1;
  }
  strncpy(tmp, "tmp malloc", 10);
  char *const p3 = tmp;
  printf("p3 is %s\n", p3);
  p3[0] = 'a';
  printf("p3 is %s\n", p3);
  // p3 = "afl"; // 这里编译的时候会报错
  //
  const char *const p4 = tmp;
  printf("p4 is %s\n", p4);
  // p4[0] = 'q'; // 这里编译的时候也会报错
  printf("p4 is %s\n", p4);
  // p4 = "afl"; // 这里编译的时候也会报错
  printf("p4 is %s\n", p4);
  free(tmp);
  return 0;
}

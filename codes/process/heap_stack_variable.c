#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <malloc/malloc.h> // macOS
#define MALLOC_SIZE(ptr) malloc_size(ptr)
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <malloc_np.h> // BSD 系统
#define MALLOC_SIZE(ptr) malloc_usable_size(ptr)
#elif defined(__linux__)
#include <malloc.h> // Linux glibc
#define MALLOC_SIZE(ptr) malloc_usable_size(ptr)
#else
#error "Unsupported platform"
#endif
struct MyStruct
{
  int testId;
  char testName[5];
};

void stack_variable();
void heap_variable();
void heap_variable_address();

extern char **environ;

int main(int argc, char const *argv[])
{
  // 找到环境变量列表的最后一个地址
  char **temp = environ;
  char **pt;
  while (1)
  {
    pt = temp;
    if (*(++temp) == NULL)
    {
      break;
    }
  }
  printf("last environment list address is %p\n", *pt);
  stack_variable();
  heap_variable_address();
  heap_variable();
  return 0;
}

/**
 * 不能这么判断，如果使用内存对齐优化的话，那么就是会出现问题
 * 0x7ffd1a7781f8, 0x7ffd1a778200, 0x7ffd1a7781f7, 0x7ffd1a7781fc
 * stack 地址是越来越大的  (这个系统上就使用了内存对齐优化)
 */
void stack_variable()
{
  printf("stack variable address -----------------\n");
  // 栈变量测试
  unsigned int d = 2282819976;                            // 0b10001000000100010001000110001000
  unsigned short *after_tb_d = (unsigned short *)&d;      // unsigned short 无符号 2 字节，取后面两个字节
  unsigned short *before_tb_d = (unsigned short *)&d + 1; // unsigned short 无符号 2 字节，取前面两个字节
  printf("d的后面两位: %d, after_tb_d: %d\nd的前面两位 %d, before_tb_d: %d\n", 0b0001000110001000, *after_tb_d, 0b1000100000010001, *before_tb_d);
  printf("stack variable address -----------------\n");
}

void heap_variable_address()
{
  printf("heap variable address -----------------\n");
  // 栈变量测试
  unsigned int *hd = malloc(sizeof(unsigned int)); // 0b10001000000100010001000110001000
  *hd = 2282819976;
  unsigned short *after_tb_d = (unsigned short *)hd;      // unsigned short 无符号 2 字节，取后面两个字节
  unsigned short *before_tb_d = (unsigned short *)hd + 1; // unsigned short 无符号 2 字节，取前面两个字节
  printf("hd的后面两位: %d, after_tb_d: %d\nhd的前面两位 %d, before_tb_d: %d\n", 0b0001000110001000, *after_tb_d, 0b1000100000010001, *before_tb_d);
  printf("heap variable address -----------------\n");
  free(hd);
}

void heap_variable()
{
  printf("使用 heap 的变量你是无法明显的看到内存对齐的，因为对于每个系统 malloc 的是一样大的\n");
  // 堆变量测试
  int *pa = malloc(sizeof(int));
  long *pb = malloc(sizeof(long));
  char *pc = malloc(sizeof(char));
  int *pd = malloc(sizeof(int));
  printf("pa: %p (malloc_size: %lu)\n", pa, MALLOC_SIZE(pa));
  printf("pb: %p (malloc_size: %lu)\n", pb, MALLOC_SIZE(pb));
  printf("pc: %p (malloc_size: %lu)\n", pc, MALLOC_SIZE(pc));
  printf("pc: %p (malloc_size: %lu)\n", pd, MALLOC_SIZE(pd));

  if (pa > pd)
  {
    printf("heap 地址是越来越小的\n");
  }
  else
  {
    printf("heap 地址是越来越大的\n");
  }

  // 释放内存
  free(pa);
  free(pb);
  free(pc);
}

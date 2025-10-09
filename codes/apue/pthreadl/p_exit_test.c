#include <stdio.h>
#include <pthread.h>

/**
 * 这个程序就是能说明，进程创建之后有一个主线程在执行程序
 */
int main(int argc, char const *argv[])
{
  printf("before pthread exit\n");
  pthread_exit((void *)0);
  // 按道理是到这里就退出了
  printf("after pthread exit\n");
  return 0;
}

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

void *thread1_func(void *args);
void *thread2_func(void *args);
/*
这个程序就是那个线程先运行，文件保存就是谁的内容，pwrite 是原子操作，但是 offset 同时 为 0 ，
不能保证结果
*/

struct Foo
{
  pthread_mutex_t lock;
  int fd;
};

int main(int argc, char const *argv[])
{
  pthread_t t1, t2;
  printf("pthread_t 在不同的平台上实现是不同的，比如在我的 MacOS 上就是使用结构体实现的，所以为了可移植性，不用 printf 打印 pthread_t 变量\n");
  NULL;
#ifdef _POSIX_THREADS
  printf("support pthread %ld\n", sysconf(_SC_THREADS));
#else
  printf("not support pthread\n");
#endif
  int fd = open("./test_write_race.txt", O_WRONLY | O_CREAT | O_TRUNC);
  if (fd == -1)
  {
    printf("open fd error: %s\n", strerror(errno));
    return -1;
  }
  struct Foo *foo = malloc(sizeof(struct Foo));
  if (foo == NULL)
  {
    printf("malloc error: %s\n", strerror(errno));
    return -1;
  }
  foo->fd = fd;
  if (pthread_mutex_init(&foo->lock, NULL) != 0)
  {
    printf("mutex lock init error: %s\n", strerror(errno));
    return -1;
  }
  if (pthread_create(&t2, NULL, thread2_func, foo) != 0)
  {
    printf("create pthread 2 error: %s\n", strerror(errno));
    return -1;
  }
  if (pthread_create(&t1, NULL, thread1_func, foo) != 0)
  {
    printf("create pthread 1 error: %s\n", strerror(errno));
    return -1;
  }
  printf("t1 address is %p, t2 address is %p\n", t1, t2);
  void *pt_ret;
  pthread_join(t1, &pt_ret);
  printf("pthread1 return value is %#lx\n", (long)pt_ret);
  pthread_join(t2, &pt_ret);
  printf("pthread2 return value is %#lx\n", (long)pt_ret);
  close(fd);
  return 0;
}

void *thread1_func(void *args)
{
  struct Foo *foo = (struct Foo *)args;
  size_t wn;
  const char *t1s = "thread 1 write\n";
  pthread_mutex_lock(&foo->lock);
  for (int i = 0; i < 100; i++)
  {
    if ((wn = write(foo->fd, t1s, strlen(t1s))) == -1)
    {
      printf("thread1 write error: %s\n", strerror(errno));
    }
    else
    {
      printf("thread1 write %zu bytes\n", wn);
    }
  }
  pthread_mutex_unlock(&foo->lock);
  return ((void *)0x11);
}
void *thread2_func(void *args)
{
  struct Foo *foo = (struct Foo *)args;
  size_t wn;
  const char *t2s = "thread 2 write\n";
  pthread_mutex_lock(&foo->lock);
  for (int i = 0; i < 100; i++)
  {
    if ((wn = write(foo->fd, t2s, strlen(t2s))) == -1)
    {
      printf("thread2 write error: %s\n", strerror(errno));
    }
    else
    {
      printf("thread2 write %zu bytes\n", wn);
    }
  }
  pthread_mutex_unlock(&foo->lock);
  return ((void *)0x22);
}

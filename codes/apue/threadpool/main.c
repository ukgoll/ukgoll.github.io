#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <signal.h>

struct ThreadPool;

typedef void (*thread_pool_func)(struct ThreadPool *tp);

typedef void (*task_func)(void *);
typedef struct ThreadTask
{
  task_func func;
  void *args;
  struct ThreadTask *prev;
  struct ThreadTask *next;
} Task;

typedef struct ThreadPool
{
  uint16_t max_worker;    // 线程池里面最多可以放多少个线程
  uint16_t noact_workder; // 当前线程池里有多少个可以用线程
  pthread_cond_t cond;    // 线程池采用 condition 实现
  pthread_mutex_t lock;   // condition 必须搭配 mutex 使用
  pthread_t *threads;     // 保存线程 id
  Task *queue;
  Task *queue_tail;
  unsigned int stop; // 是否停止线程 1 是 0 否
} ThreadPool;

ThreadPool *create_thread_pool(int max_worker); // 创建线程池
void release_thread_poll(ThreadPool *tp);       // 销毁线程池
void *thread_pool_item(void *tp);               // 线程池里面的线程运行的函数
void add_task(ThreadPool *tp, task_func tf, void *task_args);
void delivery_signal_to_thread(ThreadPool *tp, int signo);

void process_test_signal(int signo)
{
  printf("process receive signal %d\n", signo);
}

void test_signal(int signo)
{
  printf("receive signal %d\n", signo);
  printf("error is %s\n", strerror(errno));
}
void print_task(void *arg)
{
  char *msg = (char *)arg;
  struct timespec op = {.tv_sec = 60, .tv_nsec = 0}, lp;
  struct sigaction sa = {0};
  sa.sa_handler = test_signal;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGINT, &sa, 0) == -1)
  {
    printf("register signal error\n");
    return;
  }
  while (nanosleep(&op, &lp) == -1)
  {
    printf("sleep 中断，开始循环 nanosleep\n");
    if (lp.tv_sec == 0 && lp.tv_nsec == 0)
      break;
    op.tv_sec = lp.tv_sec;
    op.tv_nsec = lp.tv_nsec;
  }
  printf("Task says: %s\n", msg);
}

int main(int argc, char const *argv[])
{
  printf("pid is %d\n", getpid());
  struct sigaction sa = {0};
  sa.sa_handler = process_test_signal;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGINT, &sa, 0) == -1)
  {
    printf("register signal error\n");
    return 1;
  }
  ThreadPool *pool = create_thread_pool(4);
  if (pool == NULL)
  {
    return -1;
  }
  printf("thread poll create successfully\n");

  add_task(pool, print_task, "abalfjl");
  release_thread_poll(pool);
  printf("thread poll close successfully\n");
  return 0;
}

ThreadPool *create_thread_pool(int max_worker)
{
  ThreadPool *tp = malloc(sizeof(ThreadPool));
  if (tp == NULL)
    return NULL;
  memset(tp, 0, sizeof(ThreadPool));
  //
  tp->threads = malloc(sizeof(pthread_t) * max_worker);
  if (tp->threads == NULL)
  {
    free(tp->threads);
    free(tp);
    return NULL;
  }
  memset(tp->threads, 0, sizeof(pthread_t) * max_worker);
  //
  tp->queue_tail = tp->queue = NULL;
  //
  tp->max_worker = max_worker;
  tp->noact_workder = max_worker;
  tp->stop = 0;
  if (tp == NULL)
  {
    printf("malloc error %s\n", strerror(errno));
    return NULL;
  }
  if (pthread_cond_init(&tp->cond, NULL))
  {
    printf("pthread cond error %s\n", strerror(errno));
    free(tp);
    return NULL;
  }
  if (pthread_mutex_init(&tp->lock, NULL))
  {
    printf("pthread mutex error %s\n", strerror(errno));
    free(tp);
    return NULL;
  }
  printf("max_worker is %d\n", max_worker);
  int err;
  for (int i = 0; i < max_worker; i++)
  {
    err = pthread_create(&tp->threads[i], NULL, thread_pool_item, tp);
    if (err != 0)
    {
      printf("create thread error %s", strerror(err));
    }
  }
  return tp;
}

void release_thread_poll(ThreadPool *tp)
{
  pthread_mutex_lock(&tp->lock);
  tp->stop = 1;
  pthread_mutex_unlock(&tp->lock);
  pthread_cond_broadcast(&tp->cond);

  for (int i = 0; i < tp->max_worker; i++)
  {
    pthread_join(tp->threads[i], NULL);
  }
  pthread_cond_destroy(&tp->cond);
  pthread_mutex_destroy(&tp->lock);
  free(tp->threads);
  free(tp);
}

void *thread_pool_item(void *tp)
{
  ThreadPool *rtp = (ThreadPool *)tp;
  Task *thread_task;
  for (;;)
  {
    pthread_mutex_lock(&rtp->lock);
    while (NULL == rtp->queue && !rtp->stop)
    {
      pthread_cond_wait(&rtp->cond, &rtp->lock);
    }
    if (rtp->stop && rtp->queue == NULL)
    {
      pthread_mutex_unlock(&rtp->lock);
      printf("thread pool exit\n");
      pthread_exit((void *)0);
    }
    if (rtp->queue_tail)
    {
      thread_task = rtp->queue_tail;
      rtp->queue_tail = rtp->queue_tail->prev;
      if (rtp->queue_tail)
        rtp->queue_tail->next = NULL;
      else
        rtp->queue = NULL;
    }
    pthread_mutex_unlock(&rtp->lock);

    if (thread_task)
    {
      thread_task->prev = NULL;
      thread_task->func(thread_task->args);
    }
  }
  return (void *)0;
}

void add_task(ThreadPool *tp, task_func tf, void *task_args)
{
  Task *task = malloc(sizeof(Task));
  task->func = tf;
  task->args = task_args;
  task->next = NULL;
  task->prev = NULL;
  pthread_mutex_lock(&tp->lock);
  if (tp->queue_tail == NULL)
  {
    tp->queue = task;
    tp->queue_tail = task;
  }
  else
  {
    task->prev = tp->queue_tail;
    tp->queue_tail->next = task;
    tp->queue_tail = task;
  }

  pthread_mutex_unlock(&tp->lock);
  pthread_cond_signal(&tp->cond);
}

void delivery_signal_to_thread(ThreadPool *tp, int signo)
{
  for (int i = 0; i < tp->max_worker; i++)
  {
    pthread_kill(tp->threads[i], signo);
  }
}

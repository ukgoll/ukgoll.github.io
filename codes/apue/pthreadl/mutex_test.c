#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void pthread_unlock()
{
  int err;
  char buf[256] = {0};
  if ((err = pthread_mutex_unlock(&mutex)) == 0)
  {
    printf("Thread A: unlocked mutex successfully.\n");
  }
  else if (err == EPERM)
  {
    printf("Thread A: unlock failed — current thread does not own the mutex (EPERM).\n");
  }
  else
  {
    strerror_r(err, buf, sizeof(buf));
    printf("Thread A: unlock failed — error: [%s]\n", buf);
  }
}

void *thread_a(void *arg)
{
  sleep(2);
  printf("Thread A: locking mutex first time...\n");
  pthread_mutex_lock(&mutex);
  printf("Thread A: first lock acquired.\n");
  printf("Thread A: locking mutex second time (will deadlock)...\n");
  int err;
  char buf[256] = {0};
  if ((err = pthread_mutex_lock(&mutex)) != 0)
  {
    strerror_r(err, buf, sizeof(buf));
    printf("Thread A: lock error: [%s]\n", buf);
  }
  pthread_unlock();
  pthread_unlock();
  return NULL;
}

void *thread_b(void *arg)
{
  printf("Thread B: trying to unlock mutex...\n");

  int ret = pthread_mutex_unlock(&mutex);
  if (ret == 0)
  {
    printf("Thread B: unlocked mutex (unexpected).\n");
  }
  else if (ret == EPERM)
  {
    printf("Thread B: unlock failed — current thread does not own the mutex (EPERM).\n");
  }
  else
  {
    printf("Thread B: unlock failed — error code: %d\n", ret);
  }
  return NULL;
}

int main()
{
  pthread_t a, b;
  int t = 1 > 2 ? 3 : 4;
  printf("t is %d\n", t);

  pthread_create(&a, NULL, thread_a, NULL);
  pthread_create(&b, NULL, thread_b, NULL);

  pthread_join(a, NULL);
  pthread_join(b, NULL);

  return 0;
}

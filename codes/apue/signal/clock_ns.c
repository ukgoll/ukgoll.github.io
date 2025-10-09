#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

/**
 * CLOCK_TAI 是 Linux 中定义的，但是默认还是需要启用系统的 ntp 服务器的 tai ，
 * 咨询查询 chrony，我是使用 chrony
define CLOCK_TAI
current real time sec 1744708853, time nano sec 307319342
current tai time sec 1744708890, time nano sec 307319426
查询的正常来说就是 37 秒
 */

int main(int argc, char const *argv[])
{
  struct timespec rt, tai_t;
#ifdef CLOCK_TAI
  printf("define CLOCK_TAI\n");
  if (clock_gettime(CLOCK_REALTIME, &rt) == -1)
  {
    printf("clock get time ot error\n");
  }
  if (clock_gettime(CLOCK_TAI, &tai_t) == -1)
  {
    printf("clock get time ot error\n");
  }
#else
  printf("not define CLOCK_TAI\n");
  if (clock_gettime(CLOCK_REALTIME, &rt) == -1)
  {
    printf("clock get time ot error\n");
  }
  if (clock_gettime(CLOCK_REALTIME, &tai_t) == -1)
  {
    printf("clock get time ot error\n");
  }
#endif
  printf("current real time sec %ld, time nano sec %ld\n", rt.tv_sec, rt.tv_nsec);
  printf("current tai time sec %ld, time nano sec %ld\n", tai_t.tv_sec, tai_t.tv_nsec);
  printf("测试 clock 绝对睡眠 3 秒\n");
  rt.tv_sec += 38; // 由于 CLOCK_TAI 和 CLOCK_REALTIME 差了 37 秒，所以下面两个睡眠没有什么区别(当然需要启用  leap seconds)
  tai_t.tv_sec += 3;
#if _POSIX_C_SOURCE >= 200112L
  if (clock_nanosleep(CLOCK_TAI, TIMER_ABSTIME, &rt, NULL) != 0)
  {
    printf("clock nanosleep error: %s\n", strerror(errno));
  }
  else
  {
    printf("clock_nanosleep end: %s\n", strerror(errno));
  }
  // 由于是绝对时间，前面睡了一秒，这里只睡两秒
  if (clock_nanosleep(CLOCK_TAI, TIMER_ABSTIME, &tai_t, NULL) != 0)
  {
    printf("clock nanosleep error: %s\n", strerror(errno));
  }
  else
  {
    printf("clock_nanosleep end: %s\n", strerror(errno));
  }
#else
  printf("not support clock_nanosleep function\n");
#endif
  return 0;
}

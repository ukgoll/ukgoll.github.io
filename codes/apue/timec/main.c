#include <stdio.h>
#include <time.h>
#include <stdlib.h>
// #include <sys/time.h>

/*
  epoch 1970-01-01 00:00:00
*/
int main(int argc, char const *argv[])
{
  // setenv("TZ", "America/New_York", 1);
  // tzset(); // 让时区更改生效
  time_t ct;
  if (time(&ct) == -1)
  {
    perror("get current unix timestamp error");
  }
  printf("%ld\n", ct);
  printf("POSIX time----\n");
  struct timespec tsp;
  // CLOCK_MONOTONIC 获取电脑启动时间
  clock_gettime(CLOCK_MONOTONIC, &tsp);
  printf("电脑启动时间 %ld 小时, %ld 分钟, %ld 秒, nano seconds %ld\n", tsp.tv_sec / 3600, (tsp.tv_sec % 3600) / 60, (tsp.tv_sec % 3600) % 60, tsp.tv_nsec);
  // CLOCK_PROCESS_CPUTIME_ID
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tsp);
  printf("seconds %ld, nano seconds %ld\n", tsp.tv_sec, tsp.tv_nsec);
  // CLOCK_THREAD_CPUTIME_ID
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsp);
  printf("seconds %ld, nano seconds %ld\n", tsp.tv_sec, tsp.tv_nsec);
  // 除了月份，都是从 0 开始的。
  // struct tm {
  //   int	tm_sec;		/* seconds after the minute [0-60] */
  //   int	tm_min;		/* minutes after the hour [0-59] */
  //   int	tm_hour;	/* hours since midnight [0-23] */
  //   int	tm_mday;	/* day of the month [1-31] */
  //   int	tm_mon;		/* months since January [0-11] */
  //   int	tm_year;	/* years since 1900 */
  //   int	tm_wday;	/* days since Sunday [0-6] */
  //   int	tm_yday;	/* days since January 1 [0-365] */
  //   int	tm_isdst;	/* Daylight Savings Time flag */
  //   long	tm_gmtoff;	/* offset from UTC in seconds */
  //   char	*tm_zone;	/* timezone abbreviation */
  // };
  struct tm *gmt = gmtime(&ct);
  struct tm *local = localtime(&ct);
  printf("UTC time: year %d, month %d, day %d, hour %d, minute %d, second %d, timezone %s\n",
         gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec, gmt->tm_zone);
  printf("Local time: year %d, month %d, day %d, hour %d, minute %d, second %d, timezone %s\n",
         local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, local->tm_zone);
  // mktime 是受 timezone 影响的，所以使用 gmtime 的结果结构体会返回错误。
  printf("mktime from local time %ld, from global time %ld\n", mktime(local), mktime(gmt));
  char buf[32];
  if (strftime(buf, 32, "strftime %G-%m-%d %H:%M:%S", local) == 0)
  {
    perror("error when strftime usage");
  }
  printf("strftime (local) 会受到 timezone 的影响 %s\n", buf);
  char buf2[32];
  if (strftime(buf2, 32, "strftime %G-%m-%d %H:%M:%S", gmt) == 0)
  {
    perror("error when strftime usage");
  }
  printf("strftime (global) 会受到 timezone 的影响 %s\n", buf2);
  return 0;
}

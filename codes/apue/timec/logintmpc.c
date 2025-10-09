#include <stdio.h>
#include <time.h>

void read_login_tmp(char *filepath);

int main(int argc, char const *argv[])
{
  printf("utmp file:-----\n");
  read_login_tmp("/var/run/utmp");
  printf("wtmp file:-----\n");
  read_login_tmp("/var/log/wtmp");
  return 0;
}

#if defined(__APPLE__) || defined(__FreeBSD__)
#include <utmpx.h>
void read_login_tmp(char *filepath)
{
  printf("utmpx=======\n");
// 设置 utmp file
#if defined(__APPLE__)
  utmpxname(filepath);
#endif
  // 把 文件的 offset 放到 0
  setutxent();

  struct utmpx *sut;
  struct tm *local_time;
  while ((sut = getutxent()) != NULL)
  {
    if (sut->ut_type != USER_PROCESS)
      continue;
    local_time = localtime(&sut->ut_tv.tv_sec);
    printf("sut timestamp: %ld, time timestamp: %ld\n", sut->ut_tv.tv_sec, time(NULL));
    printf("Inittab ID %-4.4s, LoginProcessId: %6d, UserName %*.*s, UserHost: %.*s, UserLine: %.*s, Login time: %4d-%02d-%02d %02d:%02d:%02d\n", sut->ut_id,
           sut->ut_pid, (int)sizeof(sut->ut_user),
           (int)sizeof(sut->ut_user), sut->ut_user, (int)sizeof(sut->ut_host), sut->ut_host, (int)sizeof(sut->ut_line), sut->ut_line,
           local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
           local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
  }

  // 关闭 utmp file
  endutxent();
}
#elif defined(__linux__)
#include <utmp.h>
void read_login_tmp(char *filepath)
{
  // 设置 utmp file
  utmpname(filepath);
  // 把 文件的 offset 放到 0
  setutent();
  struct utmp *sut;
  struct tm *local_time;
  while ((sut = getutent()) != NULL)
  {
    if (sut->ut_type != USER_PROCESS)
      continue;
    local_time = localtime(&sut->ut_time);
    printf("Inittab ID %-4.4s, Sessionid: %6ld, LoginProcessId: %6d, UserName %*.*s, UserHost: %.*s, UserLine: %.*s, Login time: %ld %4d-%02d-%02d %02d:%02d:%02d\n", sut->ut_id,
           sut->ut_session, sut->ut_pid, UT_NAMESIZE,
           UT_NAMESIZE, sut->ut_user, UT_HOSTSIZE, sut->ut_host, UT_LINESIZE, sut->ut_line,
           sut->ut_time,
           local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
           local_time->tm_hour + 1, local_time->tm_min + 1, local_time->tm_sec);
  }

  // 关闭 utmp file
  endutent();
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

// 写一个输入和输出重定向

void get_win_size(int sig_num);
// 这个函数是很必要的，对于不同的系统，对于 cfgetispeed 等获取波特率的结果是不同的（
// 在不同的系统中定义不同， BSD 类的就喜欢(#define B150 150 这种实打实的定义，但是 ubuntu 这种喜欢 bit mask (#define  B150	0000005) 八进制。
int speed_to_baud(speed_t speed);

int main(int argc, char const *argv[])
{
  struct termios tc;
  if (isatty(STDIN_FILENO))
  {
    printf("STDIN_FILENO is a terminal device, and path name is %s\n", ttyname(STDIN_FILENO));
  }
  else
  {
    printf("STDIN_FILENO is not a terminal device\n");
    exit(EXIT_FAILURE);
  }

  if (tcgetattr(STDIN_FILENO, &tc) == -1)
  {
    printf("tcgetattr invoke error: %s\n", strerror(errno));
  }
  printf("input speed is: %d, output speed is: %d\n", speed_to_baud(tc.c_ispeed), speed_to_baud(tc.c_ospeed));
  speed_t input_speed = cfgetispeed(&tc), output_speed = cfgetospeed(&tc);
  printf("real input speed is: %d, real output speed is: %d\n", speed_to_baud(input_speed), speed_to_baud(output_speed));
  printf("初始化调用获取终端大小\n");
  get_win_size(-1);
  if (signal(SIGWINCH, get_win_size) == SIG_ERR)
  {
    printf("register win change signal error: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  for (;;)
  {
    pause();
  }
  return 0;
}

void get_win_size(int sig_num)
{
  if (sig_num != -1)
  {
    printf("[SIGWINCH received: %d]: 接收到 terminal size 尺寸变化\n", sig_num);
  }
  struct winsize size;
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *)&size) < 0)
  {
    printf("ioctl get win size error: %s \n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf("row: %u, column: %u, x pixel(not used): %u, y pixel(not used): %u\n", size.ws_row, size.ws_col, size.ws_xpixel, size.ws_ypixel);
}

int speed_to_baud(speed_t speed)
{
  switch (speed)
  {
  case B0:
    return 0;
  case B50:
    return 50;
  case B75:
    return 75;
  case B110:
    return 110;
  case B134:
    return 134;
  case B150:
    return 150;
  case B200:
    return 200;
  case B300:
    return 300;
  case B600:
    return 600;
  case B1200:
    return 1200;
  case B1800:
    return 1800;
  case B2400:
    return 2400;
  case B4800:
    return 4800;
  case B9600:
    return 9600;
  case B19200:
    return 19200;
  case B38400:
    return 38400;
#ifdef B57600
  case B57600:
    return 57600;
#endif
#ifdef B115200
  case B115200:
    return 115200;
#endif
#ifdef B230400
  case B230400:
    return 230400;
#endif
  default:
    return -1; // Unknown speed
  }
}

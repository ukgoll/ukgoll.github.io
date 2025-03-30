#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>

void eexit(char *str);
void print_ctime();

int main(int argc, char const *argv[])
{
  int fd;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    eexit("create client socket error");
  }
  struct sockaddr_in sdi;
  sdi.sin_port = htons(6001);
  sdi.sin_family = AF_INET;
  int ret = inet_pton(AF_INET, "127.0.0.1", &sdi.sin_addr);
  if (ret == 0)
  {
    eexit("Invalid IP Address");
  }
  else if (ret == -1)
  {
    eexit("Parse Error");
  }
  if (connect(fd, (struct sockaddr *)&sdi, sizeof(sdi)) < 0)
  {
    eexit("connect to server error");
  }

  char recvline[256] = {0}, sendline[256] = {0};
  fd_set readfd;
  FD_ZERO(&readfd);
  int maxfd = fileno(stdin) + 1;
  if (fd >= maxfd)
  {
    maxfd = fd + 1;
  }
  int stdineof = 0;
  int fdeof = 0;
  for (;;)
  {
    if (stdineof == 0)
    {
      FD_SET(fileno(stdin), &readfd);
    }
    // if(fdeof == 0){
    FD_SET(fd, &readfd);
    // }
    int ret = select(maxfd, &readfd, NULL, NULL, NULL);
    printf("ret is %d, eof %d, %d\n", ret, stdineof, fdeof);
    if (ret == 0)
    {
      printf("loop over time\n");
    }
    else if (ret == -1)
    {
      if (errno == EINTR)
      {
        continue;
      }
      perror("select error\n");
      break;
    }
    else
    {
      if (FD_ISSET(fd, &readfd))
      {
        memset(recvline, 0, sizeof(recvline));
        int n = read(fd, recvline, 255);
        if (n == -1)
        {
          eexit("recv error");
        }
        else if (n == 0)
        {
          printf("connect close\n");
          fdeof = 1;
          break;
        }
        print_ctime();
        printf("[recv from server:]%s, length is %ld, %d\n", recvline, strlen(recvline), n);
        if (strcmp(recvline, "end\n") == 0 && stdineof)
        {
          break;
        }
        /*
        如果是 测试脚本运行，那么直接发送接收一次直接直接结束
        */
        if (argc == 2 && strcmp(argv[1], "test") == 0)
        {
          sleep(1);
          break;
        }
      }

      if (FD_ISSET(STDIN_FILENO, &readfd))
      {
        if (fgets(sendline, 255, stdin) != NULL)
        {
          int len = strlen(sendline);
          // sendline[len-1] = '\0';
          int n = write(fd, sendline, sizeof(sendline));
          printf("n is %d %d\n", n, errno);
          if (n <= 0)
          {
            if (errno == EINTR)
            {
              fprintf(stderr, "write error\n");
              break;
            };
            perror("send error");
          }
        }
        else
        {
          fprintf(stderr, "read eof from stdin, %d\n", errno);
          stdineof = 1;
          // write(fd, "ttend", 6);
          FD_CLR(STDIN_FILENO, &readfd);
        }
      }
    }
  }
  close(fd);
  return 0;
}

void eexit(char *str)
{
  perror(str);
  exit(EXIT_FAILURE);
}

void print_ctime()
{
  time_t t;
  struct tm *tm_info;

  time(&t);                // 获取当前时间（时间戳）
  tm_info = localtime(&t); // 转换为本地时间

  printf("[%02d-%02d-%02d %02d:%02d:%02d]: ",
         tm_info->tm_year + 1900, // 年份是从 1900 开始的
         tm_info->tm_mon + 1,     // 月份是从 0 开始的
         tm_info->tm_mday,
         tm_info->tm_hour,
         tm_info->tm_min,
         tm_info->tm_sec);
}

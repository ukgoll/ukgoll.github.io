#include <stdlib.h>
#include <stdio.h>
#include <sys/event.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <time.h>
#include <arpa/inet.h>
#include <signal.h>

#define ClientL 1024

void eexit(char *str);
void print_ctime();

int main()
{
  signal(SIGPIPE, SIG_IGN);
  printf("kqueue multi io server start\n");
  int kq;
  printf("create kqueue start\n");
  if ((kq = kqueue()) == -1)
  {
    eexit("create kqueue error");
  }
  printf("create socket start\n");
  int listen_fd, client_fd;
  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    eexit("create socket fd error");
  }
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(6001);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("bind start\n");
  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    eexit("bind port error");
  }
  printf("bind successful\n");
  if (listen(listen_fd, 256) < 0)
  {
    eexit("listen error");
  }
  printf("listen successfully\n");
  struct kevent sk;
  EV_SET(&sk, listen_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  if (kevent(kq, &sk, 1, NULL, 0, NULL) < 0)
  {
    eexit("register error");
  }
  struct kevent ck[ClientL];
  int ret;
  socklen_t client_len;
  printf("开始 kqueue 事件循环\n");
  char buf[256] = {0};
  for (;;)
  {
    ret = kevent(kq, NULL, 0, ck, 1024, NULL); // todo timeout
    if (ret == -1)
    {
      eexit("kevent response error");
    }
    for (int i = 0; i < ret; i++)
    {
      struct kevent item = ck[i];
      if (!(item.filter & EVFILT_READ) && item.flags & EV_EOF)
      {
        printf("conn occur EOF, errno %d\n", errno);
        if (item.ident == listen_fd)
        {
          eexit("server closed!!!!!!!!!!");
        }
        if (item.data != 0)
        {
          printf("data can readable, leave %ld byte!!\n", item.data);
        }
        EV_SET(&sk, item.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        if (kevent(kq, &sk, 1, NULL, 0, NULL) < 0)
        {
          eexit("Delete client fd error");
        }
        close(item.ident);
        free(item.udata);
        printf("conn closed success\n");
      }
      else
      {
        if ((item.ident == listen_fd) && (item.filter & EVFILT_READ))
        {
          // 分配 udata 结构体
          struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
          if (!client_addr)
          {
            eexit("malloc failed");
          }
          memset(client_addr, 0, sizeof(struct sockaddr_in));
          client_len = sizeof(*client_addr);
          if ((client_fd = accept(listen_fd, (struct sockaddr *)client_addr, &client_len)) < 0)
          {
            eexit("accept error");
          }
          EV_SET(&sk, client_fd, EVFILT_READ, EV_ADD, 0, 0, client_addr);
          if (kevent(kq, &sk, 1, NULL, 0, NULL) < 0)
          {
            eexit("add client fd error");
          }
          printf("client conn come\n");
        }
        else if (item.filter & EVFILT_READ)
        {
          struct sockaddr_in *cs = (struct sockaddr_in *)item.udata;
          size_t bytes_read;
          // bzero(buf, sizeof(buf));
          memset(buf, 0, sizeof(buf));
          while ((bytes_read = read(item.ident, buf, sizeof(buf))) >= 0)
          {
            if (bytes_read == 0)
            {
              printf("client disconnected\n");
              EV_SET(&sk, item.ident, EVFILT_READ, EV_DELETE | EV_CLEAR, 0, 0, NULL);
              kevent(kq, &sk, 1, NULL, 0, NULL);
              close(item.ident);
              free(item.udata);
              break;
            }
            // print_ctime();
            char str[INET_ADDRSTRLEN];
            // printf("client[%s:%d]: %slength is %ld and byte read is %ld\n", inet_ntop(AF_INET, &cs->sin_addr, str, sizeof(str)), ntohs(cs->sin_port),
            //        buf, item.data, bytes_read);
            write(item.ident, buf, bytes_read);
          }
        }
      }
    }
  }
  close(listen_fd);
  close(kq);
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

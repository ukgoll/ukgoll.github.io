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
#include <fcntl.h>

#define ClientL 1024

void delete_kevent(struct kevent *kt, int kq);
void eexit(const char *str);
void print_ctime(const char *msg);
void set_non_blocking(int fd);

int main()
{
  signal(SIGPIPE, SIG_IGN);
  print_ctime("服务器启动");

  int kq = kqueue();
  if (kq == -1)
    eexit("创建 kqueue 失败");

  print_ctime("创建 socket");
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  int client_fd;
  if (listen_fd < 0)
    eexit("创建 socket 失败");

  set_non_blocking(listen_fd);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(6001);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  print_ctime("绑定端口");
  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    eexit("端口绑定失败");

  print_ctime("监听端口");
  if (listen(listen_fd, 10) < 0)
    eexit("监听失败");

  struct kevent sk;
  EV_SET(&sk, listen_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  if (kevent(kq, &sk, 1, NULL, 0, NULL) < 0)
    eexit("注册监听事件失败");

  struct kevent ck[ClientL];
  int ret;
  socklen_t client_len;
  print_ctime("开始 kqueue 事件循环");
  struct timespec timeout = {0, 1000000000}; // 100 毫秒
  char buf[256];

  for (;;)
  {
    ret = kevent(kq, NULL, 0, ck, ClientL, NULL);

    if (ret == -1)
    {
      if (errno == EINTR)
        continue; // 被信号打断，继续
      eexit("kevent 响应错误");
    }
    if (ret == 0)
    {
      // print_ctime("100 millseconds over time");
    }
    print_ctime("here is loop ret");
    for (int i = 0; i < ret; i++)
    {
      struct kevent item = ck[i];

      if ((item.ident == listen_fd) && (item.filter & EVFILT_READ))
      {
        struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
        if (!client_addr)
        {
          perror("malloc 失败");
          continue;
        }

        client_len = sizeof(*client_addr);
        while ((client_fd = accept(listen_fd, (struct sockaddr *)client_addr, &client_len)) > 0)
        {
          set_non_blocking(client_fd);
          EV_SET(&sk, client_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, client_addr);
          if (kevent(kq, &sk, 1, NULL, 0, NULL) < 0)
          {
            perror("添加客户端 fd 失败");
            close(client_fd);
            free(client_addr);
          }
          else
          {
            print_ctime("客户端连接成功");
            printf("客户端 IP: %s, 端口: %d\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
          }
        }

        if (client_fd < 0 && (errno != EAGAIN && errno != EWOULDBLOCK))
        {
          perror("accept 失败");
        }

        continue;
      }

      if (item.filter & EVFILT_READ)
      {
        struct sockaddr_in *cs = (struct sockaddr_in *)item.udata;
        print_ctime("读取客户端数据");
        size_t bytes_read;
        if (item.data < 0)
        {
          eexit("error on eof!!!");
        }
        printf("item.data %ld\n", item.data);
        while (1)
        {
          // 这里其实有隐患的，我们需要界定消息边界，不然如果客户端口一直发，我们就一直接受。
          bytes_read = read(item.ident, buf, sizeof(buf));
          if (bytes_read > 0)
          {
            // print_ctime("收到数据 %d\n", item.data);
            write(item.ident, buf, bytes_read); // 回显
          }
          else if (bytes_read == 0)
          {
            print_ctime("客户端断开连接 when read");
            delete_kevent(&item, kq);
            break;
          }
          else if (errno == EAGAIN || errno == EWOULDBLOCK)
          {
            break; // 读完了
          }
          else
          {
            perror("read 失败");
            break;
          }
        }
      }

      if (item.flags & EV_EOF) // 处理连接关闭
      {
        print_ctime("客户端断开连接 when EOF");
        printf("item ident %lu, flag is %d, udata is %p\n", item.ident, item.flags, item.udata);

        delete_kevent(&item, kq);
        continue;
      }
    }
  }

  close(listen_fd);
  close(kq);
  return 0;
}

void delete_kevent(struct kevent *kt, int kq)
{
  struct kevent sk;
  if (kt->udata == NULL)
    return;
  EV_SET(&sk, kt->ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
  kevent(kq, &sk, 1, NULL, 0, NULL);
  close(kt->ident);
  free(kt->udata);
  kt->udata = NULL;
}

void eexit(const char *str)
{
  perror(str);
  exit(EXIT_FAILURE);
}

void print_ctime(const char *msg)
{
  time_t t;
  struct tm *tm_info;

  time(&t);
  tm_info = localtime(&t);

  printf("[%02d-%02d-%02d %02d:%02d:%02d] %s\n",
         tm_info->tm_year + 1900,
         tm_info->tm_mon + 1,
         tm_info->tm_mday,
         tm_info->tm_hour,
         tm_info->tm_min,
         tm_info->tm_sec,
         msg);
}

void set_non_blocking(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
  {
    perror("fcntl F_GETFL 失败");
    return;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
  {
    perror("fcntl F_SETFL O_NONBLOCK 失败");
  }
  print_ctime("设置非阻塞成功");
  ;
}

/*
./nbtc 127.0.0.1 6001 < 2000.byte > /tmp/nbtc.outserver  0.05s user 0.20s system 54% cpu 0.473 total
./nbtc 127.0.0.1 6001 < 2000.byte > /tmp/nbtc.outnbs  0.05s user 0.20s system 95% cpu 0.259 total
*/

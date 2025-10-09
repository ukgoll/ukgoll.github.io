#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/poll.h>
#include <errno.h>

#include "utils.h"

#define POLLTIMEOUT 100

typedef struct sockaddr_in SAI;

void on_exit();

int main(int argc, char const *argv[])
{
  atexit(on_exit);
  if (argc != 3)
  {
    printf("usage ./nbtc <IP> <Port>");
    // 无需担心，进程结束之后会关闭当前进程的所有文件表项。
    exit(0);
  }
  int fd;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    eexit("socket create error");
  }
  SAI c_addr;
  int port = atoi(argv[2]);
  if (port == 0)
  {
    nexit("Invalid Port %s", argv[2]);
  }
  int an;
  if ((an = inet_pton(AF_INET, argv[1], &c_addr.sin_addr)) == -1)
  {
    eexit("Parse IP Address error");
  }
  else if (an == 0)
  {
    nexit("Invalid IP\n");
  }
  c_addr.sin_port = htons(port);
  c_addr.sin_family = AF_INET;
  int tcp_recv_buf, tcp_send_buf;
  socklen_t tcp_recv_buf_len = sizeof(tcp_recv_buf), tcp_send_buf_len = sizeof(tcp_send_buf);
  if (-1 == getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &tcp_recv_buf, &tcp_recv_buf_len))
  {
    eexit("get tcp recv buffer error");
  }
  if (-1 == getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &tcp_send_buf, &tcp_send_buf_len))
  {
    eexit("get tcp send buffer error");
  }
  tcp_recv_buf = 1024;
  tcp_send_buf = 512;
  printf("tcp recv buf %d, send buf %d\n", tcp_recv_buf, tcp_send_buf);
  char to[tcp_recv_buf], fr[tcp_send_buf];
  memset(to, 0, sizeof(to));
  memset(fr, 0, sizeof(fr));
  char *to_sptr, *to_eptr, *fr_sptr, *fr_eptr;
  to_sptr = to_eptr = to;
  fr_sptr = fr_eptr = fr;
  // 给我们需要处理的三个 IO 设置 no blocking
  if (set_no_blocking(STDIN_FILENO) == -1)
  {
    eexit("set stdin no blocking error");
  }
  if (set_no_blocking(fd) == -1)
  {
    eexit("set fd no blocking error");
  }
  if (set_no_blocking(STDOUT_FILENO) == -1)
  {
    eexit("set stdout no blocking error");
  }
  int cn;
  if (-1 == (cn = connect(fd, (struct sockaddr *)&c_addr, sizeof(c_addr))))
  {
    perror("blocking connect error is");
    if (errno != EINPROGRESS)
    {
      eexit("connect error");
    }
  }
  printf("cn is %d\n", cn);
  // 正式处理
  struct pollfd fds[3] = {0};
  fds[0].fd = STDIN_FILENO;
  fds[0].events = 0;

  fds[1].fd = fd;
  fds[1].events = 0;

  fds[2].fd = STDOUT_FILENO;
  fds[2].events = 0;
  int ret = 0, rn, wn, stdineof = 0, can_write_fd = 0, can_write_out = 0;
  fprintf(stderr, "start time %s\n", gf_time());
  for (;;)
  {
    // cn == -1 连接还没有建立
    can_write_fd = 0, can_write_out = 0;
    if (cn == -1)
    {
      // POLLWRITE | POLLIN, POLLWRITE 是正常连接 no-blocking-connect 之后的事件触发，POLLIN 是 server 不能连接的事件触发，也要处理
      fds[0].events = 0;
      fds[1].events = POLLOUT | POLLIN;
      fds[2].events = 0;
    }
    else
    {
      fds[0].events = 0;
      fds[1].events = 0;
      fds[2].events = 0;
      // 如果发送缓存数组没有满的话，打开 STDIN_FILENO 的 read
      if (stdineof == 0 && to_eptr < (to + tcp_recv_buf))
      {
        fds[0].events = POLLIN;
      }
      // 如果接收缓存数组没有满的话，打开 fd 的 read
      if (fr_eptr < (fr + tcp_send_buf))
      {
        fds[1].events = POLLIN;
      }
      // 如果 to 缓冲里面还有数据可以发送 打开 fd 的 write
      if (to_sptr != to_eptr)
      {
        fds[1].events |= POLLOUT;
      }
      // 如果 fr 缓冲里面还有数据可以发送 打开 STDOUT_FILENO 的 write
      if (fr_sptr != fr_eptr)
      {
        fds[2].events = POLLOUT;
      }
    }

    ret = poll(fds, 3, POLLTIMEOUT);
    // printf("ret is %d\n", ret);
    if (ret == -1)
    {
      if (EINTR != errno)
      {
        perror("poll loop error");
        break;
      }
    }
    else if (ret == 0)
    {
      // printf("%d millsecond timer over normal, next loop\n", POLLTIMEOUT);
    }
    else
    {
      if (fds[0].revents & POLLIN)
      {
        if ((rn = read(STDIN_FILENO, to_eptr, to + tcp_recv_buf - to_eptr)) == -1)
        {
          if (errno != EWOULDBLOCK)
          {
            fprintf(stderr, "errno %d\n", errno);
            eexit("read error from stdin\n");
          }
        }
        else if (rn == 0)
        {
          stdineof = 1;
          fprintf(stderr, "read eof from stdin\n");
          // to buf 的数据已经发送完毕，没有的话就继续发送
          if (to_sptr == to_eptr)
          {
            printf("close fd from stdin here\n");
            shutdown(fd, SHUT_WR);
          }
        }
        else
        {
          printf("%s read %d bytes from stdin\n", gf_time(), rn);
          to_eptr += rn;
          can_write_fd = 1;
        }
      }
      if (can_write_fd || (fds[1].revents & (POLLIN | POLLOUT)))
      {
        if (cn == -1)
        {
          int err;
          socklen_t len = sizeof(err);
          getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
          if (err == 0)
          {
            printf("connect() completed successfully.\n");
            cn = 0; // 连接成功
          }
          else
          {
            printf("connect() failed: %s\n", strerror(err));
            close(fd);
            exit(EXIT_FAILURE);
          }
        }
        else
        {
          if (fds[1].revents & POLLIN)
          {
            if ((rn = read(fd, fr_eptr, fr + tcp_send_buf - fr_eptr)) == -1)
            {
              if (errno != EWOULDBLOCK)
              {
                eexit("read error from fd");
              }
            }
            else if (rn == 0)
            {
              if (stdineof == 1)
              {
                printf("user close conn\n");
                break;
              }
              else
              {
                printf("server close conn\n");
                fds[1].fd = -1;
                close(fd);
                break;
              }
            }
            else
            {
              fr_eptr += rn;
              can_write_out = 1;
            }
          }
          else if (can_write_fd || (fds[1].revents & POLLOUT))
          {
            int count = to_eptr - to_sptr;
            printf("count is %d\n", count);
            if (count > 0)
            {
              if ((wn = write(fd, to_sptr, count)) < 0)
              {
                if (errno != EWOULDBLOCK)
                {
                  eexit("write to fd error");
                }
              }
              else
              {
                printf("write wn is %d\n", wn);
                to_sptr += wn;
                // 重置一下
                if (to_sptr == to_eptr)
                {
                  to_sptr = to_eptr = to;
                  if (stdineof)
                  {
                    printf("close fd from fd here\n");
                    shutdown(fd, SHUT_WR);
                  }
                }
              }
            }
          }
        }
      }
      if (can_write_out || fds[2].revents & POLLOUT)
      {
        int count = fr_eptr - fr_sptr;
        if (count <= 0)
          continue;
        ;
        if ((wn = write(STDOUT_FILENO, fr_sptr, count)) == -1)
        {
          if (errno != EWOULDBLOCK)
          {
            eexit("write error to stdout");
          }
        }
        else
        {
          // printf("%s read %d bytes from stdout\n", gf_time(), wn);
          fr_sptr += wn;
          // 这个缓冲区用完了。重置一下
          if (fr_sptr == fr_eptr)
          {
            fr_sptr = fr_eptr = fr;
          }
        }
      }
    }
  }
  //
  close(fd);
  fprintf(stderr, "end time %s\n", gf_time());
  return 0;
}

void on_exit()
{
  if (set_blocking(STDIN_FILENO) == -1)
  {
    eexit("set stdin blocking error");
  }
  if (set_blocking(STDOUT_FILENO) == -1)
  {
    eexit("set stdout blocking error");
  }
  printf("on_exit execute\n");
}

// 442

/*
./nbtc 127.0.0.1 6001 < 2000.byte > /tmp/a.txt  0.02s user 0.33s system 35% cpu 0.998 total
./nbtc 127.0.0.1 6001 < 2000.byte > /tmp/a.txt  0.06s user 0.42s system 32% cpu 1.494 total
./tcp_client < 2000.byte > /tmp/a.txt  0.12s user 0.14s system 16% cpu 1.495 total
*/

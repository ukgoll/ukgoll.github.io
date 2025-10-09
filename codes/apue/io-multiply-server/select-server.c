#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void eexit(char *str);

int main(int argc, char const *argv[])
{
  int sfd, cfd;
  if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    eexit("create server socket error");
  }
  struct sockaddr_in s_addr, c_addr;
  memset(&c_addr, 0, sizeof(c_addr));
  socklen_t client_len = sizeof(c_addr);
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(6001);
  s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
  {
    eexit("create server ");
  }
  if (listen(sfd, 256) < 0)
  {
    eexit("server listen fd error");
  }
  fd_set readfd, readfd_cache;
  FD_ZERO(&readfd);
  FD_ZERO(&readfd_cache);
  //
  FD_SET(sfd, &readfd);
  FD_SET(sfd, &readfd_cache);
  int maxfd = sfd + 1;
  printf("start select server\n");
  for (;;)
  {
    printf("reafd pointer before is %p\n", &readfd);
    readfd = readfd_cache;
    if (maxfd >= FD_SETSIZE)
    {
      eexit("file fd greater");
    }
    int ret = select(maxfd, &readfd, NULL, NULL, NULL);
    printf("reafd pointer after is %p\n", &readfd);
    printf("ret is %d\n", ret);
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
      perror("select error");
      break;
    }
    else
    {
      // 文件描述符号只会增大
      for (int i = sfd; i < maxfd && ret != 0; i++)
      {
        if (FD_ISSET(i, &readfd))
        {
          ret--;
          if (i == sfd)
          {
            if ((cfd = accept(sfd, (struct sockaddr *)&c_addr, &client_len)) < 0)
            {
              perror("accpet from client error");
              continue;
            }
            FD_SET(cfd, &readfd_cache);
            if (cfd >= maxfd)
            {
              maxfd = cfd + 1;
            }
            printf("connect establish\n");
          }
          else
          {
            char buf[256];
            ssize_t n = read(i, buf, sizeof(buf));
            if (n <= 0)
            { // 客户端断开
              printf("[Client %d] closed\n", i);
              if (i == maxfd - 1)
              {
                int new_maxfd = sfd + 1;
                for (int j = sfd + 1; j < maxfd; j++)
                {
                  if (FD_ISSET(j, &readfd_cache))
                  {
                    new_maxfd = j + 1;
                  }
                }
                maxfd = new_maxfd;
              }
              FD_CLR(i, &readfd_cache);
              close(i);
            }
            else
            {
              write(i, buf, n);
              printf("[Client %d]: %s and send echo\n", i, buf);
              shutdown(i, SHUT_RD);
            }
          }
        }
      }
    }
  }
  close(sfd);
  return 0;
}

void eexit(char *str)
{
  perror(str);
  exit(EXIT_FAILURE);
}

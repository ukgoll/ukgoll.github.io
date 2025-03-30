#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#define PollSize 1024

void eexit(char *str);
void print_ctime();

int main(int argc, char const *argv[])
{
  // signal(SIGPIPE, SIG_IGN);
  int sfd, cfd;
  if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    eexit("create server socket error");
  }
  struct sockaddr_in s_addr, c_addr;
  memset(&c_addr, 0, sizeof(c_addr));
  socklen_t client_len = sizeof(c_addr);
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(6001);
  s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sfd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0){
    eexit("create server ");
  }
  if(listen(sfd, 256) < 0){
    eexit("server listen fd error");
  }
  printf("start poll server\n");
  struct pollfd clientPoll[PollSize];
  for(int i=0;i<PollSize;i++){
    clientPoll[i].fd = -1;
  }
  clientPoll[0].fd = sfd;
  clientPoll[0].events = POLLIN;
  int nfds = 1;
  for(;;){
    int ret = poll(clientPoll, nfds, -1);
    print_ctime();
    printf("ret is %d\n", ret);
    if(ret == 0){
      printf("loop over time\n");
    }else if(ret == -1){
      if(errno == EINTR){
        continue;
      }
      perror("poll error");
      break;
    }else{
      if(clientPoll[0].revents & POLLIN){
        if((cfd = accept(sfd, (struct sockaddr *)&c_addr, &client_len)) < 0){
          perror("accpet from client error");
          continue;
        }
        int  i;
        for(i=1;i<PollSize;i++){
          if(clientPoll[i].fd == -1){
            clientPoll[i].fd = cfd;
            clientPoll[i].events = POLLIN;
            if (i >= nfds) nfds = i + 1;
            break;
          }
        }
        if(i == PollSize){
          printf("to many conn\n");
          close(cfd);  // 直接关闭新连接
        }
        printf("connect establish\n");
      }
      for(int i=1;i<nfds;i++){
        int tfd = clientPoll[i].fd;
        if(tfd == -1) continue;;
        if(clientPoll[i].revents & (POLLIN | POLLHUP)){
          char buf[256] = {0};
            ssize_t n = read(tfd, buf, sizeof(buf));
            if (n <= 0) {  // 客户端断开
              printf("[Client %d] closed\n", i);
              close(tfd);
              clientPoll[i].fd = -1;
            } else {
              write(tfd, buf, strlen(buf));
              printf("[Client %d]: %s and send echo\n", tfd, buf);
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


void print_ctime(){
  time_t t;
  struct tm *tm_info;

  time(&t);  // 获取当前时间（时间戳）
  tm_info = localtime(&t);  // 转换为本地时间

  printf("[%02d-%02d-%02d %02d:%02d:%02d]: ",
          tm_info->tm_year + 1900,  // 年份是从 1900 开始的
          tm_info->tm_mon + 1,      // 月份是从 0 开始的
          tm_info->tm_mday,
          tm_info->tm_hour,
          tm_info->tm_min,
          tm_info->tm_sec);
}

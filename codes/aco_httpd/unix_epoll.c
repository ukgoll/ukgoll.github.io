#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib/aco.h"
#include "poll.h"
#include "logger.h"

poll_t *get_loop(){
  static poll_t default_loop;
  return &default_loop;
}

int poll_init(poll_t *loop)
{
  loop->fd = epoll_create1(0);  // 创建 epoll 实例
  if(loop->fd == -1){
    log_Test("创建 epoll 实例失败: %s\n", strerror(errno));
    return -1;
  }
  // 创建主协程
  aco_t *main_co = aco_create(NULL, NULL, 0, NULL, NULL);
  if(main_co == NULL){
    log_Test("创建主协程失败\n");
    return -1;
  }
  loop->main_co = main_co;
  // 创建 share_stack
  aco_share_stack_t *s_stack = aco_share_stack_new(1 << 10);
  if(s_stack == NULL){
    log_Test("创建 share stack 失败\n");
    return -1;
  }
  loop->s_stack = s_stack;
  return 0;
}

int poll_add(poll_t *loop, int fd, epe pe, conn_t *c)
{
  struct epoll_event event;
  memset(&event, 0, sizeof(event));

  if(pe == eread){
    event.events = EPOLLIN | EPOLLET;  // 边缘触发
  }else if(pe == ewrite){
    event.events = EPOLLOUT | EPOLLET;
  }

  event.data.ptr = c;  // 存储连接指针，方便取回

  if(epoll_ctl(loop->fd, EPOLL_CTL_ADD, fd, &event) == -1){
    log_Test("fd %d 添加 epoll 失败: %s\n", fd, strerror(errno));
    return -1;
  }
  return 0;
}

int poll_delete(conn_t *conn)
{
  // 删除监听事件时，传最后一个参数为 NULL 即可
  if(epoll_ctl(get_loop()->fd, EPOLL_CTL_DEL, conn->fd, NULL) == -1){
    log_Test("fd %d 删除 epoll 失败: %s\n", conn->fd, strerror(errno));
    return -1;
  }
  free(conn->sa);
  close(conn->fd);
  return 0;
}

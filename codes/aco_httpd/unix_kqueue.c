#include <sys/types.h>
#include <sys/event.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include "lib/aco.h"
#include "poll.h"
#include "logger.h"



poll_t *get_loop(){
  static poll_t default_loop;
  return &default_loop;
}



int poll_init(poll_t *loop)
{
  loop->fd = kqueue();
  if(loop->fd == -1){
    log_Test("创建 kqueue 队列失败\n");
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
  struct kevent event;
  int filter = 0;
  if(pe == eread){
    filter |= EVFILT_READ;
  }else if(pe == ewrite){
    filter |= EVFILT_WRITE;
  }
  EV_SET(&event, fd, filter, EV_ADD, 0, 0, c);
  if(kevent(loop->fd, &event, 1, NULL, 0, NULL) == -1){
    log_Test("%d 添加 fd 失败\n", fd);
    return -1;
  }
  return 0;
}


int poll_delete(conn_t *conn)
{
  struct kevent del_ke;
  EV_SET(&del_ke, conn->fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
  if(kevent(get_loop()->fd, &del_ke, 1, NULL, 0, NULL) == -1){
    log_Test("%d 删除 fd 失败: %s\n", conn->fd, strerror(errno));
    return -1;
  }
  free(conn->sa);
  close(conn->fd);
  return 0;
}

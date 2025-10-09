#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>


#include "lib/aco.h"
#include "poll.h"
#include "logger.h"
#include "utils.h"


const char *resp =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 14\r\n"
    "\r\n"
    "Hello, world!\n";


void handle_accept(poll_t *loop, int server_fd);
void client_routine();
// 由于同一个时间内只有一个 routine 运行，所以我们使用全局 的 share stack，如果有问题，后续优化

#define PORT 6000


int main(int argc, char const *argv[])
{
  aco_thread_init(NULL);
  printf("A simple libaco http server at port %d\n", PORT);
  // 创建 loop 结构体
  poll_t *loop = get_loop();
  if(poll_init(loop) < 0){
    goto error;
  }
  //
  // 创建 socket
  int s_fd;
  if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    log_Test("创建 socket 失败\n");
    goto error;
  }
  if(set_non_blocking(s_fd) < 0){
    log_Test("设置 server no blocking error\n");
    goto error;
  }
  // 设置监听
  struct sockaddr_in server;
  server.sin_port = htons(PORT);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  //
  if(bind(s_fd, (struct sockaddr *)&server, sizeof(server)) < 0){
    log_Test("bind socket 失败\n");
    goto error;
  }
  if(listen(s_fd, 512) < 0){
    log_Test("listen socket 失败\n");
    goto error;
    return -1;
  }
  struct kevent evs[1024];
  // 设置每次循环超时
  struct timespec timeout;
  timeout.tv_sec = 0;
  timeout.tv_nsec = 100000000;
  // 添加 server accept 的 read
  struct kevent ke;
  EV_SET(&ke, s_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  if(kevent(loop->fd, &ke, 1, NULL, 0, NULL) == -1){
    log_Test("添加 server fd 失败");
    goto error;
  }
  for(;;)
  {
    int ret = kevent(loop->fd, NULL, 0, evs, 1024, &timeout);
    if(ret == -1){
      if(errno != EINTR){
        char buf[128];
        strerror_r(errno, buf, sizeof(buf));
        log_Test("kevent error: %s\n", buf);
      }
    }else{
      for(int i=0;i<ret;i++){
        struct kevent item = evs[i];
        // 处理 accept
        if(item.ident == s_fd && (item.filter & EVFILT_READ)){
          handle_accept(loop, s_fd);
        }else{
          if(item.filter & EVFILT_READ){
            conn_t *conn = (conn_t *)item.udata;
            aco_resume(conn->routine);
            if(conn->routine->is_end){
              aco_destroy(conn->routine);
              printf("客户端断开链接\n");
              free(conn);
            }
          }
        }
      }
    }
  }
  error:
  if(loop->s_stack){
    aco_share_stack_destroy(loop->s_stack);
  }
  aco_destroy(loop->main_co);
  return 0;
}


void handle_accept(poll_t *loop, int server_fd)
{
  int c_fd;
  socklen_t client_len;
  while (1)
  {
    struct sockaddr_in *client = malloc(sizeof(struct sockaddr_in));
    if(client == NULL){
      log_Test("malloc client addr error\n");
      return ;
    }
    conn_t *client_conn = malloc(sizeof(conn_t));
    if(client_conn == NULL){
      log_Test("malloc client conn error\n");
      break;
    }
    client_len = sizeof(*client);
    if((c_fd = accept(server_fd, (struct sockaddr *)client, &client_len)) < 0){
      if(errno != EAGAIN){
        log_Test("accept client conn error: %d, %s\n", errno, strerror(errno));
        free(client);
        free(client_conn);
        break;
      } else if(errno == EINTR){
        continue;
      } else{
        break;
      }
    }
    if(set_non_blocking(c_fd) < 0){
      log_Test("设置 client no blocking error\n");

      free(client_conn);
      break;
    }
    client_conn->fd = c_fd;
    client_conn->sa = client;
    // 创建 routine
    aco_t *cr = aco_create(loop->main_co, loop->s_stack, 0, client_routine, client_conn);
    if(cr == NULL){
        log_Test("create client routine error\n");
        free(client_conn);
        break;
    }
    client_conn->routine = cr;
    if(poll_add(loop, c_fd, eread, client_conn) < 0){
        log_Test("poll add client error\n");
    }
    log_Test("客户端链接成功\n");
  }
  return;
}


void client_routine()
{
  conn_t *conn = (conn_t *) aco_get_arg();
  ssize_t bytes_read;
  char buf[256];
  memset(buf, 0, sizeof(buf));
  while (1)
  {
    bytes_read = read(conn->fd, buf, sizeof(buf));
    // printf("bytes_read is %zd\n", bytes_read);
    if (bytes_read == 0)
    {
      poll_delete(conn);
      break;
    }else if(bytes_read == -1){
      if(errno == EAGAIN){
        ssize_t wn;
        if((wn = write(conn->fd, resp, strlen(resp))) <= 0){
          printf("writer http resp error\n");
          poll_delete(conn);
          break;
        }
        aco_yield();
        continue;
      }else if(errno == EINTR){
        continue;
      } else{
        poll_delete(conn);
        break;
      }
    }
    // ssize_t wn;
    // if((wn = write(conn->fd, buf, bytes_read)) <= 0){
    //     printf("writer error: %s, and wn is %zd\n", strerror(errno), wn);
    //     poll_delete(conn);
    //     break;
    // }
  }
  aco_exit();
}

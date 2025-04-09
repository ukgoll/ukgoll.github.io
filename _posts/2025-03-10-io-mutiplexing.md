---
title: "服务器开发 IO 多路复用"
date: 2025-03-19 09:00:00 +0800
categories: [IO, APUE]
tags: [IO, APUE]
---
# Preface
记录学习 Tcp 服务器开发，以及 IO 多路复用的一些常用技术。主要也是把一些常用的记录下来，方便写的时候方便查询，毕竟书本还是太厚了，不方便查询。

## Socket
socket api，也是套接字编程，服务器开发的入门基础，tcp 通信便是建立在 socket api 的基础上进行的通信。在学习的过程中，只是简单的考虑到 IPv4，不考虑 IPv6 的处理与兼容。

### IPv4 结构体
结构体通过 `#include <netinet/in.h>` 导入
```c
/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
  in_addr_t s_addr;
};
/*
 * Socket address, internet style.
 */
struct sockaddr_in {
  __uint8_t       sin_len;
  sa_family_t     sin_family;
  in_port_t       sin_port;
  struct  in_addr sin_addr;
  char            sin_zero[8];
};
```
在一般的时候，我们只需要处理
1. `sin_family` 表示使用什么协议进行交流，有如下的可以选择，一般选择 `AF_INET` 表示 IPv4 协议，`AF_LOCAL` 在 mysql 中就使用过，当本地服务器本地连接的时候，创建使用这个，不需要处理 **网络协议** 数据的各种包的头部文件，更加快速。
   ```shell
   | family   | 说明        |
   | -------- | ----------- |
   | AF_INET  | IPv4 协议   |
   | AF_INET6 | IPv6 协议   |
   | AF_LOCAL | Unix 域协议 |
   | AF_ROUTE | 路由套接字  |
   | AF_KEY   | 密钥套接字  |
   ```
2. `sin_port` 表示 socket 监听（server）或者连接（client）的端口。
3. `sin_addr` 表示 socket 的地址，对于 server 和 client 的定义不同。

其中还需要注意的就是，socket api 的那些函数 `bind`, `connect` 等，在接受各种不同协议的时候，考虑兼容性，需要转成 `(struct sockaddr *)`. `#include <sys/scoket.h>` 引入
```c
struct sockaddr_in server_addr;
/*
init addr
*/
bind(..., (struct sockaddr *)&server_addr, sizeof(server_addr));
```

### 值-结果参数
在 socket 函数中，在操作 `sockaddr_in` 的时候，基本是通过引用传入给函数的，同时还要通过 `sizeof` 函数传递这个结构体的长度，有时候要传长度变量的指针，分下面两种情况。
1. **进程到内核**: 有 `bind`, `connect`, `sendto`，上面就有一个 `bind` 的案例，这种情况下 `sizeof(server_addr)` 传递结构体的长度是为了 **告诉内核需要从进程复制多长的数据（已经通过 (`&server_addr`) 传递了地址）**
2. **内核到进程**: 有 `accpet`, `recvfrom`, `getsocketname`, `getpeername`
   ```c
   # 这个结合 accept 函数非常好理解

   socklen_t client_len;
   struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
   if (!client_addr) {
       eexit("malloc failed");
   }
   memset(client_addr, 0, sizeof(struct sockaddr_in));
   client_len = sizeof(*client_addr);
   if((client_fd = accept(listen_fd, (struct sockaddr *)client_addr, &client_len)) < 0){
     eexit("accept error");
   }
   ```
   在上面的代码中，`client_len` 刚开始存储了一个 **空的 struct sockaddr_in** 的大小，但是 `accept` 接受从客户端的连接之后，`client_addr` 的字段会存储客户端的信息，比如 `sin_port`, `sin_addr` 这些信息，内核需要修改 `client_len` 的大小来更新表示结构体的大小（这个概念我第一次看的时候没有看懂，觉得很懵逼，后面结合代码就比较好理解：**这个参数作为值传递给函数，在函数调用结束之后，会修改这个参数作为结果。因为要修改，所以一般都是指针**）
3. 在后面的 `select` IO 多路复用的情况下，也存在这个 **值-结果参数** ，后面代码一看就非常容易理解。



### 字节排序
在网络传输数据的时候，使用的 **大端序(big-endian)**
1. 小端序(little-endian): 低序号的字节的数据存在高内存
2. 大端序(big-endian): 高序号的字节的数据存在高内存，这是我们正常逻辑看到的数据处理。

如下代码就可以判断电脑是大端序还是小端序
```c
#include <stdio.h>
#include <stdint.h>

int main(int argc, char const *argv[])
{
  u_int16_t num = 0x1234;
  printf("num is %d, hex print is %#x\n", num, num);
  u_int8_t *le  = (u_int8_t *)&num;
  u_int8_t high_byte = *le++;
  u_int8_t low_byte = *le;
  printf("high byte num is %#x, and low byte num is %#x\n", high_byte, low_byte);
  if(low_byte == 0x34 && high_byte == 0x12){
    printf("big endian 大端序\n");
  }else{
    printf("little endian 小端序\n");
  }
  return 0;
}

```
编译运行
```shell
clang byteorder.c -o byteorder && ./byteorder
num is 4660, hex print is 0x1234
high byte num is 0x34, and low byte num is 0x12
little endian 小端序
```

书上也有一个判断大端序和小端序的，巧妙的应用了 `union` 访问同一片内存的原理，很有意思
```c
#include <stdio.h>
#include <stdint.h>

int main(int argc, char const *argv[])
{
  union
  {
      uint16_t s;
      uint8_t c[sizeof(uint16_t)];
  } bun;
  bun.s = 0x1234;
  if(bun.c[0] == 0x12 && bun.c[1] == 0x34){
    printf("big endian 大端序\n");
  }else if(bun.c[1] == 0x12 && bun.c[0] == 0x34){
    printf("big endian 小端序\n");
  }else{
    printf("unknown\n");
  }
  return 0;
}
```
#### 字节排序转换
在网络传输过程中，都是采用大端序，所以在网络字节序和主机字节序转换，`#include <netinet/in.h>` 导入，这四个函数很好记录的 `h` 代表 `host`，`n` 代表 `network`， `s` 16 位 短一点的是用于转换端口，`l` 32 位 长一点的是用于转换 ip，当然，只能转 32 位的 IPv4 了。
1. `htons()`
2. `ntohs()`
3. `htonl()`
4. `ntohl()`

同时也有一下， `#include <apra/inet.h>`，虽然不是为了处理字节序。

如果将 p 理解 print 显示的意思，n 理解 network 网络展示的意思，那么就很好记录了
1. `int		 inet_pton(int, const char *, void *)` 函数，一般是 `192.168.31.1` 这种字符串类型的 IP 地址绑定到 `struct sockaddr_in` 的 `struct in_addr` 地址上
2. `const char	*inet_ntop(int, const void *, char *, socklen_t)` 函数，和上面相反，将结构体的 IP 转成可读性高的形式，用户 client 的 ip 来源非常不错。





## IO 多路复用

### 客户端
客户端由如下代码编写，这个使用 `select` 处理了从 `stdin` 读取输入的时候，`server` 突然关闭的情况，运行绑定在本地的 `6001` 端口。（读取的数据也是就是考虑在 256 之内）

```c
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

int main(int argc, char const *argv[])
{
  int fd;
  if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    eexit("create client socket error");
  }
  struct sockaddr_in sdi;
  sdi.sin_port = htons(6001);
  sdi.sin_family = AF_INET;
  int ret = inet_pton(AF_INET, "127.0.0.1", &sdi.sin_addr);
  if(ret == 0){
    eexit("Invalid IP Address");
  }else if(ret == -1){
    eexit("Parse Error");
  }
  if(connect(fd, (struct sockaddr *)&sdi, sizeof(sdi)) < 0){
    eexit("connect to server error");
  }

  char recvline[256] = {0}, sendline[256] = {0};
  fd_set readfd;
  FD_ZERO(&readfd);
  int maxfd = fileno(stdin) + 1;
  if(fd >= maxfd){
    maxfd = fd + 1;
  }
  for(;;){
    FD_ZERO(&readfd);
    FD_SET(fileno(stdin), &readfd);
    FD_SET(fd, &readfd);
    int ret = select(maxfd, &readfd, NULL, NULL, NULL);
    if(ret == 0){
      printf("loop over time\n");
    }else if(ret == -1){
      if(errno == EINTR){
        continue;
      }
      perror("select error\n");
      break;
    }else{
      if(FD_ISSET(fd, &readfd)){
        int n = read(fd, recvline, 255);
        if(n == -1){
          eexit("recv error");
        }else if(n == 0){
          printf("connect close\n");
          break;
        }
        print_ctime();
        printf("[recv from server:]%s, length is %ld\n", recvline, strlen(recvline));

        /*
        如果是 测试脚本运行，那么直接发送接收一次直接直接结束
        */
       if(argc == 2 && strcmp(argv[1], "test") == 0){
        sleep(1);
        break;
       }
      }

      if(FD_ISSET(fileno(stdin), &readfd)){
        if(fgets(sendline, 255, stdin) != NULL){
          int len = strlen(sendline);
          sendline[len-1] = '\0';
          int n = write(fd, sendline, len);

          if(n <= 0){
            if(errno == EINTR) continue;
            perror("send error");
          }
        }
      }
    }
  }
  close(fd);
  return 0;
}
```

## 阻塞IO

### select 版本
使用 select 处理多路复用还是比较麻烦的，**最主要的 fd_set 的长度被限制在了 1024（基本上**。
编写的时候有一下需要注意的
1. 在使用 loop 循环处理 select 的时候，每次都需要重置 `read_fd_set`, `write_fd_set`, `except_fd_set`。
   > Note well: Upon return, each of the file descriptor sets is modified in place to indicate which file descriptors are currently "ready". Thus, if using select() within a loop, the sets must be reinitialized before each call.
2. 在每次接受新的连接或者关闭连接，更新 `maxfd`。

在学习介绍 **kqueue** 的文章，我们已经了解过，select 的一些缺点；在用户态和内核态切换太频繁，以及 `select` 在本身结构上，无法保存 client 和 server 的信息，使用起来比较麻烦，还得是 **kqueue** 以及 **epoll** 这种高级一点 IO 多路复用。

```c
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
  fd_set readfd, readfd_cache;
  FD_ZERO(&readfd);
  FD_ZERO(&readfd_cache);
  //
  FD_SET(sfd, &readfd);
  FD_SET(sfd, &readfd_cache);
  int maxfd = sfd + 1;
  printf("start select server\n");
  for(;;){
    readfd = readfd_cache;
    if(maxfd >= FD_SETSIZE){
      eexit("file fd greater");
    }
    int ret = select(maxfd, &readfd, NULL, NULL, NULL);
    printf("ret is %d\n", ret);
    if(ret == 0){
      printf("loop over time\n");
    }else if(ret == -1){
      if(errno == EINTR){
        continue;
      }
      perror("select error");
      break;
    }else{
      // 文件描述符号只会增大
      for(int i=sfd;i<maxfd&&ret!=0;i++){
        if(FD_ISSET(i, &readfd)){
          ret--;
          if(i == sfd){
            if((cfd = accept(sfd, (struct sockaddr *)&c_addr, &client_len)) < 0){
              perror("accpet from client error");
              continue;
            }
            FD_SET(cfd, &readfd_cache);
            if (cfd >= maxfd) {
              maxfd = cfd + 1;
            }
            printf("connect establish\n");
          }else{
            char buf[256] = {0};
            ssize_t n = read(i, buf, sizeof(buf));
            if (n <= 0) {  // 客户端断开
              printf("[Client %d] closed\n", i);
              if (i == maxfd - 1) {
                int new_maxfd = sfd + 1;
                for (int j = sfd + 1; j < maxfd; j++) {
                    if (FD_ISSET(j, &readfd_cache)) {
                      new_maxfd = j + 1;
                    }
                }
                maxfd = new_maxfd;
              }
              FD_CLR(i, &readfd_cache);
              close(i);
            } else {
              write(i, buf, strlen(buf));
              printf("[Client %d]: %s and send echo\n", i, buf);
            }
          }
        }
      }
    }
  }
  close(sfd);
  return 0;

}

```
### poll 版本

[The implementation of the fd_set arguments as value-result arguments is a design error that is avoided in poll(2) and epoll(7).](https://man7.org/linux/man-pages/man2/select.2.html)，从此可以知道，poll 的 fd 不是 `value-result` 值-结果参数。

（对于这个，我就感到非常疑惑，`value-result argument` 如果是一个参数既可以作为参数又可以作为结果返回，那么 **poll 的 fd** 参数也算，所以我个人只能理解为 `value-result` 改变参数，但是没有保留原来的状态。  

| Constant     | events | revents | Description                              |
| ------------ | ------ | ------- | ---------------------------------------- |
| `POLLIN`     | ✓      | ✓       | Normal or priority band data can be read |
| `POLLRDNORM` | ✓      | ✓       | Normal data can be read                  |
| `POLLRDBAND` | ✓      | ✓       | Priority band data can be read           |
| `POLLPRI`    | ✓      | ✓       | High-priority data can be read           |
| `POLLOUT`    | ✓      | ✓       | Normal data can be written               |
| `POLLWRNORM` | ✓      | ✓       | Normal data can be written               |
| `POLLWRBAND` | ✓      | ✓       | Priority band data can be written        |
| `POLLERR`    |        | ✓       | Error has occurred                       |
| `POLLHUP`    |        | ✓       | Hangup has occurred                      |
| `POLLNVAL`   |        | ✓       | Descriptor is not an open file           |


poll 版本的 TCP 服务器，修改核心部分就可以了，也就是多路复用的处理部分，`poll` 也不是很适用于网络编程，他的 size 初始化是固定，当然，我们可以判断是否扩容。


```c
struct pollfd {
  int     fd;
  short   events;
  short   revents;
};
```
1. 注册的事件一定要处理，如何注册了但是没有处理的话，会一直触发 `poll` 函数。
2. `POLLIN` 的描述虽然是 normal read 和 priority read，但是不等于 `POLLRDNORM | POLLRDBAND`。

```c

#define PollSize 1024

/*
...
*/

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
    perror("select error");
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
            write(tfd, buf, sizeof(buf));
            printf("[Client %d]: %s and send echo\n", tfd, buf);
          }
      }
    }
  }
}
```


## 非阻塞 IO
在默认的情况下， `socket` 是阻塞的，而在 **阻塞** 和 **非阻塞** 的两种情况下，**socket api** 的相应的函数表现的不一致。
由于历史原因，不同的系统设置不同的 `errno`，所以在 `Posix` 标准中，错误码 `EAGAIN` 和 `EWOULDBLOCK` 是一样的。

1. 输入的操作函数：`read`, `readv`, `recv`, `recvfrom`, `recvmsg` 函数；在 **blocking socket** 中，没有数据到来的时候，会一直休眠直到数据的到来（这也是我们前面代码一直使用处理的方式）。 UDP 是数据报协议，也是这样处理的；在 **no-blocing socket** 中，会立即执行完毕这些函数，不会休眠等待，如果不满足如下条件，设置 `error=EWOULDBLOCK`。
   1. 对于 TCP 的流，至少有一个字节的数据到达
   2. 对于 UDP 的数据报，一个完整的数据报到达。
2. 输出操作的函数：`write`, `writev`, `send`, `sendto`, `sendmsg`； TCP 协议中对于接收发送的缓冲区有大小设置的；对于 **blocking socket**，如果发送缓冲区满了，那么也会休眠直到有空间发送数据；对于 **no-blocking socket** 如果没有空间的话，立即返回，设置 `error=EWOULDBLOCK`。
   1. 对于 `Macos os`, `sysctl net.inet.tcp.recvspace` 获取接收缓冲大小；`sysctl net.inet.tcp.sendspace` 获取发送缓冲区大小，一般是 `131072 byte`，也就是 `128kb`。
   2. 对于 `Linux`, `sysctl net.ipv4.tcp_rmem` 获取接收缓冲大小, `131072 byte`；`sysctl net.ipv4.tcp_wmem` 获取发送缓冲区大小 `16384 byte`；
3. `accept` 函数：用于接收客户端连接的函数；对于 **blocking socket**，没有连接来的话直接休眠等待连接；对于 **no-blocking socket**，没有连接也是直接返回 `-1`，设置 `error=EWOULDBLOCK`。
4. `connect` 函数：对于 UDP 两种情况没有影响，因为是 UDP 没有连接的概念，没有三次握手建立连接的过程。对于 **no-blocking socket**，在等 client 发送一个 `SYN=q`（第一次握手） 到 server 的时候，内核会存储 IP 和 PORT，但是这个时候连接还没有建立成功，这个时候是在内核维护的队列中的 `in-complete-queue` 中，`connect` 返回（**三次握手还是在进行中**），设置 **error=EINPROGRESS**；在 server 发送 `ACK=q+1; SYN=k`（第二次握手） 之后，client 的 `connect` 成功返回（在 IO 多路复用中，**write 事件触发**），发送 `ACK=k+1`（第三次握手） 给服务器，进入到内核维护的 `complete-queue` 中，连接成功建立，可以被 `accept` 接收。
   1. `in-complete-queue` 和 `complete-queue` 是内核为每个 server socket 维护的， `int listen(int sockfd, int backlog);` 中的 `backlog` 一般是这两个队列的长度总和。
   2. 一般来说，在 `no-blocking socket` 中，connect 不会直接连接成功，会设置 `errno=EINPROGRESS` 并且立即返回，但是如果实在本机上连接可能会直接建立成功，所以要判断处理。

5. 对于 `read` 和 `write` 这样的 `slow system call` 还要处理中断的情况 `EINTR`，这个时候重新读取就好了。


**对于我现阶段的学习 socket 来说，绝不部分都是纸上谈兵，有很多很多的细节需要处理，1. 必须处理消息边界。2. 对于连接的处理的情况是有限的。**

最近发生的一系列事情无心再学习多路复用了，等再过上一段时间，将 `Unix-network-programming volume-1` 剩余的部分通读一下，我想尝试一下写一个 websocket 服务器。大概是在一个月之后了。

```c
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
}
```

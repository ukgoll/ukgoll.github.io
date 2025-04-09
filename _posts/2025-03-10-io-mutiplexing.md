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





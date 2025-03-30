#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/ip.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int socketfd, n;
  char recvline[257];  // 256 + 1，确保可以存 null 终止符
  struct sockaddr_in serveraddr;

  if (argc != 2) {
    std::cout << "usage: ./client <IP>" << std::endl;
    exit(1);
  }

  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "socket error" << std::endl;
    exit(1);
  }
  std::cout << "socketfd create success" << std::endl;

  bzero(&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(6001);

  if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
    std::cout << "server ip error" << std::endl;
    close(socketfd);
    exit(1);
  }
  std::cout << "server ip success" << std::endl;

  if (connect(socketfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    std::cout << "connect error" << std::endl;
    close(socketfd);
    exit(1);
  }
  std::cout << "connect success" << std::endl;

  // 发送数据
  strcpy(recvline, "Hello, Server!");
  send(socketfd, recvline, strlen(recvline), 0);

  std::cout << "send successfull start" << std::endl;
  sleep(1);
  std::cout << "send successfully end" << recvline << std::endl;
  while ((n = read(socketfd, recvline, 256)) > 0) {
    std::cout << "n is" << n << std::endl;
    if (n < 256) {
      recvline[n] = 0;  // 确保字符串终止
    } else {
      recvline[255] = 0;  // 防止溢出
    }

    if (fputs(recvline, stdout) == EOF) {
      std::cout << "fputs error" << std::endl;
      break;
    }
  }

  if (n == 0) {
    std::cout << "server closed connection" << std::endl;
  } else if (n < 0) {
    std::cout << "read error" << std::endl;
  }

  close(socketfd);
  std::cout << "close client success" << std::endl;
  return 0;
}

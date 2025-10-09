#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
  int err_txt_fd = open("./err.txt", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  if (err_txt_fd < 0)
  {
    printf("open err txt error %d\n", errno);
    return -1;
  }
  int out_txt_fd = open("./output.txt", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  if (out_txt_fd < 0)
  {
    printf("open out txt error %d\n", errno);
    return -1;
  }
  int in_txt_fd = open("input_fake.txt", O_RDONLY);
  if (in_txt_fd < 0)
  {
    printf("open in txt error %d\n", errno);
    return -1;
  }
  // 错误重定向到 err txt
  if (dup2(err_txt_fd, STDERR_FILENO) == -1)
  {
    printf("dup2 err error %d\n", errno);
    return -1;
  }
  // 输出重定向到 out txt
  if (dup2(out_txt_fd, STDOUT_FILENO) == -1)
  {
    printf("dup2 err error %d\n", errno);
    return -1;
  }
  if (dup2(in_txt_fd, STDIN_FILENO) == -1)
  {
    printf("dup2 in txt error %d\n", errno);
    return -1;
  }
  char buf[1024];
  ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
  if (n > 0)
  {
    buf[n] = '\0';
    printf("Read from stdin: %s", buf); // 这将写入 output.txt
  }
  // 将 out 重定向到 err
  if (dup2(STDERR_FILENO, STDOUT_FILENO) == -1)
  {
    printf("dup2 stderr to stdout error %d\n", errno);
    return -1;
  }
  dprintf(STDERR_FILENO, "test error\n");
  dprintf(STDOUT_FILENO, "test output\n");
  close(err_txt_fd);
  close(out_txt_fd);
  close(in_txt_fd);
  return 0;
}

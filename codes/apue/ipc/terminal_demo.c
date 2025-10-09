#include <stdio.h>
#include <unistd.h>

int main()
{
  // 清除整个屏幕
  printf("\x1b[2J");
  // 设置光标位置为第 1 行，第 0 列
  printf("\x1b[1;0H_____\x1b[1;3H");
  // 输出字符 'a'
  printf("a");

  fflush(stdout);
  sleep(1);

  // 删除字符 'a'，向左移动光标并覆盖它
  printf("\x1b[D\x1b[K"); // 左移一格并清除当前位置

  sleep(1);

  // 重新输出字符 'b'，替换之前的 'a'
  printf("\x1b[1;4H"); // 重新设置光标位置
  printf("b");

  fflush(stdout);
  sleep(1);

  return 0;
}

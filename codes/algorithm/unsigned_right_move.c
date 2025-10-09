#include <stdio.h>
#include <stdint.h>
#define unsign_right_move (x, y)((x &))

int main(int argc, char const *argv[])
{
  int32_t num = -3;
  printf("num is %d\n", num);
  uint32_t un_num = (uint32_t)num;
  printf("unsigned %d is %u\n", num, un_num);
  printf("%d unsigned right move %d is %u\n", num, 2, un_num >> 2);
  printf("%d signed right move %d is %d\n", num, 2, num >> 2);
  return 0;
}

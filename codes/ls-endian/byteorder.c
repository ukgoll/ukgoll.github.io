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

#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>

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


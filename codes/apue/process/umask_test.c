#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char const *argv[])
{
  mode_t mask = umask(0);
  printf("mask is %03o\n", mask);
  return 0;
}

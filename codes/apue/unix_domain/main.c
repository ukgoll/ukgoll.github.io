#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>

int main(int argc, char const *argv[])
{
  printf("temp directory is %s, second method get it is %s\n", P_tmpdir, getenv("TMPDIR"));
  return 0;
}

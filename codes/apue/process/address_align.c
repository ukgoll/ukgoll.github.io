#include <stdio.h>
#include <stdalign.h>
#include <unistd.h>

struct T1
{
  char s;
  int b;
} t1;

struct T2
{
  char s;
  double b;
} t2;

struct T3
{
  char s;
  int a;
  double b;
} t3;

struct T4
{
  char s;
  double b;
  int a;
} t4;

struct T5
{
  double b;
  char s;
  int a;
} t5;

#pragma pack(1)
struct T6
{
  char s;
  double b;
  int a;
} t6;
#pragma pack()

void print_basic_type_align();
void test_stack_de(int times);
int main(int argc, char const *argv[])
{
  printf("pid is %d\n", getpid());
  test_stack_de(3);
  printf("%lu, %lu, %lu, %lu, %lu, %lu\n", sizeof(t1), sizeof(t2),
         sizeof(t3), sizeof(t4), sizeof(t5), sizeof(t6));
  sleep(10);
  return 0;
}

void test_stack_de(int times)
{
  u_int64_t a = 1;
  printf("a address: %p|\n", &a);
  if (times > 0)
  {
    test_stack_de(times - 1);
  }
  else
  {
    printf("\n");
  }
}

void print_basic_type_align()
{
  printf("char align size is %ld\n", __alignof__(char));
  printf("short align size is %ld\n", __alignof__(short));
  printf("int align size is %ld\n", __alignof__(int));
  printf("float align size is %ld\n", __alignof__(float));
  printf("double align size is %ld\n", __alignof__(double));
  printf("long align size is %ld\n", __alignof__(long));
  printf("long long align size is %ld\n", __alignof__(long long));
}

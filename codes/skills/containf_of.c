#include <stdio.h>
#include <stddef.h> // for offsetof

// GCC/Clang 扩展的 typeof 宏
#define container_of(ptr, type, member) ({           \
  const typeof(((type *)0)->member) *__mptr = (ptr); \
  (type *)((char *)__mptr - offsetof(type, member)); \
})

// 定义一个示例结构体
struct Person
{
  int age;
  char name[20];
};

int main()
{
  struct Person p = {25, "Alice"};
  // 获取结构体成员的指针
  char *name_ptr = p.name;

  // 使用 container_of 从成员指针反推出结构体指针
  struct Person *p_ptr = container_of(name_ptr, struct Person, name);

  printf("Original struct address: %p\n", (void *)&p);
  printf("Recovered struct address from name_ptr: %p\n", (void *)p_ptr);

  // 验证是否一致
  if (p_ptr == &p)
  {
    printf("container_of worked correctly!\n");
    printf("Name is: %s, Age is: %d\n", p_ptr->name, p_ptr->age);
  }
  else
  {
    printf("container_of failed.\n");
  }

  return 0;
}

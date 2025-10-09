#include <iostream>


void reference_func(int &a);

void pointer_func(int *a);

int main(int argc, char const *argv[])
{
  int a = 0;
  std::cout << "a init value:" << a << std::endl;
  reference_func(a);
  std::cout << "a after reference_func invoke value:" << a << std::endl;
  pointer_func(&a);
  std::cout << "a after pointer_func invoke value:" << a << std::endl;
  return 0;
}

// 在汇编层面，两者的代码是一样的，说明在 reference 阶段他就穿了指针。
void reference_func(int &a)
{
  a = 10;
}

void pointer_func(int *a)
{
  *a = 100;
}

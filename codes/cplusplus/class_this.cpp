#include <iostream>


// 通过在 get_age 来两个断点，在 lldb 中分别打印 `p this` 和 `p &test`
// 这样就可以发现 this 指向的就是 test 这个对象的启始地址。
class Test{
  int age;
public:
  int get_age(){
    std::cout << this << std::endl;
    std::cout << this->age << std::endl;
    return age;
  };
};


int main(int argc, char const *argv[])
{
  Test test;
  test.get_age();
  std::cout << &test << std::endl;
  std::cout << "test.age is " << test.get_age() << std::endl;
  return 0;
}


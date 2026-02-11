#include <iostream>


// 通过在 get_age 来两个断点，在 lldb 中分别打印 `p this` 和 `p &test`
// 这样就可以发现 this 指向的就是 test 这个对象的启始地址。
// 添加普通的函数和 static 变量是不会增加实例的大小，如果有一个虚函数就是添加一个指针指向虚函数表
class Test{
  int age;
  // static const int height = 100;
  enum {height = 100};
  // double costs[height];
public:
  int get_age(){
    std::cout << this << std::endl;
    std::cout << this->age << std::endl;
    return age;
  };
  int get_height(){
    return this->height;
  }
private:
  virtual void vir_func() {}
  virtual void vir_func2() {};;
};


class TestInt{
  int age;
  public:
  int get_age(){
    std::cout << this << std::endl;
    std::cout << this->age << std::endl;
    return age;
  };
  private:
  virtual void vir_func() {}
  virtual void vir_func2() {};;
};


class __attribute__((packed)) TestIntNoPadding{
  int age;
  public:
  int get_age(){
    std::cout << this << std::endl;
    std::cout << this->age << std::endl;
    return age;
  };
  private:
  virtual void vir_func() {}
  virtual void vir_func2() {};;
};

int main(int argc, char const *argv[])
{
  Test test;
  test.get_age();
  std::cout << &test << std::endl;
  std::cout << "test.age is " << test.get_age() << std::endl;
  std::cout << "sizeof(test)" << sizeof(test) << std::endl;
  std::cout << "sizeof(int)" << sizeof(int) << "sizeof(void *)" << sizeof(void *) << std::endl;
  TestInt testInt;
  std::cout << "sizeof(testInt)" << sizeof(testInt) << std::endl;
  TestIntNoPadding testIntNoPadding;
  std::cout << "sizeof(testIntNoPadding)" << sizeof(testIntNoPadding) << std::endl;
  return 0;
}


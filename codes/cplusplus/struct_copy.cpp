#include <iostream>
#include <functional>
#include <string.h>

struct Person{
  char *name;
  int age;
  Person& operator=(const Person& other) {
        std::cout << "Copy assignment called\n";
        if (this != &other) {
            if(name != nullptr){
              std::cout << "name is nullptr" << std::endl;
              delete[] name;  // 释放旧资源
            }
            std::cout << "name is copy here" << std::endl;
            name = new char[strlen(other.name) + 1];
            strcpy(name, other.name);
        }
        return *this;
    }
};

void increment(int& n) {
    n++;
}

struct Person get_person()
{
  int age = 100;
  struct Person p = {.name= "vzgoll", .age=age};
  return p;
}


struct Person get_person2()
{
  char local[] = "vzgoll";
  int age = -100;
  struct Person p = {.name = local,.age=age};
  return p;  // ❌ 返回局部变量的地址，未定义行为
}

struct Person get_person3()
{
  char *local = new char[13];
  strncpy(local, "local person3", 13);
  struct Person p = {.name = local};
  return p;  // ❌ 返回局部变量的地址，未定义行为
}


struct Qt
{
  struct Person *p;
};

void set_person_pointer(struct Qt *q)
{
  struct Person p = {.name= "vzgoll", .age=10};
  q->p = &p;
  std::cout << "q person is" << q->p->name << std::endl;
}

int main(int argc, char const *argv[])
{
  struct Person p = get_person();
  std::cout << "persion1 is " << p.name << "age is " << p.age << std::endl;
  struct Person p2 = get_person2();
  std::cout << "persion2 is " << p2.name << "age is " << p2.age << std::endl;
  // try{
  //   delete p2.name;
  // }catch(...){
  //   std::cout << "delete stack varaiable" << std::endl;
  // }
  struct Person p3 = get_person3();
  std::cout << "persion3 is " << p3.name << "age is " << p3.age << std::endl;
  struct Person p4;
  p4.name = nullptr;
  p4.age = 400;
  p4 = p3;
  std::cout << "persion4 is " << p4.name << "age is " << p4.age << std::endl;
  delete[] p3.name;
  // strncpy(p3.name, "local personq", 13);
  std::cout << "persion3 is " << p3.name << "age is " << p3.age << std::endl;
  std::cout << "persion4 is " << p4.name << "age is " << p4.age << std::endl;
  int x = 10;
  increment(x);
  std::cout << "x is" << x << std::endl;
  auto f1 = std::bind(increment, x);
  f1();
  std::cout << "x after f1: " << x << std::endl;  // x 还是 10

  // 用 std::ref 包装 x，传引用
  auto f2 = std::bind(increment, std::ref(x));
  f2();
  std::cout << "x after f2: " << x << std::endl;  // x 变成 11
  struct Qt q;
  set_person_pointer(&q);
  std::cout << "q person is " << q.p->name << std::endl;
  return 0;
}

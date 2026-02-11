#include <iostream>
using namespace std;

int external_main_age = 100;
void child_print();
void update(int age);

void child_print(){
  std::cout << "main child parnt" << std::endl;
}


int main(int argc, char const *argv[])
{
  std::cout << "external_main_age: " << external_main_age << std::endl;
  update(10);
  std::cout << "external_main_age: " << external_main_age << std::endl;
  update(-20);
  std::cout << "external_main_age: " << external_main_age << std::endl;
  return 0;
}

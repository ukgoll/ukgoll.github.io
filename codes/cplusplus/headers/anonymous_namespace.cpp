
#include <iostream>
#include "anonymous_namespace.h"
using namespace std;

namespace
{
  int internal_age = 100;
} // namespace

namespace ANameSpace
{
  int age = 20;
  void print(){
    std::cout << "internal_age:" << internal_age << ", and my age is:" << ANameSpace::age << std::endl;
  }
} // namespace ANameSpace


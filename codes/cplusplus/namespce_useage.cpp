#include <iostream>
#include "headers/anonymous_namespace.h"

int main(int argc, char const *argv[])
{
  extern int externalAge;
  std::cout << "externalAge is: " << externalAge << std::endl;
  externalAge = 100;
  std::cout << "externalAge is: " << externalAge << std::endl;
  ANameSpace::print();
  ANameSpace::age = 30;
  ANameSpace::print();
  return 0;
}

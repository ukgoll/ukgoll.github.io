#include <iostream>
#include "headers/anonymous_namespace.h"

int main(int argc, char const *argv[])
{
  ANameSpace::print();
  ANameSpace::age = 30;
  ANameSpace::print();
  return 0;
}

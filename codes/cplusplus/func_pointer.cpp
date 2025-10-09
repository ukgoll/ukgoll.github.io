#include <iostream>


int pam(int x)
{
  return x * x;
}

int main(int argc, char const *argv[])
{
  int (*pf)(int);
  pf = pam;
  std::cout << pam(10) << "|" << (*pf)(10) << "|" << pf(10) << std::endl;
  return 0;
}

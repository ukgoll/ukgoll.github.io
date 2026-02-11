#include <iostream>
using namespace std;

class NoVTableClass
{
private:
int num;
  /* data */
public:
  NoVTableClass(/* args */);
  ~NoVTableClass();
};

NoVTableClass::NoVTableClass(/* args */)
{
}

NoVTableClass::~NoVTableClass()
{
}

class VTableClass
{
private:
virtual void virtual_func(){};
double num;
  /* data */
public:
  VTableClass(/* args */);
  ~VTableClass();
};


VTableClass::VTableClass(/* args */)
{
}

VTableClass::~VTableClass()
{
}

int main(int argc, char const *argv[])
{
  VTableClass vtc = VTableClass();
  NoVTableClass nvtc = NoVTableClass();
  std::cout << sizeof(vtc) << std::endl;
  std::cout << sizeof(nvtc) << std::endl;
  return 0;
}

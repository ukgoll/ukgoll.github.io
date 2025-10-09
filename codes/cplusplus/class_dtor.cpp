#include <iostream>


class Base
{
public:
  virtual ~Base(){
    std::cout << "base dtor" << std::endl;
    std::cout << "--" << std::endl;
  }
};


class Derived: public Base
{
public:
  ~Derived(){
    std::cout << "derived dtor" << std::endl;
  }
};


int main(int argc, char const *argv[])
{
  // Derived de = Derived();
  // Base be = Derived();
  // Derived be2;
  // std::cout << "be.a is:" << be.a << std::endl;
  Base *be = new Derived();
  delete be;
  Derived *de = new Derived();
  delete de;
  return 0;
}

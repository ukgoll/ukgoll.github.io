#include <iostream>
#include <typeinfo>
using namespace std;


class InitClass
{

};

int main(int argc, char const *argv[])
{
  int a = 10;
  InitClass ic;
  cout << "ic type name is:" << typeid(ic).name() << ", and a type name is:" << typeid(a).name() << endl;
  return 0;
}

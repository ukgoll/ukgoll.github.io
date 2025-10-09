#include <iostream>

using namespace std;


class Base
{
public:
  virtual string  get_name() {
    return "get base name";
  };
  virtual uint32_t get_age() = 0;
  virtual ~Base()=default;
};


class Derived: public Base
{
public:
  string get_name() override;
  virtual uint32_t get_age() override;
};


string Derived::get_name()
{
  return "get derived name";
}


uint32_t Derived::get_age()
{
  return 19;
}

int main(int argc, char const *argv[])
{
  cout << __cplusplus << endl;
  Base *bs = new Derived();
  cout << bs->get_name() << endl;
  Derived de = Derived();
  Base &bsr = de;
  cout << bsr.get_name() << endl;
  delete bs;
  return 0;
}

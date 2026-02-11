#include <string>
#include <iostream>
using namespace std;

class OpeartorClass{
private:
  int age;
  int height;
  std::string name;

public:
  OpeartorClass(int age, int height): age(age), height(height) {
  };
  OpeartorClass(int age, int height, std::string name): age(age), height(height), name(name) {
  };
  OpeartorClass operator+(const OpeartorClass &other){
    return OpeartorClass(age+other.age, height+other.height, name+other.name);
  }
  OpeartorClass operator+(const OpeartorClass *other){
    return OpeartorClass(age+other->age, height+other->height, name+other->name);
  }
  friend ostream& operator<<(ostream &out, const OpeartorClass &obj);
  friend ostream& operator<<(ostream &out, const OpeartorClass *obj);
};

ostream& operator<<(ostream &out, const OpeartorClass &obj){
  out << "name is " << obj.name << "; age is " << obj.age << "; height is " << obj.height;
  return out;
}

ostream& operator<<(ostream &out, const OpeartorClass *obj){
  if(obj != nullptr) out << *obj;
  else out << "nullptr";
  return out;
}


int main(int argc, char const *argv[])
{
  OpeartorClass *op1 = new OpeartorClass(24, 168, "vzgoll");
  OpeartorClass *op2 = new OpeartorClass(39, 150, "da uk ge");
  std::cout << "op1 is " << op1 << std::endl;
  std::cout << "op2 is " << op2 << std::endl;
  OpeartorClass op3 = *op1 + *op2;
  std::cout << "op3 is " << op3 << std::endl;
  OpeartorClass *op5 = nullptr;
  std::cout << "op5 is " << op5 << std::endl;
  delete op1;
  delete op2;
  return 0;
}


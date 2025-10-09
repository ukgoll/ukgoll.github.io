#include <iostream>

using namespace std;


class Pointer
{
public:
  Pointer(int x, int y);
private:
  int x;
  int y;

friend bool operator==(const Pointer &p1, const Pointer &p2);
friend ostream& operator<<(ostream &os, const Pointer &p);
// 同时存在的话，那么就是优先调用 class 内部的。
// public:
// bool operator==(const Pointer &other)
// {
//  return x == other.x && y == other.y;
// }
};

Pointer::Pointer(int x, int y): x(x), y(y)
{

}


bool operator==(const Pointer &p1, const Pointer &p2)
{
  cout << "invoke friend operator ==" << endl;
  return p1.x == p2.x && p1.y == p2.y;
}

ostream& operator<<(ostream&os, const Pointer&p)
{
  os << "Pointer x is:" << p.x << ", Pointer y is:" << p.y;
  return os;
}

int main(int argc, char const *argv[])
{
  Pointer p1(3, 20);
  Pointer p2(2, 20);
  cout << p1 << endl << p2 << endl;
  if(p1 == p2)
  {
    cout << "p1 is equal p2" << endl;
  }else{
    cout << "p1 is not equal p2" << endl;
  }
  return 0;
}

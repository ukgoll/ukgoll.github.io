#include <iostream>
using namespace std;


/**
 * 这个程序能很好的演示 ref 的作用，一个变量就是一个盒子，而 ref 就是盒子的内容(所以说不管如何操作 ref，变量盒子本身不会变)
 * 通过阅读 pointer_ref 代码和 static_p 变量能很好的理解。
 */
int *static_p;

void normal_ref(int& p)
{
  p += 10;
}
void pointer_ref(int * &p)
{
  *p += 100;
  static_p = p;
  int *qq = new int(1010);
  p = qq;
}


int main(int argc, char const *argv[])
{
  int x = 10;
  cout << "before x is " << x << endl;
  normal_ref(x);
  cout << "after x is " << x << endl;
  int *y = new int(101);
  int* &z = y;
  cout << "before y is:" << &y << ", value is:" << *y << "z address:" << z << *z << endl;
  pointer_ref(y);
  cout << "static pointer is:" << static_p << ", value is:" << *static_p << endl;
  cout << "after y is:" << &y << ", value is:" <<  *y << "z address:" << z << *z << endl;
  delete y;
  if(static_p != nullptr) delete static_p;
  return 0;
}

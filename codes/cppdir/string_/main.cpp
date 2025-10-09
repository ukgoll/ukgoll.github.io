#include <iostream>
#include <cstring>
using namespace std;

// sizeof 编译时候就会执行，会包括 null terminate `\0` 的长度
// strlen 运行时执行，不会包括 `\0` 的长度
int main(int argc, char const *argv[])
{
  char s[] = "test string";
  char t = "S";
  cout << "sizeof(s):" << sizeof(s) << "\nstrlen(s):" << strlen(s) << endl;
  return 0;
}

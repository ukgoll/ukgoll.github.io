#include <iostream>
#include <string>
using namespace std;


/**
 * 对于字符串而言，不同的存储方式对于 size 的结果是十分不同的。
 */


int main(int argc, char const *argv[])
{
  string name = "vzoll";
  cout << "name is " << name << endl;
  name += " so handsome!";
  cout << "name is " << name << endl;
  cout << "max length of string is " << string::npos << " and negative is " << (int64_t)string::npos << endl;
  cout << "name find q is " << name.find('q') << "find l is " << name.find("l") << endl;
  string repeat_n(10, 'n');
  cout << "repeat_n is " << repeat_n << endl;
  string cn_name = "大聪明";
  u16string cn16_name = u"大聪明";
  cout << "cn name size is " << cn_name.size() << "\ncn16 name size is " << cn16_name.size() << endl;
  cout << "cn name size is " << cn_name.size() << "\ncn16 name size is " << cn16_name.size() * sizeof(char16_t) << endl;
  return 0;
}


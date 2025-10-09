#include <iostream>
#include <string>
using namespace std;


int main(int argc, char const *argv[])
{
  string t = {"test string"};
  std::cout << __cplusplus << "\n";
  cout << t.length() << endl;
  std::cout << "object address: "
              << static_cast<const void*>(&t) << '\n';
  t += "argc";
  cout << t.length() << endl;
  std::cout << "object address: "
              << static_cast<const void*>(&t) << '\n';
  cout << string::npos << endl;
  return 0;
}

/**
 * incorporate 组合
 * recipe 食谱
 * nonzero
 * reinforce 强调，突出
 * bedevil 魔鬼
 */

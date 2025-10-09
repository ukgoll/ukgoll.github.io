#include <iostream>
using namespace std;

namespace TT
{
  int tt_age = 100;
} // namespace TT


int main(int argc, char const *argv[])
{
#ifdef TEST
  std::cout << "define marcon TEST" << std::endl;
#endif
#if TEST
  std::cout << "define marcon TEST 1" << std::endl;
#endif
  int a = 10;
  {
    int a = 0;
    std::cout << "a is:" << a << std::endl;
  }
  std::cout << "a is:" << a << std::endl;
  std::cout << "tt age is:" << TT::tt_age << std::endl;
  return 0;
}

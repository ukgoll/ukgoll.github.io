#include <iostream>
#include <memory>
using namespace std;

class TestUniquePtr
{
public:
  ~TestUniquePtr();
};



TestUniquePtr::~TestUniquePtr()
{
  cout << "delete test unique ptr obj" << endl;
}

void unique_ptr_use()
{
  unique_ptr<TestUniquePtr> ua = make_unique<TestUniquePtr>();
  cout << ua << endl;
}

int main(int argc, char const *argv[])
{
  unique_ptr_use();
  unique_ptr<TestUniquePtr> uu = make_unique<TestUniquePtr>();
  cout << "uu is:" << uu << endl;
  // unique_ptr<TestUniquePtr> ua = uu;  // 这行是错误的。 unique ptr 只能 std::move，不能 copy
  unique_ptr<TestUniquePtr> ut = std::move(uu);
  cout << "ut is:" << ut << ", and uu is:" << uu << endl; // 到这里 uu 已经被移动了，所以 地址为 0x00
  if(uu == nullptr)
  {
    cout << "uu is moved to ut" << endl;
  }
  return 0;
}

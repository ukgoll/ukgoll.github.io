#include <iostream>
using namespace std;


/**
 * 这里的代码就能很好的说明 class 作为参数为啥不要传 value ，
 * class 会 copy 一次是一个问题，
 * 另外一个问题是默认的 copy 是浅拷贝，指针指向同一片区域，但是当 临时的 class 释放的时候，把正主的 指针也释放了（因为指向的是同一个地方。
 */
class TestCopyDelete
{
public:
  int *x;
  ~TestCopyDelete();
};

TestCopyDelete::~TestCopyDelete()
{
  if(x){
    cout << "free x" << endl;
    delete x;
  }
}

class TestCopyDeleteWithDeepCopy
{
public:
  int *x;
  TestCopyDeleteWithDeepCopy() = default;
  TestCopyDeleteWithDeepCopy(const TestCopyDeleteWithDeepCopy&other);
  ~TestCopyDeleteWithDeepCopy();

};


TestCopyDeleteWithDeepCopy::TestCopyDeleteWithDeepCopy(const TestCopyDeleteWithDeepCopy&other)
{
  x = new int();
  if(other.x != nullptr)
  {
    cout << "deep copy other x is:" << *other.x << endl;
    *x = *other.x;
  }
}


TestCopyDeleteWithDeepCopy::~TestCopyDeleteWithDeepCopy()
{
  if(x){
    cout << "free x" << endl;
    delete x;
  }
}


void test_value_copy(TestCopyDelete tp)
{
  *(tp.x) += 100;
}

void test_value_copy_in_deep(TestCopyDeleteWithDeepCopy tp)
{
  *(tp.x) += 100;
}


void test_ref_copy(TestCopyDelete &tp)
{
  *(tp.x) += 100;
}

int main(int argc, char const *argv[])
{
  TestCopyDelete t;
  t.x = new int(101);
  cout << "t.x addresss is:" << t.x << ", and value is:" << *(t.x) << endl;
  // test_value_copy(t); // 传 tp 值拷贝，x 的地址拷贝过去了，当 tp 被释放的时候，x 也被释放了，当 main 结束的时候就会 double free
  test_ref_copy(t); // 传 tp referece，不会浅宝贝， tp 变量不会构建一个新的 class，x 不会 被释放
  cout << "t.x addresss is:" << t.x << ", and value is:" << *(t.x) << endl;
  ///

  TestCopyDeleteWithDeepCopy td;
  td.x = new int(1010);
  cout << "td.x addresss is:" << td.x << ", and value is:" << *(td.x) << endl;
  test_value_copy_in_deep(td); // 传 tp 值拷贝，x 的值，拷贝过去了，是 deep copy，会 free 掉 tp 的 x，但是不会 double free td 的 x
  cout << "td.x addresss is:" << td.x << ", and value is:" << *(td.x) << endl;
  // test_ref_copy(t); // 传 tp referece，不会浅宝贝， tp 变量不会构建一个新的 class，x 不会 被释放
  return 0;
}

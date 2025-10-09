#include <iostream>
#include <memory>
using namespace std;


class PtrTest
{
public:
  PtrTest();
  ~PtrTest();
};

PtrTest::PtrTest()
{
  cout << "ptr test create" << endl;
}

PtrTest::~PtrTest()
{
  cout << "ptr test delete" << endl;
}


void test_ptr()
{
  // PtrTest *pt = new PtrTest(); // 这个就会泄漏
  unique_ptr<PtrTest> pt = make_unique<PtrTest>(); // c++14
  shared_ptr<PtrTest> ps = make_shared<PtrTest>(); // c++14
  // unique_ptr<PtrTest> pt(new PtrTest());
  // shared_ptr<PtrTest> ps(new PtrTest());
}
struct SMT
{
  shared_ptr<PtrTest> t;
};

void pass_ptr(SMT *smt)
{
  shared_ptr<PtrTest> ps = make_shared<PtrTest>(); // c++14
  smt->t = ps;
}

void test_count_add(shared_ptr<PtrTest> spt)
{
  cout << "test count add" << spt.use_count() << endl;
}

void test_count_add_ref(shared_ptr<PtrTest> &spt)
{
  cout << "test count add ref" << spt.use_count() << endl;
}
int main(int argc, char const *argv[])
{
  // 这一段函数可以看到智能指针的自动释放 start
  // test_ptr();
  // 这一段函数可以看到智能指针的自动释放 end
  // 这一段函数可以看到智能指针的引用记数 start
  struct SMT *smt = new SMT();
  struct SMT *smt2 = new SMT();
  pass_ptr(smt);
  cout << smt->t.use_count() << endl;
  smt2->t = smt->t;
  cout << smt->t.use_count() << endl;
  test_count_add(smt->t);  // 这里函数构建了一个 spt 临时变量 ，这样 count 也加了
  cout << smt->t.use_count() << endl;
  test_count_add_ref(smt->t);  // 这里是 ref 没有构建 spt 的临时变量
  cout << smt->t.use_count() << endl;
  delete smt;
  cout << smt2->t.use_count() << endl;
  delete smt2;
  // 这一段函数可以看到智能指针的引用记数 end
  return 0;
}


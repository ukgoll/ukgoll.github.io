#include <iostream>
extern int external_main_age;


static void child_print(){
  using std::cout;
  using std::endl;
  cout << "sub child parnt" << endl;
}


void update(int age){
  external_main_age += age;
  child_print();
}


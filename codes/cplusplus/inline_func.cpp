// file: inline_add.cpp
#include <iostream>

inline int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 20;
    int z = add(x, y);

    int z2 = add(x, y);
    int z3 = add(x, y);
    int z4 = add(x, y);
    int z5 = add(x, y);
    std::cout << "z = " << z << std::endl;
    return 0;
}

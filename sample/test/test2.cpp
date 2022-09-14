#include <iostream>
#include <typeinfo>

struct A1 {};
struct B1 : A1 {};

struct A2 { virtual ~A2() {} };
struct B2 : A2 {};

int main() {
    B1 b1;
    A1& a1 = b1;
    A1* pa1 = &a1;

    B2 b2;
    A2& a2 = b2;
    A2* pa2 = &a2;

    std::cout << "[non-polymorphic] " << typeid(a1).name() // 2A1
              << " " << typeid(b1).name()                  // 2B1
              << " " << typeid(pa1).name()                 // P2A1
              << "\n[polymorphic] " << typeid(a2).name()   // 2B2
              << " " << typeid(b2).name()                  // 2B2
              << " " << typeid(pa2).name()                 // P2A2
              << "\n";

    return 0;
}


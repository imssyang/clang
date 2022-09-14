#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

struct A {
    virtual ~A() {}
};

struct B : A {};
struct C : A {};

int main() {
    std::unordered_map<std::type_index, std::string> type_names;

    type_names[std::type_index(typeid(int))] = "int";
    type_names[std::type_index(typeid(double))] = "double";
    type_names[std::type_index(typeid(A))] = "A";
    type_names[std::type_index(typeid(B))] = "B";
    type_names[std::type_index(typeid(C))] = "C";

    int i;
    double d;
    A a;
    std::unique_ptr<A> b(new B);
    std::unique_ptr<A> c(new C);

    std::cout << type_names[std::type_index(typeid(i))] << '\n';  // int
    std::cout << type_names[std::type_index(typeid(d))] << '\n';  // double
    std::cout << type_names[std::type_index(typeid(a))] << '\n';  // A
    std::cout << type_names[std::type_index(typeid(*b))] << '\n'; // B
    std::cout << type_names[std::type_index(typeid(*c))] << '\n'; // C

    return 0;
}
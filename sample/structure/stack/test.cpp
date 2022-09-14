#include "ArrayStack.hpp"
#include "LinkedStack.hpp"

int main() {
    std::cout << "\n-----------ArrayStack-----------\n";
    ArrayStack<int> as;
    as.push(3);
    as.push(5);
    as.push(2);
    as.push(4);
    as.push(1);
    as.print();
    std::cout << as.top() << std::endl;
    while (!as.isEmpty()) {
        std::cout << as.pop() << "-";
    }

    std::cout << "\n-----------LinkedStack-----------\n";
    LinkedStack<int> ls;
    ls.push(3);
    ls.push(5);
    ls.push(2);
    ls.push(4);
    ls.push(1);
    ls.print();
    std::cout << ls.top() << std::endl;
    while (!ls.isEmpty()) {
        std::cout << ls.pop() << "-";
    }

    return 0;
}


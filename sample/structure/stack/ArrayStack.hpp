/**
 * Stack:
 *
 *   (Last In) push -> E -> pop (First Out)
 *                     D
 *                     C
 *                     B
 *                     A
 */

#ifndef _ARRAY_STACK_
#define _ARRAY_STACK_

#include <iostream>
#include <vector>

template<typename T, int capacity = 30>
class ArrayStack {
public:
    ArrayStack() {
        pool.reserve(capacity);
    }
    void clear() {
        pool.clear();
    }
    bool isEmpty() const {
        return pool.empty();
    }
    T& top() {
        return pool.back();
    }
    T pop() {
        T el = pool.back();
        pool.pop_back();
        return el;
    }
    void push(const T& el) {
        pool.push_back(el);
    }
    void print() const {
        for (int i = 0; i < pool.size(); i++) {
            std::cout << pool[i] << " ";
        }
        std::cout << std::endl;
    }

private:
    std::vector<T> pool;
};

#endif // _ARRAY_STACK_

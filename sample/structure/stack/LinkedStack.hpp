/**
 * generic stack defined as a doubly linked list
 */

#ifndef _LINKED_STACK_
#define _LINKED_STACK_

#include <iostream>
#include <list>

template<typename T>
class LinkedStack {
public:
    LinkedStack() {
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
        typename std::list<T>::const_iterator iter = pool.begin();
        for (; iter != pool.end(); iter++) {
            std::cout << *iter << " ";
        }
        std::cout << std::endl;
    }

private:
    std::list<T> pool;
};

#endif // _LINKED_STACK_

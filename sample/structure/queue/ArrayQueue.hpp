/**
 * Queue:
 *
 *       IN -> A B C D E F G H I J -> OUT
 *  (Last In Last Out)   (First In First Out)
 */

#ifndef _ARRAY_QUEUE_
#define _ARRAY_QUEUE_

#include <iostream>

template<typename T, int size = 100>
class ArrayQueue {
public:
    ArrayQueue() {
        first = last = -1;
    }
    void enqueue(const T&);
    T dequeue();
    bool isFull() {
        return first == 0 && last == size - 1 || first == last + 1;
    }
    bool isEmpty() {
        return first == -1;
    }

private:
    int first;
    int last;
    T storage[size];
};

#endif // _ARRAY_QUEUE_

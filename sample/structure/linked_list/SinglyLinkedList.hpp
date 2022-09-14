/**
 * Singly-linked list(单向链表):
 * 1. 空间复杂度为 O(n)
 * 2. 查找的时间复杂度为 O(n)
 * 3. 插入和删除的时间复杂度为 O(1)
 *   
 *   |  VAL |    |  VAL |    |  VAL |
 *   |   5  |    |   5  |    |   5  |
 *   | NEXT | -> | NEXT | -> | NEXT | -> NONE
 *     head                    tail
 */

#ifndef _SINGLY_LINKED_LIST_
#define _SINGLY_LINKED_LIST_

#include <iostream>

template<typename T>
class SLLNode {
public:
    SLLNode() {
        next = nullptr;
    }
    SLLNode(const T& el, SLLNode *in = nullptr) {
        element = el;
        next = in;
    }

    T element;
    SLLNode *next;
};

template<typename T>
class SinglyLinkedList {
public:
    SinglyLinkedList() {
        head = tail = nullptr;
    }
    ~SinglyLinkedList();
    int isEmpty() {
        return head == nullptr;
    }
    void addToHead(const T&);
    void addToTail(const T&);
    T deleteFromHead(); // call when not empty
    T deleteFromTail(); // call when not empty
    void deleteNode(const T&);
    bool hasElement(const T&) const;
    T getElementByIndex(int) const;
    T getElementByRIndex(int) const;
    void reverse();
    void print() const;

private:
    SLLNode<T> *head;
    SLLNode<T> *tail;
};

template<typename T>
SinglyLinkedList<T>::~SinglyLinkedList() {
    for (SLLNode<T> *p; head; ) {
        p = head;
        head = head->next;
        delete p;
    }
}

template<typename T>
void SinglyLinkedList<T>::addToHead(const T& el) {
    head = new SLLNode<T>(el, head);
    if (nullptr == tail)
        tail = head;
}

template<typename T>
void SinglyLinkedList<T>::addToTail(const T& el) {
    if (nullptr == tail) {
        head = tail = new SLLNode<T>(el);
    } else {
        tail->next = new SLLNode<T>(el);
        tail = tail->next;
    }
}

template<typename T>
T SinglyLinkedList<T>::deleteFromHead() {
    T el = head->element;
    SLLNode<T> *tmp = head;
    if (head == tail) {
        head = tail = nullptr;
    } else {
        head = head->next;
    }
    delete tmp;
    return el;
}

template<typename T>
T SinglyLinkedList<T>::deleteFromTail() {
    T el = tail->element;
    SLLNode<T> *tmp = tail;
    if (head == tail) {
        head = tail = nullptr;
    } else {
        for (SLLNode<T> *p = head; p; p = p->next) {
            if (p->next == tail) {
                p->next = nullptr;
                tail = p;
                break;
            }
        }
    }
    delete tmp;
    return el;
}

template<typename T>
void SinglyLinkedList<T>::deleteNode(const T& el) {
    for (SLLNode<T> *prev = nullptr, *cur = head; cur; prev = cur, cur = cur->next) {
        if (el == cur->element) {
            if (nullptr == prev) {
                head = head->next;
            } else {
                prev->next = cur->next;
            }
            delete cur;
            break;
        }
    }
}

template<typename T>
bool SinglyLinkedList<T>::hasElement(const T& el) const {
    for (SLLNode<T> *p = head; p; p = p->next) {
        if (el == p->element)
            return true;
    }
    return false;
}

template<typename T>
T SinglyLinkedList<T>::getElementByIndex(int pos) const {
    int index = 0;
    for (SLLNode<T> *p = head; p; p = p->next) {
        if (index == pos) {
            return p->element;
        }
        index++;
    }
    return T();
}

template<typename T>
T SinglyLinkedList<T>::getElementByRIndex(int pos) const {
    int index = 0;
    for (SLLNode<T> *prev = nullptr, *cur = head; cur; cur = cur->next) {
        if (cur == tail) {
            if (prev && prev->next) {
                return prev->next->element;
            }
        }
        if (index == pos) {
            prev = head;
        } else if (index > pos) {
            prev = prev->next;
        }
        index++;
    }
    return T();
}

template<typename T>
void SinglyLinkedList<T>::reverse() {
    for (SLLNode<T> *prev2 = nullptr, *prev1 = nullptr, *cur = head;
            cur;
            prev2 = prev1, prev1 = cur, cur = cur->next) {
        if (prev1) {
            prev1->next = prev2;
        }
        if (cur == tail) {
            tail->next = prev1;
            prev2 = head;
            head = tail;
            tail = prev2;
            break;
        }
    }
}

template<typename T>
void SinglyLinkedList<T>::print() const {
    for (SLLNode<T> *p = head; p; p = p->next) {
        std::cout << p->element << " ";
    }
    std::cout << std::endl;
}

#endif // _SINGLY_LINKED_LIST_

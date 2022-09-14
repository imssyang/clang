/**
 * Double-linked list(双向链表):
 * 1. 空间复杂度为 O(n)
 * 2. 查找的时间复杂度为 O(n)
 * 3. 插入和删除的时间复杂度为 O(1)
 *
 *         |  VAL |    |  VAL |    |  VAL |
 *         |   5  |    |   5  |    |   5  |
 *         | NEXT | -> | NEXT | -> | NEXT | -> NONE
 * NONE <- | PREV | <- | PREV | <- | PREV |
 *           head                    tail
 */

#ifndef _DOUBLY_LINKED_LIST_
#define _DOUBLY_LINKED_LIST_

#include <iostream>

template<typename T>
class DLLNode {
public:
    DLLNode() {
        prev = next = nullptr;
    }
    DLLNode(const T& el, DLLNode *p = nullptr, DLLNode *n = nullptr) {
        element = el;
        next = n;
        prev = p;
    }

    T element;
    DLLNode *next;
    DLLNode *prev;
};

template<typename T>
class DoublyLinkedList {
public:
    DoublyLinkedList() {
        head = tail = nullptr;
    }
    bool isEmpty() const {
        return nullptr == head;
    }
    void addToHead(const T&);
    void addToTail(const T&);
    T deleteFromHead(); // call when not empty
    T deleteFromTail(); // call when not empty
    void deleteNode(const T&);
    bool hasElement(const T&) const;
    void reverse();
    void print() const;

private:
    DLLNode<T> *head;
    DLLNode<T> *tail;
};

template<typename T>
void DoublyLinkedList<T>::addToHead(const T& el) {
    if (nullptr == head) {
        head = tail = new DLLNode<T>(el);
    } else {
        head->prev = new DLLNode<T>(el, nullptr, head);
        head = head->prev;
    }
}

template<typename T>
void DoublyLinkedList<T>::addToTail(const T& el) {
    if (nullptr == tail) {
        head = tail = new DLLNode<T>(el);
    } else {
        tail->next = new DLLNode<T>(el, tail);
        tail = tail->next;
    }
}

template<typename T>
T DoublyLinkedList<T>::deleteFromHead() {
    T el = head->element;
    DLLNode<T> *tmp = head;
    if (head == tail) {
        head = tail = nullptr;
    } else {
        head->next->prev = nullptr;
        head = head->next;
    }
    delete tmp;
    return el;
}

template<typename T>
T DoublyLinkedList<T>::deleteFromTail() {
    T el = tail->element;
    DLLNode<T> *tmp = tail;
    if (head == tail) {
        head = tail = nullptr;
    } else {
        tail->prev->next = nullptr;
        tail = tail->prev;
    }
    delete tmp;
    return el;
}

template<typename T>
void DoublyLinkedList<T>::deleteNode(const T& el) {
    for (DLLNode<T> *p = head; p; p = p->next) {
        if (el == p->element) {
            if (head == tail) {
                head = tail = nullptr;
            } else if (p == head) {
                head = head->next;
                head->prev = nullptr;
            } else if (p == tail) {
                p->prev->next = nullptr;
            } else {
                p->prev->next = p->next;
                p->next->prev = p->prev;
            }
            delete p;
            break;
        }
    }
}

template<typename T>
bool DoublyLinkedList<T>::hasElement(const T& el) const {
    for (DLLNode<T> *p = head; p; p = p->next) {
        if (p->element == el) {
            return true;
        }
    }
    return false;
}

template<typename T>
void DoublyLinkedList<T>::reverse() {
    for (DLLNode<T> *prev = nullptr, *cur = head; cur; prev = cur, cur = cur->next) {
        if (prev) {
            prev->next = prev->prev;
            prev->prev = cur;
        }
        if (cur == tail) {
            tail->next = prev;
            tail->prev = nullptr;
            prev = head;
            head = tail;
            tail = prev;
            break;
        }
    }
}

template<typename T>
void DoublyLinkedList<T>::print() const {
	for (DLLNode<T> *p = head; p; p = p->next) {
        std::cout << p->element << " ";
    }
    std::cout << std::endl;
}

#endif // _DOUBLY_LINKED_LIST_

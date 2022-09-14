#ifndef _BINARY_SEARCH_TREE_
#define _BINARY_SEARCH_TREE_

#include <iostream>
#include <queue>
#include <stack>

using namespace std;

template<class T>
class Stack : public stack<T> {
public:
    T pop() {
        T el = this->top();
        stack<T>::pop();
        return el;
    }
};

template<class T>
class Queue : public queue<T> {
public:
    T pop() {
        T el = this->front();
        queue<T>::pop();
        return el;
    }
};

template<class T>
class BSTNode {
public:
    BSTNode() {
        depth = 0;
        left = right = 0;
    }
    BSTNode(const T& e, int d = 0, BSTNode<T> *l = 0, BSTNode<T> *r = 0) {
        depth = d;
        el = e;
        left = l;
        right = r;
    }

    int depth;
    T el;
    BSTNode<T> *left, *right;
};

template<class T>
class BST {
public:
    BST() {
        root = 0;
    }
    ~BST() {
        root = 0;
    }
    bool isEmpty() const {
        return 0 == root;
    }

    void insert(const T&);
    void insertRecursive(const T& el);

    T* search(const T& el) const;
    T* recursiveSearch(const T& el) const;

    void printBreadthFirst() const; // 广度优先遍历
    void printBreadthFirstRecursive(const T& el);

    void printPreorder() const;     // 深度优先遍历（前序树遍历）
    void printInorder() const;      // 深度优先遍历（中序树遍历）
    void printPostorder() const;    // 深度优先遍历（后序树遍历）
    void printPreorderRecursive() const;
    void printInorderRecursive() const;
    void printPostorderRecursive() const;

    void deleteByCopying(BSTNode<T>*&);
    void findAndDeleteByMerging(const T&);
    void deleteByMerging(BSTNode<T>*&);
    void iterativePreorder();
    void iterativeInorder();
    void iterativePostorder();
    void morrisPreorder();
    void morrisInorder();
    void morrisPostorder();
    void balance(T*, int, int);

private:
    void preorderRecursive(BSTNode<T>*) const;
    void inorderRecursive(BSTNode<T>*) const;
    void postorderRecursive(BSTNode<T>*) const;
    void visit(BSTNode<T>* p) const {
        if (0 == p->depth % 2)
            cout << '-';
        else
            cout << '+';
        cout << p->el << ' ';
    }

private:
    BSTNode<T> *root;
};

template<class T>
void BST<T>::insert(const T& el) {
    BSTNode<T> *prev = 0, *p = root;
    while (p) {
        prev = p;
        if (el < p->el)
            p = p->left;
        else
            p = p->right;
    }
    if (0 == root)
        root = new BSTNode<T>(el);
    else if (el < prev->el)
        prev->left = new BSTNode<T>(el, prev->depth+1);
    else
        prev->right = new BSTNode<T>(el, prev->depth+1);
}

template<class T>
void BST<T>::printBreadthFirst() const {
    Queue<BSTNode<T>*> que;
    BSTNode<T> *p = root;
    if (p) {
        que.push(p);
        while (!que.empty()) {
            p = que.pop();
            visit(p);

            if (p->left)
                que.push(p->left);
            if (p->right)
                que.push(p->right);
        }
    }
}

template<class T>
void BST<T>::printPreorder() const {
    Stack<BSTNode<T>*> stk;
    BSTNode<T> *p = root;
    if (p) {
        stk.push(p);
        while (!stk.empty()) {
            p = stk.pop();
            visit(p);

            if (p->right)
                stk.push(p->right);
            if (p->left)
                stk.push(p->left);
        }
    }
}

template<class T>
void BST<T>::printInorder() const {
    Stack<BSTNode<T>*> stk;
    BSTNode<T>* p = root;
    while (p) {
        while (p) {
            if (p->right)
                stk.push(p->right);
            stk.push(p);
            p = p->left;
        }
        p = stk.pop();
        while (!stk.empty() && !p->right) {
            visit(p);
            p = stk.pop();
        }
        visit(p);
        if (!stk.empty())
            p = stk.pop();
        else
            p = 0;
    }
}

template<class T>
void BST<T>::printPostorder() const {
    Stack<BSTNode<T>*> stk;
    BSTNode<T> *p = root, *prev = root;
    while (p) {
        for (; p->left; p = p->left)
            stk.push(p);
        while (!p->right || p->right == prev) {
            visit(p);
            prev = p;
            if (stk.empty())
                return;
            p = stk.pop();
        }
        stk.push(p);
        p = p->right;
    }
}

template<class T>
void BST<T>::printPreorderRecursive() const { 
    preorderRecursive(root);
}

template<class T>
void BST<T>::printInorderRecursive() const {
    inorderRecursive(root);
}

template<class T>
void BST<T>::printPostorderRecursive() const { 
    postorderRecursive(root);
}

template<class T>
void BST<T>::preorderRecursive(BSTNode<T>* p) const {
    if (p) {
        visit(p);
        preorderRecursive(p->left);
        preorderRecursive(p->right);
    }
}

template<class T>
void BST<T>::inorderRecursive(BSTNode<T>* p) const {
    if (p) {
        inorderRecursive(p->left);
        visit(p);
        inorderRecursive(p->right);
    }
}

template<class T>
void BST<T>::postorderRecursive(BSTNode<T>* p) const {
    if (p) {
        postorderRecursive(p->left);
        postorderRecursive(p->right);
        visit(p);
    }
}

#endif // _BINARY_SEARCH_TREE_

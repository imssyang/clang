#include <climits>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <vector>
using namespace std;
class BSNode {
public:
    BSNode(int e, shared_ptr<BSNode> L = nullptr, shared_ptr<BSNode> R = nullptr)
        : e(e), L(L), R(R) {}
public:
    int e;
    shared_ptr<BSNode> L;
    shared_ptr<BSNode> R;
};
class BSTree {
public:
    BSTree(const vector<int>& A) {
        for (auto& e : A) {
            //insert(root, e);
            insert(e);
        }
    }
    bool isBST(shared_ptr<BSNode> x, int p, int r) {
        if (!x) 
            return true;
        if (p > x->e || x->e > r)
            return false;
        return isBST(x->L, p, x->e) && isBST(x->R, x->e, r);
    }
    void insert(shared_ptr<BSNode>& x, int e) {
        if (!x) {
            x = make_shared<BSNode>(e);
        } else if (x->e > e) {
            insert(x->L, e);
        } else {
            insert(x->R, e);
        }
    }
    void insert(int e) {
        auto z = make_shared<BSNode>(e);
        decltype(z) y = nullptr;
        decltype(z) x = root;
        while (x) {
            y = x;
            if (x->e > e)
                x = x->L;
            else
                x = x->R;
        }
        if (!y) {
            root = z;
        } else if (y->e > e) {
            y->L = z;
        } else {
            y->R = z;
        }
    }
    void remove(shared_ptr<BSNode> p, shared_ptr<BSNode> x, int e) {
        if (x->e > e) {
            remove(x, x->L, e);
            return;
        }
        if (x->e < e) {
            remove(x, x->R, e);
            return;
        }
        decltype(x) z = x;
        if (z->L && z->R) {
            decltype(p) q = nullptr;
            auto y = findMin(z->R, q);
            z->e = y->e;
            remove(q, y, y->e);
        } else if (z->L) {
            replaceChild(p, z, z->L);
        } else if (z->R) {
            replaceChild(p, z, z->R);
        } else {
            replaceChild(p, z, nullptr);
        }
    }
    void remove(int e) {
        auto x = root;
        decltype(x) p = nullptr;
        while (x) {
            if (x->e == e)
                break;
            p = x;
            if (x->e > e)
                x = x->L;
            else
                x = x->R;
        }
        if (!x) return;
        if (x->L && x->R) {
            decltype(x) q = nullptr;
            auto y = findMin(x->R, q);
            x->e = y->e;
            replaceChild(q, y, y->R);
        } else if (x->L) {
            replaceChild(p, x, x->L);
        } else if (x->R) {
            replaceChild(p, x, x->R);
        } else {
            replaceChild(p, x, nullptr);
        }
    }
    auto search(shared_ptr<BSNode> x, int e) -> decltype(x) {
        if (!x || x->e == e)
            return x;
        if (x->e > e)
            return search(x->L, e);
        else
            return search(x->R, e);
    }
    auto search(int e) -> shared_ptr<BSNode> {
        auto x = root;
        while (x) {
            if (x->e == e)
                return x;
            if (x->e > e)
                x = x->L;
            else
                x = x->R;
        }
        return nullptr;
    }
    void levelorder() {
        if (!root) return;
        queue<shared_ptr<BSNode>> Q;
        Q.push(root);
        while (!Q.empty()) {
            auto& x = Q.front();
            Q.pop();
            cout << x->e << " ";
            if (x->L)
                Q.push(x->L);
            if (x->R)
                Q.push(x->R);
        }
    }
    void preorder(shared_ptr<BSNode> x) {
        if (x) {
            cout << x->e << " ";
            preorder(x->L);
            preorder(x->R);
        }
    }
    void inorder(shared_ptr<BSNode> x) {
        if (x) {
            inorder(x->L);
            cout << x->e << " ";
            inorder(x->R);
        }
    }
    void postorder(shared_ptr<BSNode> x) {
        if (x) {
            postorder(x->L);
            postorder(x->R);
            cout << x->e << " ";
        }
    }
    void clear(shared_ptr<BSNode> x) {
        if (x) {
            clear(x->L);
            clear(x->R);
            x = nullptr;
        }
    }
private:
    class OSNode {
    public:
        enum Operate {T, V};
        OSNode(Operate o, shared_ptr<BSNode> b) : o(o), b(b) {}
    public:
        Operate o;
        shared_ptr<BSNode> b;
    };
    auto findMin(shared_ptr<BSNode> x, shared_ptr<BSNode>& p) -> decltype(x) {
        decltype(x) z = x;
        while (z->L) {
            p = z;
            z = z->L;
        }
        return z;
    }
    void replaceChild(shared_ptr<BSNode> p, shared_ptr<BSNode> x, shared_ptr<BSNode> y) {
        if (p->L == x)
            p->L = y;
        else if (p->R == x)
            p->R = y;
    }
public:
    shared_ptr<BSNode> root;
};
void Print(const vector<int>& A, const string& prefix = string(), const string& suffix = string()) {
    cout << prefix;
    for (int i = 0; i < A.size(); i++) {
        cout << A[i] << " ";
    }
    cout << suffix;
}
int main() {
    //vector<int> A {3,1,5,0,7,9,2,6,8,4};
    vector<int> A {-1,3,1,5,0,7,9,-2,2,6,8,4};
    Print(A, "", "\n");

    BSTree T(A);
    cout << T.isBST(T.root, INT_MIN, INT_MAX) << endl;

    T.levelorder();
    cout << endl;

    //T.remove(nullptr, T.root, 5);
    //T.remove(5);

    T.levelorder();
    cout << endl;

    //auto x = T.search(5);
    //cout << x->e << "|" << x->L->e << "|" << x->R->e << endl;
    auto x = T.search(T.root, 5);
    cout << x->e << "|" << x->L->e << "|" << x->R->e << endl;

    T.preorder(T.root);
    cout << endl;

    T.inorder(T.root);
    cout << endl;

    T.postorder(T.root);
    cout << endl;

    return 0;
}

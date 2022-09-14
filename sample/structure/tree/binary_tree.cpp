#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <vector>
using namespace std;
class BNode {
public:
    BNode(int e, shared_ptr<BNode> L = nullptr, shared_ptr<BNode> R = nullptr)
        : e(e), L(L), R(R) {}
public:
    int e;
    shared_ptr<BNode> L;
    shared_ptr<BNode> R;
};
class BTree {
public:
    BTree(const vector<int>& A) {
        queue<shared_ptr<BNode>> Q;
        for (auto& e : A) {
            auto x = make_shared<BNode>(e);
            if (!root) {
                root = x;
                Q.push(x);
            } else {
                auto& y = Q.front();
                if (!y->L) {
                    y->L = x;
                    Q.push(y->L);
                } else if (!y->R) {
                    y->R = x;
                    Q.push(y->R);
                    Q.pop();
                }
            }
        }
    }
    void levelorder() {
        if (!root) return;
        queue<shared_ptr<BNode>> Q;
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
    void preorder(shared_ptr<BNode> x) {
        if (x) {
            cout << x->e << " ";
            preorder(x->L);
            preorder(x->R);
        }
    }
    void preorder() {
        if (!root) return;
        stack<shared_ptr<BNode>> S;
        S.push(root);
        while (!S.empty()) {
            auto x = S.top();
            S.pop();
            cout << x->e << " ";
            if (x->R)
                S.push(x->R);
            if (x->L)
                S.push(x->L);
        }
    }
    void preorder2() {
        if (!root) return;
        stack<shared_ptr<ONode>> S;
        S.push(make_shared<ONode>(ONode::T, root));
        while (!S.empty()) {
            auto x = S.top();
            S.pop();
            if (x->o == ONode::T) {
                if (x->b->R)
                    S.push(make_shared<ONode>(ONode::T, x->b->R));
                if (x->b->L)
                    S.push(make_shared<ONode>(ONode::T, x->b->L));
                S.push(make_shared<ONode>(ONode::V, x->b));
            } else {
                cout << x->b->e << " ";
            }
        }
    }
    void inorder(shared_ptr<BNode> x) {
        if (x) {
            inorder(x->L);
            cout << x->e << " ";
            inorder(x->R);
        }
    }
    void inorder() {
        if (!root) return;
        stack<shared_ptr<BNode>> S;
        auto x = root;
        while (!S.empty() || x) {
            if (x) {
                S.push(x);
                x = x->L;
            } else {
                x = S.top();
                S.pop();
                cout << x->e << " ";
                x = x->R;
            }
        }
    }
    void inorder2() {
        if (!root) return;
        stack<shared_ptr<ONode>> S;
        S.push(make_shared<ONode>(ONode::T, root));
        while (!S.empty()) {
            auto x = S.top();
            S.pop();
            if (x->o == ONode::T) {
                if (x->b->R)
                    S.push(make_shared<ONode>(ONode::T, x->b->R));
                S.push(make_shared<ONode>(ONode::V, x->b));
                if (x->b->L)
                    S.push(make_shared<ONode>(ONode::T, x->b->L));
            } else {
                cout << x->b->e << " ";
            }
        }
    }
    void postorder(shared_ptr<BNode> x) {
        if (x) {
            postorder(x->L);
            postorder(x->R);
            cout << x->e << " ";
        }
    }
    void postorder() {
        if (!root) return;
        stack<shared_ptr<BNode>> S;
        auto x = root;
        decltype(x) y = nullptr;
        while (!S.empty() || x) {
            if (x) {
                S.push(x);
                x = x->L;
            } else {
                auto z = S.top();
                if (z->R && z->R != y) {
                    x = z->R;
                } else {
                    cout << z->e << " ";
                    y = z;
                    S.pop();
                }
            }
        }
    }
    void postorder2() {
        if (!root) return;
        stack<shared_ptr<ONode>> S;
        S.push(make_shared<ONode>(ONode::T, root));
        while (!S.empty()) {
            auto x = S.top();
            S.pop();
            if (x->o == ONode::T) {
                S.push(make_shared<ONode>(ONode::V, x->b));
                if (x->b->R)
                    S.push(make_shared<ONode>(ONode::T, x->b->R));
                if (x->b->L)
                    S.push(make_shared<ONode>(ONode::T, x->b->L));
            } else {
                cout << x->b->e << " ";
            }
        }
    }
    void clear(shared_ptr<BNode> x) {
        if (x) {
            clear(x->L);
            clear(x->R);
            x = nullptr;
        }
    }
private:
    class ONode {
    public:
        enum Operate {T, V};
        ONode(Operate o, shared_ptr<BNode> b) : o(o), b(b) {}
    public:
        Operate o;
        shared_ptr<BNode> b;
    };
public:
    shared_ptr<BNode> root;
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

    BTree T(A);
    T.levelorder();
    cout << endl;
    T.preorder(T.root);
    cout << endl;
    T.preorder();
    cout << endl;
    T.preorder2();
    cout << endl;

    T.inorder(T.root);
    cout << endl;
    T.inorder();
    cout << endl;
    T.inorder2();
    cout << endl;

    T.postorder(T.root);
    cout << endl;
    T.postorder();
    cout << endl;
    T.postorder2();
    cout << endl;

    return 0;
}

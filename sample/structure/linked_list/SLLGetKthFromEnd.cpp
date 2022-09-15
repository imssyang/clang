#include <iostream>
#include <vector>

using namespace std;

struct ListNode {
    int val;
    ListNode *next;
    ListNode(int x, ListNode *p = NULL) : val(x), next(p) {}
};

class Environ {
public:
    ListNode* init(const vector<int>& v) {
        ListNode *head = NULL, *cur = NULL;
        for (int i = 0; i < v.size(); i++) {
            auto *node = new ListNode(v[i]);
            if (!head) {
                head = cur = node;
                continue;
            }
            cur->next = node;
            cur = cur->next;
        }
        return head;
    }

    void print(ListNode* head) {
        while (head) {
            cout << head->val << " ";
            head = head->next;
        }
        cout << endl;
    }
};

class DoublePointSolution {
public:
    ListNode* getKthFromEnd(ListNode* head, int k) {
        ListNode *fast = head;
        ListNode *slow = head;
        while (fast && k > 0) {
            fast = fast->next;
            k--;
        }
        while (fast) {
            slow = slow->next;
            fast = fast->next;
        }
        return slow;
    }
};

class RecursionSolution {
public:
    int i = 0;
    ListNode* getKthFromEnd(ListNode* head, int k) {
        if (!head || !head->next) return head;
        ListNode *node = getKthFromEnd(head->next, k);
        i++;
        if (k <= i)
            return node;
        else
            return head;
    }
};

int main() {
    Environ env;
    auto vec = vector<int>{6, 5, 4, 3, 2, 1};
    auto *head = env.init(vec);
    env.print(head);

    for (int i = 1; i < vec.size(); i++) {
        DoublePointSolution dps;
        auto* kth = dps.getKthFromEnd(head, i);
        env.print(kth);
    }

    for (int i = 1; i < vec.size(); i++) {
        RecursionSolution rs;
        auto* kth = rs.getKthFromEnd(head, i);
        env.print(kth);
    }

    return 0;
}

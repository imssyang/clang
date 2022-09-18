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
        // 快指针先跑k次
        while (fast && k > 0) {
            fast = fast->next;
            k--;
        }
        // 快慢指针同步跑到结尾
        while (fast) {
            slow = slow->next;
            fast = fast->next;
        }
        // 慢指针指向倒数第k个节点
        return slow;
    }

    ListNode* removeKthFromEnd(ListNode* head, int k) {
        ListNode *fast = head;
        ListNode *slow = head;
        // 快指针先跑k次
        while (fast && k > 0) {
            fast = fast->next;
            k--;
        }
        // 快指针跑到结尾，说明头节点即为倒数第k个节点
        if (!fast)
            return head->next;
        // 快慢指针同步跑到倒数第一个节点
        while (fast->next) {
            slow = slow->next;
            fast = fast->next;
        }
        // 慢指针指向倒数第k+1个节点，方便删除第k个节点
        slow->next = slow->next->next;
        return head;
    }
};

class RecursionSolution {
public:
    int i = 0;
    ListNode* getKthFromEnd(ListNode* head, int k) {
        // 遇到倒数第一个节点时终止入栈，出栈时返回倒数第一个节点
        if (!head->next)
            return head;
        // 从头节点开始入栈，出栈节点从倒数第一个节点开始
        ListNode *node = getKthFromEnd(head->next, k);
        i++;
        if (i < k)
            // 出栈到倒数第k个节点前，返回入栈节点
            return head;
        // 出栈到倒数第k个节点后（含），返回出栈节点
        return node;
    }

    int j = 0;
    ListNode* removeKthFromEnd(ListNode* head, int k) {
        // 遇到NULL节点时终止入栈，出栈时返回NULL节点
        if (!head)
            return NULL;
        // 从头节点开始入栈，出栈节点从倒数第零个节点（即NULL节点）开始，且在出栈后立即重建节点关系
        head->next = removeKthFromEnd(head->next, k);
        j++;
        if (j == k)
            // 出栈到倒数第k个节点时，返回入栈节点的下一个节点以忽略它
            return head->next;
        // 其余情况，返回入栈节点
        return head;
    }
};

int main() {
    Environ env;
    ListNode *head = NULL;
    auto vec = vector<int>{6, 5, 4, 3, 2, 1};

    cout << "getKthFromEnd-DoublePoint:" << endl;
    head = env.init(vec);
    for (int i = 1; i < vec.size(); i++) {
        DoublePointSolution dps;
        auto* kth = dps.getKthFromEnd(head, i);
        env.print(kth);
    }

    cout << "getKthFromEnd-Recursion:" << endl;
    head = env.init(vec);
    for (int i = 1; i < vec.size(); i++) {
        RecursionSolution rs;
        auto* kth = rs.getKthFromEnd(head, i);
        env.print(kth);
    }

    cout << "removeKthFromEnd-DoublePoint:" << endl;
    head = env.init(vec);
    for (int i = 1; i < vec.size(); i++) {
        DoublePointSolution dps;
        auto* kth = dps.removeKthFromEnd(head, i);
        env.print(kth);
    }

    cout << "removeKthFromEnd-Recursion:" << endl;
    head = env.init(vec);
    for (int i = 1; i < vec.size(); i++) {
        DoublePointSolution dps;
        auto* kth = dps.removeKthFromEnd(head, i);
        env.print(kth);
    }

    return 0;
}

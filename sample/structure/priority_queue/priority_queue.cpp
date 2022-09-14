/**
 * PriorityQueue(优先队列):
 * 1. 正常进入，按照自定义的优先级取出；
 * 2. 实现方式有两种：
 *    a) Heap（按照优先级创建堆数据，比如最大堆、最小堆）
 *    b) Binary Search Tree (按优先级排序树)
 */

#include <iostream>
#include <queue>
#include <vector>

using namespace std;

template<class T>
void print(const T& q) {
    for (auto& e : q) {
        cout << e << ' ';
    }
    cout << endl;
}

template<class T>
void print_queue(T q) {
    while (!q.empty()) {
        cout << q.top() << ' ';
        q.pop();
    }
    cout << endl;
}

int main() {
    const auto data = {1,8,5,6,3,4,0,9,7,2};
    priority_queue<int, vector<int>, greater<int>> q(data.begin(), data.end());
    print(data);
    print_queue(q);


    return 0;
}


/**
 * 快速排序：平均 O(nlogn)，最坏 O(n^2)
 */

#ifndef _QUICK_SORT_
#define _QUICK_SORT_

#include <stack>
#include "deps/Common.hpp"

template<class T>
int partition(T data[], int first, int last) {
    int lower = first;
    int upper = last - 1;
    T bound = data[last];
    while (lower <= upper) {
        while (data[lower] < bound)
            lower++;
        while (data[upper] > bound)
            upper--;
        if (lower < upper) {
            swap(data[lower++], data[upper--]);
        } else {
            upper++;
            break;
        }
    }
    if (upper != last)
        swap(data[upper], data[last]);
    return upper;
}

template<class T>
void quicksort(T data[], int first, int last) {
    int middle = partition(data, first, last);
    if (middle - first > 1)
        quicksort(data, first, middle-1);
    if (last - middle > 1)
        quicksort(data, middle+1, last);
}

// 非递归实现
template<class T>
void quicksort_stack(T data[], int first, int last) {
    std::stack<T> s;
    s.push(first);
    s.push(last);

    while (!s.empty()) {
        last = s.top();
        s.pop();
        first = s.top();
        s.pop();

        if (first < last) {
            int middle  = partition(data, first, last);
            if (middle - first > 1) {
                s.push(first);
                s.push(middle-1);
            }
            if (last - middle > 1) {
                s.push(middle+1);
                s.push(last);
            }
        }
    }
}

template<class T>
void quick_sort(T data[], int n) {
    int first = 0, last = n - 1;
    //quicksort(data, first, last);
    quicksort_stack(data, first, last);
}

#endif // _QUICK_SORT_

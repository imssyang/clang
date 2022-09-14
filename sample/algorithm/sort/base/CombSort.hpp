/**
 * 堆排序，最坏情况为O(n^2)
 */

#ifndef _COMB_SORT_
#define _COMB_SORT_

#include "deps/Common.hpp"

template<class T>
void comb_sort(T data[], int n) {
    // 预处理（1.3的步长缩短因子）
    for (int step = n; (step = int(step/1.3)) > 1;) {
        for (int j = n-1; j >= step; j--) {
            int k = j - step;
            if (data[j] < data[k]) {
                swap(data[j], data[k]);
            }
        }
    }

    // 冒泡排序（只要遍历一遍，没有发现需要互换位置的元素，即停止排序）
    bool again = true;
    for (int i = 0; i < n-1 && again; i++) {
        for (int j = 1, again = false; j < n-i; j++) {
            if (data[j-1] > data[j]) {
                swap(data[j-1], data[j]);
                again = true;
            }
        }
    }
}

#endif // _COMB_SORT_

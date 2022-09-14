/**
 * 归并排序：最坏情况 O(nlogn)
 *    弱点：合并数组需要额外的存储空间，对于大量数据，难以忍受，解决的一个办法是使用链表。
 */

#ifndef _MERGE_SORT_
#define _MERGE_SORT_

#include "deps/Common.hpp"

template<class T>
void merge(T data[], int first, int last) {
    int length = last - first + 1;
    int middle = (first + last) / 2;
    int i1 = 0;
    int i2 = first;
    int i3 = middle + 1;
    T *temp = new T[length];
    while (i2 <= middle && i3 <= last) {
        if (data[i2] < data[i3]) {
            temp[i1++] = data[i2++];
        } else {
            temp[i1++] = data[i3++];
        }
    }
    while (i2 <= middle)
        temp[i1++] = data[i2++];
    while (i3 <= last)
        temp[i1++] = data[i3++];
    memcpy(data+first, temp, sizeof(T)*length);
    delete[] temp;
}

template<class T>
void mergesort(T data[], int first, int last) {
    if (first < last) {
        int middle = (first + last) / 2;
        mergesort(data, first, middle);
        mergesort(data, middle+1, last);
        merge(data, first, last);
    }
}

template<class T>
void merge_sort(T data[], int n) {
    mergesort(data, 0, n-1);
}

#endif // _MERGE_SORT_

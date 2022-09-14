#ifndef _BUBBLE_SORT_
#define _BUBBLE_SORT_

#include "deps/Common.hpp"

template<class T>
void bubble_sort(T data[], int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 1; j < n-i; j++) {
            if (data[j-1] > data[j]) {
                swap(data[j-1], data[j]);
            }
        }
    }
}

#endif // _BUBBLE_SORT_

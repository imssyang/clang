#ifndef _SELECT_SORT_
#define _SELECT_SORT_

#include "deps/Common.hpp"

template<class T>
void select_sort(T data[], int n) {
    for (int i = 0, sel = 0; i < n-1; i++, sel = i) {
        for (int j = i+1; j < n; j++) {
            if (data[sel] > data[j]) {
                sel = j;
            }
        }
        if (i != sel) {
            swap(data[i], data[sel]);
        }
    }
}

#endif // _SELECT_SORT_

#ifndef _INSERT_SORT_
#define _INSERT_SORT_

#include "deps/Common.hpp"

template<class T>
void insert_sort(T data[], int n) {
    for (int i = 1; i < n; i++) {
        for (int j = i; j > 0; j--) {
            if (data[j] < data[j-1]) {
                swap(data[j], data[j-1]);
            }
        }
    }
}

#endif // _INSERT_SORT_

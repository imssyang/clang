#ifndef _COMMON_
#define _COMMON_

#include <cstdio>

template<class T>
void swap(T& a, T& b) {
    a = a ^ b;
    b = a ^ b;
    a = a ^ b;
}

#endif // _COMMON_

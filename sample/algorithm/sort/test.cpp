#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "base/BubbleSort.hpp"
#include "base/CombSort.hpp"
#include "base/InsertSort.hpp"
#include "base/SelectSort.hpp"
#include "divide/MergeSort.hpp"
#include "divide/QuickSort.hpp"

constexpr int N = 9;

int main() {
    int arr[N] = {5, 3, 7, 9, 1, 4, 2, 6, 8};
    printf("\n--------origin---------\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", arr[i]);
    }

    printf("\n--------insert_sort---------\n");
    int it[N] = {0};
    memcpy(it, arr, sizeof(arr));
    insert_sort(it, N);
    for (int i = 0; i < N; i++) {
        printf("%d ", it[i]);
    }

    printf("\n--------bubble_sort---------\n");
    int bt[N] = {0};
    memcpy(bt, arr, sizeof(arr));
    bubble_sort(bt, N);
    for (int i = 0; i < N; i++) {
        printf("%d ", bt[i]);
    }

    printf("\n--------select_sort---------\n");
    int st[N] = {0};
    memcpy(st, arr, sizeof(arr));
    select_sort(st, N);
    for (int i = 0; i < N; i++) {
        printf("%d ", st[i]);
    }

    printf("\n--------comb_sort---------\n");
    int ct[N] = {0};
    memcpy(ct, arr, sizeof(arr));
    comb_sort(ct, N);
    for (int i = 0; i < N; i++) {
        printf("%d ", ct[i]);
    }

    printf("\n--------quick_sort---------\n");
    int qt[N] = {0};
    memcpy(qt, arr, sizeof(arr));
    quick_sort(qt, N);
    for (int i = 0; i < N; i++) {
        printf("%d ", qt[i]);
    }

    printf("\n--------merge_sort---------\n");
    int mt[N] = {0};
    memcpy(mt, arr, sizeof(arr));
    merge_sort(mt, N);
    for (int i = 0; i < N; i++) {
        printf("%d ", mt[i]);
    }
    return 0;
}

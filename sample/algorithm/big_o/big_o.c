/**
 * Big O notation:
 * O(1)     Constant Complexity     常数时间复杂度
 * O(logN)  Logarithmic Complexity  对数时间复杂度
 * O(n)     Linear Complexity       线性时间复杂度
 * O(n^2)   N square Complexity     平方时间复杂度
 * O(n^3)   N square Complexity     立方时间复杂度
 * O(2^n)   Exponential Growth      指数时间复杂度
 * O(n!)    Factorial               阶乘时间复杂度
 *
 * Master Theorem (主定理论证递归实现复杂度):
 * Binary search                   T(n) = T(n/2) + O(1)         O(logN)   二分搜索
 * Binary tree traversal           T(n) = 2T(n/2) + O(1)        O(N)      二叉树遍历
 * Optimal sorted matrix search    T(n) = 2T(n/2) + O(logN)     O(N)      排序后的二维矩阵查找
 * Quick/Merge sort                T(n) = 2T(n/2) + O(n)        O(NlogN)  快速/归并排序
 */

#include <stdio.h>

#define N 10

void bo_1() {
    printf("O(1): ");
    int i = N;
    printf("%d ", i);

    int sum = N * (N - 1) / 2;
    printf("sum[%d]", sum);
    printf("\n");
}

void bo_logN() {
    printf("O(logN): ");
    for (int i = 1; i < N; i *= 2) {
        printf("%d ", i);
    }
    printf("\n");
}

void bo_N() {
    printf("O(N): ");
    for (int i = 0; i < N; i++) {
        printf("%d ", i);
    }

    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += i;
    }
    printf("sum[%d] ", sum);

    int fibonacci[N] = {0};
    fibonacci[0] = 0;
    fibonacci[1] = 1;
    for (int i = 2; i <= N; i++) {
        fibonacci[i] = fibonacci[i-1] + fibonacci[i-2];
    }
    printf("fibonacci[%d] ", fibonacci[N]);
    printf("\n");
}

void bo_N_2() {
    printf("O(N^2): ");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("(%d,%d) ", i, j);
        }
    }
    printf("\n");
}

void bo_N_3() {
    printf("O(N^3): ");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                printf("(%d,%d,%d) ", i, j, k);
            }
        }
    }
    printf("\n");
}

int fibonacci_recurrence(int n) {
    if (0 == n || 1 == n)
        return n;
    return fibonacci_recurrence(n-1) + fibonacci_recurrence(n-2);
}

void bo_2_N() {
    printf("O(2^N): ");
    printf("fibonacci_recurrence[%d]", fibonacci_recurrence(N));
    printf("\n");
}

void bo_N_N() {
    printf("O(N!): ");
    printf("\n");
}

int main() {
    bo_1();
    bo_logN();
    bo_N();
    bo_N_2();
    bo_N_3();
    bo_2_N();
    bo_N_N();
    return 0;
}


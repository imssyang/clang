
/**
 * 动态规划要点：
 * 1. 递归 + 缓存 ==> 递推
 * 2. 状态定义：f[n]
 * 3. 状态转移方程：f[n] = f[n-1] + f[n-2]
 * 4. 最优子结构
 */

/**
 * 斐波纳契数列：0，1，1，2，3，5，8，13 ...
 * 输入: 索引 N
 * 输出：索引 N 对应的值
 */

#include <stdio.h>

/**
 * 递归：
 * f[0] = 0
 * f[1] = 1
 * f[n] = f[n-1] + f[n-2]
 *
 * 复杂度: 2^n，展开图示
 *                           ____ f(5)____
 *                          /             \
 *                      f(4)              f(3)
 *                     /    \             /  \
 *                 f(3)     f(2)      f(2)   f(1)
 *                /   \     /  \     /   \
 *             f(2)  f(1) f(1) f(0) f(1) f(0)
 *            /   \
 *         f(1)  f(0)
 *
 * 缺点：存在大量重复计算。
 */
int fibonacci1(int n) {
    if (0 == n) 
        return 0;
    else if (1 == n)
        return 1;
    else
        return fibonacci1(n-1) + fibonacci1(n-2);
}

/**
 * 递归（带缓存）：
 * f[0] = 0
 * f[1] = 1
 * f[n] = f[n-1] + f[n-2]
 *
 * 复杂度: n，展开图示
 *                         f(5)
 *                        /   \
 *                      f(4)  f(3)
 *                     /   \
 *                   f(3)  f(2)
 *                   /  \
 *                f(2)  f(1)
 *               /   \
 *             f(1) f(0)
 */
int fibonacci2(int n, int a[]) {
    if (0 == n)
        return 0;
    else if (1 == n)
        return 1;
    else if (!a[n])
        a[n] = fibonacci2(n-1, a) + fibonacci2(n-2, a);
    return a[n];
}

/**
 * 递推（由带缓存递归演化, 也叫顺推）
 */
int fibonacci3(int n, int a[]) {
    a[0] = 0;
    a[1] = 1;
    for (int i = 2; i <= n; i++)
        a[n] = a[n-1] + a[n-2];
    return a[n];
}

#define N 20
int main() {
    for (int i = 0; i < N; i++)
        printf("%d ", fibonacci1(i));
    printf("\n");

    int a2[N] = {0};
    for (int i = 0; i < N; i++)
        printf("%d ", fibonacci2(i, a2));
    printf("\n");

    int a3[N] = {0};
    for (int i = 0; i < N; i++)
        printf("%d ", fibonacci3(i, a3));
    printf("\n");
    return 0;
}


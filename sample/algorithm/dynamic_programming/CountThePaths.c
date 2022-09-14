/**
 *
 */

/**
 * 计算从S到E的路径数量：
 * 1. 每次只能向右或向下走一步；
 * 2. x位置为石头；
 *
 *      0 1 2 3 4 5 6 7
 *    0|S|b|e| | | | | |
 *    1|a|c|x| | | |x| |
 *    2|d| | | |x| | | |
 *    3|x| |x| | |x| | |
 *    4| | |x| | | | | |
 *    5| | | |x|x| |x| |
 *    6| |x| | | |x| | |
 *    7| | | | | | | |E|
 */

#include <stdio.h>

#ifndef __cpluscplus
# define bool int
# define true 1
# define false 0
#endif

#define N 7

int x[][2] = {
    {1, 2}, {1, 6}, {2, 4},
    {3, 0}, {3, 2}, {3, 5},
    {4, 2}, {5, 3}, {5, 4},
    {5, 6}, {6, 1}, {6, 5},
};

int x3[][2] = {
    {1, 2}, {1, 6}, {2, 1},
    {2, 3}, {2, 4}, {3, 5},
    {4, 2}, {4, 5}, {4, 7},
    {5, 3}, {6, 1}, {6, 5}
};

/**
 * 递归：
 * paths(S,E) = paths(a,E) + paths(b,E)
 * paths(a,E) = paths(c,E) + paths(d,E)
 * paths(b,E) = paths(c.E) + paths(e,E)
 */
bool isX(int row, int col) {
    for (int i = 0; i < sizeof(x)/sizeof(x[0]); i++) {
        if (x[i][0] == row & x[i][1] == col) {
            return true;
        }
    }
    return false;
}

bool isEnd(int row, int col) {
    if (N == row || N == col)
        return true;
    return false;
}

int count_paths1(int row, int col) {
    if (isX(row, col)) return 0;
    if (isEnd(row, col)) return 1;
    return count_paths1(row+1, col) + count_paths1(row, col+1); 
}

/**
 * 递归带缓存：
 * paths(S,E) = paths(a,E) + paths(b,E)
 * paths(a,E) = paths(c,E) + paths(d,E)
 * paths(b,E) = paths(c.E) + paths(e,E)
 */
int grid2[N+1][N+1] = {0};

bool isX2(int row, int col) {
    for (int i = 0; i < sizeof(x)/sizeof(x[0]); i++) {
        if (x[i][0] == row & x[i][1] == col) {
            grid2[row][col] = 0;
            return true;
        }
    }
    return false;
}

bool isEnd2(int row, int col) {
    if (N == row || N == col) {
        if ((row == N && col <= N) || (row <= N && col == N)) {
            grid2[row][col] = 1;
        }
        return true;
    }
    return false;
}

int count_paths2(int row, int col) {
    if (isX2(row, col)) return 0;
    if (isEnd2(row, col)) return 1;
    if (!grid2[row+1][col])
        grid2[row+1][col] = count_paths2(row+1, col);
    if (!grid2[row][col+1])
        grid2[row][col+1] = count_paths2(row, col+1); 
    return grid2[row+1][col] + grid2[row][col+1];
}

/**
 *
 */
bool isX3(int row, int col) {
    for (int i = 0; i < sizeof(x3)/sizeof(x3[0]); i++) {
        if (x3[i][0] == row & x3[i][1] == col) {
            return true;
        }
    }
    return false;
}

int grid3[N+1][N+1] = {0};
int count_paths3(int row, int col) {
    for (int i = 0; i <= N; i++)
        grid3[0][i] = 1;
    for (int i = 0; i <= N; i++)
        grid3[i][0] = 1;
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            if (isX3(i, j))
                grid3[i][j] = 0;
            else
                grid3[i][j] = grid3[i-1][j] + grid3[i][j-1];
        }
    }
    return grid3[N][N-1] + grid3[N-1][N];
}

int main() {
    printf("%d\n", count_paths1(0, 0));
    printf("%d\n", count_paths2(0, 0));
    printf("%d\n", count_paths3(0, 0));
    for (int i = 0; i <= N; i++) {
        for (int j = 0; j <= N; j++)
            printf("%d ", grid3[i][j]);
        printf("\n");
    }
    return 0;
}

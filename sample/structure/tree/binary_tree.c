#include <stdio.h>
#include <stdlib.h>
typedef struct BNode BNode;
typedef struct BSTree BSTree;
typedef struct Heap Heap;
struct BNode {
    int e;
    BNode *L;
    BNode *R;
};
struct BSTree {
    BNode *root;
};
struct Heap {
    BNode *root;
};
void BSTInsert_r(BNode **x, int e) {
    if (NULL == x || NULL == (*x)) {
        (*x) = (BNode*) malloc(sizeof(BNode));
        (*x)->e = e;
        (*x)->L = (*x)->R = NULL;
    } else {
        if ((*x)->e > e)
            BSTInsert_r(&(*x)->L, e);
        else
            BSTInsert_r(&(*x)->R, e);
    }
}
void LevelOrder(BNode *x) {
    if (NULL != x) {
        printf("%d ", x->e);
        LevelOrder(x->L);
        LevelOrder(x->R);
    }
}
int main() {
    int A[] = {3,1,5,7,9,2,6,8,4};
    int n = sizeof(A)/sizeof(A[0]);
    int i;
    BNode *x;
    for (i = 0; i < n; i++) {
        BSTInsert_r(&x, A[i]);
    }
    LevelOrder(x);

    return 0;
}

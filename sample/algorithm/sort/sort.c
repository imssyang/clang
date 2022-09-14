#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void print(int A[], int p, int r) {
	for (int i = p; i <= r; i++) {
		printf("%d ", A[i]);
	}
    printf("\n");
}

void InsertSort(int A[], int n) {
    int i, j;
    for (i = 1; i < n; i++) {
        int tmp = A[i];
        for (j = i; j > 0 && tmp < A[j-1]; j--) {
            A[j] = A[j-1];
        }           
        A[j] = tmp;
    }         
}

void SelectSort(int A[], int n) {
    int i, j, min, tmp;
    for (i = 0; i < n-1; i++) {
        min = i;
        for (j = i+1; j < n; j++) {
            if (A[min] > A[j]) {
                min = j;
            }
        }
        tmp = A[i];
        A[i] = A[min];
        A[min] = tmp;
    }   
}   

void CycleSort(int A[], int n) {
    int i, j, p, m, tmp;
    for (i = 0; i < n; i++) {
        p = A[i];
        m = 0;
        for (j = i+1; j < n; j++) {
            if (A[j] < p)
                m++;
        }
        if (0 == m)
            continue;
        while (0 < m) {
            tmp = p;
            p = A[i+m];
            A[i+m] = tmp;

            m = 0;
            for (j = i+1; j < n; j++) {
                if (A[j] < p)
                    m++;
            }
        }
        A[i] = p;
    }
}

void BubbleSort(int A[], int n) {
    int i, j, tmp;
    for (i = 0; i < n-1; i++) {
        for (j = n-1; j > i; j--) {
            if (A[j] < A[j-1]) {
                tmp = A[j];
                A[j] = A[j-1];
                A[j-1] = tmp;
            }               
        }                   
    }
}

void CombSort(int A[], int n) {
    int i, j, step = n, tmp, again;
    while ((step = (int)(step/1.3)) > 1) {
        for (i = n-1; i >= step; i--) {
            if (A[i] < A[i-step]) {
                tmp = A[i];
                A[i] = A[i-step];
                A[i-step] = tmp;
            }
        }
    }
    again = 1;
    for (i = 0; i < n-1 && again; i++) {
        again = 0;
        for (j = n-1; j > i; j--) {
            if (A[j] < A[j-1]) {
                tmp = A[j];
                A[j] = A[j-1];
                A[j-1] = tmp;
                again = 1;
            }          
        }
    }
}

void ShellSort(int A[], int n) {
    int h, i, j, k, l, tmp;
    int *G = (int*)malloc(n/3*sizeof(int)); 
    for (h = 1, i = 0; h < n; h = 3*h+1) {
        G[i++] = h;
    }    
    for (i = i-1; i >= 0; i--) {
        h = G[i];
        for (j = h; j < 2*h; j++) {
            for (k = j; k < n; k += h) { 
                tmp = A[k];
                l = k; 
                for (; l-h >= 0 && tmp < A[l-h]; l -= h) {
                    A[l] = A[l-h];
                }
                A[l] = tmp;
            }
        }
    }
    free(G);
}

void ShellSort2(int data[], const int n) {
    register int i, j, hCnt, h;
    int increments[20], k;
	// create an appropriate number of increments h
    for (h = 1, i = 0; h < n; i++) {
        increments[i] = h;
        h = 3*h + 1;
    }
	// loop on the number of different increments h
    for (i--; i >= 0; i--) {
        h = increments[i];
		// loop on the number of subarrays h-sorted in ith pass
        for (hCnt = h; hCnt < 2*h; hCnt++) {
			printf("%d] ", hCnt);
			// insertion sort for subarray containing every hth element of array data
            for (j = hCnt; j < n; ) {
                int tmp = data[j];
                k = j;
                while (k-h >= 0 && tmp < data[k-h]) {
                    data[k] = data[k-h];
                    k -= h;
                }
                data[k] = tmp;
                j += h;
            }
        }
    }
}

typedef struct BSNode BSNode;
typedef struct BSTree BSTree;
struct BSNode {
    int e;
    BSNode *L;
    BSNode *R;
};
struct BSTree {
    BSNode *root;
};
void BSTInsert_i(BSTree *T, int e) {
    BSNode *z = NULL;
    BSNode *y = NULL;
    BSNode *x = T->root;
    while (NULL != x) {
        y = x;
        if (x->e > e)
            x = x->L;
        else
            x = x->R;
    }
    z = (BSNode*) malloc(sizeof(BSNode));
    z->e = e;
    z->L = z->R = NULL;
    if (NULL == y) {
        T->root = z;
    } else {
        if (y->e > e)
            y->L = z;
        else
            y->R = z;
    }
}
void BSTInsert_r(BSNode **x, int e) {
    if (NULL == x || NULL == (*x)) {
        (*x) = (BSNode*) malloc(sizeof(BSNode));
        (*x)->e = e;
        (*x)->L = (*x)->R = NULL;
    } else {
        if ((*x)->e > e)
            BSTInsert_r(&(*x)->L, e);
        else
            BSTInsert_r(&(*x)->R, e);
    }
}
void BSTInorder_r(const BSNode *x, int A[], int *i) {
    if (NULL != x) {
        BSTInorder_r(x->L, A, i);
        A[(*i)++] = x->e;
        BSTInorder_r(x->R, A, i);
    }
}
void BSTClear_r(BSNode *x) {
    if (NULL != x) {
        if (NULL != x->L)
            BSTClear_r(x->L);
        if (NULL != x->R)
            BSTClear_r(x->R);
        printf("%d}", x->e);
        free(x);
    }
}
void TreeSort(int A[], int n) {
    int i, j, t = 0;
    BSTree *T = (BSTree*) malloc(sizeof(BSTree));
    T->root = NULL;
    switch (t) {
        case 0:
            for (i = 0; i < n; i++)
                BSTInsert_r(&T->root, A[i]);
            j = 0;
            BSTInorder_r(T->root, A, &j);
            break;
        case 1:
            for (i = 0; i < n; i++)
                BSTInsert_i(T, A[i]);
            j = 0;
            BSTInorder_r(T->root, A, &j);
            break;
        default: break;
    }
    BSTClear_r(T->root);
    free(T);
}

void Heapify(int A[], int start, int end) {
    int L, R, largest, tmp;
    L = 2 * start + 1;
    while (L <= end) {
        largest = L;
        R = L + 1; 
        if (R <= end && A[largest] < A[R]) {
            largest = R;
        }          
        if (A[start] < A[largest]) {
            tmp = A[start];
            A[start] = A[largest];
            A[largest] = tmp;
            start = largest;
            L = 2 * start + 1;
        } else {
            L = end + 1;
        }      
    }
}   
void HeapSort(int A[], int n) {
    int i, tmp;
    for (i = n/2-1; i >= 0; i--) {
        Heapify(A, i, n-1);
    }
    for (i = n-1; i > 0; i--) {
        tmp = A[i];
        A[i] = A[0];
        A[0] = tmp;
        Heapify(A, 0, i-1);
    }                
}  


int Partition_l(int A[], int p, int r) {
    int pivot, i, j, tmp;
    pivot = A[r];
    i = p;
    for (j = p; j <= r; j++) {
        if (A[j] < pivot) {
            tmp = A[i];
            A[i] = A[j];
            A[j] = tmp;
            i++;
        }
    }
    tmp = A[i];
    A[i] = A[r];
    A[r] = tmp;
    return i;
}
void QuickSort_lr(int A[], int p, int r) {
    int pivot; 
    if (p < r) {
        pivot = Partition_l(A, p, r);
        QuickSort_lr(A, p, pivot-1);
        QuickSort_lr(A, pivot+1, r);
    }  
}
int Partition_h(int A[], int p, int r) {
    int q, pivot, i, j, tmp;
    q = (int)floor((p+r)/2);
    pivot = A[q];
    i = p - 1;
    j = r + 1;
    for (;;) {
        do {
            i++;
        } while (A[i] < pivot);
        do {
            j--;
        } while (A[j] > pivot);
        if (i >= j)
            break;
        tmp = A[i];
        A[i] = A[j];
        A[j] = tmp;
    }
    return j;
}
void QuickSort_hr(int A[], int p, int r) {
    int pivot;
    if (p < r) {
        pivot = Partition_h(A, p, r);
        QuickSort_hr(A, p, pivot);
        QuickSort_hr(A, pivot+1, r);
    }                     
}
void quickSort(int A[], int n) {
    int t = 1;
    if (0 == t) {
        QuickSort_lr(A, 0, n-1);
    } else {
        QuickSort_hr(A, 0, n-1);
    }
}

void Merge(int A[], int p, int mid, int r, int B[]) {
    int i, j, k;
    i = p;
    j = mid+1;
    k = p;
    while (i <= mid && j <= r) {
        if (A[i] < A[j]) {
            B[k++] = A[i++];
        } else {
            B[k++] = A[j++];
        }
    }
    while (i <= mid) {
        B[k++] = A[i++];
    }
    while (j <= r) {
        B[k++] = A[j++];
    }
    for (i = p; i <= r; i++) {
        A[i] = B[i];
    }
}
void MergeSort_t(int A[], int p, int r, int B[]) {
    int mid;
    if (p < r) {
        mid = (p+r)/2; 
        MergeSort_t(A, p, mid, B);
        MergeSort_t(A, mid+1, r, B);
        Merge(A, p, mid, r, B);
    }  
}      
void MergeSort_b(int A[], int n, int B[]) {
    int width, i, p, r;
    for (width = 1; width < n; width *= 2) {
        for (i = 0; i < n; i += 2*width) {
            p = i+width-1;
            r = i+width*2-1;
            p = p > n-1 ? n-1 : p;
            r = r > n-1 ? n-1 : r;
            Merge(A, i, p, r, B);
        }
    }       
}       
void MergeSort(int A[], int n) {
    int *B = (int*)malloc(n*sizeof(int));
    int t = 1;
    switch (t) {
        case 0: MergeSort_t(A, 0, n-1, B); break;
        case 1: MergeSort_b(A, n, B); break;
        default: break;
    }   
    free(B);
} 


void CountingSort(int A[], int n) {
    int *B;
    unsigned *C;
    int i, k;
    k = A[0];
    for (i = 1; i < n; i++) {
        if (k < A[i])
            k = A[i];
    }
    C = (unsigned*)malloc((k+1)*sizeof(unsigned));
    for (i = 0; i <= k; i++)
        C[i] = 0;
    for (i = 0; i < n; i++)
        C[A[i]]++;
    for (i = 1; i <= k; i++)
        C[i] += C[i-1];
    B = (int*)malloc(n*sizeof(int));
    for (i = n-1; i >= 0; i--) {
        B[C[A[i]]-1] = A[i];
        C[A[i]]--;
    }
    for (i = 0; i < n; i++)
        A[i] = B[i];
    free(B);
    free(C);
}

int QCompare(const void *p1, const void *p2) {
    int n1, n2;
    n1 = *(int*)p1;
    n2 = *(int*)p2;
    return n1 > n2;
}
void QSortDemo(int A[], int n) {
    qsort(A, n, sizeof(int), QCompare);
}

void Print(int A[], int n) {
	for (int i = 0; i < n; i++) {
		printf("%d ", A[i]);
	}
}

int main() {
	int A[] = {3,1,5,7,9,2,6,8,4};
    int n = sizeof(A)/sizeof(A[0]);
	Print(A, n);
    printf("\n");
	//InsertSort(A, n);
	//SelectSort(A, n);
	//CycleSort(A, n);
	//BubbleSort(A, n);
	//CombSort(A, n);
	//ShellSort(A, n);
    //TreeSort(A, n);
	//HeapSort(A, n);
    //QuickSort(A, n);
	//MergeSort(A, n);
    //CountingSort(A, n);
	QSortDemo(A, n);
	Print(A, n);

	return 0;
}


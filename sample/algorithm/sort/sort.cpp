#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <memory>
#include <stack>
#include <vector>
using namespace std;
void InsertSort(vector<int>& A) {
    for (int i = 1; i < A.size(); i++) {
        int tmp = A[i];
		int j = i;
        for (; j > 0 && tmp < A[j-1]; j--) {
            A[j] = A[j-1];
        }           
        A[j] = tmp; 
    }         
}

void SelectSort(vector<int>& A) {
    int n = A.size();
    for (int i = 0; i < n-1; i++) {
        int min = i;
        for (int j = i+1; j < n; j++) {
            if (A[min] > A[j]) {
                min = j;
            }  
        }      
        swap(A[i], A[min]);
    }               
}   

void CycleSort(vector<int>& A) {
    int n = A.size();
    for (int i = 0; i < n; i++) {
        int p = A[i];
        int m = 0;
        for (int j = i+1; j < n; j++) {
            if (A[j] < p)
                m++;
        }
        if (0 == m)
            continue;
        while (0 < m) {
            swap(p, A[i+m]);
            m = 0;
            for (int j = i+1; j < n; j++) {
                if (A[j] < p) 
                    m++; 
            }
        }
        A[i] = p;
    }
}

void BubbleSort(vector<int>& A) {
    int n = A.size();
    for (int i = 0; i < n-1; i++) {
        for (int j = n-1; j > i; j--) {
            if (A[j] < A[j-1]) {
                swap(A[j], A[j-1]);
            }               
        }                   
    }
}

void CombSort(vector<int>& A) {
    int n = A.size();
    int step = n;  
    while ((step = (int)(step/1.3)) > 1) {
        for (int i = n-1; i >= step; i--) {
            if (A[i] < A[i-step]) {
                swap(A[i], A[i-step]);
            }
        }   
    }       
    bool again = true;
    for (int i = 0; i < n-1; i++) {
        again = false;
        for (int j = n-1; j > i; j--) {
            if (A[j] < A[j-1]) {
                swap(A[j], A[j-1]);
                again = true;
            }
        }
    }
}

void ShellSort(vector<int>& A) {
    stack<int> G;
    int n = A.size();
    for (int h = 1; h < n; h = 3*h+1) {
        G.push(h);
    }   
    while (!G.empty()) {
        int h = G.top();
        G.pop(); 
        for (int i = h; i < 2*h; i++) {
            for (int j = i; j < n; j++) {
                int tmp = A[j];
                int k = j;
                for (; k-h >= 0 && tmp < A[k-h]; k -= h) {
                    A[k] = A[k-h];
                }           
                A[k] = tmp; 
            }
        }
    }
}

class BSNode {
    typedef shared_ptr<BSNode> PBSNode;
public:
    BSNode(int e, PBSNode L = nullptr, PBSNode R = nullptr)
        : e(e), L(L), R(R) {}
    auto& getE() { return e; }
    auto& getL() { return L; }
    auto& getR() { return R; }
private:
    int e;
    PBSNode L;
    PBSNode R;
};
class BSTree {
    typedef shared_ptr<BSNode> PBSNode;
public:
    auto& getRoot() { return root; }
    void insert_R(PBSNode& x, int e) {
        if (!x) {
            x = make_shared<BSNode>(e);
        } else {
            if (x->getE() > e)
                insert_R(x->getL(), e);
            else
                insert_R(x->getR(), e);
        }
    }
    void inorder_R(PBSNode x, vector<int>& A, int& i) {
        if (x) {
            inorder_R(x->getL(), A, i);
            A[i++] = x->getE();
            inorder_R(x->getR(), A, i);
        }
    }
    void clear_R(PBSNode x) {
        if (x) {
            clear_R(x->getL());
            clear_R(x->getR());
            x = nullptr;
        }
    }
    void insert_I(int e) {
        PBSNode z = make_shared<BSNode>(e);
        decltype(z) y = nullptr;
        decltype(z) x = root;
        while (x) {
            y = x;
            if (x->getE() > e)
                x = x->getL();
            else
                x = x->getR();
        }
        if (!y) {
            root = z;
        } else {
            if (y->getE() > e)
                y->getL() = z;
            else
                y->getR() = z;
        }
    }
    void inorder_I(vector<int>& A) {
        int i = 0;
        PBSNode x = root;
        stack<decltype(x)> S;
        while (x) {
            while (x) {
                if (x->getR())
                    S.push(x->getR());
                S.push(x);
                x = x->getL();
            }
            x = S.top();
            S.pop();
            while (!S.empty() && !x->getR()) {
                A[i++] = x->getE();
                x = S.top();
                S.pop();
            }
            A[i++] = x->getE();
            if (S.empty())
                break;
            x = S.top();
            S.pop();
        }
    }
    void clear_I() {
        PBSNode x = root;
        stack<decltype(x)> S;
        while (x) {
            if (x->getR())
                S.push(x->getR());
            if (x->getL())
                S.push(x->getL());
            x = nullptr;
            if (S.empty())
                break;
            x = S.top();
            S.pop();
        }
    }
private:
    PBSNode root;
};
void TreeSort(vector<int>& A) {
    int t = 0;
    int n = A.size();
    BSTree T;
    switch(t) {
        case 0:
            {
                for (auto& e : A)
                    T.insert_R(T.getRoot(), e);
                int i = 0;
                T.inorder_R(T.getRoot(), A, i);
                T.clear_R(T.getRoot());
            }
            break;
        case 1:
            {
                for (auto& e : A)
                    T.insert_I(e);
                int i = 0;
                T.inorder_I(A);
                T.clear_I();
            }
            break;
        default:
            break;
    }
}

void Heapify(vector<int>& A, int start, int end) {
    int n = A.size();
    int L = 2 * start + 1;
    while (L <= end) {
        int largest = L;
        int R = L + 1;
        if (R <= end && A[largest] < A[R]) {
            largest = R;
        }          
        if (A[start] < A[largest]) {
            swap(A[start], A[largest]);
            start = largest;
            L = 2 * start + 1;
        } else {
            L = end + 1;
        }      
    }          
}              
void HeapSort(vector<int>& A) {
    int n = A.size();
    for (int i = n/2-1; i >= 0; i--) {
        Heapify(A, i, n-1);
    }                
    for (int i = n-1; i > 0; i--) {
        swap(A[0], A[i]);
        Heapify(A, 0, i-1);
    }                
}

int Partition_l(vector<int>& A, int p, int r) {
    int pivot = A[r];
    int i = p; 
    for (int j = p; j <= r; j++) {
        if (A[j] < pivot) {
            swap(A[i], A[j]);
            i++;
        }
    }
    swap(A[i], A[r]);
    return i;
}
void QuickSort_lr(vector<int>& A, int p, int r) {
    if (p < r) {
        int pivot = Partition_l(A, p, r);
        QuickSort_lr(A, p, pivot-1);
        QuickSort_lr(A, pivot+1, r);
    }
}
void QuickSort_lr2(vector<int>::iterator p, vector<int>::iterator r) {
    if (p < r) {
        // 选择最后一个元素作为支点元素pivot
        auto pivot = prev(r);
        // 根据pivot元素分区为[p,q)和[q,r)（pivot元素不参与分区过程）
        auto q = partition(p, pivot, [pivot](int& x) { return x < *pivot; });
        // 将pivot元素交换至正确位置
        iter_swap(pivot, q);
        // 递归分区[p,pivot)和[pivot+1, r)
        QuickSort_lr2(p, pivot);
        QuickSort_lr2(pivot+1, r);
    }
}
void QuickSort_ls(vector<int>& A, int p, int r) {
    stack<int> S;
    S.push(p);
    S.push(r);
    while (!S.empty()) {
        r = S.top();
        S.pop();
        p = S.top();
        S.pop();
        if (p < r) {
            int pivot = Partition_l(A, p, r);
            if (pivot-1 > p) {
                QuickSort_ls(A, p, pivot-1);
                S.push(p);
                S.push(pivot-1);
            }
            if (pivot+1 < r) {
                QuickSort_ls(A, pivot+1, r);
                S.push(pivot+1);
                S.push(r);
            }
        }
    }
}
int Partition_h(vector<int>& A, int p, int r) {
    int q = (int)floor((p+r)/2);
    int pivot = A[q];
    int i = p - 1;
    int j = r + 1;
    while (true) {
        do {
            i++;
        } while (A[i] < pivot);
        do {
            j--;
        } while (A[j] > pivot);
        if (i >= j)
            break;
        swap(A[i], A[j]);
    }  
    return j;
}   
void QuickSort_hr(vector<int>& A, int p, int r) {
    if (p < r) {
        int pivot = Partition_h(A, p, r);
        QuickSort_hr(A, p, pivot);
        QuickSort_hr(A, pivot+1, r);
    }                  
}                      
void QuickSort_hr2(vector<int>::iterator p, vector<int>::iterator r) {
    if (p < r) {
        // 选择中间元素作为支点元素pivot（偶数时偏左）
        auto pivot = *next(p, distance(p,r)/2);
        // 分区为[p,mid1)都是小于pivot的元素和[mid1,r)
        auto mid1 = partition(p, r, [pivot](int& x) { return x < pivot; });
        // 再分区一次，确保[mid2,r)都是大于Pivot的元素
        auto mid2 = partition(mid1, r, [pivot](int& x) { return x <= pivot; });
        QuickSort_hr2(p, mid1);
        QuickSort_hr2(mid2, r);
    }
}
void QuickSort(vector<int>& A) {
    int n = A.size();
    int t = 1; 
    switch (t) {
        case 0: QuickSort_lr(A, 0, n-1); break;
        case 1: QuickSort_lr2(A.begin(), A.end()); break;
        case 2: QuickSort_ls(A, 0, n-1); break;
        case 3: QuickSort_hr(A, 0, n-1); break;
        case 4: QuickSort_hr2(A.begin(), A.end()); break;
        default: break;
    }
}


void Merge(vector<int>& A, int p, int mid, int r, vector<int>& B) {
    int i = p;                                          
    int j = mid+1;
    int k = p;
    while (i <= mid && j <= r) {
        if (A[i] < A[j])
            B[k++] = A[i++];
        else
            B[k++] = A[j++];
    }   
    while (i <= mid)
        B[k++] = A[i++];
    while (j <= r)
        B[k++] = A[j++];
    for (int i = p; i <= r; i++)
        A[i] = B[i];
}              
void MergeSort_t(vector<int>& A, int p, int r, vector<int>& B) {
    if (p < r) {
        int mid = (p+r)/2;
        MergeSort_t(A, p, mid, B);
        MergeSort_t(A, mid+1, r, B);
        Merge(A, p, mid, r, B);
    }          
}              
void MergeSort_b(vector<int>& A, vector<int>& B) {
    int n = A.size();
    for (int width = 1; width < n; width *= 2) {
        for (int i = 0; i < n; i += width*2) {
            int p = min(i+width-1, n-1);
            int r = min(i+width*2-1, n-1);
            Merge(A, i, p, r, B);
        }      
    }          
}              
void MergeSort(vector<int>& A) {
    int t = 1;
    int n = A.size();
    vector<int> B(n);
    switch (t) {
        case 0: MergeSort_t(A, 0, n-1, B); break;
        case 1: MergeSort_b(A, B); break;
        default: break;
    }
}

void CountingSort(vector<int>& A) {
    int n = A.size();
    if (n < 2)
        return;
    int k = A[0];
    for (auto& e : A)
        k = max(k, e);
    vector<unsigned> C(k+1, 0);
    for (auto& e : A)
        C[e]++;
    for (int i = 1; i <= k; i++)
        C[i] += C[i-1];
    vector<int> B(n);
    for (int i = n-1; i >= 0; i--) {
        B[C[A[i]]-1] = A[i];
        C[A[i]]--;
    }
    A = B;
}

void BucketSort_k(vector<int>& A, int k) {
    int n = A.size();          
    if (n < 2)                 
        return;                
    int p = A[0];
    int r = A[0];
    for (auto& e : A) {
        if (p > e)
            p = e;
        if (r < e)
            r = e;
    }
    int m = ceil((double)(r-p+1)/k);
    vector<list<int>> B(k);
    for (auto& e : A) {
        int j = floor((e-p)/m);
        auto& L =  B[j];
        L.push_back(e);
    }
    for (auto& L : B) {
        L.sort();
    }
	int i = 0;
    for (auto& L : B) {
        for (auto& e : L) {
            A[i++] = e;
        }
    }
}       
void BucketSort(vector<int>& A) {
    BucketSort_k(A, 4);
}

void RadixSort_b(vector<int>& A, int d) {
    int n = A.size();
    int k = 10;
    vector<list<int>> B(k);
    for (int i = d-1; i >= 0; i--) {
        for (auto& e : A) {
            int pos = e/(int)pow(10, d-i-1)%10;
            B[pos].push_back(e);
        }               
        int idx = 0;    
        for (auto& L : B) {
            for (auto& e : L)
                A[idx++] = e;
            L.clear();
        }
    }
}
void RadixSort(vector<int>& A) {
    RadixSort_b(A, 3);
}

void SortDemo(vector<int>& A) {
    //sort(A.begin(), A.end(), less<int>());
    //sort(A.begin(), A.end(), greater<int>());
    sort(A.begin(), A.end(), [](int a, int b) { return a < b; });
}

void StableSortDemo(vector<int>& A) {
    //stable_sort(A.begin(), A.end(), less<int>());
    //sort(A.begin(), A.end(), greater<int>());
    sort(A.begin(), A.end(), [](int a, int b) { return a < b; });
}

void PartialSortDemo(vector<int>& A) {
    //partial_sort(A.begin(), A.begin()+3, A.end());
    partial_sort(A.begin(), A.begin()+3, A.end(), greater<int>());
}

void PartitionDemo(vector<int>& A) {
    partition(A.begin(), A.end(), [](int x) { return x%2==0; });
    is_partitioned(A.begin(), A.end(), [](int x) { return x%2==0; }) ? true : false;
}

void HeapSortDemo(vector<int>& A) {
    make_heap(A.begin(), A.end());
    sort_heap(A.begin(), A.end());
}

void Print(const vector<int>& A) {
for (int i = 0; i < A.size(); i++) {
cout << A[i] << " ";
	}
}

int main() {
	vector<int> A {-1,3,1,5,0,7,9,-2,2,6,8,4};
	Print(A);
    cout << endl;
	//vector<int> A {3,1,5,0,7,9,2,6,8,4};
	//InsertSort(A);
	//SelectSort(A);
    //CycleSort(A);
	//BubbleSort(A);
	//CombSort(A);
	//ShellSort(A);
    //TreeSort(A);
	//HeapSort(A);
	QuickSort(A);
	//MergeSort(A);
    //CountingSort(A);
    //BucketSort(A);
    //RadixSort(A);
    //SortDemo(A);
    //StableSortDemo(A);
    //PartialSortDemo(A);
    //PartitionDemo(A);
    //HeapSortDemo(A);
	Print(A);

	return 0;
}


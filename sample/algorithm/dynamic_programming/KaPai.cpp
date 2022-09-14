/**
 * 有N张卡牌堆成一摞，每张卡牌上都会有一个整数标记其分数。现有两个人要交替从牌堆顶拿牌，
 * 每次至少拿一张，至多拿M张，直到牌堆被拿完。每个人拿至手中的牌的分数和即为其最终得分。
 * 假设两个人都会采取最佳策略拿牌来使自己的得分最大化，请问先手拿牌的人的得分为多少？
 */

#include <iostream>
#include <vector>
using namespace std;

constexpr int M = 3;

int sampleIndex(vector<int>& kaPai, int start, int end) {
    vector<int> sums;
    sums.assign(end-start, 0);
    sums[0] = kaPai[start];
    for (int i = start + 1; i < end; i++) {
        sums[i-start] = sums[i-start-1] + kaPai[i];
    }
    int msIndex = 0;
    int maxSum = sums[0];
    for (int i = 1; i < sums.size(); i++) {
        if (sums[i] > maxSum) {
            maxSum = sums[i];
            msIndex = i;
        }
    }
    return start + msIndex;
}

int main() {
    int count = 0;
    int sum = 0;
    vector<int> kaPai {3, -5, 8, 7, 2, 9, -4, 6, 1};
    for (int i = 0; i < kaPai.size();) {
        if (count >= kaPai.size())
            break;

        int edIndex = i + M;
        edIndex = edIndex > kaPai.size() ? kaPai.size() : edIndex;
        int saIndex = sampleIndex(kaPai, i, edIndex);
        if (0 == count % 2) {
            for (int j = i; j <= saIndex; j++) {
                sum += kaPai[j];
            }
        }

        i = saIndex + 1;
        count++;
    }
    cout << sum;

    return 0;
}

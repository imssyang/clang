#include <vector>
#include "DoublyLinkedList.hpp"
#include "SinglyLinkedList.hpp"

using namespace std;

int main() {
    vector<int> arr {5, 2, 8, 9, 1, 4, 3, 6, 7};
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }

    cout << "\n--------SinglyLinkedLIst-------------\n";
    SinglyLinkedList<int> sl;
    for (int i = 0; i < arr.size(); i++) {
        if (5 > i) {
            sl.addToHead(arr[i]);
        } else {
            sl.addToTail(arr[i]);
        }
    }
    sl.print();
    sl.deleteNode(4);
    sl.print();
    sl.deleteFromHead();
    sl.deleteFromTail();
    sl.print();
    cout << sl.getElementByIndex(3)
        << sl.getElementByRIndex(3)
        << sl.hasElement(6)
        << sl.hasElement(7)
        << endl;
    sl.print();
    sl.reverse();
    sl.print();

    cout << "\n--------DoublyLinkedLIst-------------\n";
    DoublyLinkedList<int> dl;
    for (int i = 0; i < arr.size(); i++) {
        if (5 > i) {
            dl.addToHead(arr[i]);
        } else {
            dl.addToTail(arr[i]);
        }
    }
    dl.print();
    dl.deleteNode(4);
    dl.print();
    std::cout << dl.deleteFromHead()
        << dl.deleteFromTail()
        << dl.hasElement(6)
        << dl.hasElement(7)
        << endl;
    dl.print();
    dl.reverse();
    dl.print();
    return 0;
}

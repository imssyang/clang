#include "BinarySearchTree.hpp"

int main() {
    BST<int> bst;
    bst.insert(5);
    bst.insert(3);
    bst.insert(8);
    bst.insert(1);
    bst.insert(4);
    bst.insert(9);
    bst.insert(6);
    bst.insert(7);
    bst.insert(2);
    cout << "\nBreadthFirst:\n";
    bst.printBreadthFirst();
    cout << "\nPreorder:\n";
    bst.printPreorder();
    cout << "\nPreorderRecursive:\n";
    bst.printPreorderRecursive();
    cout << "\nInorder:\n";
    bst.printInorder();
    cout << "\nInorderRecursive:\n";
    bst.printInorderRecursive();
    cout << "\nPostorder:\n";
    bst.printPostorder();
    cout << "\nPostorderRecursive:\n";
    bst.printPostorderRecursive();

    return 0;
}

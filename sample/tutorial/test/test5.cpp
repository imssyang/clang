#include <iostream>
#include <typeinfo>

using namespace std;

int main() {
    int i = 1;
    float j = 1.0;
    char c = 'a';

    const type_info& ti = typeid(i);
    const type_info& tj = typeid(j);
    const type_info& tk = typeid(i + j);
    const type_info& tc = typeid(c);

    cout << ti.name() << endl; // i
    cout << tj.name() << endl; // f
    cout << tk.name() << endl; // f
    cout << tc.name() << endl; // c
    cout << (ti == tj) << endl; // 0
    cout << (tk == tj) << endl; // 1

    return 0;
}


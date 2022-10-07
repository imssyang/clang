#include <iostream>
#include <unordered_set>
#include <algorithm>

struct StringHasher {
public:
    size_t operator()(const std::string& s) const {
        return std::hash<int>()(s.length());
    }
};

struct StringComparator {
public:
    bool operator()(const std::string& s1, const std::string& s2) const {
        return s1.length() == s2.length();
    }
};

int main() {
    std::unordered_set<std::string, StringHasher, StringComparator> US;
    US.insert("a");
    US.insert("b");
    US.insert("ccc");
    US.insert("dddd");
    for (const std::string& s : US)
        std::cout << s;
    return 0;
}
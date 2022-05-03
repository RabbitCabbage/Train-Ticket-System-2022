#include "System.hpp"
#include "tools.hpp"
#include <iostream>

void file() {
    freopen("../testdata/basic_1/1.in", "r", stdin);
}

hnyls2002::System sys;

int main() {
    //file();
    std::string str;
    while (getline(std::cin, str)) {
        auto s = hnyls2002::splitCN(str);
        for (auto x: s)
            std::cout << x << std::endl;
    }
    return 0;
}
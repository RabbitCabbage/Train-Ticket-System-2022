#include "System.hpp"
#include "tools.hpp"
#include <iostream>

void file() {
    freopen("../testdata/basic_1/1.in", "r", stdin);
}

hnyls2002::System sys;

void test() {
    srand((unsigned) time(NULL));
    sjtu::map<std::pair<hnyls2002::fstr<100>, int>, int> mp;
    for (int i = 1; i <= 1000; ++i) {
        std::string tmp;
        for (int j = 1; j <= 10; ++j)
            tmp += rand() % 26 + 'a';
        for (int j = 10; j >= 1; --j)
            mp[{tmp, j}] = i;
    }
    auto it = mp.begin();
    while (it != mp.end()) {
        std::cout << it->first.first << " " << it->first.second << " " << it->second << std::endl;
        ++it;
    }
}

void test1() {
    hnyls2002::fstr<100> a("asdasdas");
    std::cout << a << std::endl;
    std::cout << a.to_string().size() << std::endl;
}

int main() {
    /*file();
    std::string str;
    while (getline(std::cin, str)) {
        auto s = hnyls2002::splitCN(str);
        for (auto x: s)
            std::cout << x << std::endl;
    }*/
    test1();
    std::cout << (char) ('9' + 1) << std::endl;
    return 0;
}
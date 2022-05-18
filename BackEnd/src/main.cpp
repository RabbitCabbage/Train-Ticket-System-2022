#include "System.hpp"
#include "tools.hpp"
#include "BPlusTree.h"
#include <iostream>
#include <random>

void file() {
    freopen("../testdata/basic_1/1.in", "r", stdin);
}

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

void test2() {
    std::string str;
    std::cin >> str;
    hnyls2002::Date t = str;
    std::cout << t.mm << "\n" << t.dd << std::endl;
}

void test3() {
    using namespace hnyls2002;
    bptree<std::pair<fstr<100>, Date>, std::string> DayTrainDb;
    srand((unsigned) time(NULL));
    for (int i = 1; i <= 100; ++i) {
        std::string tmp;
        for (int j = 1; j <= 10; ++j)
            tmp += rand() % 26 + 'a';
        for (int i = 1; i <= 30; ++i) {
            Date day(7, i);
            DayTrainDb[{tmp, day}] = std::to_string(day.mm) + "-" + std::to_string(day.dd) + " " + tmp;
        }
    }
    for (auto it: DayTrainDb)
        std::cout << it.second << std::endl;
}

void test4() {
    freopen("../testdata/xjb.out", "w", stdout);
    using namespace hnyls2002;
    Time a(1, 1, 0, 0);
    for (int i = 0; i < 365 * 24 * 60; ++i)
        std::cout << (a + i).to_string() << std::endl;
}

void logic_test() {
    hnyls2002::System sys;
    freopen("../testdata/basic_1/my.in", "r", stdin);
    freopen("../testdata/basic_1/my.out", "w", stdout);
    std::string str;
    while (getline(std::cin, str)) {
        // std::cout << str.substr(0, 7) << " ";
        auto res = sys.Opt(str);
        for (auto it: res)
            std::cout << it << std::endl;
    }
}

static ds::BPlusTree<int, hnyls2002::fstr<200> > mp("../data/index", "../data/record");

void show() {
    std::cout << "size = " << mp.GetSize() << std::endl;
    auto it = mp.FindBigger(0);
    for (; !it.AtEnd(); ++it)
        std::cout << (*it).first << " " << (*it).second << std::endl;
}


void add(int l, int r, int len) {
    srand((unsigned) time(NULL));
    for (int i = l; i <= r; ++i) {
        std::string tmp;
        for (int j = 1; j <= len; ++j) {
            tmp += 'a' + rand() % 26;
        }
        mp.Insert(i, tmp);
    }
}

void erase(int l, int r) {
    std::cerr << "Erasing\n";
    for (int i = l; i <= r; ++i) {
        auto res = mp.Remove(i);
        if (res)std::cout << "erase " << i << " Success\n";
        else std::cout << "erase " << i << "Failed\n";
    }
}

int main() {

    clock_t sts = clock();

    using namespace hnyls2002;

    srand((unsigned) time(NULL));

    std::string cmd;

    while (std::getline(std::cin, cmd)) {
        std::cout << "Commander is " << cmd << "\n";
        auto arg = split_cmd(cmd, ' ');
        if (arg[0] == "add")add(std::stoi(arg[1]), std::stoi(arg[2]), std::stoi(arg[3]));
        else if (arg[0] == "erase")erase(std::stoi(arg[1]), std::stoi(arg[2]));
        else if (arg[0] == "show")show();
        else if (arg[0] == "exit")break;
    }

    clock_t end = clock();
    std::cout << (end - sts) / CLOCKS_PER_SEC << std::endl;

    return 0;
}
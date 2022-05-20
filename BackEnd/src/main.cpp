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

void test_logic() {
    freopen("./testdata/basic_2/my.in", "r", stdin);
    freopen("./testdata/basic_2/my.out", "w", stdout);
    clock_t sts = clock();
    hnyls2002::System sys;
    std::string str;
    while (getline(std::cin, str)) {
        std::cerr << "Command : " << str << std::endl;
        auto res = sys.Opt(str);
        //std::cout << str.substr(0, 6) << std::endl;
        for (auto it: res) {
            std::cout << it << std::endl;
            std::cerr << it << std::endl;
        }
    }
    clock_t end = clock();
    std::cerr << "Running Time : Using  " << (double) (end - sts) / CLOCKS_PER_SEC << " seconds " << std::endl;
}

static ds::BPlusTree<int, hnyls2002::fstr<9000> > mp("../data/index", "../data/record");

void show() {
    auto it = mp.FindBigger(0);
    if (!it.AtEnd())std::cout << "Not at end" << std::endl;
    for (; !it.AtEnd(); ++it)
        std::cout << (*it).first << " , " << (*it).second << std::endl;
    std::cout << "size = " << mp.GetSize() << std::endl;
}

void lower_bound(int x) {
    auto it = mp.FindBigger(x);
    if (!it.AtEnd())std::cout << "Not at end" << std::endl;
    for (; !it.AtEnd(); ++it)
        std::cout << (*it).first << " , " << (*it).second << std::endl;
}

void add(int l, int r, int len) {
    srand((unsigned) time(NULL));
    int las = -1, status = 0;
    for (int i = l; i <= r; ++i) {
        std::string tmp;
        for (int j = 1; j <= len; ++j)
            tmp += 'a' + rand() % 26;
        auto res = mp.Insert(i, tmp);
        if (las == -1)las = i, status = res;
        if (res != status) {
            std::cout << "Inserting...\n" << (status ? "success" : "failed") << " : from " << las << " to " << i - 1
                      << "\n";
            las = i, status = res;
        }
        if (i == r) {
            std::cout << "Inserting...\n" << (status ? "success" : "failed") << " : from " << las << " to " << i
                      << "\n";
        }
    }
}

void erase(int l, int r) {
    int las = -1, status = 0;
    for (int i = l; i <= r; ++i) {
        auto res = mp.Remove(i);
        if (las == -1)las = i, status = res;
        if (res != status) {
            std::cout << "Erasing...\n" << (status ? "success" : "failed") << " : from " << las << " to " << i - 1
                      << "\n";
            las = i, status = res;
        }
        if (i == r) {
            std::cout << "Erasing...\n" << (status ? "success" : "failed") << " : from " << las << " to " << i
                      << "\n";
        }
    }
}

void test_bptree() {
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
        else if (arg[0] == "clear")mp.Clear();
        else if (arg[0] == "lower_bound")lower_bound(std::stoi(arg[1]));
        clock_t end = clock();
        std::cout << "Running Time : Using  " << (double) (end - sts) / CLOCKS_PER_SEC << " seconds " << std::endl;
    }

    clock_t end = clock();
    std::cout << "Running Time : Using  " << (double) (end - sts) / CLOCKS_PER_SEC << " seconds " << std::endl;

}

int main() {
    //test_logic();
    test_bptree();
    return 0;
}
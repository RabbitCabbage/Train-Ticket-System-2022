#include "System.hpp"
#include <iostream>

#include "RollBack.hpp"

void test_rollback() {
    hnyls2002::Stack<int, int, 1> stk("rollback_data0.txt");
    std::string str;
    while (getline(std::cin, str)) {
        auto cmd = hnyls2002::split_cmd(str, ' ');
        if (cmd[0] == "push")stk.push(0, std::stoi(cmd[1]), std::stoi(cmd[1]));
        else if (cmd[0] == "top")std::cout << stk.top() << std::endl;
        else if (cmd[0] == "pop") {
            std::cout << stk.pop().info << std::endl;
            std::cout << "now size is " << stk.size() << std::endl;
        } else if (cmd[0] == "size")std::cout << stk.size() << std::endl;
        else if (cmd[0] == "exit")break;
    }
}

int main() {
/*
    system("rm index*");
    system("rm record*");
    system("rm roll*");
    freopen("2.in", "r", stdin);
    freopen("my.out", "w", stdout);
*/
    std::ios::sync_with_stdio(false);
    clock_t sts = clock();
    hnyls2002::System sys;
    //sys.GetSize();
    //sys.GetCachedSize();
    std::string str;
    while (getline(std::cin, str))sys.Opt(str);
    clock_t end = clock();
    std::cerr << "Running Time : Using  " << (double) (end - sts) / CLOCKS_PER_SEC << " seconds " << std::endl;
    return 0;
}

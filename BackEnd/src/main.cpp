#include "System.hpp"
#include <iostream>

int main() {
/*
    system("rm index*");
    system("rm record*");
    freopen("./testdata/normal/basic_1/1.in", "r", stdin);
    freopen("./testdata/normal/basic_1/my.out", "w", stdout);
*/
    std::ios::sync_with_stdio(false);
    clock_t sts = clock();
    hnyls2002::System sys;
    //sys.GetSize();
    sys.GetCachedSize();
    std::string str;
    while (getline(std::cin, str))sys.Opt(str);
    clock_t end = clock();
    std::cerr << "Running Time : Using  " << (double) (end - sts) / CLOCKS_PER_SEC << " seconds " << std::endl;
    return 0;
}

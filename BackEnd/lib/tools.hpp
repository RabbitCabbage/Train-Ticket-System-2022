//
// Created by m1375 on 2022/5/8.
//

#ifndef TRAINTICKETSYSTEM_TOOLS_HPP
#define TRAINTICKETSYSTEM_TOOLS_HPP

#include "vector.hpp"
#include <string>
#include <iostream>

namespace hnyls2002 {

    template<int LEN>
    class fstr {
    public:
        char s[LEN];
        int siz;

        fstr() : siz(0) { s[0] = '\0'; }

        fstr(const std::string &str) {
            siz = str.size();
            for (int i = 0; i < siz; ++i)
                s[i] = str[i];
            s[siz] = '\0';
        }

        bool operator==(const fstr &oth) const {
            if (siz != oth.siz)return false;
            for (int i = 0; i < siz; ++i)
                if (s[i] != oth.s[i])return false;
            return true;
        }
    };

    sjtu::vector<std::string> splitCN(const std::string str) {
        sjtu::vector<std::string> ret;
        int len = str.size();
        std::string tmp;
        for (int i = 0; i < len; ++i) {
            if (str[i] != '|')tmp = tmp + str[i];
            else ret.push_back(tmp), tmp.clear();
        }
        if (!tmp.empty())ret.push_back(tmp);
        return ret;
    }

    struct Date {
        int mm, dd;
    };

    struct Time : private Date {
        int hh, mi;
    };

}

#endif //TRAINTICKETSYSTEM_TOOLS_HPP
//
// Created by m1375 on 2022/5/8.
//

#ifndef TRAINTICKETSYSTEM_TOOLS_HPP
#define TRAINTICKETSYSTEM_TOOLS_HPP

#include "vector.hpp"
#include <string>
#include <iostream>

namespace hnyls2002 {

    template<typename T>
    void sort(T *begin, T *end) {
        if (begin >= end - 1)return;
        int len = end - begin;
        T mid_val = begin[rand() % len], *l = begin, *r = end - 1;
        while (l <= r) {
            while (*l < mid_val)++l;
            while (mid_val < *r)--r;
            if (l <= r)std::swap(*l, *r), ++l, --r;
        }
        if (l < end)sort(l, end);
        if (r > begin)sort(begin, r + 1);
    }

    template<typename T>
    void sort(T *begin, T *end, T(*cmp)(const T &, const T &)) {
        if (begin >= end - 1)return;
        int len = end - begin;
        T mid_val = begin[rand() % len], *l = begin, *r = end - 1;
        while (l <= r) {
            while (cmp(*l, mid_val))++l;
            while (cmp(mid_val, *r))--r;
            if (l <= r)std::swap(*l, *r), ++l, --r;
        }
        if (l < end)sort(l, end);
        if (r > begin)sort(begin, r + 1);
    }

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

        bool operator==(const fstr<LEN> &oth) const {
            if (siz != oth.siz)return false;
            for (int i = 0; i < siz; ++i)
                if (s[i] != oth.s[i])return false;
            return true;
        }

        bool operator!=(const fstr<LEN> &oth) const {
            return !(*this == oth);
        }

        bool operator<(const fstr<LEN> &oth) const {
            return strcmp(s, oth.s) < 0;
        }

        std::string to_string() {
            return std::string(s, siz);
        }

        friend std::ostream &operator<<(std::ostream &os, fstr<LEN> value) {
            os << value.s;
            return os;
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
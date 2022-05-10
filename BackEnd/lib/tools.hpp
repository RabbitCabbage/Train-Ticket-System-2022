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

        fstr<LEN> &operator=(const fstr<LEN> &oth) {
            siz = oth.siz;
            memcpy(s, oth.s, sizeof(s));
            return *this;
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

    sjtu::vector<std::string> split_cmd(const std::string str, const char &ch) {
        sjtu::vector<std::string> ret;
        int len = str.size();
        std::string tmp;
        for (int i = 0; i < len; ++i) {
            if (str[i] != ch)tmp = tmp + str[i];
            else ret.push_back(tmp), tmp.clear();
        }
        if (!tmp.empty())ret.push_back(tmp);
        return ret;
    }

    int mon[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int mon_s[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

    struct Date {
        int mm, dd;

        Date(int _mm = 0, int _dd = 0) : mm(_mm), dd(_dd) {}

        Date(const std::string &str) {
            int p = 0;
            for (int b = str.size(); str[p] != '-' && p < b; ++p);
            mm = std::stoi(str.substr(0, p));
            dd = std::stoi(str.substr(p + 1, str.size() - 1 - p));
        }

        bool operator<(const Date &d) const {
            if (mm != d.mm)return mm < d.mm;
            return dd < d.dd;
        }

        Date operator+(int x) {
            Date ret = *this;
            ret.dd += x;
            if (ret.dd > mon[mm])ret.dd -= mon[mm], ++ret.mm;
            return ret;
        }
    };

    struct Time : private Date {
        int hr, mi;

        Time(int _mm = 0, int _dd = 0, int _hr = 0, int _mi = 0) : Date(_mm, _dd), hr(_hr), mi(_mi) {}

        Time(const std::string &str) : Date(0, 0) {
            int p = 0;
            for (int b = str.size(); str[p] != ':' && p < b; ++p);
            hr = std::stoi(str.substr(0, p));
            mi = std::stoi(str.substr(p + 1, str.size() - 1 - p));
        }

        Time(const Date &date, const Time &time) : Date(date) {// 取date的日期和time的时间部分
            hr = time.hr, mi = time.mi;
        }

        std::string to_string() {
            std::string ret;
            ret += (mm < 10 ? '0' + std::to_string(mm) : std::to_string(mm)) + '-';
            ret += (dd < 10 ? '0' + std::to_string(dd) : std::to_string(dd)) + ' ';
            ret += (hr < 10 ? '0' + std::to_string(hr) : std::to_string(hr)) + ':';
            ret += mi < 10 ? '0' + std::to_string(mi) : std::to_string(mi);
            return ret;
        }

        Time operator+(int x) {
            Time ret = *this;
            int sum = mi + 1 + hr * 60 + (dd - 1) * 24 * 60 + mon_s[mm - 1] * 24 * 60 + x;

            for (int i = 1; i <= 12; ++i)
                if (mon_s[i] * 24 * 60 >= sum) {
                    sum -= mon_s[i - 1] * 24 * 60;
                    ret.mm = i;
                    break;
                }
            for (int i = 1; i <= mon[ret.mm]; ++i)
                if (i * 24 * 60 >= sum) {
                    sum -= (i - 1) * 24 * 60;
                    ret.dd = i;
                    break;
                }
            for (int i = 0; i <= 23; ++i)
                if ((i + 1) * 60 >= sum) {
                    sum -= i * 60;
                    ret.hr = i, ret.mi = sum - 1;
                    break;
                }
            return ret;
        }

        Time &operator+=(int x) {
            *this = *this + x;
            return *this;
        }
    };

}

#endif //TRAINTICKETSYSTEM_TOOLS_HPP
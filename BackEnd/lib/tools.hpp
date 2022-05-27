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
    void sort(typename sjtu::vector<T>::iterator it1, typename sjtu::vector<T>::iterator it2,
              bool(*cmp)(const T &, const T &)) {
        if (it1 - it2 >= -1)return;
        int len = it2 - it1;
        T mid_val = *(it1 + rand() % len);
        auto l = it1, r = it2 - 1;
        while (l - r <= 0) {
            while (cmp(*l, mid_val))++l;
            while (cmp(mid_val, *r))--r;
            if (l - r <= 0)std::swap(*l, *r), ++l, --r;
        }
        if (l - it2 < 0)sort(l, it2, cmp);
        if (r - it1 > 0)sort(it1, r + 1, cmp);
    }

    template<int LEN>
    class fstr {
    public:
        char s[LEN]{};
        int siz;

        fstr() : siz(0) { s[0] = '\0'; }

        fstr(const std::string &str) {
            siz = (int) str.size();
            for (int i = 0; i < siz; ++i)
                s[i] = str[i];
            s[siz] = '\0';
        }

        fstr<LEN> &operator=(const fstr<LEN> &oth) {
            if (&oth == this)return *this;
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
            return !(operator==(oth));
        }

        bool operator<(const fstr<LEN> &oth) const {
            return strcmp(s, oth.s) < 0;
        }

        std::string to_string() const {
            return std::string(s, siz);
        }

        int to_int() const { return std::stoi(to_string()); }

        friend std::ostream &operator<<(std::ostream &os, fstr<LEN> value) {
            os << value.s;
            return os;
        }

    };

    sjtu::vector<std::string> split_cmd(const std::string &str, const char &ch) {
        sjtu::vector<std::string> ret;
        int len = (int) str.size();
        std::string tmp;
        for (int i = 0; i < len; ++i) {
            if (str[i] != ch)tmp += str[i];
            else ret.push_back(tmp), tmp.clear();
        }
        if (!tmp.empty())ret.push_back(tmp);
        return ret;
    }

    int mon[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int mon_s[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

    int time_to_int(int mm, int dd, int hr, int mi) {
        return mon_s[mm - 1] * 24 * 60 + (dd - 1) * 24 * 60 + hr * 60 + mi + 1;
    }

    struct Time;

    struct Date {
        int num_d;

        explicit Date(int num = 0) : num_d(num) {}

        Date(const std::string &str) {// 为了方便，设置成implicit
            auto tmp = split_cmd(str, '-');
            num_d = time_to_int(std::stoi(tmp[0]), std::stoi(tmp[1]), 0, 0);
        }

        explicit Date(const Time &time);

        bool operator==(const Date &oth) const { return num_d == oth.num_d; }

        bool operator<(const Date &oth) const { return num_d < oth.num_d; }

        Date operator+(int x) const { return Date(num_d + x * 24 * 60); }

        Date &operator+=(int x) { return *this = *this + x; }

        Date operator-(int x) const { return Date(num_d - x * 24 * 60); }

        Date &operator-=(int x) { return *this = *this - x; }

        friend int operator-(const Date &d1, const Date &d2) {
            return (d1.num_d - d2.num_d) / (24 * 60);
        }
    };

    struct Time {
        int num_t;

        explicit Time(int num = 0) { num_t = num; };

        // 取date的日期部分和time的时间部分
        explicit Time(const std::string &str1, const std::string &str2) {
            auto tmp1 = split_cmd(str1, '-');
            auto tmp2 = split_cmd(str2, ':');
            num_t = time_to_int(std::stoi(tmp1[0]), std::stoi(tmp1[1]), std::stoi(tmp2[0]), std::stoi(tmp2[1]));
        }

        std::string to_string() const {
            std::string ret;
            int sum = num_t, mm, dd, hr, mi;
            for (int i = 1; i <= 12; ++i)
                if (mon_s[i] * 24 * 60 >= sum) {
                    sum -= mon_s[i - 1] * 24 * 60;
                    mm = i;
                    break;
                }
            dd = (sum - 1) / (24 * 60) + 1, sum -= (dd - 1) * (24 * 60);
            hr = (sum - 1) / 60, mi = (sum - 1) % 60;
            ret += (mm < 10 ? '0' + std::to_string(mm) : std::to_string(mm)) + '-';
            ret += (dd < 10 ? '0' + std::to_string(dd) : std::to_string(dd)) + ' ';
            ret += (hr < 10 ? '0' + std::to_string(hr) : std::to_string(hr)) + ':';
            ret += mi < 10 ? '0' + std::to_string(mi) : std::to_string(mi);
            return ret;
        }

        Time operator+(int x) const { return Time(num_t + x); }

        Time &operator+=(int x) { return *this = *this + x; }

        friend int operator-(const Time &t1, const Time &t2) { return t1.num_t - t2.num_t; }

        bool operator<(const Time &oth) const { return num_t < oth.num_t; }

        Time DayStep(int x) const { return Time(num_t + x * 24 * 60); }// 这个时间往后跳x天
    };

    Date::Date(const Time &time) : num_d((time.num_t - 1) / (24 * 60) * (24 * 60)+1) {}// 取time的日期部分

}

#endif //TRAINTICKETSYSTEM_TOOLS_HPP
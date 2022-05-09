#ifndef TrainTicketSystem_System_HPP
#define TrainTicketSystem_System_HPP

#include "../lib/map.hpp"
#include "../lib/vector.hpp"
#include "../lib/linked_hashmap.hpp"
#include "../lib/tools.hpp"
#include "Commander.hpp"

#define bptree sjtu::map

namespace hnyls2002 {

    class System {

#define ret_value(x) ret_type(std::to_string(x))

        typedef sjtu::vector<std::string> ret_type;

        static const int UserNameMax = 21, PasswdMax = 31, NameMax = 16, mailAddMax = 31, privilegeMax = 3;
        static const int TrainIDMax = 21, StationNameMax = 31, StationNumMax = 105;

        struct UserInfo {
            fstr<UserNameMax> UserName;
            fstr<PasswdMax> Passwd;
            fstr<NameMax> Name;
            fstr<mailAddMax> maillAdd;
            fstr<privilegeMax> privilege;// 用户的权限直接用字符串的形式存储。第一个超级用户的权限为'9'+ 1 = ':'
        };

        bptree<fstr<UserNameMax>, UserInfo> UserDb;
        sjtu::map<fstr<UserNameMax>, bool> Logged;
        // 登录状态用一颗内存中的map，登录有second=true，没有登录则节点不存在（记得删除）

        struct TrainInfo {
            fstr<TrainIDMax> TrainID;
            fstr<StationNameMax> StName[StationNumMax];
            int StationNum, SeatNum, Prices[StationNumMax];
            Time StartTime, TravelTimes[StationNumMax], StopOverTimes[StationNumMax];
            Date SaleDate;
            char Type;
        };

        struct DayTrain {
            int RemainSeats[StationNumMax];
        };


        //reference to stackoverflow
        //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
        ret_type (System::*func[100])(const CmdType &) ={&System::add_user, &System::login, &System::logout,
                                                         &System::query_profile, &System::modify_profile,
                                                         &System::add_train, &System::delete_train,
                                                         &System::release_train, &System::query_train,
                                                         &System::query_ticket, &System::query_transfer,
                                                         &System::buy_ticket, &System::query_order,
                                                         &System::refund_ticket, &System::rollback,
                                                         &System::clean, &System::exit};

        ret_type Opt(const std::string &str) {
            CmdType a = Parser(str);
            return ret_type();
        }

        ret_type add_user(const CmdType &arg) {

            if (!UserDb.empty() && UserDb[arg['c']].privilege.to_string() <= arg['g'])// 用户总数非空 && 权限不满足要求
                return ret_value(-1);

            if (!UserDb.empty() && UserDb.find(arg['u']) != UserDb.end())// 不是第一次加 && 已经有这个用户了
                return ret_value(-1);

            UserInfo User;

            User.UserName = arg['u'];
            User.Passwd = arg['p'];
            User.Name = arg['n'];
            User.maillAdd = arg['m'];
            User.privilege = UserDb.empty() ? ":" : arg['g'];

            UserDb[arg['u']] = User;

            return ret_value(0);
        }

        ret_type login(const CmdType &arg) {
            if (UserDb.find(arg['u']) == UserDb.end())// 用户不存在
                return ret_value(-1);
            if (Logged.find(arg['u']) != Logged.end())//用户已经登录了
                return ret_value(-1);
            if (UserDb.find(arg['u'])->second.Passwd != arg['p'])// 密码不匹配
                return ret_value(-1);
            Logged[arg['u']] = true;
            return ret_value(0);
        }

        ret_type logout(const CmdType &arg) {
            if (Logged.find(arg['u']) == Logged.end())// 都没有登录，怎么登出呢？
                return ret_value(-1);
            Logged.erase(Logged.find(arg['u']));
            return ret_value(0);
        }

        ret_type query_profile(const CmdType &arg) {
        }

        ret_type modify_profile(const CmdType &arg) {
        }

        ret_type add_train(const CmdType &arg) {
        }

        ret_type delete_train(const CmdType &arg) {
        }

        ret_type release_train(const CmdType &arg) {
        }

        ret_type query_train(const CmdType &arg) {
        }

        ret_type query_ticket(const CmdType &arg) {
        }

        ret_type query_transfer(const CmdType &arg) {
        }

        ret_type buy_ticket(const CmdType &arg) {
        }

        ret_type query_order(const CmdType &arg) {
        }

        ret_type refund_ticket(const CmdType &arg) {
        }

        ret_type rollback(const CmdType &arg) {
        }

        ret_type clean(const CmdType &arg) {
        }

        ret_type exit(const CmdType &arg) {
        }

#undef ret_value

    };
}

#endif
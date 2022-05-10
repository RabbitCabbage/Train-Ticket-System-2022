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
        static const int TrainIDMax = 21, StNameMax = 31, StNumMax = 105;

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
            fstr<StNameMax> StName[StNumMax];
            int StNum, SeatNum, Prices[StNumMax];
            int TravelTimes[StNumMax], StopOverTimes[StNumMax];
            Time StartTime;
            std::pair<Date, Date> SaleDate;
            char Type;
            bool is_released;
        };

        bptree<fstr<TrainIDMax>, TrainInfo> TrainDb;

        struct DayTrain {
            int RemainSeats[StNumMax];
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

            if (!UserDb.empty()) { // 不是第一次创建用户时
                if (Logged.find(arg['c']) == Logged.end())return ret_value(-1);// 没有登录
                if (UserDb[arg['c']].privilege.to_string() <= arg['g'])return ret_value(-1);// 权限不满足要求
                if (UserDb.find(arg['u']) != UserDb.end())return ret_value(-1);// 已经存在用户u
            }

            UserInfo User;

            User.UserName = arg['u'], User.Passwd = arg['p'], User.Name = arg['n'], User.maillAdd = arg['m'];
            User.privilege = UserDb.empty() ? ":" : arg['g'];

            UserDb[arg['u']] = User;

            return ret_value(0);
        }

        ret_type login(const CmdType &arg) {
            if (UserDb.find(arg['u']) == UserDb.end())return ret_value(-1);// 用户不存在
            if (Logged.find(arg['u']) != Logged.end())return ret_value(-1);// 用户已经登录
            if (UserDb[arg['u']].Passwd != arg['p'])return ret_value(-1);// 密码不对
            Logged[arg['u']] = true;
            return ret_value(0);
        }

        ret_type logout(const CmdType &arg) {
            if (Logged.find(arg['u']) == Logged.end())return ret_value(-1);// 没有登录
            Logged.erase(Logged.find(arg['u']));
            return ret_value(0);
        }

        bool JudgeUserQM(const CmdType &arg) {// 用户c要查询（修改）用户u的时候
            if (Logged.find(arg['c']) == Logged.end())return false;// 没有登录
            if (UserDb.find(arg['u']) == UserDb.end())return false;// 没有用户u
            if (UserDb[arg['c']].privilege < UserDb[arg['u']].privilege)return false;// 不满足权限要求
            return true;
        }

        ret_type query_profile(const CmdType &arg) {
            if (!JudgeUserQM(arg))return ret_value(-1);
            UserInfo User = UserDb[arg['u']];
            ret_type ret;
            ret.push_back(User.UserName.to_string());
            ret.push_back(User.Name.to_string());
            ret.push_back(User.maillAdd.to_string());
            ret.push_back(User.privilege.to_string());// 注意一下，这里可能会返回":"。
            return ret;
        }

        ret_type modify_profile(const CmdType &arg) {
            if (!JudgeUserQM(arg))return ret_value(-1);
            if (!arg['g'].empty() && UserDb[arg['c']].privilege.to_string() <= arg['g'])return ret_value(-1);
            UserInfo User = UserDb[arg['u']];
            if (!arg['p'].empty())User.Passwd = arg['p'];
            if (!arg['n'].empty())User.Name = arg['n'];
            if (!arg['m'].empty())User.maillAdd = arg['m'];
            if (!arg['g'].empty())User.privilege = arg['g'];
            UserDb[arg['u']] = User;
            return query_profile(arg);// 直接利用上一个函数，可能常数会比较大？
        }

        ret_type add_train(const CmdType &arg) {
            if (TrainDb.find(arg['i']) != TrainDb.end())return ret_value(-1);// 已经存在，添加失败
            TrainInfo Train;
            Train.TrainID = arg['i'];
            Train.StNum = std::stoi(arg['n']);
            Train.SeatNum = std::stoi(arg['m']);
            Train.StartTime = arg['x'];
            Train.Type = arg['y'][0];
            Train.is_released = false;

            ret_type tmp = split_cmd(arg['s'], '|');// StationName
            for (int i = 0; i < Train.StNum; ++i)
                Train.StName[i] = tmp[i];

            tmp = split_cmd(arg['p'], '|'); // Prices n-1
            for (int i = 0; i < Train.StNum - 1; ++i)
                Train.Prices[i] = std::stoi(tmp[i]);

            tmp = split_cmd(arg['t'], '|');
            for (int i = 0; i < Train.StNum - 1; ++i)// TravelTimes n-1
                Train.TravelTimes[i] = std::stoi(tmp[i]);

            tmp = split_cmd(arg['o'], '|');
            for (int i = 0; i < Train.StNum - 2; ++i)// StopOverTimes n-2 , if only two , replaced with '_'
                Train.StopOverTimes[i] = std::stoi(tmp[i]);

            tmp = split_cmd(arg['d'], '|');
            Train.SaleDate.first = tmp[0];
            Train.SaleDate.second = tmp[1];

            TrainDb[Train.TrainID] = Train;

            return ret_value(0);
        }

        ret_type delete_train(const CmdType &arg) {
            if (TrainDb.find(arg['i']) == TrainDb.end())return ret_value(-1);// 没有这个车次
            if (TrainDb[arg['i']].is_released)return ret_value(-1);// 已经发布了
            TrainDb.erase(TrainDb.find(arg['i']));
            return ret_value(0);
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
#ifndef TrainTicketSystem_System_HPP
#define TrainTicketSystem_System_HPP

#include "../lib/map.hpp"
#include "../lib/vector.hpp"
#include "../lib/linked_hashmap.hpp"
#include "../lib/tools.hpp"
#include "Commander.hpp"

#include<map>

#define bptree std::map

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
            int StNum, SeatNum, Prices[StNumMax];// 用前缀和来存票价 i 到第i站的票价
            int TravelTimes[StNumMax], StopOverTimes[StNumMax];
            Time StartTime;
            std::pair<Date, Date> SaleDate;
            char Type;
            bool is_released;
            int TimeStamp;// 记录发布这个火车的时间戳，相当于一个TrainID
        };

        bptree<fstr<TrainIDMax>, TrainInfo> TrainDb;

        struct DayTrainInfo {
            int RemainSeats[StNumMax];// 第1项为SeatNum，以此类推
        };

        bptree<std::pair<fstr<TrainIDMax>, Date>, DayTrainInfo> DayTrainDb;

        // 对于每一个车站，存储有多少辆火车经过它 {站名，序号} 序号采用发布这个火车时的时间戳
        bptree<std::pair<fstr<StNameMax>, int>, fstr<TrainIDMax>> TrainSet;


        //reference to stackoverflow
        //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
        ret_type (System::* func[17])(const CmdType &) ={&System::add_user, &System::login, &System::logout,
                                                         &System::query_profile, &System::modify_profile,
                                                         &System::add_train, &System::delete_train,
                                                         &System::release_train, &System::query_train,
                                                         &System::query_ticket, &System::query_transfer,
                                                         &System::buy_ticket, &System::query_order,
                                                         &System::refund_ticket, &System::rollback,
                                                         &System::clean, &System::exit};

    public:
        ret_type Opt(const std::string &str) {
            CmdType a = Parser(str);
            return (this->*func[a.FuncID])(a);
        }

    private:

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
            std::string tmp;
            tmp += User.UserName.to_string() + " ";
            tmp += User.Name.to_string() + " ";
            tmp += User.maillAdd.to_string() + " ";
            tmp += User.privilege.to_string();// 注意一下，这里可能会返回":"。
            ret.push_back(tmp);
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
                Train.StName[i + 1] = tmp[i];

            tmp = split_cmd(arg['p'], '|'); // Prices n-1 前缀和 第0项为0
            Train.Prices[0] = 0;
            for (int i = 0; i < Train.StNum - 1; ++i)
                Train.Prices[i + 1] = Train.Prices[i] + std::stoi(tmp[i]);

            tmp = split_cmd(arg['t'], '|');
            for (int i = 0; i < Train.StNum - 1; ++i)// TravelTimes n-1
                Train.TravelTimes[i + 1] = std::stoi(tmp[i]);

            tmp = split_cmd(arg['o'], '|');
            for (int i = 0; i < Train.StNum - 2; ++i)// StopOverTimes n-2 , if only two , replaced with '_'
                Train.StopOverTimes[i + 1] = std::stoi(tmp[i]);

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

        ret_type release_train(const CmdType &arg) {// 先不把DayTrain加进去，有购票的时候再加？
            if (TrainDb.find(arg['i']) == TrainDb.end())return ret_value(-1);// 没有找到这辆车
            auto &tmp = TrainDb[arg['i']];// 引用的形式，后面可以直接修改tmp
            if (tmp.is_released)return ret_value(-1);// 已经发布了
            tmp.is_released = true;
            tmp.TimeStamp = arg.TimeStamp;

            // 火车发布了，就可以买票了，所以把这列车的时间戳加入TrainSet
            for (int i = 1; i <= tmp.StNum; ++i)
                TrainSet[{tmp.StName[i], tmp.TimeStamp}] = tmp.TrainID;
            return ret_value(0);
        }

        ret_type query_train(const CmdType &arg) {
            if (TrainDb.find(arg['i']) == TrainDb.end())return ret_value(-1);// 没有找到这辆车
            TrainInfo Train = TrainDb[arg['i']];
            Date Day = arg['d'];
            if (Day < Train.SaleDate.first || Train.SaleDate.second < Day)return ret_value(-1);// 这段时间不发车
            // 注意格式
            bool is_in = true;
            DayTrainInfo DayTrain;
            if (DayTrainDb.find({arg['i'], arg['d']}) == DayTrainDb.end())is_in = false;
            if (is_in) DayTrain = DayTrainDb[{arg['i'], arg['d']}];

            ret_type ret;
            ret.push_back(arg['i']);
            ret.push_back(std::string(1, Train.Type));

            Time Timeline(arg['d'], Train.StartTime);

            for (int i = 1; i <= Train.StNum; ++i) {
                std::string tmp;
                tmp += Train.StName[i].to_string() + " ";
                if (i > 1)Timeline += Train.TravelTimes[i - 1];
                tmp += i == 1 ? "xx-xx xx:xx" : Timeline.to_string();
                tmp += " -> ";
                if (i > 1 && i < Train.StNum)
                    Timeline = Timeline + Train.StopOverTimes[i - 1];
                tmp += i == Train.StNum ? "xx-xx xx:xx" : Timeline.to_string();
                tmp += " " + std::to_string(Train.Prices[i]) + " ";
                if (is_in)tmp += std::to_string(DayTrain.RemainSeats[i]);
                else tmp += std::to_string(Train.StNum);
                ret.push_back(tmp);
            }

            return ret;
        }

        struct TicketType {
            fstr<TrainIDMax> TrainID;
            int TravelTime, Cost, RemainSeat;
            Time Leaving, Arriving;
        };

        static bool cmp_time(const TicketType &t1, const TicketType &t2) {
            return t1.TravelTime < t2.TravelTime;
        }

        static bool cmp_cost(const TicketType &t1, const TicketType &t2) {
            return t1.Cost < t2.Cost;
        }

        ret_type query_ticket(const CmdType &arg) {
            auto it = TrainSet.lower_bound({arg['s'], 0});
            sjtu::vector<TicketType> tickets;
            for (; it != TrainSet.end() && it->first.first == arg['s']; ++it) {// 这里需要访问经过起点站的所有车次
                auto tmp = TrainDb[it->second];
                Time Basic_Time(6, 1, tmp.StartTime.hr, tmp.StartTime.mi);// 基准时间设定为儿童节
                Time Leaving_time = Basic_Time;// 计算到购票起点站的发车时间
                bool direction_flag = true, find_destination = false;
                int pl = 1;
                for (; pl <= tmp.StNum; ++pl) {
                    if (tmp.StName[pl] == arg['t']) {// 先有终点站，方向就不对了
                        direction_flag = false;
                        break;
                    }
                    if (pl > 1 && pl < tmp.StNum)
                        Leaving_time += tmp.TravelTimes[pl - 1] + tmp.StopOverTimes[pl - 1];// 计算出发时间
                    if (tmp.StName[pl] == arg['s']) { break; }
                }
                if (!direction_flag)continue;
                Time Arriving_time = Leaving_time;
                int pr = pl + 1;
                for (++pr; pr <= tmp.StNum; ++pr) {// 计算到达时间
                    if (pr > 1)Arriving_time += tmp.TravelTimes[pr - 1];
                    if (tmp.StName[pr] == arg['t']) {
                        find_destination = true;
                        break;
                    }
                    if (pr > 1 && pr < tmp.StNum)Arriving_time += tmp.StopOverTimes[pr - 1];
                }
                if (!find_destination)continue;
                // 计算时间
                int IntervalDays = Date(arg['d']) - Date(Basic_Time);
                Basic_Time = Basic_Time.DayStep(IntervalDays);
                Leaving_time = Leaving_time.DayStep(IntervalDays);
                Arriving_time = Arriving_time.DayStep(IntervalDays);
                // 统计剩余座位
                int RemainSeat = tmp.SeatNum;
                Date StartDate = Date(Basic_Time);
                if (DayTrainDb.find({tmp.TrainID, StartDate}) != DayTrainDb.end()) {// 如果DayTrain没有实例化，不需要改动RemainSeat
                    auto tmp1 = DayTrainDb[{tmp.TrainID, StartDate}];
                    for (int i = pl + 1; i <= pr; ++i)
                        RemainSeat = std::min(RemainSeat, tmp1.RemainSeats[i]);
                }
                TicketType tik;
                tik.TrainID = tmp.TrainID;
                tik.TravelTime = Arriving_time - Leaving_time;
                tik.Cost = tmp.Prices[pr] - tmp.Prices[pl];
                tik.RemainSeat = RemainSeat;
                tik.Leaving = Leaving_time;
                tik.Arriving = Arriving_time;
                tickets.push_back(tik);
            }
            auto cmp = arg['p'] == "cost" ? &System::cmp_cost : &System::cmp_time;
            sort(tickets.begin(), tickets.end(), cmp);
            ret_type ret;
            ret.push_back(std::to_string(tickets.size()));
            for (auto it: tickets) {
                std::string line;
                line += it.TrainID.to_string() + " " + arg['s'] + " " + it.Leaving.to_string() + " -> ";
                line += arg['t'] + " " + it.Arriving.to_string() + " ";
                line += std::to_string(it.Cost) + " " + std::to_string(it.RemainSeat);
                ret.push_back(line);
            }
            return ret;
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
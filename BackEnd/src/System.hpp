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
            fstr<mailAddMax> mailAdd;
            fstr<privilegeMax> privilege;
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
            int Get_Remain(TrainInfo &Train, const std::string &s, const std::string &t) {
                int Min = 0x3f3f3f3f;
                bool flag = false;
                for (int i = 1; i <= Train.StNum; ++i) {
                    if (flag)Min = std::min(Min, RemainSeats[i]);
                    if (Train.StName[i] == s)flag = true;
                    if (Train.StName[i] == t)break;
                }
                return Min;
            }

            void Modify(TrainInfo &Train, const std::string &s, const std::string &t, int x) {
                bool flag = false;
                for (int i = 1; i <= Train.StNum; ++i) {
                    if (flag)RemainSeats[i] -= x;
                    if (Train.StName[i] == s)flag = true;
                    if (Train.StName[i] == t)break;
                }
            }
        };

        bptree<std::pair<fstr<TrainIDMax>, Date>, DayTrainInfo> DayTrainDb;

        // 对于每一个车站，存储有多少辆火车经过它 {站名，序号} 序号采用发布这个火车时的时间戳
        // TrainSet 里面可以用来存这是第几个站，会好很多!!!
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
                if (UserDb[arg['c']].privilege.to_int() <= std::stoi(arg['g']))return ret_value(-1);// 权限不满足要求
                if (UserDb.find(arg['u']) != UserDb.end())return ret_value(-1);// 已经存在用户u
            }
            UserInfo User;
            User.UserName = arg['u'], User.Passwd = arg['p'], User.Name = arg['n'];
            User.mailAdd = arg['m'], User.privilege = arg['g'];
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
            if (arg['u'] == arg['c'])return true;// 用户名相同，不需要判断权限（坑！！！）
            if (UserDb[arg['c']].privilege.to_int() <= UserDb[arg['u']].privilege.to_int())return false;// 不满足权限要求
            return true;
        }

        ret_type query_profile(const CmdType &arg) {
            if (!JudgeUserQM(arg))return ret_value(-1);
            UserInfo User = UserDb[arg['u']];
            ret_type ret;
            std::string tmp;
            tmp += User.UserName.to_string() + " " + User.Name.to_string() + " ";
            tmp += User.mailAdd.to_string() + " " + User.privilege.to_string();
            ret.push_back(tmp);
            return ret;
        }

        ret_type modify_profile(const CmdType &arg) {
            if (!JudgeUserQM(arg))return ret_value(-1);
            if (!arg['g'].empty() && UserDb[arg['c']].privilege.to_int() <= std::stoi(arg['g']))return ret_value(-1);
            UserInfo User = UserDb[arg['u']];// 最好不用引用传递，引用似乎不能在外存上实现
            if (!arg['p'].empty())User.Passwd = arg['p'];
            if (!arg['n'].empty())User.Name = arg['n'];
            if (!arg['m'].empty())User.mailAdd = arg['m'];
            if (!arg['g'].empty())User.privilege = arg['g'];
            UserDb[arg['u']] = User;
            ret_type ret;
            std::string tmp;
            tmp += User.UserName.to_string() + " " + User.Name.to_string() + " ";
            tmp += User.mailAdd.to_string() + " " + User.privilege.to_string();
            ret.push_back(tmp);
            return ret;
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
            auto Train = TrainDb[arg['i']]; // 不采用引用的形式，不便于B+树的操作
            if (Train.is_released)return ret_value(-1);// 已经发布了
            Train.is_released = true;
            Train.TimeStamp = arg.TimeStamp;
            TrainDb[arg['i']] = Train;

            // 火车发布了，就可以买票了，所以把这列车的时间戳加入TrainSet
            for (int i = 1; i <= Train.StNum; ++i)
                TrainSet[{Train.StName[i], Train.TimeStamp}] = Train.TrainID;
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
            ret.push_back(arg['i'] + std::string(1, Train.Type));
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
                else tmp += std::to_string(Train.SeatNum);
                ret.push_back(tmp);
            }
            return ret;
        }

        struct TicketType {
            fstr<TrainIDMax> TrainID;
            int TravelTime, Cost, RemainSeat;
            Time Leaving, Arriving;

            std::string to_string(const std::string &s, const std::string &t) {
                std::string ret;
                ret += TrainID.to_string() + " " + s + " " + Leaving.to_string() + " -> ";
                ret += t + " " + Arriving.to_string() + " ";
                ret += std::to_string(Cost) + " " + std::to_string(RemainSeat);
                return ret;
            }
        };

        static bool cmp_ticket_time(const TicketType &t1, const TicketType &t2) {
            return t1.TravelTime < t2.TravelTime;
        }

        static bool cmp_ticket_cost(const TicketType &t1, const TicketType &t2) {
            return t1.Cost < t2.Cost;
        }

        struct TicketRequest {
            TrainInfo &Train;// 把列车信息设置成引用，不用查找了。
            fstr<StNameMax> From, To;
            Date Day;

            explicit TicketRequest(TrainInfo &train, const std::string &from, const std::string &to,
                                   const std::string &day) : Train(train), From(from), To(to), Day(day) {}

            explicit TicketRequest(TrainInfo &train, const fstr<StNameMax> &from, const std::string &to,
                                   const Date &day) : Train(train), From(from), To(to), Day(day) {}
        };

        // 改 : 一个专门check方向和目的地，一个用来计算Arriving 和 Leaving !!!
        // Prices 和ticket都可以分开写。!!!

        std::pair<int, TicketType> Get_Ticket(const TicketRequest &Info) {
            TicketType ret;
            auto Train = Info.Train;
            Time Basic_Time(6, 1, Train.StartTime.hr, Train.StartTime.mi);// 基准时间设定为儿童节
            Time Leaving_time = Basic_Time;// 计算到购票起点站的发车时间
            bool direction_flag = true, find_destination = false;
            int pl = 1;
            for (; pl <= Train.StNum; ++pl) {
                if (Train.StName[pl] == Info.To) {// 先有终点站，方向就不对了
                    direction_flag = false;
                    break;
                }
                if (pl > 1 && pl < Train.StNum)
                    Leaving_time += Train.TravelTimes[pl - 1] + Train.StopOverTimes[pl - 1];// 计算出发时间
                if (Train.StName[pl] == Info.From) { break; }
            }
            if (!direction_flag)return {-1, ret};// 方向不对
            Time Arriving_time = Leaving_time;// 到达目的地的时间
            int pr = pl + 1;
            for (++pr; pr <= Train.StNum; ++pr) {// 计算到达时间
                if (pr > 1)Arriving_time += Train.TravelTimes[pr - 1];
                if (Train.StName[pr] == Info.To) {
                    find_destination = true;
                    break;
                }
                if (pr > 1 && pr < Train.StNum)Arriving_time += Train.StopOverTimes[pr - 1];
            }
            if (!find_destination)return {-2, ret};// 找不到终点站
            // 计算时间
            int IntervalDays = Date(Info.Day) - Date(Leaving_time);
            Basic_Time = Basic_Time.DayStep(IntervalDays);
            Leaving_time = Leaving_time.DayStep(IntervalDays);
            Arriving_time = Arriving_time.DayStep(IntervalDays);
            // 注意一下车票的售卖区间
            if (Date(Basic_Time) < Train.SaleDate.first || Train.SaleDate.second < Date(Basic_Time))
                return {-3, ret};// 时间不对。
            // 统计剩余座位
            int RemainSeat = Train.SeatNum;
            Date StartDate = Date(Basic_Time);
            if (DayTrainDb.find({Train.TrainID, StartDate}) != DayTrainDb.end()) {
                // 如果DayTrain没有实例化，不需要改动RemainSeat
                auto DayTrain = DayTrainDb[{Train.TrainID, StartDate}];
                for (int i = pl + 1; i <= pr; ++i)
                    RemainSeat = std::min(RemainSeat, DayTrain.RemainSeats[i]);
            }
            if (RemainSeat == 0)return {-4, ret};// 没有票了
            ret.TrainID = Train.TrainID;
            ret.TravelTime = Arriving_time - Leaving_time;
            ret.Cost = Train.Prices[pr] - Train.Prices[pl];
            ret.RemainSeat = RemainSeat;
            ret.Leaving = Leaving_time;
            ret.Arriving = Arriving_time;
            return {0, ret};
        }

        ret_type query_ticket(const CmdType &arg) {
            auto it = TrainSet.lower_bound({arg['s'], 0});
            sjtu::vector<TicketType> tickets;
            for (; it != TrainSet.end() && it->first.first == arg['s']; ++it) {// 这里需要访问经过起点站的所有车次
                auto Train = TrainDb[it->second];
                TicketRequest Info(Train, arg['s'], arg['t'], arg['d']);
                auto tik = Get_Ticket(Info);
                if (tik.first == 0)tickets.push_back(tik.second);
            }
            auto cmp = arg['p'] == "cost" ? &System::cmp_ticket_cost : &System::cmp_ticket_time;
            sort(tickets.begin(), tickets.end(), cmp);
            ret_type ret;
            ret.push_back(std::to_string(tickets.size()));
            for (auto tik: tickets) ret.push_back(tik.to_string(arg['s'], arg['t']));
            return ret;
        }

        struct TransType {
            TicketType tik1, tik2;
            std::string trans;

            int get_time() const { return tik2.Arriving - tik1.Leaving; }

            int get_time1() const { return tik1.Arriving - tik1.Leaving; }

            int get_cost() const { return tik1.Cost + tik2.Cost; }
        };

        static bool cmp_trans_time(const TransType &t1, const TransType &t2) {
            if (t1.get_time() == t2.get_time())return t1.get_time1() < t2.get_time1();
            return t1.get_time() < t2.get_time();
        }

        static bool cmp_trans_cost(const TransType &t1, const TransType &t2) {
            if (t1.get_cost() == t2.get_cost())return t1.get_time1() < t2.get_time1();
            return t1.get_cost() < t2.get_cost();
        }

        ret_type query_transfer(const CmdType &arg) {
            auto it_s = TrainSet.lower_bound({arg['s'], 0});
            auto it_t = TrainSet.lower_bound({arg['t'], 0});
            sjtu::vector<TrainInfo> lis_s, lis_t;
            for (; it_s != TrainSet.end() && it_s->first.first == arg['s']; ++it_s)
                lis_s.push_back(TrainDb[it_s->second]);
            for (; it_t != TrainSet.end() && it_t->first.first == arg['s']; ++it_t)
                lis_t.push_back(TrainDb[it_t->second]);
            // 得到了两个list
            TransType tik;
            bool flag = false;
            auto cmp = arg['p'] == "cost" ? &System::cmp_trans_cost : &System::cmp_trans_time;
            for (auto &S: lis_s) {// 起点车次 S
                sjtu::linked_hashmap<std::string, bool> mp;
                for (int i = S.StNum; i >= 1; ++i) {// map 中存 -s 后面所有可以到的站 (-s) -> (-trans)
                    if (S.StName[i] == arg['s'])break;
                    mp[S.StName[i].to_string()] = true;
                }
                for (auto &T: lis_t) {// 终点车次 T
                    for (int i = 1; i <= T.StNum; ++i) {// 中转站，要求 -t 前面的车站 (-trans) -> (-t)
                        fstr<StNameMax> trans = T.StName[i];
                        if (trans == arg['t'])break;
                        if (mp.find(trans.to_string()) == mp.end())continue;// 没有这个站
                        auto tik1 = Get_Ticket(TicketRequest(S, arg['s'], trans.to_string(), arg['d']));
                        if (tik1.first < 0) continue;// 没有票了
                        Date Arrival = Date(tik1.second.Arriving);// 注意这里不仅仅需要满足日期，还要满足时间
                        for (auto d = Arrival;; d += 1) {
                            auto tik2 = Get_Ticket(TicketRequest(T, trans, arg['t'], d));
                            if (tik2.first == 0) {// 0 才是满足条件的 (-3 日期超出) (-4 没有票了)
                                // 这里记得还要判断一下是否大于上一次的Arriving Time!!!
                                if (!flag) tik = {tik1.second, tik2.second, trans.to_string()}, flag = true;
                                else tik = std::min(tik, {tik1.second, tik2.second}, cmp);
                                break;
                            }
                            if (tik2.first == -3)break;
                        }
                    }
                }
            }
            if (!flag)return ret_value(0);
            ret_type ret;
            ret.push_back(tik.tik1.to_string(arg['s'], tik.trans));
            ret.push_back(tik.tik2.to_string(tik.trans, arg['t']));
            return ret;
        }

        ret_type buy_ticket(const CmdType &arg) {
            if (Logged.find(arg['u']) == Logged.end())return ret_value(-1);// 没有登录
            if (TrainDb.find(arg['i']) == TrainDb.end())return ret_value(-1);// 没有这列车
            auto Train = TrainDb[arg['i']];
            if (!Train.is_released)return ret_value(-1);
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
            return ret_type{"bye"};
        }

#undef ret_value

    };
}

#endif
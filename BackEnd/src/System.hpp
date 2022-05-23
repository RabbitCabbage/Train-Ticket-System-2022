#ifndef TrainTicketSystem_System_HPP
#define TrainTicketSystem_System_HPP

#include "../lib/map.hpp"
#include "../lib/vector.hpp"
#include "../lib/linked_hashmap.hpp"
#include "../lib/tools.hpp"
#include "../db/BPlusTree.h"
#include "Commander.hpp"

#include<map>
#include <utility>

#define bptree std::map

#define STORAGE_DIR ""

namespace hnyls2002 {

    class System {

#define ret_value(x) ret_type(std::to_string(x))

        typedef sjtu::vector<std::string> ret_type;

        static const int UserNameMax = 21, PasswdMax = 31, NameMax = 16, mailAddMax = 31, privilegeMax = 3;
        static const int TrainIDMax = 21, StNameMax = 31, StNumMax = 105;

    public:

        static void get_size() {
            std::cerr << "fstr<UserNameMax> :  " << sizeof(fstr<UserNameMax>) << std::endl;
            std::cerr << "UserInfo : " << sizeof(UserInfo) << std::endl;
            std::cerr << "----------------------------------" << std::endl;

            std::cerr << "fstr<TrainIDMax> : " << sizeof(fstr<TrainIDMax>) << std::endl;
            std::cerr << "TrainInfo : " << sizeof(TrainInfo) << std::endl;
            std::cerr << "----------------------------------" << std::endl;

            std::cerr << "std::pair<fstr<TrainIDMax>,Data> : " << sizeof(std::pair<fstr<TrainIDMax>, Date>)
                      << std::endl;
            std::cerr << "DayTrainInfo : " << sizeof(DayTrainInfo) << std::endl;
            std::cerr << "----------------------------------" << std::endl;

            std::cerr << "std::pair<fstr<StNameMax>, int> : " << sizeof(std::pair<fstr<StNameMax>, int>) << std::endl;
            std::cerr << "std::pair<fstr<TrainIDMax>, int> : " << sizeof(std::pair<fstr<TrainIDMax>, int>) << std::endl;
            std::cerr << "----------------------------------" << std::endl;

            std::cerr << "std::pair<fstr<UserNameMax>, int> : " << sizeof(std::pair<fstr<StNameMax>, int>) << std::endl;
            std::cerr << "Order : " << sizeof(Order) << std::endl;
            std::cerr << "----------------------------------" << std::endl;

            std::cerr << "std::pair<std::pair<fstr<TrainIDMax>, Date>, int> : "
                      << sizeof(std::pair<std::pair<fstr<TrainIDMax>, Date>, int>) << std::endl;
            std::cerr << "PendType : " << sizeof(PendType) << std::endl;
            std::cerr << "----------------------------------" << std::endl;

        }

    private:

        struct UserInfo {
            fstr<UserNameMax> UserName;
            fstr<PasswdMax> Passwd;
            fstr<NameMax> Name;
            fstr<mailAddMax> mailAdd;
            fstr<privilegeMax> privilege;
            int OrderNum{};

            std::string to_string() {
                std::string ret;
                ret += UserName.to_string() + " " + Name.to_string() + " ";
                ret += mailAdd.to_string() + " " + privilege.to_string();
                return ret;
            }
        };

    public:
        System() : UserDb(STORAGE_DIR "index1", STORAGE_DIR "record1"),
                   TrainDb(STORAGE_DIR "index2", STORAGE_DIR "record2"),
                   DayTrainDb(STORAGE_DIR "index3", STORAGE_DIR "record3"),
                   TrainSet(STORAGE_DIR "index4", STORAGE_DIR "record4"),
                   OrderDb(STORAGE_DIR "index5", STORAGE_DIR "record5"),
                   PendDb(STORAGE_DIR "index6", STORAGE_DIR "record6") {
        }

    private:

        ds::BPlusTree<fstr<UserNameMax>, UserInfo, 146, 31> UserDb;
        //bptree<fstr<UserNameMax>, UserInfo> UserDb;

        sjtu::map<fstr<UserNameMax>, bool> Logged;
        // 登录状态用一颗内存中的map，登录有second=true，没有登录则节点不存在（记得删除）

        struct TrainInfo {
            fstr<TrainIDMax> TrainID;
            fstr<StNameMax> StName[StNumMax];
            int StNum{}, SeatNum{}, Prices[StNumMax]{};// 用前缀和来存票价 i 到第i站的票价
            int TravelTimes[StNumMax]{}, StopOverTimes[StNumMax]{};
            Time StartTime;
            std::pair<Date, Date> SaleDate;
            char Type{};
            bool is_released{};
            std::pair<Time, Time> TimeTable[StNumMax];
            int TimeStamp{};// 记录发布这个火车的时间戳，相当于一个TrainID
        };

        ds::BPlusTree<fstr<TrainIDMax>, TrainInfo, 146, 4> TrainDb;
        //bptree<fstr<TrainIDMax>, TrainInfo> TrainDb;

        struct DayTrainInfo {
            int RemainSeats[StNumMax]{};// 第1项为SeatNum，以此类推
            int Get_Remain(int l, int r) { // 从第l站坐到第r站
                int ret = 0x3f3f3f3f;
                for (int i = l + 1; i <= r; ++i)
                    ret = std::min(ret, RemainSeats[i]);
                return ret;
            }

            void Modify(int l, int r, int x) {
                for (int i = l + 1; i <= r; ++i)
                    RemainSeats[i] -= x;
            }
        };

        ds::BPlusTree<std::pair<fstr<TrainIDMax>, Date>, DayTrainInfo, 113, 9> DayTrainDb;
        //bptree<std::pair<fstr<TrainIDMax>, Date>, DayTrainInfo> DayTrainDb;

        // 对于每一个车站，存储有多少辆火车经过它 {站名，序号} 序号采用发布这个火车时的时间戳
        // TrainSet 里面可以用来存这是第几个站，会好很多!!!

        ds::BPlusTree<std::pair<fstr<StNameMax>, int>, std::pair<fstr<TrainIDMax>, int>, 101, 128> TrainSet;
        // bptree<std::pair<fstr<StNameMax>, int>, std::pair<fstr<TrainIDMax>, int> > TrainSet;


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
            if (UserDb.GetSize()) { // 不是第一次创建用户时
                if (Logged.find(arg['c']) == Logged.end())return ret_value(-1);// 没有登录
                if (UserDb[arg['c']].privilege.to_int() <= std::stoi(arg['g']))return ret_value(-1);// 权限不满足要求
                if (UserDb.Find(arg['u']).first)return ret_value(-1);// 已经存在用户u
            }
            UserInfo User;
            User.UserName = arg['u'], User.Passwd = arg['p'], User.Name = arg['n'];
            User.mailAdd = arg['m'], User.privilege = arg['g'];
            User.OrderNum = 0;
            //UserDb[arg['u']] = User;
            UserDb.Insert(arg['u'], User);
            return ret_value(0);
        }

        ret_type login(const CmdType &arg) {
            if (!UserDb.Find(arg['u']).first)return ret_value(-1);// 用户不存在
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
            if (!UserDb.Find(arg['u']).first)return false;// 没有用户u
            if (arg['u'] == arg['c'])return true;// 用户名相同，不需要判断权限（坑！！！）
            if (UserDb[arg['c']].privilege.to_int() <= UserDb[arg['u']].privilege.to_int())return false;// 不满足权限要求
            return true;
        }

        ret_type query_profile(const CmdType &arg) {
            if (!JudgeUserQM(arg))return ret_value(-1);
            UserInfo User = UserDb[arg['u']];
            return ret_type{User.to_string()};
        }

        ret_type modify_profile(const CmdType &arg) {
            if (!JudgeUserQM(arg))return ret_value(-1);
            if (!arg['g'].empty() && UserDb[arg['c']].privilege.to_int() <= std::stoi(arg['g']))return ret_value(-1);
            UserInfo User = UserDb[arg['u']];// 最好不用引用传递，引用似乎不能在外存上实现
            if (!arg['p'].empty())User.Passwd = arg['p'];
            if (!arg['n'].empty())User.Name = arg['n'];
            if (!arg['m'].empty())User.mailAdd = arg['m'];
            if (!arg['g'].empty())User.privilege = arg['g'];
            //UserDb[arg['u']] = User;
            UserDb.Modify(arg['u'], User);
            return ret_type{User.to_string()};
        }

        ret_type add_train(const CmdType &arg) {
            if (TrainDb.Find(arg['i']).first)return ret_value(-1);// 已经存在，添加失败
            TrainInfo Train;
            Train.TrainID = arg['i'];
            Train.StNum = std::stoi(arg['n']);
            Train.SeatNum = std::stoi(arg['m']);
            Train.Type = arg['y'][0];
            Train.is_released = false;// 初始都是未发布
            Train.TimeStamp = arg.TimeStamp;// 时间戳改成add_train的时间戳了。

            ret_type tmp = split_cmd(arg['s'], '|');// StationName
            for (int i = 0; i < Train.StNum; ++i)
                Train.StName[i + 1] = tmp[i];

            tmp = split_cmd(arg['p'], '|'); // Prices n-1 前缀和 第1项为0
            Train.Prices[0] = 0;
            for (int i = 0; i < Train.StNum - 1; ++i)
                Train.Prices[i + 2] = Train.Prices[i + 1] + std::stoi(tmp[i]);

            tmp = split_cmd(arg['t'], '|');
            for (int i = 0; i < Train.StNum - 1; ++i)// TravelTimes n-1项，从 1 开始
                Train.TravelTimes[i + 1] = std::stoi(tmp[i]);

            tmp = split_cmd(arg['o'], '|');
            for (int i = 0; i < Train.StNum - 2; ++i)// StopOverTimes n-2项，从1开始
                Train.StopOverTimes[i + 1] = std::stoi(tmp[i]);

            tmp = split_cmd(arg['d'], '|');
            Train.SaleDate.first = tmp[0];
            Train.SaleDate.second = tmp[1];
            Train.StartTime = Time(tmp[0], arg['x']);

            Time BasicTime(tmp[0], arg['x']);// 发售日期的第一趟车为基准的TimeTable
            for (int i = 1; i <= Train.StNum; ++i) {
                // first-Arriving second-Leaving
                if (i > 1)Train.TimeTable[i].first = (BasicTime += Train.TravelTimes[i - 1]);
                if (i > 1 && i < Train.StNum)Train.TimeTable[i].second = (BasicTime += Train.StopOverTimes[i - 1]);
                if (i == 1)Train.TimeTable[i].second = BasicTime;
            }

            //TrainDb[Train.TrainID] = Train;
            TrainDb.Insert(Train.TrainID, Train);

            return ret_value(0);
        }

        ret_type delete_train(const CmdType &arg) {
            if (!TrainDb.Find(arg['i']).first)return ret_value(-1);// 没有这个车次
            if (TrainDb[arg['i']].is_released)return ret_value(-1);// 已经发布了
            TrainDb.Remove(arg['i']);
            return ret_value(0);
        }

        ret_type release_train(const CmdType &arg) {// 先不把DayTrain加进去，有购票的时候再加？
            if (!TrainDb.Find(arg['i']).first)return ret_value(-1);// 没有找到这辆车
            auto Train = TrainDb[arg['i']]; // 不采用引用的形式，不便于B+树的操作
            if (Train.is_released)return ret_value(-1);// 已经发布了
            Train.is_released = true;
            //TrainDb[arg['i']] = Train;
            TrainDb.Modify(arg['i'], Train);

            // 火车发布了，就可以买票了，所以把这列车的时间戳加入TrainSet
            for (int i = 1; i <= Train.StNum; ++i) {
                //TrainSet[{Train.StName[i], Train.TimeStamp}] = {Train.TrainID, i};
                TrainSet.Insert({Train.StName[i], Train.TimeStamp}, {Train.TrainID, i});
            }
            return ret_value(0);
        }

        ret_type query_train(const CmdType &arg) {
            if (!TrainDb.Find(arg['i']).first)return ret_value(-1);// 没有找到这辆车
            TrainInfo Train = TrainDb[arg['i']];
            Date Day = arg['d'];
            if (Day < Train.SaleDate.first || Train.SaleDate.second < Day)return ret_value(-1);// 这段时间不发车
            // 注意格式
            bool is_in = true;
            DayTrainInfo DayTrain;
            if (!DayTrainDb.Find({arg['i'], arg['d']}).first)is_in = false;
            if (is_in) DayTrain = DayTrainDb[{arg['i'], arg['d']}];

            ret_type ret;
            ret.push_back(arg['i'] + ' ' + std::string(1, Train.Type));
            int IntervalDays = GetDate(Train, 1, arg['d']);
            for (int i = 1; i <= Train.StNum; ++i) {
                std::string tmp;
                tmp += Train.StName[i].to_string() + " ";
                tmp += (i == 1 ? "xx-xx xx:xx" : Train.TimeTable[i].first.DayStep(IntervalDays).to_string()) + " -> ";
                tmp += i == Train.StNum ? "xx-xx xx:xx" : Train.TimeTable[i].second.DayStep(IntervalDays).to_string();
                tmp += " " + std::to_string(Train.Prices[i]) + " ";
                // 这里查询的是从这一站到下一站的票数
                if (i == Train.StNum)tmp += "x";
                else if (is_in)tmp += std::to_string(DayTrain.RemainSeats[i + 1]);
                else tmp += std::to_string(Train.SeatNum);
                ret.push_back(tmp);
            }
            return ret;
        }

        struct TicketType {
            fstr<TrainIDMax> TrainID;
            int TravelTime{}, Cost{}, RemainSeat{};
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
            if (t1.TravelTime != t2.TravelTime) return t1.TravelTime < t2.TravelTime;
            return t1.TrainID < t2.TrainID;
        }

        static bool cmp_ticket_cost(const TicketType &t1, const TicketType &t2) {
            if (t1.Cost != t2.Cost) return t1.Cost < t2.Cost;
            return t1.TrainID < t2.TrainID;
        }

        std::pair<int, TicketType>
        Get_Ticket(TrainInfo &Train, int pl, int pr, const Date &Day) {// 确定了站台的编号，需要判断有无票，在不在售票时间？
            TicketType ret;
            int IntervalDays = GetDate(Train, pl, Day);
            Date BuyDate = Date(Train.StartTime.DayStep(IntervalDays));
            if (BuyDate < Train.SaleDate.first)return {-1, ret};// 售票区间前
            if (Train.SaleDate.second < BuyDate)return {-2, ret};// 售票区间后
            if (DayTrainDb.Find({Train.TrainID, BuyDate}).first)
                ret.RemainSeat = DayTrainDb[{Train.TrainID, BuyDate}].Get_Remain(pl, pr);
            else ret.RemainSeat = Train.SeatNum;// 没有实例化
            //if (!ret.RemainSeat)return {-3, ret};// 没有票了

            ret.Leaving = Train.TimeTable[pl].second.DayStep(IntervalDays);
            ret.Arriving = Train.TimeTable[pr].first.DayStep(IntervalDays);
            ret.TravelTime = ret.Arriving - ret.Leaving;
            ret.Cost = Train.Prices[pr] - Train.Prices[pl];
            ret.TrainID = Train.TrainID;
            return {0, ret};
        }

        // 一个专门check方向和目的地 !!! (顺便把编号也返回了)
        static int CheckTrain(TrainInfo &Train, int k, const std::string &Des) {// 返回Des的编号
            for (int i = k + 1; i <= Train.StNum; ++i)
                if (Train.StName[i] == Des)return i;
            return 0;
        }

        // 相当于推算出来Arriving和Leaving了
        static int GetDate(TrainInfo &Train, int pl, const Date &Day) {// 推算出火车票始发站的发车日期
            return Day - Date(Train.TimeTable[pl].second);
        }

        ret_type query_ticket(const CmdType &arg) {
            auto it = TrainSet.FindBigger({arg['s'], 0});
            sjtu::vector<TicketType> tickets;
            for (; !it.AtEnd() && (*it).first.first == arg['s']; ++it) {// 这里需要访问经过起点站的所有车次
                auto Train = TrainDb[(*it).second.first];
                auto pl = (*it).second.second;
                auto pr = CheckTrain(Train, pl, arg['t']);
                if (!pr)continue;// 查询车次是否合法
                auto tik = Get_Ticket(Train, pl, pr, arg['d']);
                //没有票也可以查到的哦！
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

            int get_cost() const { return tik1.Cost + tik2.Cost; }
        };

        static bool cmp_trans_time(const TransType &t1, const TransType &t2) {
            if (t1.get_time() != t2.get_time())return t1.get_time() < t2.get_time();
            else if (t1.get_cost() != t2.get_cost())return t1.get_cost() < t2.get_cost();
            else if (t1.tik1.TrainID != t2.tik1.TrainID)return t1.tik1.TrainID < t2.tik1.TrainID;
            return t1.tik2.TrainID < t2.tik2.TrainID;
        }

        static bool cmp_trans_cost(const TransType &t1, const TransType &t2) {
            if (t1.get_cost() != t2.get_cost())return t1.get_cost() < t2.get_cost();
            else if (t1.get_time() != t2.get_time())return t1.get_time() < t2.get_time();
            else if (t1.tik1.TrainID != t2.tik1.TrainID)return t1.tik1.TrainID < t2.tik1.TrainID;
            return t1.tik2.TrainID < t2.tik2.TrainID;
        }

        ret_type query_transfer(const CmdType &arg) {
            auto it_s = TrainSet.FindBigger({arg['s'], 0});
            auto it_t = TrainSet.FindBigger({arg['t'], 0});
            sjtu::vector<std::pair<TrainInfo, int> > lis_s, lis_t;
            for (; !it_s.AtEnd() && (*it_s).first.first == arg['s']; ++it_s)
                lis_s.push_back({TrainDb[(*it_s).second.first], (*it_s).second.second});
            for (; !it_t.AtEnd() && (*it_t).first.first == arg['t']; ++it_t)
                lis_t.push_back({TrainDb[(*it_t).second.first], (*it_t).second.second});
            // 得到了两个list
            TransType tik;
            bool flag = false;
            auto cmp = arg['p'] == "cost" ? &System::cmp_trans_cost : &System::cmp_trans_time;
            for (auto &S: lis_s) {// 起点车次 S
                sjtu::linked_hashmap<std::string, int> mp;
                for (int i = S.second + 1; i <= S.first.StNum; ++i) // map 中存 -s 后面所有可以到的站 (-s) -> (-trans)
                    mp[S.first.StName[i].to_string()] = i;
                for (auto &T: lis_t) {// 终点车次 T
                    for (int i = 1; i < T.second; ++i) {// 中转站，要求 -t 前面的车站 (-trans) -> (-t)
                        fstr<StNameMax> trans = T.first.StName[i];
                        if (mp.find(trans.to_string()) == mp.end())continue;// 没有这个站
                        int pl = S.second, pr = mp[trans.to_string()], ql = i, qr = T.second;
                        auto tik1 = Get_Ticket(S.first, pl, pr, arg['d']);
                        if (tik1.first < 0) continue;// 第一张票不合法
                        Time Arrival = std::max(tik1.second.Arriving,
                                                T.first.TimeTable[ql].second);
                        for (auto d = (Date) Arrival;; d += 1) {
                            auto tik2 = Get_Ticket(T.first, ql, qr, d);
                            if (tik2.first == -2)break;// 超过日期，可以退出循环
                            if (tik2.first < 0)continue;
                            if (tik2.second.Leaving < Arrival)continue;// 这里记得还要判断一下是否大于上一次的Arriving Time!!!
                            if (S.first.TrainID == T.first.TrainID) {
                                //if (GetDate(S.first, pl, arg['d']) == GetDate(T.first, ql, d))
                                continue;// 换乘的是同一辆车
                            }
                            if (!flag) tik = {tik1.second, tik2.second, trans.to_string()}, flag = true;
                            else tik = std::min(tik, {tik1.second, tik2.second, trans.to_string()}, cmp);
                            break;
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

        enum StatusType {
            success = 0, pending = 1, refunded = 2
        };

        const std::string StatusToString[3] = {"[success]", "[pending]", "[refunded]"};

        struct Order {
            StatusType Status{};
            TicketType tik;
            fstr<StNameMax> From, To;
            Date Day;
            int pl{}, pr{}, TimeStamp{};
        };

        ds::BPlusTree<std::pair<fstr<UserNameMax>, int>, Order, 101, 24> OrderDb;
        //bptree<std::pair<fstr<UserNameMax>, int>, Order> OrderDb;// 第二维存这是第几个订单

        struct PendType {
            fstr<UserNameMax> UserName;
            int TicketNum, pl, pr, id;// 存了车站顺序和订单的编号
        };

        ds::BPlusTree<std::pair<std::pair<fstr<TrainIDMax>, Date>, int>, PendType, 101, 93> PendDb;
        //bptree<std::pair<std::pair<fstr<TrainIDMax>, Date>, int>, PendType> PendDb;// 第二维存[-时间戳] 购票的时间戳

        ret_type buy_ticket(const CmdType &arg) {
            if (Logged.find(arg['u']) == Logged.end())return ret_value(-1);// 没有登录
            auto Train_res = TrainDb.Find(arg['i']);
            if (!Train_res.first)return ret_value(-1);// 没有这列车
            auto &Train = Train_res.second.second;
            if (!Train.is_released)return ret_value(-1);// 没有被release
            if (std::stoi(arg['n']) > Train.SeatNum)return ret_value(-1);// 当你特牛逼想买很多票的时候应该直接返回-1
            int TimeStampTrain = Train.TimeStamp;
            // 可能没有这个站
            auto TrainSet_res_f = TrainSet.Find({arg['f'], TimeStampTrain});
            auto TrainSet_res_t = TrainSet.Find({arg['t'], TimeStampTrain});
            if (!TrainSet_res_f.first)return ret_value(-1);
            if (!TrainSet_res_t.first)return ret_value(-1);
            int pl = TrainSet_res_f.second.second.second, pr = TrainSet_res_t.second.second.second;
            if (pl >= pr)return ret_value(-1);
            int IntervalDays = GetDate(Train, pl, arg['d']);
            Date Day = Date(Train.StartTime.DayStep(IntervalDays));
            if (Day < Train.SaleDate.first || Train.SaleDate.second < Day)return ret_value(-1);// 不在区间内
//            if (!DayTrainDb.Find({Train.TrainID, Day}).first) {// 没有实例化，现在实例化
            DayTrainInfo tmp;
            for (int i = 1; i <= Train.StNum; ++i)
                tmp.RemainSeats[i] = Train.SeatNum;
            //DayTrainDb[{Train.TrainID, Day}] = tmp;
            DayTrainDb.Insert({Train.TrainID, Day}, tmp);
//            }
            auto DayTrain = DayTrainDb[{Train.TrainID, Day}];
            auto User = UserDb[{arg['u']}];
            int RemainSeat = DayTrain.Get_Remain(pl, pr);
            int TicketNum = std::stoi(arg['n']);
            if (RemainSeat < TicketNum && (arg['q'].empty() || arg['q'] == "false")) return ret_value(-1);
            Order order;
            if (RemainSeat >= TicketNum) {// 可以买票
                DayTrain.Modify(pl, pr, TicketNum);
                //DayTrainDb[{Train.TrainID, Day}] = DayTrain;
                DayTrainDb.Modify({Train.TrainID, Day}, DayTrain);
                DayTrainDb.Modify({Train.TrainID, Day}, DayTrain);
                order.Status = success;
            } else {
                //PendDb[{{Train.TrainID, Day}, arg.TimeStamp}] = PendType{arg['u'], TicketNum, pl, pr, User.OrderNum + 1};
                PendDb.Insert({{Train.TrainID, Day}, arg.TimeStamp},
                              PendType{arg['u'], TicketNum, pl, pr, User.OrderNum + 1});
                order.Status = pending;
            }
            TicketType tik;
            tik.Leaving = Train.TimeTable[pl].second.DayStep(IntervalDays);
            tik.Arriving = Train.TimeTable[pr].first.DayStep(IntervalDays);
            tik.RemainSeat = TicketNum;
            // 这里Cost记录单价!!!
            tik.Cost = Train.Prices[pr] - Train.Prices[pl];
            tik.TrainID = arg['i'];
            order.tik = tik, order.From = arg['f'], order.To = arg['t'], order.Day = Day;
            order.pl = pl, order.pr = pr, order.TimeStamp = arg.TimeStamp;
            //OrderDb[{User.UserName, -(++User.OrderNum)}] = order;
            OrderDb.Insert({User.UserName, -(++User.OrderNum)}, order);
            //UserDb[arg['u']] = User;
            UserDb.Modify(arg['u'], User);
            if (order.Status == success)return ret_type{std::to_string(tik.Cost * TicketNum)};
            else return ret_type{"queue"};
        }

        ret_type query_order(const CmdType &arg) {
            if (Logged.find(arg['u']) == Logged.end())return ret_value(-1);// 没有登录
            ret_type ret;
            auto it = OrderDb.FindBigger({arg['u'], -0x3f3f3f3f});
            ret.push_back(std::to_string(UserDb[arg['u']].OrderNum));
            for (; !it.AtEnd() && (*it).first.first == arg['u']; ++it) {
                std::string tmp;
                tmp += StatusToString[(*it).second.Status] + ' ';
                tmp += (*it).second.tik.to_string((*it).second.From.to_string(), (*it).second.To.to_string());
                ret.push_back(tmp);
            }
            return ret;
        }

        ret_type refund_ticket(const CmdType &arg) {
            if (Logged.find(arg['u']) == Logged.end())return ret_value(-1);// 没有登录
            int tot_order = UserDb[arg['u']].OrderNum;
            int id = arg['n'].empty() ? tot_order : tot_order - std::stoi(arg['n']) + 1;
            if (id <= 0)return ret_value(-1);// 没有这么多订单
            auto order = OrderDb[{arg['u'], -id}];// order 要退的订单 order2 要候补的订单
            if (order.Status == refunded)return ret_value(-1);
            bool flag = true;
            if (order.Status == pending)flag = false;
            order.Status = refunded;
            //OrderDb[{arg['u'], -id}] = order;
            OrderDb.Modify({arg['u'], -id}, order);

            std::pair<fstr<TrainIDMax>, Date> info = {order.tik.TrainID, order.Day};
            if (!flag) {// 为假说明要把这个退订的候补在PendDb中删除
                PendDb.Remove({info, order.TimeStamp});
            } else {// 为真代表会有多的票空出来
                auto DayTrain = DayTrainDb[info];
                DayTrain.Modify(order.pl, order.pr, -order.tik.RemainSeat);

                // 处理候补的订单
                auto it = PendDb.FindBigger({info, 0});
                for (; !it.AtEnd() && (*it).first.first == info;) {
                    int RemainTickets = DayTrain.Get_Remain((*it).second.pl, (*it).second.pr);
                    if (RemainTickets >= (*it).second.TicketNum) {// 候补成功
                        auto order2 = OrderDb[{(*it).second.UserName, -(*it).second.id}];
                        order2.Status = success;
                        //OrderDb[{(*it).second.UserName, -(*it).second.id}] = order2;
                        OrderDb.Modify({(*it).second.UserName, -(*it).second.id}, order2);
                        DayTrain.Modify((*it).second.pl, (*it).second.pr, (*it).second.TicketNum);
                        PendDb.Remove((*it).first);// 可能一次删除很多个
                        ++it;
                    } else ++it;
                }
                //DayTrainDb[info] = DayTrain;
                DayTrainDb.Modify(info, DayTrain);
            }
            return ret_value(0);
        }

        ret_type rollback(const CmdType &arg) {
            return ret_value(0);
        }

        ret_type clean(const CmdType &arg) {
            return ret_value(0);
        }

        ret_type exit(const CmdType &arg) {
            Logged.clear();
            return ret_type{"bye"};
        }

#undef ret_value

    };
}

#endif
#ifndef TrainTicketSystem_System_HPP
#define TrainTicketSystem_System_HPP

#include "../lib/map.hpp"
#include "../lib/vector.hpp"
#include "../lib/linked_hashmap.hpp"
#include "../lib/tools.hpp"
#include "../db/BPlusTree.h"
#include "../db/CacheMap.h"
#include "Commander.hpp"
#include "RollBack.hpp"

#define STORAGE_DIR ""

namespace hnyls2002 {

    class System {

#define ret_value(x) ret_type(std::to_string(x))

        typedef sjtu::vector<std::string> ret_type;

        static const int UserNameMax = 21, PasswdMax = 31, NameMax = 16, mailAddMax = 31, privilegeMax = 3;
        static const int TrainIDMax = 21, StNameMax = 31, StNumMax = 101;
        static std::hash<std::string> Hash;

    public:

        System() : UserDb(STORAGE_DIR "index1", STORAGE_DIR "record1"),
                   BasicTrainDb(STORAGE_DIR "index2", STORAGE_DIR "record2"),
                   TrainDb(STORAGE_DIR "index3", STORAGE_DIR "record3"),
                   StDb(STORAGE_DIR "index4", STORAGE_DIR "record4"),
                   DayTrainDb(STORAGE_DIR "index5", STORAGE_DIR "record5"),
                   OrderDb(STORAGE_DIR "index6", STORAGE_DIR "record6"),
                   PendDb(STORAGE_DIR "index7", STORAGE_DIR "record7"),
                   UserRoll("roll1"),
                   BasicTrainRoll("roll2"),
                   TrainRoll("roll3"),
                   StRoll("roll4"),
                   DayTrainRoll("roll5"),
                   OrderRoll("roll6"),
                   PendRoll("roll7") {
        }

        void GetSize() const {
            UserDb.tree->GetSizeInfo();
            BasicTrainDb.tree->GetSizeInfo();
            TrainDb.tree->GetSizeInfo();
            StDb.tree->GetSizeInfo();
            DayTrainDb.tree->GetSizeInfo();
            OrderDb.tree->GetSizeInfo();
            PendDb.tree->GetSizeInfo();
        }

        static void GetCachedSize() {
            std::cerr << "UserDb : " << sizeof(UserDb) / 1e6 << std::endl;
            std::cerr << "BasicTrainDb : " << sizeof(BasicTrainDb) / 1e6 << std::endl;
            std::cerr << "TrainDb : " << sizeof(TrainDb) / 1e6 << std::endl;
            std::cerr << "StDb : " << sizeof(StDb) / 1e6 << std::endl;
            std::cerr << "DayTrainDb : " << sizeof(DayTrainDb) / 1e6 << std::endl;
            std::cerr << "OrderDb : " << sizeof(OrderDb) / 1e6 << std::endl;
            std::cerr << "PendDb : " << sizeof(PendDb) / 1e6 << std::endl;
        }

    private:

        struct UserInfo {
            fstr<UserNameMax> UserName;
            fstr<PasswdMax> Passwd;
            fstr<NameMax> Name;
            fstr<mailAddMax> mailAdd;
            fstr<privilegeMax> privilege;
            int OrderNum{};

            std::string to_string() const {
                std::string ret;
                ret += UserName.to_string() + " " + Name.to_string() + " ";
                ret += mailAdd.to_string() + " " + privilege.to_string();
                return ret;
            }
        };

        // UserName
        ds::CacheMap<size_t, UserInfo, 29989, 339, 29> UserDb;
        hnyls2002::Stack<size_t, UserInfo> UserRoll;

        // UserName
        sjtu::map<size_t, bool> Logged;

        struct BasicTrainInfo { // 列车最最基本的信息，每次构造System时先统一读入到内存里面，exit的时候再写会到外存中。
            bool is_released{};
            int StNum{}, SeatNum{};
            std::pair<Date, Date> SaleDate;
            char Type{};

            BasicTrainInfo() : SaleDate{0, 0} {
            }
        };

        // UserName
        ds::CacheMap<size_t, BasicTrainInfo, 49999, 339, 127> BasicTrainDb;// 外存对象
        hnyls2002::Stack<size_t, BasicTrainInfo> BasicTrainRoll;

        struct TrainInfo { // 列车的数据量最大的信息，只在query_train的时候会用到。
            fstr<StNameMax> StName[StNumMax];
            int Prices[StNumMax]{};// 用前缀和来存票价 i 到第i站的票价
            std::pair<Time, Time> TimeTable[StNumMax];

            TrainInfo() {}
        };

        //TrainID
        ds::CacheMap<size_t, TrainInfo, 1499, 339, 2> TrainDb;
        hnyls2002::Stack<size_t, TrainInfo> TrainRoll;

        struct StInfo {// 车站的信息，不同列车的相同车站都是不同的车站，维护了不同的信息。
            int Rank{}, Price{};// Rank是第几个车站，为了能够查询剩余票数,Price对应了Prices[]
            fstr<TrainIDMax> TrainID;
            Time Arriving{0}, Leaving{0};// 对应了TimeTable[]
        };

        class PairHash {
        public:
            size_t operator()(const std::pair<size_t, size_t> &x) {
                return x.first + x.second;
            }
        };

        //StName
        ds::CacheMap<std::pair<size_t, size_t>, StInfo, 69997, 208, 68, PairHash> StDb;
        hnyls2002::Stack<std::pair<size_t, size_t>, StInfo> StRoll;

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

        //TrainID
        class DayTrainHash {
        public:
            size_t operator()(const std::pair<size_t, Date> &x) {
                return x.first + x.second.num_d;
            }
        };

        ds::CacheMap<std::pair<size_t, Date>, DayTrainInfo, 9973, 208, 9, DayTrainHash> DayTrainDb;
        hnyls2002::Stack<std::pair<size_t, Date>, DayTrainInfo> DayTrainRoll;

        //reference to stackoverflow
        //https://stackoverflow.com/questions/26331628/reference-to-non-static-member-function-must-be-called
        void (System::* func[17])(const CmdType &) ={&System::add_user, &System::login, &System::logout,
                                                     &System::query_profile, &System::modify_profile,
                                                     &System::add_train, &System::delete_train,
                                                     &System::release_train, &System::query_train,
                                                     &System::query_ticket, &System::query_transfer,
                                                     &System::buy_ticket, &System::query_order,
                                                     &System::refund_ticket, &System::rollback,
                                                     &System::clean, &System::exit};

    public:

        void Opt(const std::string &str) {
            CmdType a = Parser(str);
            (this->*func[a.FuncID])(a);
        }

    private:

        void add_user(const CmdType &arg) {
            size_t u_h = Hash(arg['u']), c_h = Hash(arg['c']);
            if (UserDb.GetSize()) { // 不是第一次创建用户时
                // 没有登录 || 权限不满足要求 || 已经存在用户u
                if (Logged.find(c_h) == Logged.end() || UserDb[c_h].privilege.to_int() <= std::stoi(arg['g']) ||
                    UserDb.Find(u_h).first) {
                    std::cout << -1 << std::endl;
                    return;
                }//
            }
            UserInfo User;
            User.UserName = arg['u'], User.Passwd = arg['p'], User.Name = arg['n'];
            User.mailAdd = arg['m'], User.privilege = arg['g'];
            User.OrderNum = 0;
            UserDb.Insert(u_h, User);
            // delete the previous one ---- 1
            UserRoll.push(arg.TimeStamp * 10 + 1, u_h, {});
            std::cout << 0 << std::endl;
        }

        void login(const CmdType &arg) {
            size_t u_h = Hash(arg['u']);
            // 用户不存在 || 用户已经登录 || 密码不对
            if (!UserDb.Find(u_h).first || Logged.find(u_h) != Logged.end() || UserDb[u_h].Passwd != arg['p']) {
                std::cout << -1 << std::endl;
                return;
            }
            Logged[u_h] = true;
            std::cout << 0 << std::endl;
        }

        void logout(const CmdType &arg) {
            size_t u_h = Hash(arg['u']);
            if (Logged.find(u_h) == Logged.end()) {
                std::cout << -1 << std::endl;
                return;
            }
            Logged.erase(Logged.find(u_h));
            std::cout << 0 << std::endl;
        }

        bool JudgeUserQM(const CmdType &arg) {// 用户c要查询（修改）用户u的时候
            size_t u_h = Hash(arg['u']), c_h = Hash(arg['c']);
            if (Logged.find(c_h) == Logged.end())return false;// 没有登录
            if (!UserDb.Find(u_h).first)return false;// 没有用户u
            if (arg['u'] == arg['c'])return true;// 用户名相同，不需要判断权限（坑！！！）
            if (UserDb[c_h].privilege.to_int() <= UserDb[u_h].privilege.to_int())return false;// 不满足权限要求
            return true;
        }

        void query_profile(const CmdType &arg) {
            size_t u_h = Hash(arg['u']);
            if (!JudgeUserQM(arg)) {
                std::cout << -1 << std::endl;
                return;
            }
            UserInfo User = UserDb[u_h];
            std::cout << User.to_string() << std::endl;
        }

        void modify_profile(const CmdType &arg) {
            size_t u_h = Hash(arg['u']), c_h = Hash(arg['c']);
            if (!JudgeUserQM(arg)) {
                std::cout << -1 << std::endl;
                return;
            }
            if (!arg['g'].empty() && UserDb[c_h].privilege.to_int() <= std::stoi(arg['g'])) {
                std::cout << -1 << std::endl;
                return;
            }
            UserInfo User = UserDb[u_h];// 最好不用引用传递，引用似乎不能在外存上实现
            // modify to the previous one ---- 2
            UserRoll.push(arg.TimeStamp * 10 + 2, u_h, User);
            if (!arg['p'].empty())User.Passwd = arg['p'];
            if (!arg['n'].empty())User.Name = arg['n'];
            if (!arg['m'].empty())User.mailAdd = arg['m'];
            if (!arg['g'].empty())User.privilege = arg['g'];
            UserDb.Modify(u_h, User);
            std::cout << User.to_string() << std::endl;
        }

        void add_train(const CmdType &arg) {
            size_t i_h = Hash(arg['i']);
            if (BasicTrainDb.Find(i_h).first) {// 已经存在，添加失败
                std::cout << -1 << std::endl;
                return;
            }
            BasicTrainInfo BasicTrain;
            TrainInfo Train;
            BasicTrain.SeatNum = std::stoi(arg['m']);
            BasicTrain.StNum = std::stoi(arg['n']);
            BasicTrain.Type = arg['y'][0];
            BasicTrain.is_released = false;// 初始都是未发布

            ret_type tmp = split_cmd(arg['s'], '|');// StationName
            for (int i = 0; i < BasicTrain.StNum; ++i)
                Train.StName[i + 1] = tmp[i];

            tmp = split_cmd(arg['p'], '|'); // Prices n-1 前缀和 第1项为0
            Train.Prices[0] = 0;
            for (int i = 0; i < BasicTrain.StNum - 1; ++i)
                Train.Prices[i + 2] = Train.Prices[i + 1] + std::stoi(tmp[i]);

            int TravelTimes[StNumMax], StopOverTimes[StNumMax];
            tmp = split_cmd(arg['t'], '|');
            for (int i = 0; i < BasicTrain.StNum - 1; ++i)// TravelTimes n-1项，从 1 开始
                TravelTimes[i + 1] = std::stoi(tmp[i]);
            tmp = split_cmd(arg['o'], '|');
            for (int i = 0; i < BasicTrain.StNum - 2; ++i)// StopOverTimes n-2项，从1开始
                StopOverTimes[i + 1] = std::stoi(tmp[i]);

            tmp = split_cmd(arg['d'], '|');
            BasicTrain.SaleDate.first = tmp[0];
            BasicTrain.SaleDate.second = tmp[1];

            Time BasicTime(tmp[0], arg['x']);// 发售日期的第一趟车为基准的TimeTable
            for (int i = 1; i <= BasicTrain.StNum; ++i) {
                // first-Arriving second-Leaving
                if (i > 1)Train.TimeTable[i].first = (BasicTime += TravelTimes[i - 1]);
                if (i > 1 && i < BasicTrain.StNum)Train.TimeTable[i].second = (BasicTime += StopOverTimes[i - 1]);
                if (i == 1)Train.TimeTable[i].second = BasicTime;
            }

            BasicTrainDb.Insert(i_h, BasicTrain);
            BasicTrainRoll.push(arg.TimeStamp * 10 + 1, i_h, {});
            TrainDb.Insert(i_h, Train);
            TrainRoll.push(arg.TimeStamp * 10 + 1, i_h, {});

            std::cout << 0 << std::endl;
        }

        void delete_train(const CmdType &arg) {
            size_t i_h = Hash(arg['i']);
            if (!BasicTrainDb.Find(i_h).first || BasicTrainDb[i_h].is_released) {// 没有这个车次 || 已经发布了
                std::cout << -1 << std::endl;
                return;
            }
            // 所有对于BasicTrain的操作，只操作Mp。
            BasicTrainRoll.push(arg.TimeStamp * 10 + 3, i_h, BasicTrainDb[i_h]);
            BasicTrainDb.Remove(i_h);
            TrainRoll.push(arg.TimeStamp * 10 + 3, i_h, TrainDb[i_h]);
            TrainDb.Remove(i_h);
            std::cout << 0 << std::endl;
        }

        void release_train(const CmdType &arg) {// 先不把DayTrain加进去，有购票的时候再加？
            size_t i_h = Hash(arg['i']);
            if (!BasicTrainDb.Find(i_h).first) {// 没有找到这辆车
                std::cout << -1 << std::endl;
                return;
            }
            auto BasicTrain = BasicTrainDb[i_h];
            if (BasicTrain.is_released) {// 已经发布了
                std::cout << -1 << std::endl;
                return;
            }
            BasicTrainRoll.push(arg.TimeStamp * 10 + 2, i_h, BasicTrain);
            BasicTrain.is_released = true;
            BasicTrainDb.Modify(i_h, BasicTrain);

            auto Train = TrainDb[i_h];
            // 火车发布了，应该就要添加StInfo数据库了
            for (int i = 1; i <= BasicTrain.StNum; ++i) {
                StInfo St;
                St.Rank = i;
                St.Leaving = Train.TimeTable[i].second;
                St.Arriving = Train.TimeTable[i].first;
                St.Price = Train.Prices[i];
                St.TrainID = arg['i'];
                StDb.Insert({Hash(Train.StName[i].to_string()), Hash(arg['i'])}, St);
                // delete the previous one ---- 1
                StRoll.push(arg.TimeStamp * 10 + 1, {Hash(Train.StName[i].to_string()), Hash(arg['i'])}, {});
            }
            std::cout << 0 << std::endl;
        }

        void query_train(const CmdType &arg) {
            size_t i_h = Hash(arg['i']);
            if (!BasicTrainDb.Find(i_h).first) {// 没有找到这辆车
                std::cout << -1 << std::endl;
                return;
            }
            auto BasicTrain = BasicTrainDb[i_h];
            auto Train = TrainDb[i_h];

            Date Day = arg['d'];
            if (Day < BasicTrain.SaleDate.first || BasicTrain.SaleDate.second < Day) {// 这段时间不发车
                std::cout << -1 << std::endl;
                return;
            }
            // 注意格式
            bool is_in = true;
            DayTrainInfo DayTrain;
            if (!DayTrainDb.Find({i_h, arg['d']}).first)is_in = false;
            if (is_in) DayTrain = DayTrainDb[{i_h, arg['d']}];

            std::string ret;
            ret += arg['i'] + ' ' + BasicTrain.Type + '\n';
            //int IntervalDays = GetDate(Train, 1, arg['d']);
            int IntervalDays = Day - Date(Train.TimeTable[1].second);
            for (int i = 1; i <= BasicTrain.StNum; ++i) {
                ret += Train.StName[i].to_string() + ' ';
                ret += (i == 1 ? "xx-xx xx:xx" : Train.TimeTable[i].first.DayStep(IntervalDays).to_string()) + " -> ";
                ret += i == BasicTrain.StNum ? "xx-xx xx:xx" : Train.TimeTable[i].second.DayStep(
                        IntervalDays).to_string();
                ret += ' ' + std::to_string(Train.Prices[i]) + ' ';
                // 这里查询的是从这一站到下一站的票数
                if (i == BasicTrain.StNum)ret += 'x';
                else if (is_in) ret += std::to_string(DayTrain.RemainSeats[i + 1]);
                else ret += std::to_string(BasicTrain.SeatNum);
                ret += '\n';
            }
            std::cout << ret;
        }

        struct TicketType {
            fstr<TrainIDMax> TrainID;
            int TravelTime{}, Cost{}, RemainSeat{};
            Time Leaving, Arriving;

            std::string to_string(const std::string &s, const std::string &t) const {
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
        Get_Ticket(const fstr<TrainIDMax> &TrainID, const BasicTrainInfo &BasicTrain, const StInfo &from,
                   const StInfo &to, const Date &Day) {// 确定了站台的编号，需要判断有无票，在不在售票时间？
            TicketType ret;
//            int IntervalDays = GetDate(Train, pl, Day);
            int IntervalDays = Day - Date(from.Leaving);
            Date BuyDate = BasicTrain.SaleDate.first + IntervalDays;
            if (BuyDate < BasicTrain.SaleDate.first)return {-1, ret};// 售票区间前
            if (BasicTrain.SaleDate.second < BuyDate)return {-2, ret};// 售票区间后
            auto res = DayTrainDb.Find({Hash(TrainID.to_string()), BuyDate});
            if (res.first)ret.RemainSeat = res.second.second.Get_Remain(from.Rank, to.Rank);
            else ret.RemainSeat = BasicTrain.SeatNum;// 没有实例化

            ret.Leaving = from.Leaving.DayStep(IntervalDays);
            ret.Arriving = to.Arriving.DayStep(IntervalDays);
            ret.TravelTime = ret.Arriving - ret.Leaving;
            ret.Cost = to.Price - from.Price;
            ret.TrainID = TrainID;
            return {0, ret};
        }

        void query_ticket(const CmdType &arg) {
            size_t s_h = Hash(arg['s']), t_h = Hash(arg['t']);
            auto it_s = StDb.tree->FindBigger({s_h, 0});
            auto it_t = StDb.tree->FindBigger({t_h, 0});
            sjtu::vector<StInfo> lis_s, lis_t;
            sjtu::vector<size_t> hash_s, hash_t;
            for (; !it_s.AtEnd() && (*it_s).first.first == s_h; ++it_s) {
                lis_s.push_back((*it_s).second);
                hash_s.push_back((*it_s).first.second);
            }
            for (; !it_t.AtEnd() && (*it_t).first.first == t_h; ++it_t) {
                lis_t.push_back((*it_t).second);
                hash_t.push_back((*it_t).first.second);
            }

            sjtu::vector<TicketType> tickets;
            for (int i = 0, j = 0; i < lis_s.size(); ++i) {
                while (j < lis_t.size() && hash_t[j] < hash_s[i])++j;
                if (j >= lis_t.size())break;
                if (hash_s[i] != hash_t[j])continue;
                if (lis_s[i].Rank >= lis_t[j].Rank)continue;
                auto TrainID = lis_t[j].TrainID.to_string();
                auto BasicTrain = BasicTrainDb[hash_s[i]];
                auto tik = Get_Ticket(TrainID, BasicTrain, lis_s[i], lis_t[j], arg['d']);
                if (tik.first == 0)tickets.push_back(tik.second);
            }

            auto cmp = arg['p'] == "cost" ? &System::cmp_ticket_cost : &System::cmp_ticket_time;
            sort(tickets.begin(), tickets.end(), cmp);
            std::string ret;
            ret += std::to_string(tickets.size()) + '\n';
            for (auto tik: tickets)
                ret += tik.to_string(arg['s'], arg['t']) + '\n';
            std::cout << ret;
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

        void query_transfer(const CmdType &arg) {
            size_t s_h = Hash(arg['s']), t_h = Hash(arg['t']);
            auto it_s = StDb.tree->FindBigger({s_h, 0});
            auto it_t = StDb.tree->FindBigger({t_h, 0});

            sjtu::vector<StInfo> lis_s, lis_t;

            for (; !it_s.AtEnd() && (*it_s).first.first == s_h; ++it_s)
                lis_s.push_back((*it_s).second);
            for (; !it_t.AtEnd() && (*it_t).first.first == t_h; ++it_t)
                lis_t.push_back((*it_t).second);

            sjtu::linked_hashmap<size_t, TrainInfo> MyMp;
            // 得到了两个list
            TransType tik;
            bool flag = false;
            auto cmp = arg['p'] == "cost" ? &System::cmp_trans_cost : &System::cmp_trans_time;
            for (auto &S: lis_s) {// 起点车次 S
                sjtu::linked_hashmap<std::string, int> mp;
                auto TrainIDS = S.TrainID.to_string();
                TrainInfo TrainS;
                if (MyMp.find(Hash(TrainIDS)) == MyMp.end())MyMp[Hash(TrainIDS)] = TrainDb[Hash(TrainIDS)];
                TrainS = MyMp[Hash(TrainIDS)];
//                auto TrainS = TrainDb[Hash(TrainIDS)];
                auto BasicTrainS = BasicTrainDb[Hash(TrainIDS)];
                for (int i = S.Rank + 1; i <= BasicTrainS.StNum; ++i) // map 中存 -s 后面所有可以到的站 (-s) -> (-trans)
                    mp[TrainS.StName[i].to_string()] = i;
                for (auto &T: lis_t) {// 终点车次 T
                    auto TrainIDT = T.TrainID.to_string();
                    if (TrainIDS == TrainIDT)continue;// 换乘的同一辆车
                    TrainInfo TrainT;
                    if (MyMp.find(Hash(TrainIDT)) == MyMp.end())MyMp[Hash(TrainIDT)] = TrainDb[Hash(TrainIDT)];
                    TrainT = MyMp[Hash(TrainIDT)];
//                    auto TrainT = TrainDb[Hash(TrainIDT)];
                    auto BasicTrainT = BasicTrainDb[Hash(TrainIDT)];
                    for (int i = 1; i < T.Rank; ++i) {// 中转站，要求 -t 前面的车站 (-trans) -> (-t)
                        fstr<StNameMax> trans = TrainT.StName[i];
                        if (mp.find(trans.to_string()) == mp.end())continue;// 没有这个站
                        int pl = S.Rank, pr = mp[trans.to_string()], ql = i, qr = T.Rank;
                        StInfo TransS, TransT;
                        TransS.Rank = pr, TransT.Rank = ql;
                        TransS.Price = TrainS.Prices[pr], TransT.Price = TrainT.Prices[ql];
                        TransS.Arriving = TrainS.TimeTable[pr].first;
                        TransT.Leaving = TrainT.TimeTable[ql].second;
                        auto tik1 = Get_Ticket(TrainIDS, BasicTrainS, S, TransS, arg['d']);
                        if (tik1.first < 0) continue;// 第一张票不合法
                        Time Arrival = std::max(tik1.second.Arriving, TransT.Leaving);
                        for (auto d = (Date) Arrival;; d += 1) {
                            auto tik2 = Get_Ticket(TrainIDT, BasicTrainT, TransT, T, d);
                            if (tik2.first == -2)break;// 超过日期，可以退出循环
                            if (tik2.first < 0)continue;
                            if (tik2.second.Leaving < Arrival)continue;// 这里记得还要判断一下是否大于上一次的Arriving Time!!!
                            if (!flag) tik = {tik1.second, tik2.second, trans.to_string()}, flag = true;
                            else tik = std::min(tik, {tik1.second, tik2.second, trans.to_string()}, cmp);
                            break;
                        }
                    }
                }
            }
            if (!flag) {
                std::cout << 0 << std::endl;
                return;
            }
            std::cout << tik.tik1.to_string(arg['s'], tik.trans) << std::endl;
            std::cout << tik.tik2.to_string(tik.trans, arg['t']) << std::endl;
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

            Order() {}
        };

        class PIHash {
        public:
            size_t operator()(const std::pair<size_t, int> &x) {
                return x.first + x.second;
            }
        };

        //bptree<std::pair<fstr<UserNameMax>, int>, Order> OrderDb;// 第二维存这是第几个订单
        ds::CacheMap<std::pair<size_t, int>, Order, 29999, 203, 26, PIHash> OrderDb;
        hnyls2002::Stack<std::pair<size_t, int>, Order> OrderRoll;

        struct PendType {
            fstr<UserNameMax> UserName;
            int TicketNum, pl, pr, id;// 存了车站顺序和订单的编号
        };

        class PDHash {
        public:
            size_t operator()(const std::pair<std::pair<size_t, Date>, int> &x) {
                return size_t(x.first.first + x.first.second.num_d) * x.second;
            }
        };

        //bptree<std::pair<std::pair<fstr<TrainIDMax>, Date>, int>, PendType> PendDb;// 第二维存[-时间戳] 购票的时间戳
        ds::CacheMap<std::pair<std::pair<size_t, Date>, int>, PendType, 49999, 145, 60, PDHash> PendDb;
        hnyls2002::Stack<std::pair<std::pair<size_t, Date>, int>, PendType> PendRoll;

        void buy_ticket(const CmdType &arg) {
            size_t f_h = Hash(arg['f']), t_h = Hash(arg['t']), i_h = Hash(arg['i']), u_h = Hash(arg['u']);
            if (Logged.find(u_h) == Logged.end()) {// 没有登录
                std::cout << -1 << std::endl;
                return;
            }
            if (!BasicTrainDb.Find(i_h).first) {// 没有这列车
                std::cout << -1 << std::endl;
                return;
            }
            auto BasicTrain = BasicTrainDb[i_h];
            if (!BasicTrain.is_released) {// 没有被release
                std::cout << -1 << std::endl;
                return;
            }
            if (std::stoi(arg['n']) > BasicTrain.SeatNum) {// 当你特牛逼想买很多票的时候应该直接返回-1
                std::cout << -1 << std::endl;
                return;
            }
            // 可能没有这个站
            auto St_res_f = StDb.Find({f_h, i_h});
            auto St_res_t = StDb.Find({t_h, i_h});
            if (!St_res_f.first || !St_res_t.first) {
                std::cout << -1 << std::endl;
                return;
            }

            auto St1 = St_res_f.second.second, St2 = St_res_t.second.second;
            if (St1.Rank >= St2.Rank) {
                std::cout << -1 << std::endl;
                return;
            }
            int IntervalDays = arg['d'] - Date(St1.Leaving);
            Date Day = BasicTrain.SaleDate.first + IntervalDays;
            if (Day < BasicTrain.SaleDate.first || BasicTrain.SaleDate.second < Day) {// 不在区间内
                std::cout << -1 << std::endl;
                return;
            }
//            if (!DayTrainDb.Find({Train.TrainID, Day}).first) {// 没有实例化，现在实例化
            DayTrainInfo tmp;
            for (int i = 1; i <= BasicTrain.StNum; ++i)
                tmp.RemainSeats[i] = BasicTrain.SeatNum;
            //DayTrainDb[{Train.TrainID, Day}] = tmp;
            auto res = DayTrainDb.Insert({i_h, Day}, tmp);
            if (res) DayTrainRoll.push(arg.TimeStamp * 10 + 1, {i_h, Day}, {});
//            }
            auto DayTrain = DayTrainDb[{i_h, Day}];
            auto User = UserDb[u_h];
            int RemainSeat = DayTrain.Get_Remain(St1.Rank, St2.Rank);
            int TicketNum = std::stoi(arg['n']);
            if (RemainSeat < TicketNum && (arg['q'].empty() || arg['q'] == "false")) {
                std::cout << -1 << std::endl;
                return;
            }
            Order order;
            if (RemainSeat >= TicketNum) {// 可以买票
                DayTrainRoll.push(arg.TimeStamp * 10 + 2, {i_h, Day}, DayTrain);
                DayTrain.Modify(St1.Rank, St2.Rank, TicketNum);
                //DayTrainDb[{Train.TrainID, Day}] = DayTrain;
                DayTrainDb.Modify({i_h, Day}, DayTrain);
                order.Status = success;
            } else {
                PendType ele{arg['u'], TicketNum, St1.Rank, St2.Rank, User.OrderNum + 1};
                PendDb.Insert({{i_h, Day}, arg.TimeStamp}, ele);
                PendRoll.push(arg.TimeStamp * 10 + 1, {{i_h, Day}, arg.TimeStamp}, {});
                order.Status = pending;
            }
            TicketType tik;
            tik.Leaving = St1.Leaving.DayStep(IntervalDays);
            tik.Arriving = St2.Arriving.DayStep(IntervalDays);
            tik.RemainSeat = TicketNum;
            // 这里Cost记录单价!!!
            tik.Cost = St2.Price - St1.Price;
            tik.TrainID = arg['i'];
            order.tik = tik, order.From = arg['f'], order.To = arg['t'], order.Day = Day;
            order.pl = St1.Rank, order.pr = St2.Rank, order.TimeStamp = arg.TimeStamp;
            UserRoll.push(arg.TimeStamp * 10 + 2, u_h, User);
            ++User.OrderNum;
            OrderDb.Insert({Hash(User.UserName.to_string()), -User.OrderNum}, order);
            OrderRoll.push(arg.TimeStamp * 10 + 1, {Hash(User.UserName.to_string()), -User.OrderNum}, {});
            UserDb.Modify(u_h, User);
            if (order.Status == success)std::cout << (long long) tik.Cost * TicketNum << std::endl;
            else std::cout << "queue" << std::endl;
        }

        void query_order(const CmdType &arg) {
            size_t u_h = Hash(arg['u']);
            if (Logged.find(u_h) == Logged.end()) {// 没有登录
                std::cout << -1 << std::endl;
                return;
            }
            OrderDb.Flush();
            auto it = OrderDb.tree->FindBigger({u_h, -0x3f3f3f3f});
            std::string ret;
            ret += std::to_string(UserDb[u_h].OrderNum) + '\n';
            for (; !it.AtEnd() && (*it).first.first == u_h; ++it) {
                ret += StatusToString[(*it).second.Status] + ' ';
                ret += (*it).second.tik.to_string((*it).second.From.to_string(), (*it).second.To.to_string()) + '\n';
            }
            std::cout << ret;
        }

        void refund_ticket(const CmdType &arg) {
            size_t u_h = Hash(arg['u']);
            if (Logged.find(u_h) == Logged.end()) {// 没有登录
                std::cout << -1 << std::endl;
                return;
            }
            int tot_order = UserDb[u_h].OrderNum;
            int id = arg['n'].empty() ? tot_order : tot_order - std::stoi(arg['n']) + 1;
            if (id <= 0) {// 没有这么多订单
                std::cout << -1 << std::endl;
                return;
            }
            auto order = OrderDb[{u_h, -id}];// order 要退的订单 order2 要候补的订单
            if (order.Status == refunded) {
                std::cout << -1 << std::endl;
                return;
            }
            bool flag = true;
            if (order.Status == pending)flag = false;
            OrderRoll.push(arg.TimeStamp * 10 + 2, {u_h, -id}, order);
            order.Status = refunded;
            OrderDb.Modify({u_h, -id}, order);

            std::pair<size_t, Date> info = {Hash(order.tik.TrainID.to_string()), order.Day};
            if (!flag) {// 为假说明要把这个退订的候补在PendDb中删除
                PendRoll.push(arg.TimeStamp * 10 + 3, {info, order.TimeStamp}, PendDb[{info, order.TimeStamp}]);
                PendDb.Remove({info, order.TimeStamp});
            } else {// 为真代表会有多的票空出来
                auto DayTrain = DayTrainDb[info];
                DayTrainRoll.push(arg.TimeStamp * 10 + 2, info, DayTrain);
                DayTrain.Modify(order.pl, order.pr, -order.tik.RemainSeat);

                // 处理候补的订单
                auto it = PendDb.tree->FindBigger({info, 0});
                for (; !it.AtEnd() && (*it).first.first == info;) {
                    int RemainTickets = DayTrain.Get_Remain((*it).second.pl, (*it).second.pr);
                    if (RemainTickets >= (*it).second.TicketNum) {// 候补成功
                        auto order2 = OrderDb[{Hash((*it).second.UserName.to_string()), -(*it).second.id}];
                        OrderRoll.push(arg.TimeStamp * 10 + 2,
                                       {Hash((*it).second.UserName.to_string()), -(*it).second.id}, order2);
                        order2.Status = success;
                        //OrderDb[{(*it).second.UserName, -(*it).second.id}] = order2;
                        OrderDb.Modify({Hash((*it).second.UserName.to_string()), -(*it).second.id}, order2);
                        DayTrain.Modify((*it).second.pl, (*it).second.pr, (*it).second.TicketNum);
                        PendRoll.push(arg.TimeStamp * 10 + 3, (*it).first, (*it).second);
                        PendDb.Remove((*it).first);// 可能一次删除很多个
                        ++it;
                    } else ++it;
                }
                //DayTrainDb[info] = DayTrain;
                DayTrainDb.Modify(info, DayTrain);
            }
            std::cout << 0 << std::endl;
        }

        bool Roll(int DesTime) {
            int Max = 0, id = -1;
            if (!UserRoll.empty() && UserRoll.top() > Max)Max = UserRoll.top(), id = 1;
            if (!BasicTrainRoll.empty() && BasicTrainRoll.top() > Max)Max = BasicTrainRoll.top(), id = 2;
            if (!TrainRoll.empty() && TrainRoll.top() > Max) Max = TrainRoll.top(), id = 3;
            if (!StRoll.empty() && StRoll.top() > Max) Max = StRoll.top(), id = 4;
            if (!DayTrainRoll.empty() && DayTrainRoll.top() > Max) Max = DayTrainRoll.top(), id = 5;
            if (!OrderRoll.empty() && OrderRoll.top() > Max) Max = OrderRoll.top(), id = 6;
            if (!PendRoll.empty() && PendRoll.top() > Max) Max = PendRoll.top(), id = 7;
            if (Max < DesTime * 10)return false;
            if (id == 1) {
                auto res = UserRoll.pop();
                if (Max % 10 == 1) UserDb.Remove(res.key);
                else if (Max % 10 == 2) UserDb.Modify(res.key, res.info);
                else UserDb.Insert(res.key, res.info);
            } else if (id == 2) {
                auto res = BasicTrainRoll.pop();
                if (Max % 10 == 1)BasicTrainDb.Remove(res.key);
                else if (Max % 10 == 2) BasicTrainDb.Modify(res.key, res.info);
                else BasicTrainDb.Insert(res.key, res.info);
            } else if (id == 3) {
                auto res = TrainRoll.pop();
                if (Max % 10 == 1)TrainDb.Remove(res.key);
                else if (Max % 10 == 2) TrainDb.Modify(res.key, res.info);
                else TrainDb.Insert(res.key, res.info);
            } else if (id == 4) {
                auto res = StRoll.pop();
                if (Max % 10 == 1)StDb.Remove(res.key);
                else if (Max % 10 == 2) StDb.Modify(res.key, res.info);
                else StDb.Insert(res.key, res.info);
            } else if (id == 5) {
                auto res = DayTrainRoll.pop();
                if (Max % 10 == 1)DayTrainDb.Remove(res.key);
                else if (Max % 10 == 2) DayTrainDb.Modify(res.key, res.info);
                else DayTrainDb.Insert(res.key, res.info);
            } else if (id == 6) {
                auto res = OrderRoll.pop();
                if (Max % 10 == 1)OrderDb.Remove(res.key);
                else if (Max % 10 == 2) OrderDb.Modify(res.key, res.info);
                else OrderDb.Insert(res.key, res.info);
            } else {
                auto res = PendRoll.pop();
                if (Max % 10 == 1)PendDb.Remove(res.key);
                else if (Max % 10 == 2) PendDb.Modify(res.key, res.info);
                else PendDb.Insert(res.key, res.info);
            }
            return true;
        }

        void rollback(const CmdType &arg) {
            Logged.clear();
            if (std::stoi(arg['t']) >= arg.TimeStamp) {
                std::cout << -1 << std::endl;
                return;
            }
            while (Roll(std::stoi(arg['t'])));
            std::cout << 0 << std::endl;
        }

        void clean(const CmdType &arg) {
            std::cout << 0 << std::endl;
        }

        void exit(const CmdType &arg) {
            Logged.clear();
            std::cout << "bye" << std::endl;
        }

#undef ret_value

    };
}

#endif

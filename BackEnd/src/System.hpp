#ifndef TrainTicketSystem_System_HPP
#define TrainTicketSystem_System_HPP

#include "../lib/map.hpp"
#include "../lib/vector.hpp"
#include "../lib/linked_hashmap.hpp"
#include "Commander.hpp"

#include <map>

#define bptree map

namespace hnyls2002 {

    const int UserNameMax=21,PasswdMax=31,NameMax=16,mailAddMax=31;
    const int TrainIDMax=21,StionNameMax=31;

    struct UserInfo {
    };

    class System {
    private:
    public:
        sjtu::vector<std::string> Opt(const std::string &str) {
            CmdType a = Parser(str);
            return sjtu::vector<std::string>();
        }
    };
}

#endif
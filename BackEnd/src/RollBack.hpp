//
// Created by m1375 on 2022/6/8.
//

#ifndef MAIN_CPP_ROLLBACK_HPP
#define MAIN_CPP_ROLLBACK_HPP

#include <fstream>
#include <utility>

namespace hnyls2002 {
    template<typename Key, typename Info, const int max_block_size = 5>
    class Stack {
        struct Block {
            int Time[max_block_size];
            Key key[max_block_size];
            Info info[max_block_size];
        } top_block;
        int stack_size{}, SIZE{};
        std::string file_name{};
        std::fstream file;

    public:
        Stack() = default;

        // when using ios::in model, if the file doesn't exist before , it wouldn't be created.
        // so the constructor firstly use ios::in to test the existence of the file.
        explicit Stack(std::string _file_name) : file_name(std::move(_file_name)) {
            SIZE = sizeof(Block);
            file.open(file_name);
            if (!file.is_open()) {
                file.open(file_name, std::ios::out);
                stack_size = 0;
                file.write(reinterpret_cast<char *>(&stack_size), 4);
            } else {
                file.read(reinterpret_cast<char *>(&stack_size), 4);
                int block_num = (stack_size - 1) / max_block_size + 1;
                file.seekg(4 + (block_num - 1) * SIZE, std::ios::beg);
                file.read(reinterpret_cast<char * >(&top_block), SIZE);
            }
            file.close();
        }

        void push(int Time, const Key &key, const Info &info) {
            file.open(file_name);
            if (stack_size % max_block_size == 0) {
                int block_num = (stack_size - 1) / max_block_size + 1;
                file.seekp(4 + (block_num - 1) * SIZE, std::ios::beg);
                file.write(reinterpret_cast<char * >(&top_block), SIZE);
                top_block.Time[0] = Time, top_block.key[0] = key, top_block.info[0] = info;
            } else {
                int index = stack_size % max_block_size;
                top_block.Time[index] = Time, top_block.key[index] = key, top_block.info[index] = info;
            }
            ++stack_size;
            file.close();
        }

        size_t size() const { return stack_size; }

        bool empty() const { return stack_size == 0; }

        struct Roll {
            Key key{};
            Info info{};
        };

        class STACK_EMPTY {
        };

        int top() const {
            if (!stack_size)exit(0);
            int index = (stack_size - 1) % max_block_size;
            return top_block.Time[index];
        }

        Roll pop() {
            int index = (stack_size - 1) % max_block_size;
            Roll ret{top_block.key[index], top_block.info[index]};
            --stack_size;
            if (index == 0 && stack_size) {
                int block_num = (stack_size - 1) / max_block_size + 1;
                file.open(file_name);
                file.seekg(4 + (block_num - 1) * SIZE, std::ios::beg);
                file.read(reinterpret_cast<char * >(&top_block), SIZE);
                file.close();
            }
            return ret;
        }

        ~Stack() {
            file.open(file_name);
            file.write(reinterpret_cast<char *>(&stack_size), 4);
            int block_num = (stack_size - 1) / max_block_size + 1;
            file.seekp(4 + (block_num - 1) * SIZE, std::ios::beg);
            file.write(reinterpret_cast<char * >(&top_block), SIZE);
            file.close();
        }
    };
}

#endif //MAIN_CPP_ROLLBACK_HPP
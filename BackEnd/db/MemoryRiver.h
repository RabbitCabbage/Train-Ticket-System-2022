//
// Created by lu'ren'jia'd's on 2022/5/2.
//

#ifndef UNTITLED2_MEMORYRIVER_H
#define UNTITLED2_MEMORYRIVER_H

#include <fstream>
#include <cstring>
#include "FileException.h"

namespace ds {
    template<typename T>
    class MemoryRiver {
    private:
        char *file_name;
        char *trash_name;
        int count{};
        struct Block {// first Block first element --- the number of the stack
            int position[1024]{};
        } top;
    public:
        MemoryRiver(char *fn) {
            std::fstream file;
            file_name = new char[strlen(fn) + 1];
            strcpy(file_name, fn);
            file.clear();
            file.open(file_name, std::ios::in);
            if (!file) {
                file.clear();
                file.open(file_name, std::ios::out | std::ios::app);
            }
            if (!file.is_open()) {
                ds::OpenException e;
                throw e;
            }
            file.close();

            int len = strlen(fn);
            trash_name = new char[len + 2];
            strcpy(trash_name, fn);
            trash_name[len] = '_';
            trash_name[len + 1] = '\0';

            file.clear();
            file.open(trash_name, std::ios::in);
            if (!file) {
                file.clear();
                file.open(trash_name, std::ios::out | std::ios::app);
                file.write(reinterpret_cast<char *>(&top), 4096);
            } else {
                file.read(reinterpret_cast<char *>(&top), 4096);
                count = top.position[0];
                int block_num = count / 1024 + 1;
                if (block_num > 1) {
                    file.seekg(4096 * (block_num - 1), std::ios::beg);
                    file.read(reinterpret_cast<char *>(&top), 4096);
                }
            }
            if (!file.is_open()) {
                ds::OpenException e;
                throw e;
            }
            file.close();
        }

        ~MemoryRiver() {
            std::fstream file;
            file.open(trash_name);
            int num_block = count / 1024 + 1;
            if (num_block != 1) {
                file.seekp(4096 * (num_block - 1), std::ios::beg);
                file.write(reinterpret_cast<char *>(&top), 4096);
                file.seekg(0, std::ios::beg);
                file.read(reinterpret_cast<char *>(&top), 4096);
            }
            top.position[0] = count;
            file.seekp(0, std::ios::beg);
            file.write(reinterpret_cast<char *>(&top), 4096);
            file.close();
            delete[]file_name;
            delete[]trash_name;
        }

        bool Write(const int &index, const T &t) {
            std::fstream file;
            file.clear();
//            if(file.fail())printf("%s","clear error\n");
            file.open(file_name);
//            if(file.fail())printf("%s","open error\n");
            if (!file.is_open()) {
                ds::OpenException e;
                throw e;
            }
            file.seekp(index, std::ios::beg);
//            if(file.fail())printf("%s","seek error\n");
            file.write(reinterpret_cast<const char *>(&t), sizeof(T));
            if (file.fail()) {
//                printf("%s", "mr write failed\n");
                return false;
            }
            file.close();
            return true;
        }

        bool Read(const int &index, T &res) {
            std::fstream file;
            file.clear();
            file.open(file_name);
            if (!file.is_open()) {
                ds::OpenException e;
                throw e;
            }
            file.seekg(index);
            file.read(reinterpret_cast<char *>(&res), sizeof(T));
            if (file.fail()) {
//                printf("%s", "mr read failed");
                return false;
            }
            file.close();
            return true;
        }

        void Delete(const int &index) {

//            std::cerr << "Index : " << index << std::endl;
//            std::cerr << "Count : " << count << std::endl;

            if ((count + 1) % 1024 == 0) {
                std::fstream file;
                file.open(trash_name);
                int block_num = count / 1024 + 1;
                file.seekp(4096 * (block_num - 1), std::ios::beg);
                file.write(reinterpret_cast<char *>(&top), 4096);
                top = Block();
                top.position[0] = index;
                file.write(reinterpret_cast<char *>(&top), 4096);
                file.close();
            } else top.position[(count + 1) % 1024] = index;
            ++count;
        }

        int FindAvailable() {
            std::fstream file;
            file.clear();
            file.open(file_name);
            if (!file.is_open()) {
                ds::OpenException e;
                throw e;
            }
            if (!count) {
                file.seekp(0, std::ios::end);
                int location = file.tellp();
                file.close();
                return location;
            } else {
                file.close();
                int ret = top.position[count % 1024];
/*
                std::cerr << "position : " << ret << std::endl;
                std::cerr << "count" << count << std::endl;
                std::cerr << "countcc " << top.position[0] << std::endl;
*/
                if (count % 1024 == 0) {
                    file.open(trash_name);
                    int block_num = count / 1024 + 1;
                    file.seekg((block_num - 2) * 1024, std::ios::beg);
                    file.read(reinterpret_cast<char *>(&top), 4096);
                }
                --count;
                return ret;
            }
        }
    };
}
#endif //UNTITLED2_MEMORYRIVER_H

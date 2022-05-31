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
    public:
        MemoryRiver(char *fn) {
            FILE *f = nullptr;
            file_name = new char[strlen(fn) + 1];
            strcpy(file_name, fn);
            f = fopen(file_name, "ab");
            if (f == nullptr) {
                ds::OpenException e;
                throw e;
            } else fclose(f);
        }

        ~MemoryRiver() {
            delete[]file_name;
        }

        bool Write(const int &index, const T &t) {
            FILE *f = nullptr;
//            if(file.fail())printf("%s","clear error\n");
            f = fopen(file_name, "rb+");
//            if(file.fail())printf("%s","open error\n");
            if (f == nullptr) {
                ds::OpenException e;
                throw e;
            }
            fseek(f, index, 0);
//            if(file.fail())printf("%s","seek error\n");
            fwrite(&t, sizeof(T), 1, f);
            if (ferror(f)) {
//                printf("%s", "mr write failed\n");
                return false;
            }
            fclose(f);
            return true;
        }

        bool Read(const int &index, T &res) {
            FILE *f = nullptr;
            f = fopen(file_name, "rb");
            if (f == nullptr) {
                ds::OpenException e;
                throw e;
            }
            fseek(f, index, 0);
            fread(&res, sizeof(T), 1, f);
            if (ferror(f)) {
//                printf("%s", "mr read failed");
                return false;
            }
            fclose(f);
            return true;
        }

        int Append(const T &t) {
            FILE *f = nullptr;
            //write a T at the end of the file and return the location writing in
            f = fopen(file_name, "ab");
            if (f == nullptr) {
                ds::OpenException e;
                throw e;
            }
            fseek(f, 0, 2);
            int location = ftell(f);
            fwrite(&t, sizeof(T), 1, f);
            if (ferror(f)) {
                ds::AppendException e;
                throw e;
            }
            fclose(f);
            return location;
        }

        int FindEnd() {
            FILE *f = nullptr;
            f = fopen(file_name, "rb");
            if (f == nullptr) {
                ds::OpenException e;
                throw e;
            }
            fseek(f, 0, 2);
            int location = ftell(f);
            fclose(f);
            return location;
        }
    };
}
#endif //UNTITLED2_MEMORYRIVER_H

//
// Created by lu'ren'jia'd's on 2022/5/2.
//

#ifndef UNTITLED2_CACHEMAP_H
#define UNTITLED2_CACHEMAP_H

#include "BPlusTree.h"
#include <cstring>

//要求max_size最好是个整数
namespace ds {
    template<typename Key, typename Info, const int max_size = 17, const int max_key_num = 5, const int max_rcd_num = 5, class Hash=std::hash<Key>, class KeyCompare = std::less<Key>>

    class CacheMap {
    private:
        bool valid[max_size];
        bool dirty[max_size];
        Key index[max_size];//记下这时的这个hash所对应的是什么键值
        Info information[max_size];
        KeyCompare cmp;
        Hash hash_func;
    public:
        ds::BPlusTree<Key, Info, max_key_num, max_rcd_num, KeyCompare> *tree;

        CacheMap(const char *index_file_name,
                 const char *record_file_name) {
            tree = new ds::BPlusTree<Key, Info, max_key_num, max_rcd_num, KeyCompare>(index_file_name,
                                                                                      record_file_name);
            memset(valid, false, sizeof(valid));
            memset(dirty, false, sizeof(dirty));
        }

        ~CacheMap() {
            for (int i = 0; i < max_size; ++i) {
                if (dirty[i])tree->Modify(index[i], information[i]);
            }
            delete tree;
        }

        //插入一个元素，参数是插入元素的键值和记录的详细信息，返回插入是否成功
        //如果说这个元素本来存在，插入失败返回false
        //直接操作到B+Tree，并且存入Cache
        bool Insert(const Key &key, const Info &info) {
            if (tree->Insert(key, info)) {
                int hash_index = (hash_func.operator()(key) % max_size);
                //然后把这个值放到缓存里面,这个值肯定是先前不存在的，不用检查碰撞了
                if (dirty[hash_index])tree->Modify(index[hash_index], information[hash_index]);
                valid[hash_index] = true;
                dirty[hash_index] = false;
                index[hash_index] = key;
                information[hash_index] = info;
                return true;
            } else return false;
        }

        //删除一个元素，参数是要删除元素的键值，返回是否删除成功，
        //如果这个元素在B+树中不存在就删除失败
        //直接操作到B+tree上
        bool Remove(const Key &key) {
            if (tree->Remove(key)) {
                int hash_index = (hash_func.operator()(key) % max_size);
                if (valid[hash_index] && !cmp.operator()(key, index[hash_index]) &&
                    !cmp.operator()(index[hash_index], key))
                    valid[hash_index] = dirty[hash_index] = false;
                return true;
            } else return false;
        }

//查询一个元素，参数是要查询元素的键值
//返回值是一个pair，bool代表有没有找到
//如果找到这个元素存在，返回true，同时返回记录信息的具体值
//如果没有找到，返回false，这时的返回struct是随机值，不应该被访问
//访问之后就要存入Cache，便于下次访问
        std::pair<bool, std::pair<Key, Info>> Find(const Key &key) {
            int hash_index = (hash_func.operator()(key) % max_size);
            if (valid[hash_index]) {
                if (!cmp.operator()(key, index[hash_index]) && !cmp.operator()(index[hash_index], key))
                    return {true, {index[hash_index], information[hash_index]}};
                else {
                    auto res = tree->Find(key);
                    if (!res.first)return res;
                    if (dirty[hash_index]) {
                        tree->Modify(index[hash_index], information[hash_index]);
                        dirty[hash_index] = false;
                        index[hash_index] = res.second.first;
                        information[hash_index] = res.second.second;
                        return res;
                    } else {
                        index[hash_index] = res.second.first;
                        information[hash_index] = res.second.second;
                        return res;
                    }
                }
            } else {
                auto res = tree->Find(key);
                if (!res.first)return res;
                dirty[hash_index] = false;
                valid[hash_index] = true;
                index[hash_index] = res.second.first;
                information[hash_index] = res.second.second;
                return res;
            }
        }

//修改一个元素，参数是要修改元素的键值和修改之后的信息
//返回一个bool，修改成功返回true，否则返回false
//如果这个要修改的元素在B+树中不存在就会返回false
//修改的时候要进行valid,index,dirty的检查
        bool Modify(const Key &key, const Info &new_info) {
            int hash_index = (hash_func.operator()(key) % max_size);
            if (valid[hash_index]) {
                if (!cmp.operator()(key, index[hash_index]) && !cmp.operator()(index[hash_index], key)) {
                    dirty[hash_index] = true;
                    information[hash_index] = new_info;
                } else {
                    if (dirty[hash_index])tree->Modify(index[hash_index], information[hash_index]);
                    index[hash_index] = key;
                    information[hash_index] = new_info;
                }
                return true;
            } else {
                if (!tree->Modify(key, new_info))return false;
                valid[hash_index] = true;
                dirty[hash_index] = false;
                index[hash_index] = key;
                information[hash_index] = new_info;
                return true;
            }
        }

        void Clear() {
            tree->Clear();
        }

//返回现在总共有多少条记录
        int GetSize() {
            return tree->GetSize();
        }
    };
}
#endif //UNTITLED2_CACHEMAP_H

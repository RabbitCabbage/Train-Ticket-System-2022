//
// Created by lu'ren'jia'd's on 2022/5/2.
//

#ifndef UNTITLED2_BPLUSTREE_H
#define UNTITLED2_BPLUSTREE_H

#include "MemoryRiver.h"
#include <functional>
#include <cstring>

namespace ds {
    template<typename Key, typename Info, class KeyCompare = std::less<Key>>

    class BPlusTree {
        //以下两个值暂定，可能以后根据实际情况更改
    public:
        static const int max_key_num = 5;//一个数据块最多记有多少个键值
        static const int max_rcd_num = 5;//一个数据块最多存多少条记录


        struct Node {
            int nxt = -1, before = -1;// B+树叶子节点构成的的链表
            bool isleaf = true;//标记是不是叶节点
            int children_num = 0;
            int children[max_key_num];
            Key keys[max_key_num];
            int location = -1;

            void operator()(Node lhs, const Node &rhs) {
                lhs.location = rhs.location;
                lhs.isleaf = rhs.isleaf;
                lhs.children_num = rhs.children_num;
                lhs.nxt = rhs.nxt;
                lhs.before = rhs.before;
                for (int i = 0; i < rhs.children_num; ++i)lhs.children[i] = rhs.children[i];
                for (int i = 0; i < rhs.children_num; ++i)lhs.keys[i] = rhs.keys[i];
            }
        };


        struct Block {
            int size = 0;
            Key keys[max_rcd_num];
            Info record[max_rcd_num];
        };

    private:
        KeyCompare cmp;
        MemoryRiver<Node> *index_memory;
        MemoryRiver<Block> *record_memory;
        MemoryRiver<int> *int_memory;
        char *index_file;
        char *record_file;
        Node root;
        int root_index;
        int scale;

        //this is a tool function to do binary search
        //find the first key that is smaller than or equal to the wanted
        //if every key is bigger than this one return -1;
        void PeekNode(const Node &node) {
            std::cout << "node location ";
            std::cout << node.location << std::endl;
            for (int i = 0; i < node.children_num; ++i) std::cout << node.keys << ' ';
            std::cout << std::endl;
        }


        int BinarySearchLess(Key array[], int size, const Key &key) {
            if (size <= 0)return -1;
            if (cmp.operator()(key, array[0]))return -1;
            //find the answer from 0 to size-1
            int l = 0, r = size - 1;
            int mid;
            while (l < r) {
                mid = (l + r + 1) >> 1;
                if (cmp.operator()(array[mid], key)) {
                    //this index may become the answer
                    //but the boundary can be more left
                    l = mid;
                } else if (cmp.operator()(key, array[mid])) r = mid - 1;
                else return mid;
            }
            return l;
        }

        //find the index of the key in the block
        // if don't find return -1;
        int BinarySearch(const Block &block, const Key &key) {
            int l = 0, r = block.size - 1, mid;
            while (l < r) {
                mid = (l + r) >> 1;
                if (cmp.operator()(block.keys[mid], key))l = mid + 1;
                else if (cmp.operator()(key, block.keys[mid]))r = mid - 1;
                else return mid;
            }
            if (l == r && (!cmp.operator()(block.keys[l], key) && !cmp.operator()(key, block.keys[l])))return l;
            return -1;
        }

        //return the index in the record file and the info struct
        // the first int is the index of the block and the second int is the index of the record in the block;
        //the index can be used to modify the struct infomation
        std::pair<std::pair<int, int>, Info> RecursionFind(const Key &key) {
            Node cur = root;
            while (!cur.isleaf) {
                int num = BinarySearchLess(cur.keys, cur.children_num, key);
                if (num == -1)num = 0;
                //num is the first element that is less than or equal to the key
                //so we need go to the children the key stand for (num)
//                !index_memory->Read(cur.children[index + 1], cur);
                if (!index_memory->Read(cur.children[num], cur))printf("%s", "read failed 6\n");
            }
            //when the cur is leaf node come out of the circle
            //then do binary search on that leaf node to fina the key
            int num = BinarySearchLess(cur.keys, cur.children_num, key);
            if (num == -1)num = 0;
            Block block;
            if (!record_memory->Read(cur.children[num], block))printf("%s", "read failed 4\n");//todo
            int key_index = BinarySearch(block, key);
            Info tmp;
            if (key_index == -1)return {{-1, -1}, tmp};
            else return {{cur.children[num], key_index}, block.record[key_index]};
        }

        //split the cur into new node and the info about new node are recorded by reference
        void Split(Node &cur, Node &new_node, int &new_node_loc, int &splited_child_index, int num) {
            for (int i = max_key_num / 2, j = 0; i < cur.children_num; ++i, ++j) {
                new_node.keys[j] = cur.keys[i];
                new_node.children[j] = cur.children[i];
            }
            new_node.children_num = max_key_num - max_key_num / 2;
            cur.children_num = max_key_num / 2;
            new_node_loc = new_node.location = index_memory->FindEnd();
            new_node.before = cur.location;
            new_node.nxt = cur.nxt;
            new_node.isleaf = cur.isleaf;//after split they are in the same level;
            if (cur.nxt != -1) {
                Node next_node;
                if (!index_memory->Read(cur.nxt, next_node))printf("%s", "read failed 16\n");
                next_node.before = new_node_loc;
                if (!index_memory->Write(cur.nxt, next_node))printf("%s", "write failed 16\n");
            }
            cur.nxt = new_node.location;
            index_memory->Append(new_node);
            splited_child_index = num;//the info about new node are recorded by reference
            if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 7\n");
            if (cur.location == root.location)root = cur;
        }

        void AddChild(Node &cur, const Key &first_key, const Key &added_first_key, int added_loc,
                      int num) {//num is the index for the added node
            for (int i = cur.children_num - 1; i >= num + 1; --i) {
                cur.children[i + 1] = cur.children[i];
                cur.keys[i + 1] = cur.keys[i];
            }
            cur.children[num + 1] = added_loc;
            cur.keys[num + 1] = added_first_key;
            cur.keys[num] = first_key;
            cur.children_num++;
            if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 5\n");
            if (cur.location == root.location)root = cur;
        }

        //to find the key in the current node;
        //the bool value it returns presents whether it needs to have changes as a father
        //the reference of the new_node is the node that should be added;
        bool
        RecursionInsert(Node &cur, const Key &key, const Info &info, bool &success, int &splited_child_index,
                        Node &new_node,
                        int &new_node_loc) {
            int num;
            if (cur.children_num <= 1)num = 0;
            else num = BinarySearchLess(cur.keys, cur.children_num, key);
            if (num == -1)num = 0;
            //num is the first element that is less than or equal to the key
            //so we need go to the children the key stand for (num+1)
            if (cur.isleaf) {
                if (cur.location == root.location && cur.children_num == 0) {
                    Block block;
                    block.record[0] = info;
                    block.keys[0] = key;
                    block.size = 1;
                    cur.children[0] = record_memory->Append(block);
                    cur.keys[0] = block.keys[0];
                    cur.children_num++;
                    cur.location = index_memory->FindEnd();
                    root_index = index_memory->Append(cur);
                    root = cur;
                    success = true;
                    return false;
                }
                Block block;
                if (!record_memory->Read(cur.children[num], block))printf("%s", "read failed 7\n");
                int key_index = BinarySearchLess(block.keys, block.size, key);
                if (key_index != -1 &&
                    (!cmp.operator()(block.keys[key_index], key) && !cmp.operator()(key, block.keys[key_index]))) {
                    success = false;//the element is already in the block
                    return false;
                    //the recursion ended;
                }
                if (block.size + 1 < max_rcd_num) {
                    //directly insert
                    //the new element should be inserted into the key_index+1;
                    for (int i = block.size - 1; i >= key_index + 1; --i) {
                        block.keys[i + 1] = block.keys[i];
                        block.record[i + 1] = block.record[i];
                    }
                    block.keys[key_index + 1] = key;
                    block.record[key_index + 1] = info;
//                    record_memory->Write(cur.children[num], block);
                    block.size++;
                    if (!record_memory->Write(cur.children[num], block))printf("%s", "write failed 4\n");
                    success = true;//the insert is succeeded and it doesn't need to change the father;
                    if (key_index + 1 == 0)cur.keys[num] = block.keys[0];
                    if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 6\n");
                    if (cur.location == root.location)root = cur;
                    return false;//the recursion ended;
                } else {
                    for (int i = block.size - 1; i >= key_index + 1; --i) {
                        block.keys[i + 1] = block.keys[i];
                        block.record[i + 1] = block.record[i];
                    }
                    block.keys[key_index + 1] = key;
                    block.record[key_index + 1] = info;
//                    record_memory->Write(cur.children[num], block);
                    block.size++;
                    Block new_block;
                    //every block should have more than size/2 elements,
                    //the first block is 0~size/2-1, the second block is size/2~size-1;
                    for (int i = block.size / 2, j = 0; i < block.size; ++i, ++j) {
                        new_block.keys[j] = block.keys[i];
                        new_block.record[j] = block.record[i];
                    }
                    block.size = max_rcd_num / 2;
                    new_block.size = max_rcd_num - block.size;
                    int new_block_loc = record_memory->Append(new_block);
                    if (!record_memory->Write(cur.children[num], block))printf("%s", "write failed 29\n");
                    //now the current node has to deal with an added child
                    if (cur.children_num + 1 < max_key_num) {
                        //the children can be directly added
                        //the former child is num;
                        // so the new children should be placed in num+1;
                        AddChild(cur, block.keys[0], new_block.keys[0], new_block_loc, num);
                        return false;
                    } else {
                        //cur need to split and tell his father that he splited
                        //use the new_node and new_node_loc
                        AddChild(cur, block.keys[0], new_block.keys[0], new_block_loc, num);
                        Split(cur, new_node, new_node_loc, splited_child_index, num);
                        if (cur.location == root.location) {
                            root.children_num = 2;
                            root.children[0] = cur.location;
                            root.children[1] = new_node.location;
                            root.keys[0] = cur.keys[0];
                            root.keys[1] = new_node.keys[0];
                            root.isleaf = false;
                            root.nxt = root.before = -1;
                            root.location = index_memory->FindEnd();
                            root_index = index_memory->Append(root);
                            return false;
                        }
                        return true;
                    }
                }
            } else {
                //we need to find the real leaf node to insert;
                Node parent = cur;
                if (!index_memory->Read(cur.children[num], cur))printf("%s", "read failed 3\n");
                //now cur is the next node to insert
                Node the_added_children;
                int former_children_index;
                int the_added_loc;
                bool need_change = RecursionInsert(cur, key, info, success, former_children_index,
                                                   the_added_children,
                                                   the_added_loc);
                if (!need_change) {
                    if (cmp.operator()(cur.keys[0], parent.keys[num])) {
                        parent.keys[num] = cur.keys[0];
                        if (!index_memory->Write(parent.location, parent))printf("%s", "write failed 30\n");
                        if (parent.location == root.location)root = parent;
                    }
                    return false;//if the father don't have to change then tell the grandfather don't change;
                } else {
                    //his parent's children is broken into two nodes
                    if (parent.children_num + 1 < max_key_num) {
                        //the children can directly added
                        AddChild(parent, cur.keys[0], the_added_children.keys[0], the_added_loc, num);
                        return false;
                    } else {
                        //the parent node has to split and tell his father that he splited;
                        //parent need to split and tell his father that he splited
                        //use the new_node and new_node_loc
                        AddChild(parent, cur.keys[0], the_added_children.keys[0], the_added_loc, num);
                        Split(parent, new_node, new_node_loc, splited_child_index, num);
                        if (parent.location == root.location) {
                            root.children_num = 2;
                            root.children[0] = parent.location;
                            root.children[1] = new_node.location;
                            root.keys[0] = parent.keys[0];
                            root.keys[1] = new_node.keys[0];
                            root.isleaf = false;
                            root.nxt = root.before = -1;
                            root.location = index_memory->FindEnd();
                            root_index = index_memory->Append(root);
                            return false;
                        }
                        cur = parent;//下面的cur还要用，所以换回要用的值
                        return true;
                    }
                }
            }
        }

        //check if the problem can be dealt with in the cur and his brother
        //if the child can be borrowed return true and the node doesn't merge
        //else return false then merge the children
        bool BorrowChild(Node cur, int num, Node parent, int &brother_to_merge_index, Node &brother_to_merge) {
            //cur is the num child of parent
            //so check the brother in num-1 and num+1
            bool flag = false;
            if (num - 1 >= 0) {
                Node left_brother;
                if (!index_memory->Read(parent.children[num - 1], left_brother))printf("%s", "read failed 9\n");
                if (left_brother.children_num > max_key_num / 2) {
                    flag = true;
                    parent.keys[num] = left_brother.keys[left_brother.children_num - 1];
                    for (int i = cur.children_num; i >= 1; --i) {
                        cur.children[i] = cur.children[i - 1];
                        cur.keys[i] = cur.keys[i - 1];
                    }
                    cur.children[0] = left_brother.children[left_brother.children_num - 1];
                    cur.keys[0] = left_brother.keys[left_brother.children_num - 1];
                    left_brother.children_num--;
                    cur.children_num++;
                    if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 8\n");
                    if (!index_memory->Write(left_brother.location, left_brother))printf("%s", "write failed 9\n");
                    if (!index_memory->Write(parent.location, parent))printf("%s", "write failed 12\n");
                } else {
                    brother_to_merge_index = num - 1;
                    brother_to_merge = left_brother;
                }
            }
            if (num + 1 < parent.children_num) {
                Node right_brother;
                if (!index_memory->Read(parent.children[num + 1], right_brother))printf("%s", "read failed 10\n");
                if (right_brother.children_num > max_key_num / 2) {
                    flag = true;
                    parent.keys[num + 1] = right_brother.keys[1];
                    cur.children[cur.children_num] = right_brother.children[0];
                    cur.keys[cur.children_num] = right_brother.keys[0];
                    for (int i = 1; i <= right_brother.children_num - 1; ++i) {
                        right_brother.children[i - 1] = right_brother.children[i];
                        right_brother.keys[i - 1] = right_brother.keys[i];
                    }
                    right_brother.children_num--;
                    cur.children_num++;
                    if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 10\n");
                    if (!index_memory->Write(right_brother.location, right_brother))
                        printf("%s", "write failed 11\n");
                    if (!index_memory->Write(parent.location, parent))printf("%s", "write failed 13\n");
                } else {
                    brother_to_merge_index = num + 1;
                    brother_to_merge = right_brother;
                }
            }
            return flag;
        }

        //merge rear into the front and then the rear will be deleted;
        void MergeBrothers(Node front, Node rear) {
            for (int i = front.children_num, j = 0; j < rear.children_num; ++i, ++j) {
                front.children[i] = rear.children[j];
                front.keys[i] = rear.keys[j];
            }
            if (rear.isleaf) {
                front.nxt = rear.nxt;
                if (rear.nxt != -1) {
                    Node after;
                    if (!index_memory->Read(rear.nxt, after))printf("%s", "read failed 11\n");
                    after.before = front.location;
                    if (!index_memory->Write(rear.nxt, after))printf("%s", "write failed 14\n");
                }
            }
            front.children_num += rear.children_num;
            if (!index_memory->Write(front.location, front))printf("%s", "write failed 15\n");
        }

        void MergeBlocks(Block front, Block rear, int front_loc) {
            for (int i = front.size, j = 0; j < rear.size; ++i, ++j) {
                front.record[i] = rear.record[j];
                front.keys[i] = rear.keys[j];
            }
            front.size += rear.size;
            if (!record_memory->Write(front_loc, front))printf("%s", "write failed 15\n");
        }

        //merged index is the child that remains 
        //we suppose that always remain the child that close to the front, merge the rare one
        bool RecursionRemove(Node &cur, const Key &key, bool &success, int &merged_index, Node &merged_child) {
            int num = BinarySearchLess(cur.keys, cur.children_num, key);
            if (num == -1)num = 0;
            if (cur.isleaf) {
                Block block;
                Node parent = cur;
                if (!record_memory->Read(cur.children[num], block))printf("%s", "read failed 13\n");
                int index = BinarySearch(block, key);
                if (index == -1) {
                    success = false;
                    return false;
                }
                for (int i = index; i < block.size; ++i) {
                    block.keys[i] = block.keys[i + 1];
                    block.record[i] = block.record[i + 1];
                }
                block.size--;
                if (index == 0) {
                    cur.keys[num] = block.keys[0];
                    if (root.location == cur.location)root = cur;
                }
                if (block.size >= max_rcd_num / 2) {
                    //directly remove
                    if (!record_memory->Write(cur.children[num], block))printf("%s", "write failed 17\n");
                    if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 31\n");
                    return false;
                } else {
                    //borrow a record from the brother block
                    //or merged
                    Block to_merge_brother;
                    int to_merge_brother_index;
                    if (num - 1 >= 0) {
                        Block left_brother;
                        if (!record_memory->Read(cur.children[num - 1], left_brother))printf("%s", "read failed 15\n");
                        if (left_brother.size > max_rcd_num / 2) {
                            cur.keys[num] = left_brother.keys[left_brother.size - 1];
                            for (int i = block.size; i >= 1; --i) {
                                block.keys[i] = block.keys[i - 1];
                                block.record[i] = block.record[i - 1];
                            }
                            block.keys[0] = left_brother.keys[left_brother.size - 1];
                            block.record[0] = left_brother.record[left_brother.size - 1];
                            block.size++;
                            left_brother.size--;
                            if (!record_memory->Write(cur.children[num - 1], left_brother))
                                printf("%s", "write failed 18\n");
                            if (!record_memory->Write(cur.children[num], block))printf("%s", "write failed 19\n");
                            if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 20\n");
                            if (cur.location == root.location)root = cur;
                            return false;
                        } else {
                            to_merge_brother = left_brother;
                            to_merge_brother_index = num - 1;
                        }
                    }
                    if (num + 1 <= cur.children_num - 1) {
                        Block right_brother;
                        if (!record_memory->Read(cur.children[num + 1], right_brother))printf("%s", "read failed 16\n");
                        if (right_brother.size > max_rcd_num / 2) {
                            cur.keys[num + 1] = right_brother.keys[1];
                            block.keys[block.size] = right_brother.keys[0];
                            block.record[block.size] = right_brother.keys[0];
                            for (int i = 1; i < right_brother.size - 1; ++i) {
                                right_brother.keys[i - 1] = right_brother.keys[i];
                                right_brother.record[i - 1] = right_brother.record[i];
                            }
                            block.size++;
                            right_brother.size--;
                            if (!record_memory->Write(cur.children[num + 1], right_brother))
                                printf("%s", "write failed 21\n");
                            if (!record_memory->Write(cur.children[num], block))printf("%s", "write failed 22\n");
                            if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 23\n");
                            if (cur.location == root.location)root = cur;
                            return false;
                        } else {
                            to_merge_brother = right_brother;
                            to_merge_brother_index = num + 1;
                        }
                    }
                    int the_merged_index;
                    Block the_merged_child;
                    if (to_merge_brother_index == num + 1) {
                        MergeBlocks(block, to_merge_brother, cur.children[num]);
                        the_merged_child = block;
                        the_merged_index = num;
                    } else {
                        MergeBlocks(to_merge_brother, block, cur.children[num - 1]);
                        the_merged_index = num - 1;
                        the_merged_child = to_merge_brother;
                    }
                    //there are children merged
                    //its index is in the reference;
                    if (cur.children_num > max_key_num / 2) {
                        //the merge can directly happen
                        //now the child in the index is completely well dealt;
                        for (int i = the_merged_index + 1; i <= cur.children_num - 2; ++i) {
                            cur.children[i] = cur.children[i + 1];
                            cur.keys[i] = cur.keys[i + 1];
                        }
                        cur.children_num--;
                        cur.keys[the_merged_index] = the_merged_child.keys[0];
                        if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 12\n");
                        if (cur.location == root.location)root = cur;
                        return false;
                    } else {
                        //borrow a child from the brother before ar after;
                        int brother_to_merge_index;
                        Node brother_to_merge;
                        bool borrow_flag = BorrowChild(cur, num, parent, brother_to_merge_index, brother_to_merge);
                        if (borrow_flag)return false;//the recursion is ended;
                        else {
                            if (brother_to_merge_index > num) {
                                MergeBrothers(cur, brother_to_merge);
                                merged_child = cur;
                                merged_index = num;
                            } else {
                                MergeBrothers(brother_to_merge, cur);
                                merged_child = brother_to_merge;
                                merged_index = num - 1;
                            }
                            return true;
                        }
                    }
                }

            } else {
                //should go to the next remove
                Node parent = cur;
                if (!index_memory->Read(cur.children[num], cur))printf("%s", "read failed 8\n3");
                int the_merged_index;//the index of the child be merged;
                Node the_merged_child;
                bool have_merged = RecursionRemove(cur, key, success, the_merged_index, the_merged_child);
                if (!have_merged) {
                    if (cmp.operator()(parent.keys[num], cur.keys[0])) {
                        parent.keys[num] = cur.keys[0];
                        if (!index_memory->Write(parent.location, parent))printf("%s", "write failed 32\n");
                        if (parent.location == root.location)root = parent;
                    }
                    return false;//the removal suceeded and the recursion ended;
                } else {
                    //there are children merged
                    //its index is in the reference;
                    if (cur.children_num > max_key_num / 2) {
                        //the merge can directly happen
                        //now the child in the index is completely well dealt;
                        for (int i = the_merged_index + 1; i <= cur.children_num - 2; ++i) {
                            cur.children[i] = cur.children[i + 1];
                            cur.keys[i] = cur.keys[i + 1];
                        }
                        cur.children_num--;
                        cur.keys[the_merged_index] = the_merged_child.keys[0];
                        if (!index_memory->Write(cur.location, cur))printf("%s", "write failed 12\n");
                        if (cur.location == root.location)root = cur;
                        return false;
                    } else {
                        //borrow a child from the brother before ar after;
                        int brother_to_merge_index;
                        Node brother_to_merge;
                        bool borrow_flag = BorrowChild(cur, num, parent, brother_to_merge_index, brother_to_merge);
                        if (borrow_flag)return false;//the recursion is ended;
                        else {
                            if (brother_to_merge_index > num) {
                                MergeBrothers(cur, brother_to_merge);
                                merged_child = cur;
                                merged_index = num;
                            } else {
                                MergeBrothers(brother_to_merge, cur);
                                merged_child = brother_to_merge;
                                merged_index = num - 1;
                            }
                            return true;
                        }
                    }
                }
            }
        }

    public:

        //B+树的构造函数,由一个文件构造
        BPlusTree(
                const char *index_file_name,
                const char *record_file_name) {
            index_file = new char[40];
            strcpy(index_file, index_file_name);
            record_file = new char[40];
            strcpy(record_file, record_file_name);
            index_memory = new MemoryRiver<Node>(index_file);
            int_memory = new MemoryRiver<int>(index_file);
            record_memory = new MemoryRiver<Block>(record_file);
            //read the number of the records
            //the index file: scale + root_index + the Nodes
            if (!int_memory->Read(0, scale)) {
                //If the reading fails it means the file is empty
                //and the scale and the root index should be initialized
                scale = 0;
                if (!int_memory->Write(0, scale))printf("%s", "write failed 27\n");
                if (!int_memory->Write(sizeof(int), root_index))printf("%s", "write failed 28\n");
            } else {
                if (!int_memory->Read(sizeof(int), root_index))printf("%s", "read failed 1\n");
                if (!index_memory->Read(root_index, root))
                    printf("%s", "read failed 2\n");//read the information for the root;
//                int_memory->Read(sizeof(int), root_index);
//                int_memory->Read(root_index, root);//read the information for the root;
            }
        }

        //B+树的析构函数
        ~BPlusTree() {
            if (!int_memory->Write(0, scale))printf("%s", "write failed 1\n");
            if (!int_memory->Write(sizeof(int), root_index))printf("%s", "write failed 2\n");
//            int_memory->Write(0, scale);
//            int_memory->Write(sizeof(int), root_index);
            delete[]index_file;
            delete[]record_file;
            delete index_memory;
            delete record_memory;
            delete int_memory;
        }

        //插入一个元素，参数是插入元素的键值和记录的详细信息，返回插入是否成功
        //如果说这个元素本来存在，插入失败返回false
        bool Insert(const Key &key, const Info &info) {
            bool flag;
            Node tmp;
            int children_index;
            int loc;
            Node cur = root;
            RecursionInsert(cur, key, info, flag, children_index, tmp, loc);
            scale += (int) flag;
            return flag;
        }

        //删除一个元素，参数是要删除元素的键值，返回是否删除成功，
        //如果这个元素在B+树中不存在就删除失败
        bool Remove(const Key &key) {
            bool flag;
            Node tmp;
            int tmp_index;
            Node cur = root;
            RecursionRemove(cur, key, flag, tmp_index, tmp);
            scale -= (int) flag;
            return flag;
        }

        //查询一个元素，参数是要查询元素的键值
        //返回值是一个pair，bool代表有没有找到
        //如果找到这个元素存在，返回true，同时返回记录信息的具体值
        //如果没有找到，返回false，这时的返回struct是随机值，不应该被访问
        std::pair<bool, Info> Find(const Key &key) {
            std::pair<std::pair<int, int>, Info> tmp = RecursionFind(key);
            if (tmp.first.first == -1)return {false, tmp.second};
            else return {true, tmp.second};
        }

        //修改一个元素，参数是要修改元素的键值和修改之后的信息
        //返回一个bool，修改成功返回true，否则返回false
        //如果这个要修改的元素在B+树中不存在就会返回false
        bool Modify(const Key &key, const Info &new_info) {
            std::pair<std::pair<int, int>, Info> tmp = RecursionFind(key);
            if (tmp.first.first == -1)return false;
            else {
//                record_memory->Write(tmp.first, new_info);
                //the first int is the index of the block, the second is the index of the record
                Block block;
                if (!record_memory->Read(tmp.first.first, block))printf("%s", "read failed 5\n");
                block.record[tmp.first.second] = new_info;
                if (!record_memory->Write(tmp.first.first, block))printf("%s", "write failed 3\n");
                return true;
            }
        }

        //清空B+树的有关文件
        void Clear() {
            scale = 0;
            root.info.isleaf = true;
            root.info.children_num = 0;
        }

        //返回现在总共有多少条记录
        int GetSize() {
            return scale;
        }

    };
}
#endif //UNTITLED2_BPLUSTREE_H

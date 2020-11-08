//
// Created by 田地 on 2020/11/6.
//

#ifndef PROJECT2_BPTREE_H
#define PROJECT2_BPTREE_H

#include "node.h"
#include "record.h"

class BpTree {
private:
    shared_ptr<Node> root;
    // the maximum number of keys a node can have
    // 1. max # of keys = degree
    // 2. max # of pointers = degree + 1
    // 3. min # of keys = (degree + 1) / 2 - 1
    // 4. min # of pointers = (degree + 1) / 2
    int64_t degree;
public:
    BpTree(int64_t d) : degree(d) {
        root = make_shared<Node>(Node(d));
    }
    inline blockNo Search(uint64_t k) {return root->Search(k);}
    inline void Insert(uint64_t k, blockNo bptr) {root = Node::Insert(root, k, bptr);}
    bool Delete(uint64_t k);

};


#endif //PROJECT2_BPTREE_H

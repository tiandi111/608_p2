//
// Created by 田地 on 2020/11/6.
//

#ifndef PROJECT2_NODE_H
#define PROJECT2_NODE_H

#include "record.h"
#include <memory>
#include <vector>
#include <deque>
#include <iostream>

using namespace std;

typedef int64_t blockNo;

class Node {
private:
    // the maximum number of keys a node can have
    // 1. max # of keys = degree
    // 2. max # of pointers = degree + 1
    // 3. min # of keys = (degree + 1) / 2 - 1
    // 4. min # of pointers = (degree + 1) / 2
    int64_t degree;
    shared_ptr<Node> next;
    shared_ptr<Node> parent;
    // all records between key1 and key2 satisfy  key1 <= record.key < key2
    vector<int64_t> keys;
    vector<shared_ptr<Node>> children;
    vector<blockNo> bptrs;

    void borrowFromLeftLeaf(shared_ptr<Node> left, uint64_t childIdx);
    void borrowFromRightLeaf(shared_ptr<Node> left, uint64_t childIdx);
    void borrowFromLeftInternal(shared_ptr<Node> right, uint64_t childIdx);
    void borrowFromRightInternal(shared_ptr<Node> right, uint64_t childIdx);
    void mergeWithLeftSibling(shared_ptr<Node> left, uint64_t childIdx);
    void mergeWithRighttSibling(shared_ptr<Node> curr, shared_ptr<Node> right, uint64_t childIdx);

public:
    Node() = default;
    ~Node() = default;
    Node(int64_t d) : degree(d) {}
    Node(int64_t d,
            shared_ptr<Node> next,
            shared_ptr<Node> parent,
            vector<int64_t> keys,
            vector<shared_ptr<Node>> children,
            vector<blockNo> bptrs
            ) : degree(d), next(next), parent(parent), keys(keys), children(children), bptrs(bptrs) {}
    blockNo Search(int64_t k);

    // inserts a block pointer with given key
    static shared_ptr<Node> Insert(shared_ptr<Node> root, int64_t k, blockNo bptr) {
        if(!root->isRoot()) {
            throw runtime_error("cannot call Insert on a non-root node");
        }
        vector<int64_t> stack;
        shared_ptr<Node> curr = root;
        while(!curr->isLeaf()) {
            int64_t i = curr->scan(k);
            stack.push_back(i);
            curr = curr->children[i];
        }
        curr->insertAt(k, bptr);
        // overflows curr
        while (curr->overflow()) {
            // splits the curr node, up the middle key to its parent
            int64_t splitIndex = curr->keys.size() / 2;
            int64_t upKey = curr->keys[splitIndex];

            uint64_t childrenStIdx = curr->children.empty()? 0 : splitIndex+1;
            uint64_t bptrsStIdx = curr->bptrs.empty()? 0 : splitIndex;

            uint64_t keysStIdx = curr->isLeaf()? splitIndex : splitIndex+1;
            shared_ptr<Node> sibling = make_shared<Node>(
                    Node(curr->degree,
                         curr->next,
                         curr->parent,
                         vector<int64_t>(curr->keys.begin()+keysStIdx, curr->keys.end()),
                         vector<shared_ptr<Node>>(curr->children.begin()+childrenStIdx, curr->children.end()),
                         vector<blockNo>(curr->bptrs.begin()+bptrsStIdx, curr->bptrs.end()))
            );
            for(auto c : sibling->children) {c->parent = sibling;}
            curr->keys.erase(curr->keys.begin()+splitIndex, curr->keys.end());
            curr->children.erase(curr->children.begin()+childrenStIdx, curr->children.end());
            curr->bptrs.erase(curr->bptrs.begin()+bptrsStIdx, curr->bptrs.end());
            curr->next = sibling;

            shared_ptr<Node> parent = curr->parent;
            int64_t upPosition = 0;
            if(parent) {
                upPosition = stack.back();
                stack.pop_back();
            } else {
                parent = make_shared<Node>(Node(curr->degree));
                parent->children.push_back(curr);
                curr->parent = parent;
                sibling->parent = parent;
            }
            parent->keys.insert(parent->keys.begin()+upPosition, upKey);
            parent->children.insert(parent->children.begin()+upPosition+1, sibling);

            curr = parent;
        }
        return root->parent? root->parent : root;
    }

    static shared_ptr<Node> Delete(shared_ptr<Node> root, int64_t k) {
        if(!root->isRoot()) {
            throw runtime_error("cannot call Delete on a non-root node");
        }
        shared_ptr<Node> curr = root;
        vector<int64_t> stack;
        while(!curr->isLeaf()) {
            int64_t i = curr->scan(k);
            stack.push_back(i);
            curr = curr->children[i];
        }
        blockNo dBlock = curr->deleteFrom(k);
        if(dBlock < 0) {
            return root;
        }
        while (!stack.empty() && curr->underflow()) {
            uint64_t childIdx = stack.back();
            stack.pop_back();
            shared_ptr<Node> leftSibling = childIdx > 0? curr->parent->children[childIdx-1] : nullptr;
            shared_ptr<Node> rightSibling = childIdx < curr->parent->children.size() - 1?
                    curr->parent->children[childIdx+1] : nullptr;
            // case1: keys# of left sibling > (degree + 1) / 2 -1
            if(leftSibling &&
            (leftSibling->keys.size() > (curr->degree + 1) / 2 -1)) {
                if (curr->isLeaf()) {
                    curr->borrowFromLeftLeaf(leftSibling, childIdx);
                } else {
                    curr->borrowFromLeftInternal(leftSibling, childIdx);
                }
            }
            // case2: keys# of right sibling > (degree + 1) / 2 -1
            else if(rightSibling &&
            (rightSibling->keys.size() > (curr->degree + 1) / 2 -1)) {
                if (curr->isLeaf()) {
                    curr->borrowFromRightLeaf(rightSibling, childIdx);
                } else {
                    curr->borrowFromRightInternal(rightSibling, childIdx);
                }
            }
            // case3: merge with left sibling or right sibling
            else {
                if(leftSibling) { // merge with left sibling
                    curr->mergeWithLeftSibling(leftSibling, childIdx);
                } else { // merge with right sibling
                    curr->mergeWithRighttSibling(curr, rightSibling, childIdx);
                }
            }
            curr = curr->parent;
        }
        if(root->children.size() == 1) {
            root->children[0]->parent = nullptr;
            return root->children[0];
        }
        return root;
    }

    // validates the b+ tree
    // always start from the root
    // return code
    // zero means valid
    // non-zero indicates which rule is violated
    static int Validate(shared_ptr<Node> node) {
        shared_ptr<Node> curr = node;
        while (!curr->isRoot()) {
            curr = curr->parent;
        }
        return curr->validate();
    }

    // prints the b+tree
    // always start from the root
    static void PrintNodeStructure(shared_ptr<Node> node) {
        shared_ptr<Node> curr = node;
        while (!curr->isRoot()) {
            curr = curr->parent;
        }
        deque<shared_ptr<Node>> q = {curr};
        while (!q.empty()) {
            uint64_t size = q.size();
            for(int i=0; i<size; i++) {
                curr = q.front();
                q.pop_front();
                for(auto c : curr->children) {
                    q.push_back(c);
                }
                for(int64_t k : curr->keys) {
                    cout<< "|" << k;
                }
                cout<< "| ";
            }
            cout<< endl;
        }
    }

    inline bool isLeaf() {return children.empty();}
    inline bool isRoot() {return !parent;}
    // returns if this node overflows
    inline bool overflow() {return keys.size() > degree;}
    // returns if this node underflows
    inline bool underflow() {return keys.size() < (degree + 1) / 2 - 1;}
    int64_t smallestKey();
    int64_t largestKey();
    // scans the first key satisfies target<key
    // this function is used to
    int64_t scan(int64_t target);
    // finds the block pointed by the given key
    // if not exist, return -1
    // only callable on leaf node
    blockNo find(int64_t key);
    // inserts the new key and the block pointer at this node
    // only callable on leaf node
    // key is in range [key1, key2)
    void insertAt(int64_t key, blockNo bptr);
    // deletes the block with the given key
    // if it exist, returns the block number
    // otherwise, returns -1
    blockNo deleteFrom(int64_t key);
    // recurrently validates the tree rooted at this node
    int validate();
    // returns if this node pass the validity test
    int isValid();
};


#endif //PROJECT2_NODE_H

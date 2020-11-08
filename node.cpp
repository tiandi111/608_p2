//
// Created by 田地 on 2020/11/6.
//

#include "node.h"
#include <iostream>

int64_t Node::scan(int64_t k) {
    int i = 0;
    for(; i < this->keys.size() &&  k >= this->keys[i]; i++) {}
    return i;
}

int64_t Node::find(int64_t k) {
    if(!isLeaf()) {
        throw runtime_error("cannot call find on a non-leaf node");
    }
    int64_t i = scan(k);
    if(i < 0 || i >= keys.size() || keys[i] != k) {
        return -1;
    }
    return bptrs[i];
}

void Node::insertAt(int64_t k, blockNo bptr) {
    if(!isLeaf()) {
        throw runtime_error("cannot call insertAt on a non-leaf node");
    }
    int64_t p = scan(k);
    keys.insert(keys.begin()+p, k);
    bptrs.insert(bptrs.begin()+p, bptr);
}

blockNo Node::deleteFrom(int64_t k) {
    if(!isLeaf()) {
        throw runtime_error("cannot call deleteFrom on a non-leaf node");
    }
    for(int i=0; i<keys.size(); i++) {
        if(keys[i] == k) {
            keys.erase(keys.begin()+i);
            blockNo dBlock = bptrs[i];
            bptrs.erase(bptrs.begin()+i);
            return dBlock;
        }
    }
    return -1;
}


blockNo Node::Search(int64_t k) {
    Node curr = *this;
    while(!curr.isLeaf()) {
        int64_t i = curr.scan(k);
        curr = *curr.children[i];
    }
    return curr.find(k);
}

int Node::validate() {
    int code = isValid();
    if(code > 0) { return code; }
    for(auto child : children) {
        code = child->validate();
        if(code > 0) { return code; }
    }
    return 0;
}

int Node::isValid() {
    if(keys.size() > degree) {
        return 1;
    }
    if(!isRoot() && (keys.size() < (degree+1) / 2 - 1)) {
        return 2;
    }
    if(!children.empty() && !bptrs.empty()) {
        return 3;
    }
    if(!children.empty() && (children.size() != keys.size()+1)) {
        return 4;
    }
//    if(!bptrs.empty() && (bptrs.size() != keys.size()+1)) {
//        return 5;
//    }
    if(!keys.empty()) {
        uint64_t last = keys.front();
        for(int i=1; i<keys.size(); i++) {
            if(keys[i] <= last) {
                return 6;
            }
            last = keys[i];
        }
        if(next && next->keys[0] <= last) {
            return 6;
        }
    }
    return 0;
}

int64_t Node::smallestKey() {
    if(!isLeaf()) {
        return children[0]->smallestKey();
    }
    return keys.front();
}

int64_t Node::largestKey() {
    if(!isLeaf()) {
        return children.back()->largestKey();
    }
    return keys.back();
}

void Node::borrowFromLeftLeaf(shared_ptr<Node> leftSibling, uint64_t childIdx) {
    // move block pointer
    blockNo newBlockNo = leftSibling->bptrs.back();
    leftSibling->bptrs.pop_back();
    bptrs.insert(bptrs.begin(), newBlockNo);
    // move key
    keys.insert(keys.begin(), leftSibling->keys.back());
    // replace parent key
    parent->keys[childIdx] = leftSibling->keys.back();
    leftSibling->keys.pop_back();
}

void Node::borrowFromLeftInternal(shared_ptr<Node> leftSibling, uint64_t childIdx) {
    // move child
    children.insert(children.begin(), leftSibling->children.back());
    leftSibling->children.pop_back();
    // move key
    keys.insert(keys.begin(), parent->keys[childIdx-1]);
    // replace parent key
    leftSibling->keys.pop_back();
}

void Node::borrowFromRightLeaf(shared_ptr<Node> rightSibling, uint64_t childIdx) {
    // move block pointer
    blockNo newBlockNo = rightSibling->bptrs.front();
    rightSibling->bptrs.erase(rightSibling->bptrs.begin());
    bptrs.push_back(newBlockNo);
    // move key
    keys.push_back(rightSibling->keys.front());
    rightSibling->keys.erase(rightSibling->keys.begin());
    // replace parent key
    parent->keys[childIdx] = rightSibling->keys.empty()?
            rightSibling->next->keys.front() : rightSibling->keys.front();
}

void Node::borrowFromRightInternal(shared_ptr<Node> rightSibling, uint64_t childIdx) {
    // move child
    children.push_back(rightSibling->children.front());
    rightSibling->children.erase(rightSibling->children.begin());
    // move key
    keys.push_back(parent->keys[childIdx]);
    // replace parent key
    parent->keys[childIdx] = rightSibling->keys.front();
    rightSibling->keys.erase(rightSibling->keys.begin());
}

void Node::mergeWithLeftSibling(shared_ptr<Node> leftSibling, uint64_t childIdx) {
    int64_t downKey = parent->keys[childIdx-1];
    parent->keys.erase(parent->keys.begin()+childIdx-1);
    parent->children.erase(parent->children.begin()+childIdx); // delete curr
    if(!isLeaf()) { leftSibling->keys.push_back(downKey); }
    leftSibling->keys.insert(leftSibling->keys.end(), keys.begin(), keys.end());
    leftSibling->children.insert(leftSibling->children.end(), children.begin(), children.end());
    leftSibling->bptrs.insert(leftSibling->bptrs.end(), bptrs.begin(), bptrs.end());
    leftSibling->next = next;
    for(auto c : children) { c->parent = leftSibling; }
}

void Node::mergeWithRighttSibling(shared_ptr<Node> curr, shared_ptr<Node> rightSibling, uint64_t childIdx) {
    int64_t downKey = parent->keys[childIdx];
    parent->keys.erase(parent->keys.begin()+childIdx);
    parent->children.erase(parent->children.begin()+childIdx+1); // delete rightSibling
    if(!isLeaf()) { keys.push_back(downKey); }
    keys.insert(keys.end(), rightSibling->keys.begin(), rightSibling->keys.end());
    children.insert(children.end(), rightSibling->children.begin(), rightSibling->children.end());
    bptrs.insert(bptrs.end(), rightSibling->bptrs.begin(), rightSibling->bptrs.end());
    next = rightSibling->next;
    for(auto c : rightSibling->children) { c->parent = curr; }
}



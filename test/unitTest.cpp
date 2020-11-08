//
// Created by 田地 on 2020/11/7.
//

#include "unitTest.h"
#include "node.h"
#include <string>

void unitTest::TestNodeIsRoot() {
    Node n = Node(3);
    assert(n.isRoot());
}

void unitTest::TestNodeIsLeaf() {
    Node n = Node(3);
    assert(n.isLeaf());
}

void unitTest::TestNodeInsert() {
    shared_ptr<Node> root = make_shared<Node>(Node(3));
    for(int i=0; i<20; i++) {
        root = Node::Insert(root, i, i);
        Node::PrintNodeStructure(root);
        cout<< "-------" <<endl;
        int code = Node::Validate(root);
        if(code != 0) {
            throw runtime_error("validate failed: " + to_string(code));
        }
    }
}

void unitTest::TestNodeDelete() {
    shared_ptr<Node> root = make_shared<Node>(Node(3));
    for(int i=0; i<20; i++) { root = Node::Insert(root, i, i); }
    Node::PrintNodeStructure(root);cout<< "-------" <<endl;
    for(int i=0; i<20; i++) {
        root = Node::Delete(root, i);
        int code = Node::Validate(root);
        Node::PrintNodeStructure(root);
        cout<< "-------" <<endl;
        if(code != 0) {
            throw runtime_error("validate failed: " + to_string(code));
        }
    }
}
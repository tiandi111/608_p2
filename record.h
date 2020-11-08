//
// Created by 田地 on 2020/11/6.
//

#ifndef PROJECT2_RECORD_H
#define PROJECT2_RECORD_H


#include <cstdint>

class Record {
private:
    int64_t key;
public:
    Record(int64_t k) : key(k) {}
    inline int64_t Key() {return key;}
};


#endif //PROJECT2_RECORD_H

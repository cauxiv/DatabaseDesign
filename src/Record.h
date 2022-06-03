//
// Created by woodi on 2022/06/02.
//

#ifndef MODERNDATABASE_RECORD_H
#define MODERNDATABASE_RECORD_H

#include <stdlib.h>
#include <string.h>

class Record {
    unsigned len;
    char *data;

public:
    Record &operator=(Record &rhs) = delete;

    Record(Record &t) = delete;

    Record(Record &&t) : len(t.len), data(t.data) {
        t.data = nullptr;
        t.len = 0;
    }

    Record(unsigned len, const char *const ptr) : len(len) {
        data = static_cast<char *>(malloc(len));
        if (data) memcpy(data, ptr, len);
    }

    ~Record() {
        free(data);
    }

    const char *getData() const {
        return data;
    }

    unsigned getLen() const {
        return len;
    }
};

#endif //MODERNDATABASE_RECORD_H

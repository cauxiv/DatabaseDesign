//
// Created by woodi on 2022/06/02.
//

#ifndef MODERNDATABASE_PAGE_H
#define MODERNDATABASE_PAGE_H

#include <atomic>

#include "BufferManager.h"

class Page {
protected:
    uint64_t id;
    std::atomic<size_t> size; // size in pages
    BufferManager &bm;

public:
    Page(BufferManager &bm, uint64_t id) : id(id), size(0), bm(bm) {}

    uint64_t getID() {
        return id;
    }

    size_t getSize() {
        return size.load();
    }
};

#endif //MODERNDATABASE_PAGE_H

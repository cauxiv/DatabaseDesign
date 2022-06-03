//
// Created by woodi on 2022/06/02.
//

#ifndef MODERNDATABASE_BUFFERMANAGER_H
#define MODERNDATABASE_BUFFERMANAGER_H

#include <pthread.h>
#include <unordered_map>

#include "BufferFrame.h"

class BufferManager {
public:
    BufferManager(size_t size);

    ~BufferManager();


    BufferFrame &loadPage(uint64_t pageID);

    void unloadPage(BufferFrame &frame);

private:

    int getSegmentFd(unsigned segmentID);

    // max number of bufered pages
    size_t maxSize;

    std::unordered_map<uint64_t, BufferFrame> frames;

    std::unordered_map<unsigned, int> segments;

};


#endif //MODERNDATABASE_BUFFERMANAGER_H

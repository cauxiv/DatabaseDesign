//
// Created by woodi on 2022/06/02.
//

#include <string>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <unistd.h>

#include "BufferManager.h"

BufferManager::BufferManager(size_t size) {
    maxSize = size;

    frames.reserve(size);
}

BufferManager::~BufferManager() {
    // write dirty pages back to file
    for (auto &bf: frames) bf.second.flush();

    // close segment file descriptors
    for (auto &bf: segments) close(bf.second);

}

BufferFrame &BufferManager::fixPage(uint64_t pageID) {

    BufferFrame *bf;

    // check whether the page is already bufffered
    auto entry = frames.find(pageID);
    // if found
    if (entry != frames.end()) {
        bf = &entry->second;

    } else {

        entry = frames.find(pageID);
        if (entry != frames.end()) {
            bf = &entry->second;

        } else {
            // check whether the buffer is full and we need to unload a frame
            if (frames.size() >= maxSize) {
                throw std::runtime_error("could not find free frame");
            }

            // pageID = 64bit
            // first 16bit: segment(=filename)
            // 48bit: actual page ID
            int fd = getSegmentFd(pageID >> 48);

            // create a new frame in the map(with key pageID)
            auto ret = frames.emplace(std::piecewise_construct,
                                      std::forward_as_tuple(pageID),
                                      std::forward_as_tuple(fd, pageID));
            bf = &ret.first->second;

            bf->currentUsers++;
        }
    }

    return *bf;
}

int BufferManager::getSegmentFd(unsigned int segmentID) {
    int fd;

    auto entry = segments.find(segmentID);
    if (entry != segments.end()) {
        fd = entry->second;
    } else {
        // this is why fixPage Method shift >> 48 to get fd
        char filename[15];
        sprintf(filename, "%d", segmentID);

        // open the segment file
        fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            throw std::runtime_error(std::strerror(errno));
        }

        // save the fd to cache and share it
        segments[segmentID] = fd;

    }
}

void BufferManager::unfixPage(BufferFrame &frame, bool isDirty) {
    if (isDirty) frame.markDirty();
}
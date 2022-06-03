//
// Created by woodi on 2022/06/02.
//

#ifndef MODERNDATABASE_BUFFERFRAME_H
#define MODERNDATABASE_BUFFERFRAME_H

#include <cstdio>
#include <atomic>

// frame state
enum state_t {
    New,    // no data loaded
    Clean,  // data loaded but no data changed
    Dirty,  // data loaded and changed
};

const size_t blocksize = 4096;

class BufferFrame {
public:
    BufferFrame(int segmentFd, u_int64_t pageID);

    ~BufferFrame();

    BufferFrame &operator=(BufferFrame &rhs) = delete;

    void *getData();

    u_int64_t getID() { return id; }

    void flush();

private:

    void markDirty() { state = state_t::Dirty; }

    void loadData();

    void writeData();

    // pageID
    u_int64_t id;

    // pointer to loaded data
    void *data;

    // page state: clean/dirty/newly created etc;
    state_t state;

    // offset in the segment file
    off_t offset;

    // file descriptor of the file the segment is mapped to
    int fd;

    unsigned currentUsers;

    friend class BufferManager;
};


#endif //MODERNDATABASE_BUFFERFRAME_H

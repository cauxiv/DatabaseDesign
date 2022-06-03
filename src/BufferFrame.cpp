//
// Created by woodi on 2022/06/02.
//

#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "BufferFrame.h"

BufferFrame::BufferFrame(int segmentFd, u_int64_t pageID) {

    id = pageID;
    data = NULL;
    state = state_t::New;
    offset = blocksize * (pageID & 0x0000FFFFFFFFFFFF);
    fd = segmentFd;
    
    currentUsers = 0;
}

BufferFrame::~BufferFrame() {

    // write modified data back to disk
    flush();

    // free allocated data
    if (data != NULL) {
        free(data);
        data = NULL;
    }
}

void *BufferFrame::getData() {
    // load data, if state is New
    if (state == state_t::New) loadData();

    return data;
}

void BufferFrame::flush() {
    // write all changes to disk, if the page is modified after loaded
    if (state == state_t::Dirty) writeData();
}

// load data from disk
void BufferFrame::loadData() {
    // alloc data as block size
    data = malloc(blocksize);
    assert(data != NULL);

    // read data from file to buffer
    pread(fd, data, blocksize, offset);

    // if data is loaded, change state to Clean
    state = state_t::Clean;
}

void BufferFrame::writeData() {
    // write data back to file on disk
    pwrite(fd, data, blocksize, offset);

    // After write data on disk, change state to clean
    state = state_t::Clean;
}
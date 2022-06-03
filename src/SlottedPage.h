//
// Created by woodi on 2022/06/02.
//

#ifndef MODERNDATABASE_SLOTTEDPAGE_H
#define MODERNDATABASE_SLOTTEDPAGE_H


#include "Page.h"
#include "TID.h"
#include "Record.h"

class SlottedPage : public Page {
public:
    struct Header {
        uint32_t slotCount;
        uint32_t firstFreeSlot;
        off_t dataStart;
        size_t freeSpace;

        Header() : slotCount(0), firstFreeSlot(0), dataStart(blocksize), freeSpace(blocksize - sizeof(Header)) {}
    };

    struct Slot {
        off_t offset;
        uint64_t length;

        Slot() : offset(0), length(0) {}

        inline bool isFree() {
            return (offset == 0);
        }

        inline bool isIndirection() {
            return (offset == 1);
        }

        inline bool isRecord() {
            return (offset >= 2);
        }

        inline TID getIndirectionTID() {
            return TID{(uint32_t) (length >> 32), (uint32_t) length};
        }

        inline void setIndirection(TID tid) {
            offset = 1;
            length = (uint64_t) (tid.pageID) << 32 || tid.slotID;
        }
    };

    SlottedPage(BufferManager &bm, uint64_t id) : Page(bm, id) {};

    TID insert(const Record &r);

    Record lookup(TID tid);

private:
    void compactPage(uint32_t pageID);

    friend class TableScan;
};


#endif //MODERNDATABASE_SLOTTEDPAGE_H

//
// Created by woodi on 2022/06/02.
//

#include <new>
#include <queue>
#include <vector>

#include "SlottedPage.h"

TID SlottedPage::insert(const Record &r) {
    char *data;
    Header *header;
    Slot *slot;
    uint32_t pageID;
    uint32_t slotID;
    size_t recLen = r.getLen();

    // segmentID prefix for the pageID
    uint64_t segPrefix = (id << 48);

    // try all existing pages
    for (pageID = 0; pageID < size; pageID++) {
        // open page for read only and read the header
        BufferFrame &rbf = bm.loadPage(segPrefix | pageID);
        data = static_cast<char *>(rbf.getData());
        header = reinterpret_cast<Header *>(data);
        size_t freeSpace = header->freeSpace;

        // enough free space in the current page?
        if (recLen + sizeof(Slot) <= freeSpace) {
            off_t headEnd = sizeof(Header) + (header->slotCount + 1) * sizeof(Slot);
            off_t dataStart = header->dataStart;

            if (dataStart <= headEnd || recLen > (dataStart - headEnd)) {
                // must compact the page
                compactPage(pageID);
            }
            break; // found a page with enough space
        }
    }

    // open page for writing
    BufferFrame &bf = bm.loadPage(segPrefix | pageID);
    data = static_cast<char *>(bf.getData());

    // new page(
    if (pageID == size) {
        size++;

        // insert new Header
        header = new(data) Header();

        // insert new Slot
        slotID = 0;
        slot = new(data + sizeof(Header)) Slot();
        header->slotCount = 1;
        header->firstFreeSlot = 1;

        // update free space
        header->freeSpace -= sizeof(Slot) + recLen;
    } else { // existing page
        uint32_t slotCount = header->slotCount;

        // look for an existing free slot
        if (header->firstFreeSlot < slotCount) {
            slotID = header->firstFreeSlot;

            // search for the next free slot
            uint32_t i = slotID + 1;
            for (; i < slotCount; i++) {
                Slot &otherSlot = reinterpret_cast<Slot *>(data + sizeof(Header))[i];
                if (otherSlot.isFree()) break;
            }
            header->firstFreeSlot = i;

            // update free space
            header->freeSpace -= recLen;

            // insert new Slot
        } else {
            slotID = slotCount;

            header->slotCount++;
            header->firstFreeSlot = header->slotCount;

            //update free space;
            header->freeSpace -= sizeof(Slot) + recLen;
        }

        slot = new(data + sizeof(Header) + slotID * sizeof(Slot)) Slot();
    }

    // Insert Record Data
    header->dataStart -= recLen;
    slot->offset = header->dataStart;
    slot->length = recLen;
    char *recPtr = data + slot->offset;
    memcpy(recPtr, r.getData(), recLen);

    bm.unloadPage(bf);
    return TID{pageID, slotID};
}


Record SlottedPage::lookup(TID tid) {
    //open page for reading
    BufferFrame &bf = bm.loadPage((id << 48) | tid.pageID);
    char *data = static_cast<char *>(bf.getData());

    // read page
    Slot &slot = reinterpret_cast<Slot *>(data + sizeof(Header))[tid.slotID];

    if (slot.isIndirection()) {
        // recursively lookup
        TID itid = slot.getIndirectionTID();
        return lookup(itid);
    } else {
        Record r(slot.length, data + slot.offset);
        return std::move(r);
    }
}

void SlottedPage::compactPage(uint32_t pageID) {
    // open page for writing
    BufferFrame &bf = bm.loadPage((id << 48) | pageID);
    char *data = static_cast<char *>(bf.getData());
    Header *header = reinterpret_cast<Header *>(data);

    uint32_t slotCount = header->slotCount;
    Slot *slots = reinterpret_cast<Slot *>(data + sizeof(Header));

    // put all non-free and non-indirection slots in a priority queue
    struct SlotPtrCmpOffset {
        bool operator()(Slot *const &a, Slot *const &b) {
            return a->offset < b->offset;
        }
    };

    std::priority_queue<Slot *, std::vector<Slot *>, SlotPtrCmpOffset> slotPtrs;
    for (uint32_t slotID = 0; slotID < slotCount; slotID++) {
        Slot &slot = slots[slotID];

        if (!slot.isIndirection() && !slot.isFree()) slotPtrs.push(&slot);
    }

    off_t offset = blocksize;

    // move records
    while (!slotPtrs.empty()) {
        Slot *slot = slotPtrs.top();
        slotPtrs.pop();

        // nothing to move, if the record length is 0 bytes
        if (slot->length == 0) continue;

        // move data
        offset -= slot->length;
        memmove(data + offset, data + slot->offset, slot->length);
        slot->offset = offset;
    }

    header->dataStart = offset;

    bm.unloadPage(bf);
}
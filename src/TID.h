//
// Created by woodi on 2022/06/02.
//

#ifndef MODERNDATABASE_TID_H
#define MODERNDATABASE_TID_H

#include <cstdint>

struct TID {
    uint32_t pageID;
    uint32_t slotID;

    bool operator==(const TID &tid) const {
        return pageID == tid.pageID && slotID == tid.slotID;
    }

    bool operator<(const TID &tid) const {
        return pageID < tid.pageID || (pageID == tid.pageID && slotID < tid.slotID);
    }
};

namespace std {
    template<>
    struct hash<TID> {
        typedef TID argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const& tid) const {
            value_type const h1 ( std::hash<uint32_t>()(tid.pageID) );
            value_type const h2 ( std::hash<uint32_t>()(tid.slotID) );
            return h1 ^ (h2 << 1);
        }
    };
}

#endif //MODERNDATABASE_TID_H

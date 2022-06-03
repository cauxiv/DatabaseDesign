//
// Created by woodi on 2022/06/01.
//

#ifndef MODERNDATABASE_SCHEMA_H
#define MODERNDATABASE_SCHEMA_H

#include <vector>
#include <string>

#include "Types.h"

struct Schema {
    struct Relation {
        struct Attribute {
            std::string name;
            Types::Tag type;
            size_t len;
            bool notNull;

            Attribute() : len(~0), notNull(true) {}
        };

        std::string name;
        std::vector<unsigned> primaryKey;
        std::vector<Schema::Relation::Attribute> attributes;
        size_t size;
        uint16_t segmentID;

        Relation(const std::string &name) : name(name), size(0), segmentID(0) {}
    };

    std::vector<Schema::Relation> relations;

    std::string toString() const;
};


#endif //MODERNDATABASE_SCHEMA_H

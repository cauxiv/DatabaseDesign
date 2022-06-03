//
// Created by woodi on 2022/06/02.
//

#ifndef MODERNDATABASE_TYPES_H
#define MODERNDATABASE_TYPES_H

#include <string>
#include <cmath>
#include <cstdint>
#include <cstring>


/**
 * Types
 */
namespace Types {
    enum class Tag : unsigned {
        Integer, Char, String
    };
}

/**
 * Integer
 */
typedef int Integer;


/**
 * Char
 */
template<unsigned len>
struct Char {
    char data[len];

    void loadString(const std::string &str);

    std::string toString();
};

template<unsigned len>
void Char<len>::loadString(const std::string &str) {
    if (str.size() >= len) {
        memcpy(data, str.c_str(), len);
    } else {
        memset(data, ' ', len);
        memcpy(data, str.c_str(), str.size());
    }
}

template<unsigned len>
std::string Char<len>::toString() {
    return std::string(data, data + len);
}


#endif //MODERNDATABASE_TYPES_H

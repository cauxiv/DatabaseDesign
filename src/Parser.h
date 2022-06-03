//
// Created by woodi on 2022/06/01.
//

#ifndef MODERNDATABASE_PARSER_H
#define MODERNDATABASE_PARSER_H

#include <string>
#include <fstream>
#include <sstream>

#include "Schema.h"

enum class State : unsigned {
    Init,
    Create,
    Table,
    CreateTableBegin,
    CreateTableEnd,
    TableName,
    Primary,
    Key,
    KeyListBegin,
    KeyName,
    KeyListEnd,
    AttributeName,
    AttributeTypeInt,
    AttributeTypeChar,
    CharBegin,
    CharValue,
    CharEnd,
    AttributeTypeNumeric,
    NumericBegin,
    NumericValue1,
    NumericSeparator,
    NumericValue2,
    NumericEnd,
    Not,
    Null,
    Separator,
    Semicolon
};

class ParseError : std::exception {
    std::string msg;
public:
    ParseError(unsigned line, const std::string &m) : msg(m) {}

    ~ParseError() throw() {}

    const char *log() const throw() {
        return msg.c_str();
    }
};

struct Parser {
    std::string fileName;
    std::ifstream ifs;

    State state;

    Parser(const std::string &fileName) : fileName(fileName), state(State::Init) {}

    ~Parser() {};

    Schema parse();

private:
    void nextToken(unsigned line, const std::string &token, Schema &s);

};

#endif //MODERNDATABASE_PARSER_H

//
// Created by woodi on 2022/06/01.
//

#include "Parser.h"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstdlib>

#include "Parser.h"

namespace keyword {
    const std::string Primary = "primary";
    const std::string Key = "key";
    const std::string Create = "create";
    const std::string Table = "table";
    const std::string Integer = "integer";
    const std::string Numeric = "numeric";
    const std::string Not = "not";
    const std::string Null = "null";
    const std::string Char = "char";
}


namespace literal {
    const char LeftBracket = '(';
    const char RightBracket = ')';
    const char Comma = ',';
    const char Semicolon = ';';
}

Schema Parser::parse() {
    std::string token;
    unsigned line = 1;

    Schema s;

    ifs.open(fileName.c_str());
    // throw error(open)
    if (!ifs.is_open()) throw ParseError(line, "cannot open file" + fileName);
    while (ifs >> token) {
        std::string::size_type pos;
        std::string::size_type prevPos = 0;

        // if , or ) or ( or found in token, find the location of it
        while ((pos = token.find_first_of(",)(;", prevPos)) != std::string::npos) {
            // slice token from the first position of ,)(; & insert it to the token
            nextToken(line, token.substr(prevPos, pos - prevPos), s);
            // insert ,)(; as a token
            nextToken(line, token.substr(pos, 1), s);
            // move position to next
            // to parse like char(2),
            prevPos = pos + 1;
        }

        nextToken(line, token.substr(prevPos), s);
        if (token.find("\n") != std::string::npos) ++line;
    }
    // close filestream
    ifs.close();
    // return Schema
    return std::move(s);
}


static bool isIdentifier(const std::string &str) {
    if (
            str == keyword::Primary ||
            str == keyword::Key ||
            str == keyword::Table ||
            str == keyword::Create ||
            str == keyword::Integer ||
            str == keyword::Numeric ||
            str == keyword::Not ||
            str == keyword::Null ||
            str == keyword::Char
            )
        return false;
    return str.find_first_not_of("abcdefghijklmnopqrstuvwxyz_1234567890") == std::string::npos;
}

static bool isInt(const std::string &str) {
    return str.find_first_not_of("0123456789") == std::string::npos;
}

void Parser::nextToken(unsigned line, const std::string &token, Schema &schema) {
    if (getenv("DEBUG"))
        std::cerr << line << ": " << token << std::endl;
    if (token.empty())
        return;
    std::string tok;
    std::transform(token.begin(), token.end(), std::back_inserter(tok), tolower);
    switch (state) {
        case State::Semicolon: /* fallthrough */
        case State::Init:
            if (tok == keyword::Create)
                state = State::Create;
            else
                throw ParseError(line, "Expected 'CREATE', found '" + token + "'");
            break;
        case State::Create:
            if (tok == keyword::Table)
                state = State::Table;
            else
                throw ParseError(line, "Expected 'TABLE', found '" + token + "'");
            break;
        case State::Table:
            if (isIdentifier(tok)) {
                state = State::TableName;
                schema.relations.push_back(Schema::Relation(token));
            } else {
                throw ParseError(line, "Expected TableName, found '" + token + "'");
            }
            break;
        case State::TableName:
            if (tok.size() == 1 && tok[0] == literal::LeftBracket)
                state = State::CreateTableBegin;
            else
                throw ParseError(line, "Expected '(', found '" + token + "'");
            break;
        case State::Separator: /* fallthrough */
        case State::CreateTableBegin:
            if (tok.size() == 1 && tok[0] == literal::RightBracket) {
                state = State::CreateTableEnd;
            } else if (tok == keyword::Primary) {
                state = State::Primary;
            } else if (isIdentifier(tok)) {
                schema.relations.back().attributes.push_back(Schema::Relation::Attribute());
                schema.relations.back().attributes.back().name = token;
                state = State::AttributeName;
            } else {
                throw ParseError(line,
                                 "Expected attribute definition, primary key definition or ')', found '" + token + "'");
            }
            break;
        case State::CreateTableEnd:
            if (tok.size() == 1 && tok[0] == literal::Semicolon)
                state = State::Semicolon;
            else
                throw ParseError(line, "Expected ';', found '" + token + "'");
            break;
        case State::Primary:
            if (tok == keyword::Key)
                state = State::Key;
            else
                throw ParseError(line, "Expected 'KEY', found '" + token + "'");
            break;
        case State::Key:
            if (tok.size() == 1 && tok[0] == literal::LeftBracket)
                state = State::KeyListBegin;
            else
                throw ParseError(line, "Expected list of key attributes, found '" + token + "'");
            break;
        case State::KeyListBegin:
            if (isIdentifier(tok)) {
                struct AttributeNamePredicate {
                    const std::string &name;

                    AttributeNamePredicate(const std::string &name) : name(name) {}

                    bool operator()(const Schema::Relation::Attribute &attr) const {
                        return attr.name == name;
                    }
                };
                const auto &attributes = schema.relations.back().attributes;
                AttributeNamePredicate p(token);
                auto it = std::find_if(attributes.begin(), attributes.end(), p);
                if (it == attributes.end())
                    throw ParseError(line,
                                     "'" + token + "' is not an attribute of '" + schema.relations.back().name + "'");
                schema.relations.back().primaryKey.push_back(std::distance(attributes.begin(), it));
                state = State::KeyName;
            } else {
                throw ParseError(line, "Expected key attribute, found '" + token + "'");
            }
            break;
        case State::KeyName:
            if (tok.size() == 1 && tok[0] == literal::Comma)
                state = State::KeyListBegin;
            else if (tok.size() == 1 && tok[0] == literal::RightBracket)
                state = State::KeyListEnd;
            else
                throw ParseError(line, "Expected ',' or ')', found '" + token + "'");
            break;
        case State::KeyListEnd:
            if (tok.size() == 1 && tok[0] == literal::Comma)
                state = State::Separator;
            else if (tok.size() == 1 && tok[0] == literal::RightBracket)
                state = State::CreateTableEnd;
            else
                throw ParseError(line, "Expected ',' or ')', found '" + token + "'");
            break;
        case State::AttributeName:
            if (tok == keyword::Integer) {
                schema.relations.back().attributes.back().type = Types::Tag::Integer;
                state = State::AttributeTypeInt;
            } else if (tok == keyword::Char) {
                schema.relations.back().attributes.back().type = Types::Tag::Char;
                state = State::AttributeTypeChar;
            } else if (tok == keyword::Numeric) {
                //schema.relations.back().attributes.back().type=Types::Tag::Numeric;
                state = State::AttributeTypeNumeric;
            } else throw ParseError(line, "Expected type after attribute name, found '" + token + "'");
            break;
        case State::AttributeTypeChar:
            if (tok.size() == 1 && tok[0] == literal::LeftBracket)
                state = State::CharBegin;
            else
                throw ParseError(line, "Expected '(' after 'CHAR', found'" + token + "'");
            break;
        case State::CharBegin:
            if (isInt(tok)) {
                schema.relations.back().attributes.back().len = std::atoi(tok.c_str());
                state = State::CharValue;
            } else {
                throw ParseError(line, "Expected integer after 'CHAR(', found'" + token + "'");
            }
            break;
        case State::CharValue:
            if (tok.size() == 1 && tok[0] == literal::RightBracket)
                state = State::CharEnd;
            else
                throw ParseError(line, "Expected ')' after length of CHAR, found'" + token + "'");
            break;
        case State::AttributeTypeNumeric:
            if (tok.size() == 1 && tok[0] == literal::LeftBracket)
                state = State::NumericBegin;
            else
                throw ParseError(line, "Expected '(' after 'NUMERIC', found'" + token + "'");
            break;
        case State::NumericBegin:
            if (isInt(tok)) {
                //schema.relations.back().attributes.back().len1=std::atoi(tok.c_str());
                state = State::NumericValue1;
            } else {
                throw ParseError(line, "Expected integer after 'NUMERIC(', found'" + token + "'");
            }
            break;
        case State::NumericValue1:
            if (tok.size() == 1 && tok[0] == literal::Comma)
                state = State::NumericSeparator;
            else
                throw ParseError(line, "Expected ',' after first length of NUMERIC, found'" + token + "'");
            break;
        case State::NumericValue2:
            if (tok.size() == 1 && tok[0] == literal::RightBracket)
                state = State::NumericEnd;
            else
                throw ParseError(line, "Expected ')' after second length of NUMERIC, found'" + token + "'");
            break;
        case State::NumericSeparator:
            if (isInt(tok)) {
                //schema.relations.back().attributes.back().len2=std::atoi(tok.c_str());
                state = State::NumericValue2;
            } else {
                throw ParseError(line, "Expected second length for NUMERIC type, found'" + token + "'");
            }
            break;
        case State::CharEnd: /* fallthrough */
        case State::NumericEnd: /* fallthrough */
        case State::AttributeTypeInt:
            if (tok.size() == 1 && tok[0] == literal::Comma)
                state = State::Separator;
            else if (tok == keyword::Not)
                state = State::Not;
            else if (tok.size() == 1 && tok[0] == literal::RightBracket)
                state = State::CreateTableEnd;
            else throw ParseError(line, "Expected ',' or 'NOT NULL' after attribute type, found '" + token + "'");
            break;
        case State::Not:
            if (tok == keyword::Null) {
                schema.relations.back().attributes.back().notNull = true;
                state = State::Null;
            } else throw ParseError(line, "Expected 'NULL' after 'NOT' name, found '" + token + "'");
            break;
        case State::Null:
            if (tok.size() == 1 && tok[0] == literal::Comma)
                state = State::Separator;
            else if (tok.size() == 1 && tok[0] == literal::RightBracket)
                state = State::CreateTableEnd;
            else throw ParseError(line, "Expected ',' or ')' after attribute definition, found '" + token + "'");
            break;
        default:
            throw;
    }
}

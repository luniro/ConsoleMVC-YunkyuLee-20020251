#pragma once
#include "json/json_value.hpp"
#include "json/lexer.hpp"
#include <string>

class Parser {
public:
    explicit Parser(const std::string& src);
    JsonValue parse();

private:
    Lexer lexer_;
    int   depth_ = 0;

    static constexpr int kMaxDepth = 512;

    JsonValue parse_value();
    JsonValue parse_object();
    JsonValue parse_array();

    Token             expect(TokenType type, const char* what);
    JsonParseError    make_error(const Token& tok, const std::string& msg) const;
};

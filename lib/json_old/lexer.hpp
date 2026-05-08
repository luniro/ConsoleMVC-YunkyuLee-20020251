#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

// ─── Error ───────────────────────────────────────────────────────────────────

class JsonParseError : public std::runtime_error {
public:
    JsonParseError(const std::string& msg, int line, int col)
        : std::runtime_error(
              msg + " (line " + std::to_string(line) +
              ", col " + std::to_string(col) + ")")
        , line_(line), col_(col) {}
    int line() const noexcept { return line_; }
    int col()  const noexcept { return col_; }
private:
    int line_, col_;
};

// ─── Token ───────────────────────────────────────────────────────────────────

enum class TokenType {
    LBrace, RBrace,           // { }
    LBracket, RBracket,       // [ ]
    Colon, Comma,             // : ,
    String,                   // "..."
    Integer,                  // 123 / -45
    Float,                    // 1.5 / -3e2
    True, False, Null,        // keywords
    Eof
};

struct Token {
    TokenType   type;
    std::string str_val;    // valid when type == String
    int64_t     int_val;    // valid when type == Integer
    double      float_val;  // valid when type == Float
    int         line;
    int         col;
};

// ─── Lexer ───────────────────────────────────────────────────────────────────

class Lexer {
public:
    explicit Lexer(const std::string& src);

    Token next();
    Token peek();

private:
    const std::string& src_;
    size_t pos_  = 0;
    int    line_ = 1;
    int    col_  = 1;

    Token peeked_;
    bool  has_peeked_ = false;

    char current() const;
    char advance();
    bool at_end()  const;
    void skip_whitespace();

    Token read_string();
    Token read_number();
    Token read_keyword();

    JsonParseError make_error(const std::string& msg) const;

    static std::string  utf32_to_utf8(uint32_t cp);
    static uint32_t     parse_hex4(const std::string& src, size_t pos);
};

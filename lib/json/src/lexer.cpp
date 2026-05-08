#include "json/lexer.hpp"
#include <cctype>
#include <charconv>
#include <cstring>

Lexer::Lexer(const std::string& src) : src_(src) {}

bool Lexer::at_end() const { return pos_ >= src_.size(); }

char Lexer::current() const { return at_end() ? '\0' : src_[pos_]; }

char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') { ++line_; col_ = 1; }
    else            { ++col_; }
    return c;
}

void Lexer::skip_whitespace() {
    while (!at_end()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            advance();
        else
            break;
    }
}

JsonParseError Lexer::make_error(const std::string& msg) const {
    return JsonParseError(msg, line_, col_);
}

// ─── Public interface ────────────────────────────────────────────────────────

Token Lexer::peek() {
    if (!has_peeked_) {
        peeked_     = next();
        has_peeked_ = true;
    }
    return peeked_;
}

Token Lexer::next() {
    if (has_peeked_) {
        has_peeked_ = false;
        return peeked_;
    }

    skip_whitespace();

    if (at_end()) return {TokenType::Eof, {}, 0, 0.0, line_, col_};

    int  tline = line_;
    int  tcol  = col_;
    char c     = current(); // peek without consuming

    switch (c) {
        case '{': advance(); return {TokenType::LBrace,   {}, 0, 0.0, tline, tcol};
        case '}': advance(); return {TokenType::RBrace,   {}, 0, 0.0, tline, tcol};
        case '[': advance(); return {TokenType::LBracket, {}, 0, 0.0, tline, tcol};
        case ']': advance(); return {TokenType::RBracket, {}, 0, 0.0, tline, tcol};
        case ':': advance(); return {TokenType::Colon,    {}, 0, 0.0, tline, tcol};
        case ',': advance(); return {TokenType::Comma,    {}, 0, 0.0, tline, tcol};
        case '"': return read_string();
        default:
            if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
                return read_number();
            if (std::isalpha(static_cast<unsigned char>(c)))
                return read_keyword();
            advance();
            throw make_error(std::string("unexpected character '") + c + "'");
    }
}

// ─── String ──────────────────────────────────────────────────────────────────

uint32_t Lexer::parse_hex4(const std::string& src, size_t pos) {
    if (pos + 4 > src.size()) return 0xFFFFFFFF;
    uint32_t val = 0;
    for (int i = 0; i < 4; ++i) {
        unsigned char ch = static_cast<unsigned char>(src[pos + i]);
        val <<= 4;
        if      (ch >= '0' && ch <= '9') val |= ch - '0';
        else if (ch >= 'a' && ch <= 'f') val |= ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') val |= ch - 'A' + 10;
        else return 0xFFFFFFFF;
    }
    return val;
}

std::string Lexer::utf32_to_utf8(uint32_t cp) {
    std::string out;
    if (cp <= 0x7F) {
        out += static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return out;
}

Token Lexer::read_string() {
    int tline = line_, tcol = col_;
    advance(); // consume opening '"'

    std::string result;
    while (true) {
        if (at_end()) throw make_error("unterminated string");
        char c = advance();
        if (c == '"') break;
        if (c == '\\') {
            if (at_end()) throw make_error("unterminated escape sequence");
            char esc = advance();
            switch (esc) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    uint32_t cp = parse_hex4(src_, pos_);
                    if (cp == 0xFFFFFFFF) throw make_error("invalid \\u escape");
                    pos_ += 4; col_ += 4;
                    // UTF-16 surrogate pair
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (pos_ + 1 < src_.size() &&
                            src_[pos_] == '\\' && src_[pos_ + 1] == 'u') {
                            pos_ += 2; col_ += 2;
                            uint32_t low = parse_hex4(src_, pos_);
                            if (low >= 0xDC00 && low <= 0xDFFF) {
                                pos_ += 4; col_ += 4;
                                cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                            } else {
                                throw make_error("invalid low surrogate");
                            }
                        } else {
                            throw make_error("lone high surrogate");
                        }
                    }
                    result += utf32_to_utf8(cp);
                    break;
                }
                default:
                    throw make_error(std::string("invalid escape '\\") + esc + "'");
            }
        } else {
            if (static_cast<unsigned char>(c) < 0x20)
                throw make_error("control character in string");
            result += c;
        }
    }
    return {TokenType::String, std::move(result), 0, 0.0, tline, tcol};
}

// ─── Number ──────────────────────────────────────────────────────────────────

Token Lexer::read_number() {
    int    tline    = line_;
    int    tcol     = col_;
    size_t start    = pos_;
    bool   is_float = false;

    if (current() == '-') advance();

    if (at_end() || !std::isdigit(static_cast<unsigned char>(current())))
        throw make_error("invalid number: expected digit");

    if (current() == '0') {
        advance();
        if (!at_end() && std::isdigit(static_cast<unsigned char>(current())))
            throw make_error("leading zero in number");
    } else {
        while (!at_end() && std::isdigit(static_cast<unsigned char>(current())))
            advance();
    }

    if (!at_end() && current() == '.') {
        is_float = true;
        advance();
        if (at_end() || !std::isdigit(static_cast<unsigned char>(current())))
            throw make_error("expected digit after decimal point");
        while (!at_end() && std::isdigit(static_cast<unsigned char>(current())))
            advance();
    }

    if (!at_end() && (current() == 'e' || current() == 'E')) {
        is_float = true;
        advance();
        if (!at_end() && (current() == '+' || current() == '-')) advance();
        if (at_end() || !std::isdigit(static_cast<unsigned char>(current())))
            throw make_error("expected digit in exponent");
        while (!at_end() && std::isdigit(static_cast<unsigned char>(current())))
            advance();
    }

    const char* begin = src_.data() + start;
    const char* end   = src_.data() + pos_;

    if (is_float) {
        double val = 0.0;
        auto [ptr, ec] = std::from_chars(begin, end, val);
        if (ec != std::errc{}) throw make_error("invalid floating-point number");
        return {TokenType::Float, {}, 0, val, tline, tcol};
    } else {
        int64_t val = 0;
        auto [ptr, ec] = std::from_chars(begin, end, val);
        if (ec != std::errc{}) throw make_error("integer out of range or invalid");
        return {TokenType::Integer, {}, val, 0.0, tline, tcol};
    }
}

// ─── Keyword ─────────────────────────────────────────────────────────────────

Token Lexer::read_keyword() {
    int    tline = line_;
    int    tcol  = col_;
    size_t start = pos_;
    while (!at_end() && std::isalpha(static_cast<unsigned char>(current())))
        advance();
    std::string kw(src_, start, pos_ - start);
    if (kw == "true")  return {TokenType::True,  {}, 0, 0.0, tline, tcol};
    if (kw == "false") return {TokenType::False, {}, 0, 0.0, tline, tcol};
    if (kw == "null")  return {TokenType::Null,  {}, 0, 0.0, tline, tcol};
    throw JsonParseError("unknown keyword '" + kw + "'", tline, tcol);
}

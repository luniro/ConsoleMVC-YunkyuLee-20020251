#include "json/parser.hpp"

Parser::Parser(const std::string& src) : lexer_(src) {}

JsonParseError Parser::make_error(const Token& tok, const std::string& msg) const {
    return JsonParseError(msg, tok.line, tok.col);
}

Token Parser::expect(TokenType type, const char* what) {
    Token tok = lexer_.next();
    if (tok.type != type)
        throw make_error(tok, std::string("expected ") + what);
    return tok;
}

// ─── Entry point ─────────────────────────────────────────────────────────────

JsonValue Parser::parse() {
    JsonValue root = parse_value();
    Token eof = lexer_.next();
    if (eof.type != TokenType::Eof)
        throw make_error(eof, "unexpected token after top-level value");
    return root;
}

// ─── Recursive descent ───────────────────────────────────────────────────────

JsonValue Parser::parse_value() {
    if (++depth_ > kMaxDepth)
        throw JsonParseError("maximum nesting depth exceeded", 0, 0);

    Token tok = lexer_.next();
    JsonValue result;

    switch (tok.type) {
        case TokenType::LBrace:   result = parse_object();     break;
        case TokenType::LBracket: result = parse_array();      break;
        case TokenType::String:   result = tok.str_val;        break;
        case TokenType::Integer:  result = tok.int_val;        break;
        case TokenType::Float:    result = tok.float_val;      break;
        case TokenType::True:     result = true;               break;
        case TokenType::False:    result = false;              break;
        case TokenType::Null:     result = nullptr;            break;
        default: throw make_error(tok, "unexpected token");
    }

    --depth_;
    return result;
}

JsonValue Parser::parse_object() {
    JsonValue obj = JsonValue::object();

    if (lexer_.peek().type == TokenType::RBrace) {
        lexer_.next();
        return obj;
    }

    while (true) {
        Token key = expect(TokenType::String, "string key");
        expect(TokenType::Colon, "':'");
        obj[key.str_val] = parse_value();

        Token sep = lexer_.next();
        if (sep.type == TokenType::RBrace) break;
        if (sep.type != TokenType::Comma)
            throw make_error(sep, "expected ',' or '}'");
    }
    return obj;
}

JsonValue Parser::parse_array() {
    JsonValue arr = JsonValue::array();

    if (lexer_.peek().type == TokenType::RBracket) {
        lexer_.next();
        return arr;
    }

    while (true) {
        arr.push_back(parse_value());

        Token sep = lexer_.next();
        if (sep.type == TokenType::RBracket) break;
        if (sep.type != TokenType::Comma)
            throw make_error(sep, "expected ',' or ']'");
    }
    return arr;
}

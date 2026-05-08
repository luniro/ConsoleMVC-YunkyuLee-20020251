#pragma once
#include "json/json_value.hpp"
#include "json/lexer.hpp"
#include "json/parser.hpp"
#include "json/serializer.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

class JsonIOError : public std::runtime_error {
public:
    explicit JsonIOError(const std::string& msg) : std::runtime_error(msg) {}
};

namespace Json {

inline JsonValue parse(const std::string& text) {
    Parser p(text);
    return p.parse();
}

inline JsonValue parse_file(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) throw JsonIOError("cannot open file for reading: " + path);
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return parse(ss.str());
}

inline std::string dump(const JsonValue& val, const SerializeOptions& opts = {}) {
    return Serializer::dump(val, opts);
}

inline void dump_file(const JsonValue& val, const std::string& path,
                      const SerializeOptions& opts = {}) {
    std::ofstream ofs(path);
    if (!ofs) throw JsonIOError("cannot open file for writing: " + path);
    ofs << Serializer::dump(val, opts);
    if (!ofs) throw JsonIOError("write error on file: " + path);
}

} // namespace Json

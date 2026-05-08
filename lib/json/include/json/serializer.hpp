#pragma once
#include "json/json_value.hpp"
#include <string>

struct SerializeOptions {
    bool pretty    = false;
    int  indent    = 4;     // spaces per level (pretty only)
    bool sort_keys = false; // sort object keys alphabetically
};

class Serializer {
public:
    static std::string dump(const JsonValue& val, const SerializeOptions& opts = {});

private:
    static void write(const JsonValue& val, std::string& out,
                      const SerializeOptions& opts, int depth);
    static void write_string(const std::string& s, std::string& out);
    static void write_newline_indent(std::string& out,
                                     const SerializeOptions& opts, int depth);
};

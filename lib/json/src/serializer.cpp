#include "json/serializer.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <stdexcept>

std::string Serializer::dump(const JsonValue& val, const SerializeOptions& opts) {
    std::string out;
    out.reserve(256);
    write(val, out, opts, 0);
    return out;
}

void Serializer::write_newline_indent(std::string& out,
                                       const SerializeOptions& opts, int depth) {
    if (!opts.pretty) return;
    out += '\n';
    out.append(static_cast<size_t>(depth * opts.indent), ' ');
}

void Serializer::write_string(const std::string& s, std::string& out) {
    out += '"';
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    out += '"';
}

void Serializer::write(const JsonValue& val, std::string& out,
                        const SerializeOptions& opts, int depth) {
    switch (val.type()) {

        case JsonValue::Type::Null:
            out += "null";
            break;

        case JsonValue::Type::Bool:
            out += val.as_bool() ? "true" : "false";
            break;

        case JsonValue::Type::Integer: {
            char buf[32];
            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), val.as_integer());
            out.append(buf, ptr);
            break;
        }

        case JsonValue::Type::Float: {
            double d = val.as_float();
            if (std::isnan(d) || std::isinf(d))
                throw std::runtime_error("cannot serialize NaN or Infinity as JSON");
            char buf[32];
            // shortest round-trip representation
            auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), d,
                                           std::chars_format::general);
            std::string_view sv(buf, static_cast<size_t>(ptr - buf));
            out.append(sv.data(), sv.size());
            // Append ".0" so the value is unambiguously a float in the output
            if (sv.find('.') == std::string_view::npos &&
                sv.find('e') == std::string_view::npos &&
                sv.find('E') == std::string_view::npos) {
                out += ".0";
            }
            break;
        }

        case JsonValue::Type::String:
            write_string(val.as_string(), out);
            break;

        case JsonValue::Type::Array: {
            const auto& arr = val.as_array();
            out += '[';
            for (size_t i = 0; i < arr.size(); ++i) {
                write_newline_indent(out, opts, depth + 1);
                write(arr[i], out, opts, depth + 1);
                if (i + 1 < arr.size()) out += ',';
            }
            if (!arr.empty()) write_newline_indent(out, opts, depth);
            out += ']';
            break;
        }

        case JsonValue::Type::Object: {
            const auto& obj = val.as_object();
            out += '{';

            // Build an index list; optionally sort by key
            std::vector<size_t> order(obj.size());
            std::iota(order.begin(), order.end(), 0);
            if (opts.sort_keys) {
                std::sort(order.begin(), order.end(),
                          [&](size_t a, size_t b) {
                              return obj[a].first < obj[b].first;
                          });
            }

            for (size_t n = 0; n < order.size(); ++n) {
                size_t i = order[n];
                write_newline_indent(out, opts, depth + 1);
                write_string(obj[i].first, out);
                out += opts.pretty ? ": " : ":";
                write(obj[i].second, out, opts, depth + 1);
                if (n + 1 < order.size()) out += ',';
            }

            if (!obj.empty()) write_newline_indent(out, opts, depth);
            out += '}';
            break;
        }
    }
}

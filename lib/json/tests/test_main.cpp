#include "json/json.hpp"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>

// ─── Minimal test harness ────────────────────────────────────────────────────

static int g_pass = 0;
static int g_fail = 0;

#define RUN(name)                                                     \
    do {                                                              \
        try {                                                         \
            test_##name();                                            \
            std::cout << "[PASS] " #name "\n";                       \
            ++g_pass;                                                 \
        } catch (const std::exception& e) {                          \
            std::cout << "[FAIL] " #name " : " << e.what() << "\n"; \
            ++g_fail;                                                 \
        }                                                             \
    } while (0)

#define ASSERT(cond)                                             \
    do {                                                         \
        if (!(cond))                                             \
            throw std::runtime_error("assertion failed: " #cond);\
    } while (0)

#define ASSERT_EQ(a, b)                                                     \
    do {                                                                    \
        if ((a) != (b))                                                     \
            throw std::runtime_error(                                       \
                std::string("ASSERT_EQ failed: " #a " != " #b));           \
    } while (0)

#define ASSERT_THROWS(expr, ExType)                             \
    do {                                                        \
        bool caught_ = false;                                   \
        try { (void)(expr); }                                   \
        catch (const ExType&) { caught_ = true; }              \
        if (!caught_)                                           \
            throw std::runtime_error(                           \
                "expected " #ExType " was not thrown");         \
    } while (0)

// ─── Tests ───────────────────────────────────────────────────────────────────

void test_null() {
    auto v = Json::parse("null");
    ASSERT(v.is_null());
}

void test_bool_true() {
    auto v = Json::parse("true");
    ASSERT(v.is_bool());
    ASSERT(v.as_bool() == true);
}

void test_bool_false() {
    auto v = Json::parse("false");
    ASSERT(v.is_bool());
    ASSERT(v.as_bool() == false);
}

void test_integer() {
    auto v = Json::parse("42");
    ASSERT(v.is_integer());
    ASSERT_EQ(v.as_integer(), 42);
}

void test_negative_integer() {
    auto v = Json::parse("-7");
    ASSERT(v.is_integer());
    ASSERT_EQ(v.as_integer(), -7);
}

void test_float() {
    auto v = Json::parse("3.14");
    ASSERT(v.is_float());
    ASSERT(v.as_float() > 3.13 && v.as_float() < 3.15);
}

void test_scientific_notation() {
    auto v = Json::parse("1e3");
    ASSERT(v.is_float());
    ASSERT_EQ(v.as_float(), 1000.0);
}

void test_string() {
    auto v = Json::parse(R"("hello")");
    ASSERT(v.is_string());
    ASSERT_EQ(v.as_string(), "hello");
}

void test_string_escapes() {
    auto v = Json::parse(R"("a\nb\tc\"d")");
    ASSERT(v.is_string());
    ASSERT_EQ(v.as_string(), "a\nb\tc\"d");
}

void test_string_unicode_escape() {
    auto v = Json::parse(R"("AB")"); // AB
    ASSERT(v.is_string());
    ASSERT_EQ(v.as_string(), "AB");
}

void test_empty_array() {
    auto v = Json::parse("[]");
    ASSERT(v.is_array());
    ASSERT_EQ(v.size(), 0u);
}

void test_array() {
    auto v = Json::parse(R"([1, "two", true, null])");
    ASSERT(v.is_array());
    ASSERT_EQ(v.size(), 4u);
    ASSERT_EQ(v[0].as_integer(), 1);
    ASSERT_EQ(v[1].as_string(), "two");
    ASSERT(v[2].as_bool() == true);
    ASSERT(v[3].is_null());
}

void test_empty_object() {
    auto v = Json::parse("{}");
    ASSERT(v.is_object());
    ASSERT_EQ(v.size(), 0u);
}

void test_object() {
    auto v = Json::parse(R"({"name":"Alice","age":30})");
    ASSERT(v.is_object());
    ASSERT_EQ(v["name"].as_string(), "Alice");
    ASSERT_EQ(v["age"].as_integer(), 30);
}

void test_nested() {
    auto v = Json::parse(R"({
        "users": [
            {"id": 1, "active": true},
            {"id": 2, "active": false}
        ]
    })");
    ASSERT(v["users"].is_array());
    ASSERT_EQ(v["users"][0]["id"].as_integer(), 1);
    ASSERT(v["users"][1]["active"].as_bool() == false);
}

void test_whitespace() {
    auto v = Json::parse("  {  \"k\"  :  42  }  ");
    ASSERT_EQ(v["k"].as_integer(), 42);
}

void test_parse_error_bad_token() {
    ASSERT_THROWS(Json::parse("@"), JsonParseError);
}

void test_parse_error_unterminated_string() {
    ASSERT_THROWS(Json::parse(R"("oops)"), JsonParseError);
}

void test_parse_error_trailing_garbage() {
    ASSERT_THROWS(Json::parse("42 garbage"), JsonParseError);
}

void test_parse_error_leading_zero() {
    ASSERT_THROWS(Json::parse("01"), JsonParseError);
}

void test_type_error_wrong_access() {
    auto v = Json::parse("42");
    ASSERT_THROWS(v.as_string(), JsonTypeError);
}

void test_dom_build_and_modify() {
    JsonValue root = JsonValue::object();
    root["version"] = 1;
    root["name"]    = std::string("test");
    root["tags"]    = JsonValue::array();
    root["tags"].push_back(std::string("a"));
    root["tags"].push_back(std::string("b"));

    ASSERT_EQ(root["version"].as_integer(), 1);
    ASSERT_EQ(root["name"].as_string(), "test");
    ASSERT_EQ(root["tags"].size(), 2u);
    ASSERT_EQ(root["tags"][0].as_string(), "a");

    root["version"] = 2; // modify existing key
    ASSERT_EQ(root["version"].as_integer(), 2);
}

void test_contains() {
    auto v = Json::parse(R"({"x":1})");
    ASSERT(v.contains("x"));
    ASSERT(!v.contains("y"));
}

void test_serialize_compact() {
    auto v = Json::parse(R"({"a":1,"b":[true,null]})");
    std::string out = Json::dump(v);
    // Re-parse and verify round-trip
    auto v2 = Json::parse(out);
    ASSERT_EQ(v2["a"].as_integer(), 1);
    ASSERT(v2["b"][0].as_bool() == true);
    ASSERT(v2["b"][1].is_null());
}

void test_serialize_pretty() {
    JsonValue v = JsonValue::object();
    v["x"] = 1;
    v["y"] = 2;
    std::string out = Json::dump(v, {.pretty = true, .indent = 2});
    ASSERT(out.find('\n') != std::string::npos);
    // Round-trip
    auto v2 = Json::parse(out);
    ASSERT_EQ(v2["x"].as_integer(), 1);
}

void test_serialize_sort_keys() {
    JsonValue v = JsonValue::object();
    v["z"] = 3;
    v["a"] = 1;
    v["m"] = 2;
    std::string out = Json::dump(v, {.sort_keys = true});
    // "a" must appear before "m" which must appear before "z"
    ASSERT(out.find("\"a\"") < out.find("\"m\""));
    ASSERT(out.find("\"m\"") < out.find("\"z\""));
}

void test_file_round_trip() {
    JsonValue orig = JsonValue::object();
    orig["greeting"] = std::string("hello");
    orig["count"]    = 99;

    const std::string path = "test_output.json";
    Json::dump_file(orig, path, {.pretty = true});
    auto loaded = Json::parse_file(path);

    ASSERT_EQ(loaded["greeting"].as_string(), "hello");
    ASSERT_EQ(loaded["count"].as_integer(), 99);

    std::remove(path.c_str());
}

void test_float_round_trip() {
    double original = 1.0 / 3.0;
    JsonValue v = original;
    std::string s = Json::dump(v);
    auto v2 = Json::parse(s);
    ASSERT(v2.is_float());
    // Round-trip must preserve the value exactly
    ASSERT(v2.as_float() == original);
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {
    RUN(null);
    RUN(bool_true);
    RUN(bool_false);
    RUN(integer);
    RUN(negative_integer);
    RUN(float);
    RUN(scientific_notation);
    RUN(string);
    RUN(string_escapes);
    RUN(string_unicode_escape);
    RUN(empty_array);
    RUN(array);
    RUN(empty_object);
    RUN(object);
    RUN(nested);
    RUN(whitespace);
    RUN(parse_error_bad_token);
    RUN(parse_error_unterminated_string);
    RUN(parse_error_trailing_garbage);
    RUN(parse_error_leading_zero);
    RUN(type_error_wrong_access);
    RUN(dom_build_and_modify);
    RUN(contains);
    RUN(serialize_compact);
    RUN(serialize_pretty);
    RUN(serialize_sort_keys);
    RUN(file_round_trip);
    RUN(float_round_trip);

    std::cout << "\n" << g_pass << " passed, " << g_fail << " failed.\n";
    return g_fail == 0 ? 0 : 1;
}

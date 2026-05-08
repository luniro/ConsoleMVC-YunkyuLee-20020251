#include "json/json_value.hpp"
#include <stdexcept>

// ─── Lifetime helpers ────────────────────────────────────────────────────────

void JsonValue::destroy() noexcept {
    switch (type_) {
        case Type::String:  u_.s.~basic_string(); break;
        case Type::Array:   u_.a.~vector();        break;
        case Type::Object:  u_.o.~vector();        break;
        default: break;
    }
}

void JsonValue::copy_from(const JsonValue& o) {
    type_ = o.type_;
    switch (type_) {
        case Type::Null:                            break;
        case Type::Bool:    u_.b = o.u_.b;         break;
        case Type::Integer: u_.i = o.u_.i;         break;
        case Type::Float:   u_.f = o.u_.f;         break;
        case Type::String:  new (&u_.s) std::string(o.u_.s); break;
        case Type::Array:   new (&u_.a) Array(o.u_.a);       break;
        case Type::Object:  new (&u_.o) Object(o.u_.o);      break;
    }
}

void JsonValue::move_from(JsonValue&& o) noexcept {
    type_ = o.type_;
    switch (type_) {
        case Type::Null:                                                  break;
        case Type::Bool:    u_.b = o.u_.b;                               break;
        case Type::Integer: u_.i = o.u_.i;                               break;
        case Type::Float:   u_.f = o.u_.f;                               break;
        case Type::String:  new (&u_.s) std::string(std::move(o.u_.s)); break;
        case Type::Array:   new (&u_.a) Array(std::move(o.u_.a));        break;
        case Type::Object:  new (&u_.o) Object(std::move(o.u_.o));       break;
    }
    o.destroy();
    o.type_ = Type::Null;
}

// ─── Constructors ────────────────────────────────────────────────────────────

JsonValue::JsonValue() noexcept             : type_(Type::Null)    {}
JsonValue::JsonValue(std::nullptr_t) noexcept: type_(Type::Null)   {}
JsonValue::JsonValue(bool v) noexcept       : type_(Type::Bool)    { u_.b = v; }
JsonValue::JsonValue(int v) noexcept        : type_(Type::Integer) { u_.i = v; }
JsonValue::JsonValue(int64_t v) noexcept    : type_(Type::Integer) { u_.i = v; }
JsonValue::JsonValue(double v) noexcept     : type_(Type::Float)   { u_.f = v; }

JsonValue::JsonValue(const char* v)         : type_(Type::String)  { new (&u_.s) std::string(v); }
JsonValue::JsonValue(const std::string& v)  : type_(Type::String)  { new (&u_.s) std::string(v); }
JsonValue::JsonValue(std::string&& v)       : type_(Type::String)  { new (&u_.s) std::string(std::move(v)); }
JsonValue::JsonValue(const Array& v)        : type_(Type::Array)   { new (&u_.a) Array(v); }
JsonValue::JsonValue(Array&& v)             : type_(Type::Array)   { new (&u_.a) Array(std::move(v)); }
JsonValue::JsonValue(const Object& v)       : type_(Type::Object)  { new (&u_.o) Object(v); }
JsonValue::JsonValue(Object&& v)            : type_(Type::Object)  { new (&u_.o) Object(std::move(v)); }

JsonValue::JsonValue(const JsonValue& o)    { copy_from(o); }
JsonValue::JsonValue(JsonValue&& o) noexcept{ move_from(std::move(o)); }

JsonValue& JsonValue::operator=(const JsonValue& o) {
    if (this != &o) { destroy(); copy_from(o); }
    return *this;
}
JsonValue& JsonValue::operator=(JsonValue&& o) noexcept {
    if (this != &o) { destroy(); move_from(std::move(o)); }
    return *this;
}

JsonValue::~JsonValue() { destroy(); }

// ─── Static factories ────────────────────────────────────────────────────────

JsonValue JsonValue::array()  { return JsonValue(Array{}); }
JsonValue JsonValue::object() { return JsonValue(Object{}); }

// ─── Value access ────────────────────────────────────────────────────────────

bool JsonValue::as_bool() const {
    if (type_ != Type::Bool) throw JsonTypeError("value is not a bool");
    return u_.b;
}

int64_t JsonValue::as_integer() const {
    if (type_ == Type::Integer) return u_.i;
    if (type_ == Type::Float)   return static_cast<int64_t>(u_.f);
    throw JsonTypeError("value is not a number");
}

double JsonValue::as_float() const {
    if (type_ == Type::Float)   return u_.f;
    if (type_ == Type::Integer) return static_cast<double>(u_.i);
    throw JsonTypeError("value is not a number");
}

const std::string& JsonValue::as_string() const {
    if (type_ != Type::String) throw JsonTypeError("value is not a string");
    return u_.s;
}
std::string& JsonValue::as_string() {
    if (type_ != Type::String) throw JsonTypeError("value is not a string");
    return u_.s;
}

const JsonValue::Array& JsonValue::as_array() const {
    if (type_ != Type::Array) throw JsonTypeError("value is not an array");
    return u_.a;
}
JsonValue::Array& JsonValue::as_array() {
    if (type_ != Type::Array) throw JsonTypeError("value is not an array");
    return u_.a;
}

const JsonValue::Object& JsonValue::as_object() const {
    if (type_ != Type::Object) throw JsonTypeError("value is not an object");
    return u_.o;
}
JsonValue::Object& JsonValue::as_object() {
    if (type_ != Type::Object) throw JsonTypeError("value is not an object");
    return u_.o;
}

// ─── Array helpers ───────────────────────────────────────────────────────────

void JsonValue::push_back(const JsonValue& val) {
    if (type_ != Type::Array) throw JsonTypeError("value is not an array");
    u_.a.push_back(val);
}
void JsonValue::push_back(JsonValue&& val) {
    if (type_ != Type::Array) throw JsonTypeError("value is not an array");
    u_.a.push_back(std::move(val));
}

size_t JsonValue::size() const {
    if (type_ == Type::Array)  return u_.a.size();
    if (type_ == Type::Object) return u_.o.size();
    throw JsonTypeError("value is not an array or object");
}

// ─── Subscript ───────────────────────────────────────────────────────────────

JsonValue& JsonValue::operator[](const std::string& key) {
    if (type_ != Type::Object) throw JsonTypeError("value is not an object");
    for (auto& [k, v] : u_.o)
        if (k == key) return v;
    u_.o.emplace_back(key, JsonValue{});
    return u_.o.back().second;
}

const JsonValue& JsonValue::operator[](const std::string& key) const {
    if (type_ != Type::Object) throw JsonTypeError("value is not an object");
    for (const auto& [k, v] : u_.o)
        if (k == key) return v;
    throw std::out_of_range("key not found: " + key);
}

JsonValue& JsonValue::operator[](size_t idx) {
    if (type_ != Type::Array) throw JsonTypeError("value is not an array");
    return u_.a.at(idx);
}
const JsonValue& JsonValue::operator[](size_t idx) const {
    if (type_ != Type::Array) throw JsonTypeError("value is not an array");
    return u_.a.at(idx);
}

bool JsonValue::contains(const std::string& key) const {
    if (type_ != Type::Object) return false;
    for (const auto& [k, v] : u_.o)
        if (k == key) return true;
    return false;
}

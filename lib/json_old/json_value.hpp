#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

// ─── Error types ────────────────────────────────────────────────────────────

class JsonTypeError : public std::runtime_error {
public:
    explicit JsonTypeError(const std::string& msg) : std::runtime_error(msg) {}
};

// ─── JsonValue ───────────────────────────────────────────────────────────────

class JsonValue {
public:
    enum class Type { Null, Bool, Integer, Float, String, Array, Object };

    using Array  = std::vector<JsonValue>;
    using Object = std::vector<std::pair<std::string, JsonValue>>;

    // ── Constructors ────────────────────────────────────────────────────────
    JsonValue() noexcept;
    JsonValue(std::nullptr_t) noexcept;
    JsonValue(bool v) noexcept;
    JsonValue(int v) noexcept;
    JsonValue(int64_t v) noexcept;
    JsonValue(double v) noexcept;
    JsonValue(const char* v);
    JsonValue(const std::string& v);
    JsonValue(std::string&& v);
    JsonValue(const Array& v);
    JsonValue(Array&& v);
    JsonValue(const Object& v);
    JsonValue(Object&& v);

    JsonValue(const JsonValue& o);
    JsonValue(JsonValue&& o) noexcept;
    JsonValue& operator=(const JsonValue& o);
    JsonValue& operator=(JsonValue&& o) noexcept;
    ~JsonValue();

    // ── Static factories ────────────────────────────────────────────────────
    static JsonValue array();
    static JsonValue object();

    // ── Type queries ────────────────────────────────────────────────────────
    Type type()       const noexcept { return type_; }
    bool is_null()    const noexcept { return type_ == Type::Null; }
    bool is_bool()    const noexcept { return type_ == Type::Bool; }
    bool is_integer() const noexcept { return type_ == Type::Integer; }
    bool is_float()   const noexcept { return type_ == Type::Float; }
    bool is_number()  const noexcept { return type_ == Type::Integer || type_ == Type::Float; }
    bool is_string()  const noexcept { return type_ == Type::String; }
    bool is_array()   const noexcept { return type_ == Type::Array; }
    bool is_object()  const noexcept { return type_ == Type::Object; }

    // ── Value access ────────────────────────────────────────────────────────
    bool               as_bool()    const;
    int64_t            as_integer() const;
    double             as_float()   const;
    const std::string& as_string()  const;
    std::string&       as_string();
    const Array&       as_array()   const;
    Array&             as_array();
    const Object&      as_object()  const;
    Object&            as_object();

    // ── Array helpers ───────────────────────────────────────────────────────
    void   push_back(const JsonValue& val);
    void   push_back(JsonValue&& val);
    size_t size() const;

    // ── Subscript ───────────────────────────────────────────────────────────
    // object: insert-or-access by key
    JsonValue&       operator[](const std::string& key);
    const JsonValue& operator[](const std::string& key) const;
    // array: access by index (throws std::out_of_range)
    JsonValue&       operator[](size_t idx);
    const JsonValue& operator[](size_t idx) const;

    bool contains(const std::string& key) const;

private:
    // Manual tagged union — avoids the recursive-type problem with std::variant.
    // Non-trivial members (string, array, object) are managed via placement new
    // and explicit destructor calls in destroy() / copy_from() / move_from().
    union Storage {
        bool        b;
        int64_t     i;
        double      f;
        std::string s;
        Array       a;
        Object      o;
        Storage() {}
        ~Storage() {}
    } u_;
    Type type_;

    void destroy() noexcept;
    void copy_from(const JsonValue& o);
    void move_from(JsonValue&& o) noexcept;
};

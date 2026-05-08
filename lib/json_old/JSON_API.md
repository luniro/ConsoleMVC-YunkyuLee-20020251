# JSON Library API Reference

서브 헤더 분석 완료 기준 정리.

---

## 포함 방법

```cpp
#include "json/json.hpp"
```

CMakeLists.txt 기준 include 경로: `${CMAKE_SOURCE_DIR}/lib`  
(서브 헤더가 `lib/json/*.hpp`에 위치하므로 `lib/`를 루트로 지정)

---

## 예외 타입

| 클래스 | 헤더 | 상속 | 발생 시점 |
|--------|------|------|----------|
| `JsonIOError` | `json/json.hpp` | `std::runtime_error` | 파일 열기/쓰기 실패 |
| `JsonParseError` | `json/lexer.hpp` | `std::runtime_error` | JSON 파싱 실패 |
| `JsonTypeError` | `json/json_value.hpp` | `std::runtime_error` | 잘못된 타입으로 값 접근 |

```cpp
// JsonParseError는 위치 정보 제공
catch (const JsonParseError& e) {
    e.line();  // int
    e.col();   // int
}
```

---

## JsonValue

### 타입 열거형

```cpp
enum class JsonValue::Type {
    Null, Bool, Integer, Float, String, Array, Object
};
```

### 내부 타입 별칭

```cpp
using JsonValue::Array  = std::vector<JsonValue>;
using JsonValue::Object = std::vector<std::pair<std::string, JsonValue>>;
```

### 생성자

```cpp
JsonValue()                          // null
JsonValue(std::nullptr_t)            // null
JsonValue(bool v)
JsonValue(int v)
JsonValue(int64_t v)
JsonValue(double v)
JsonValue(const char* v)
JsonValue(const std::string& v)
JsonValue(std::string&& v)
JsonValue(const Array& v)
JsonValue(Array&& v)
JsonValue(const Object& v)
JsonValue(Object&& v)
```

### 정적 팩토리 (빈 컨테이너 생성)

```cpp
JsonValue::array()    // 빈 Array
JsonValue::object()   // 빈 Object
```

### 타입 확인

```cpp
val.type()        // JsonValue::Type 반환
val.is_null()
val.is_bool()
val.is_integer()  // 정수 전용
val.is_float()    // 실수 전용
val.is_number()   // integer || float
val.is_string()
val.is_array()
val.is_object()
```

### 값 추출 (타입 불일치 시 JsonTypeError)

```cpp
val.as_bool()                // bool
val.as_integer()             // int64_t  ← as_int() 아님
val.as_float()               // double
val.as_string()              // const std::string& / std::string&
val.as_array()               // const Array& / Array&
val.as_object()              // const Object& / Object&
```

### 서브스크립트

```cpp
// 오브젝트: 키로 접근 (없으면 null로 삽입)
val["key"]                   // operator[](const std::string&)

// 배열: 인덱스로 접근 (범위 초과 시 std::out_of_range)
val[size_t(0)]               // operator[](size_t)
```

### 배열 헬퍼

```cpp
val.push_back(const JsonValue&)
val.push_back(JsonValue&&)
val.size()                   // size_t
```

### 기타

```cpp
val.contains("key")          // bool, 오브젝트 키 존재 여부
```

---

## SerializeOptions

```cpp
struct SerializeOptions {
    bool pretty    = false;   // 들여쓰기 출력
    int  indent    = 4;       // pretty 시 스페이스 수
    bool sort_keys = false;   // 오브젝트 키 알파벳 정렬
};
```

---

## 공개 함수 (namespace Json)

```cpp
// 파싱 — 실패 시 JsonParseError
JsonValue   Json::parse(const std::string& text);
JsonValue   Json::parse_file(const std::string& path);  // 파일 없으면 JsonIOError

// 직렬화
std::string Json::dump(const JsonValue& val, const SerializeOptions& opts = {});
void        Json::dump_file(const JsonValue& val, const std::string& path,
                             const SerializeOptions& opts = {});  // 실패 시 JsonIOError
```

---

## 사용 패턴 예시

```cpp
#include "json/json.hpp"

// 오브젝트 구성
JsonValue item = JsonValue::object();
item["id"]   = JsonValue(int64_t(1));
item["name"] = JsonValue(std::string("alice"));

// 배열에 추가
JsonValue arr = JsonValue::array();
arr.push_back(item);

// 파일 저장/로드
Json::dump_file(arr, "data.json", {.pretty=true});
JsonValue loaded = Json::parse_file("data.json");

// 값 읽기
int64_t id   = loaded[0]["id"].as_integer();
std::string name = loaded[0]["name"].as_string();

// 예외 처리
try {
    JsonValue v = Json::parse_file("missing.json");
} catch (const JsonIOError& e)    { /* 파일 I/O 오류 */ }
  catch (const JsonParseError& e) { /* 파싱 오류, e.line() / e.col() */ }
  catch (const JsonTypeError& e)  { /* 타입 접근 오류 */ }
```

# ConsoleMVC POC — 구현 계획

## 1. 목표

`src/mvc/` 하위에 Model / View / Controller 구조의 콘솔 애플리케이션 POC를 구현한다.  
해당 디렉터리는 **독립적인 `CMakeLists.txt`** 를 가지며, 폴더를 통째로 다른 프로젝트의 `src/` 아래로 복사한 뒤 `add_subdirectory` 한 줄만 추가하면 이식이 완료된다.

---

## 2. 제약 조건

| 항목 | 내용 |
|------|------|
| 외부 의존성 | 없음 (STL만 사용) |
| C++ 표준 | C++17 |
| 빌드 시스템 | CMake (mvc/CMakeLists.txt → static library `mvc`) |
| 이식 단위 | `src/mvc/` 폴더 전체 |
| 진입점 | `src/main.cpp` (이식 대상 아님, 프로젝트별로 별도 작성) |

---

## 3. 디렉터리 구조

```
src/
├── main.cpp                        # POC 진입점 (이식 대상 아님)
└── mvc/                            # ← 이식 단위
    ├── CMakeLists.txt              # static library 'mvc' 정의
    ├── model/
    │   ├── IModel.hpp              # Model 인터페이스
    │   └── TaskModel.hpp/.cpp      # 구체 Model (할 일 목록)
    ├── view/
    │   ├── IView.hpp               # View 인터페이스
    │   └── ConsoleView.hpp/.cpp    # 구체 View (콘솔 출력)
    ├── controller/
    │   ├── IController.hpp         # Controller 인터페이스
    │   └── TaskController.hpp/.cpp # 구체 Controller (입력 처리 루프)
    └── App.hpp/.cpp                # 조립·실행 클래스
```

---

## 4. 컴포넌트 설계

### 4-1. IModel

```cpp
class IModel {
public:
    virtual ~IModel() = default;
    virtual void addItem(const std::string& title) = 0;
    virtual void removeItem(int id) = 0;
    virtual void toggleItem(int id) = 0;
    virtual const std::vector<Item>& items() const = 0;
};
```

- `Item` : `{ int id; std::string title; bool done; }`
- 상태 변경 시 등록된 `Observer`(View)에 통지 (Observer 패턴)

### 4-2. IView

```cpp
class IView {
public:
    virtual ~IView() = default;
    virtual void render(const std::vector<Item>& items) = 0;
    virtual void showMessage(const std::string& msg) = 0;
    virtual std::string prompt(const std::string& hint) = 0;
};
```

- 순수 콘솔 I/O 담당, 비즈니스 로직 없음
- `prompt()` 로 사용자 입력을 받아 문자열로 반환

### 4-3. IController

```cpp
class IController {
public:
    virtual ~IController() = default;
    virtual void run() = 0;      // 메인 루프 진입
    virtual void stop() = 0;     // 루프 종료
};
```

### 4-4. TaskController (구체 구현)

- `IModel&` + `IView&` 를 생성자에서 주입받음 (의존성 역전)
- `run()` : 입력 루프 → 명령 파싱 → Model 조작 → View 갱신

### 4-5. App

```cpp
class App {
public:
    void run();   // 객체 조립 후 controller.run() 호출
};
```

- 구체 클래스를 생성하고 조립하는 유일한 장소 (Composition Root)
- `main.cpp` 은 `App().run()` 만 호출

---

## 5. 의존성 흐름

```
main.cpp
  └─► App
        ├─► TaskModel   (implements IModel)
        ├─► ConsoleView (implements IView)
        └─► TaskController(IModel&, IView&)
                ├─► IModel   ← TaskModel
                └─► IView    ← ConsoleView
```

- Controller / View / Model 은 **인터페이스에만** 의존
- App 만 구체 타입을 알고 있음 → 교체 시 App 만 수정

---

## 6. POC 시나리오 (할 일 목록 관리)

```
=== ConsoleMVC Task Manager ===
[1] 목록 보기
[2] 항목 추가
[3] 완료 토글
[4] 항목 삭제
[q] 종료
> _
```

| 명령 | 동작 |
|------|------|
| `1` | 전체 할 일 출력 (id / 상태 / 제목) |
| `2` | 제목 입력 후 추가 |
| `3` | id 입력 후 완료/미완료 토글 |
| `4` | id 입력 후 삭제 |
| `q` | 프로그램 종료 |

---

## 7. CMake 구성

### src/mvc/CMakeLists.txt (이식 핵심)

```cmake
add_library(mvc STATIC
    model/TaskModel.cpp
    view/ConsoleView.cpp
    controller/TaskController.cpp
    App.cpp
)
target_include_directories(mvc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(mvc PUBLIC cxx_std_17)
```

- `PUBLIC` include로 외부에서 `#include "mvc/App.hpp"` 형태로 접근
- 별도 외부 의존성 없음

### 루트 CMakeLists.txt 변경

```cmake
add_subdirectory(src/mvc)          # mvc static lib 추가

add_executable(ConsoleMVC src/main.cpp)
target_link_libraries(ConsoleMVC PRIVATE mvc)
```

### 이식 시 대상 프로젝트 적용 방법

```cmake
# 대상 프로젝트 CMakeLists.txt
add_subdirectory(src/mvc)
target_link_libraries(YourTarget PRIVATE mvc)
```

---

## 8. 구현 순서

| 단계 | 작업 | 파일 |
|------|------|------|
| 1 | 데이터 타입 정의 | `model/IModel.hpp` (Item struct 포함) |
| 2 | Model 인터페이스 + 구현 | `model/TaskModel.hpp/.cpp` |
| 3 | View 인터페이스 + 구현 | `view/IView.hpp`, `view/ConsoleView.hpp/.cpp` |
| 4 | Controller 인터페이스 + 구현 | `controller/IController.hpp`, `controller/TaskController.hpp/.cpp` |
| 5 | App 조립 클래스 | `App.hpp/.cpp` |
| 6 | CMakeLists.txt 작성 | `src/mvc/CMakeLists.txt` |
| 7 | 루트 CMake 수정 + main.cpp 수정 | `CMakeLists.txt`, `src/main.cpp` |
| 8 | 빌드 및 동작 확인 | — |

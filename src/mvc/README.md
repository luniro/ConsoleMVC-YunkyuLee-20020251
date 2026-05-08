# mvc — 이식 단위 모듈

이 폴더(`src/mvc/`) 전체가 **이식 단위**입니다.  
폴더를 통째로 대상 프로젝트의 `src/` 아래로 복사한 뒤 CMakeLists.txt에 두 줄만 추가하면 이식이 완료됩니다.

---

## 이식 방법

```cmake
add_subdirectory(src/mvc)
target_link_libraries(YourTarget PRIVATE mvc)
```

이후 아래 형태로 include 가능합니다.

```cpp
#include "mvc/App.hpp"
```

---

## 디렉터리 구조

```
mvc/
├── CMakeLists.txt          # static library 'mvc' 정의 (이식 핵심)
├── App.hpp / App.cpp       # Composition Root — 구체 클래스 조립·실행
├── model/
│   ├── IModel.hpp          # Model 인터페이스 + Item 구조체
│   ├── TaskModel.hpp
│   └── TaskModel.cpp       # 할 일 목록 관리 구체 Model
├── view/
│   ├── IView.hpp           # View 인터페이스
│   ├── ConsoleView.hpp
│   └── ConsoleView.cpp     # 콘솔 I/O 구체 View
└── controller/
    ├── IController.hpp     # Controller 인터페이스
    ├── TaskController.hpp
    └── TaskController.cpp  # 입력 처리 루프 구체 Controller
```

---

## 의존성 흐름

```
App
 ├─► TaskModel   (implements IModel)
 ├─► ConsoleView (implements IView)
 └─► TaskController(IModel&, IView&)
          ├─► IModel   ← TaskModel
          └─► IView    ← ConsoleView
```

- Controller / View / Model은 **인터페이스에만** 의존합니다.
- `App`만 구체 타입을 알고 있으므로, 구현체를 교체할 때는 `App`만 수정하면 됩니다.

---

## 외부 의존성

없음 — STL만 사용합니다 (C++17).

---

## 진입점

`App::run()`을 호출하면 콘솔 할 일 관리 루프가 시작됩니다.  
`main.cpp`은 이식 대상이 아닙니다. 대상 프로젝트에서 별도로 작성하세요.

```cpp
// main.cpp 예시
#include "mvc/App.hpp"
int main() { mvc::App().run(); }
```

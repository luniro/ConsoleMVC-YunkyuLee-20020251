# Build Guide

## 툴체인

### 일반 빌드 (MSVC)
| 항목 | 버전 |
|------|------|
| 컴파일러 | MSVC 19.50.35730.0 (`cl.exe`) |
| IDE / 빌드 도구 | Visual Studio 2026 Community (msbuild 18.5.4) |
| CMake | VS 내장 CMake (`Visual Studio 18 2026` 제너레이터 사용) |
| Windows SDK | 10.0.26100.0 |
| 아키텍처 | x64 |
| C++ 표준 | C++17 |

### 테스트 빌드 (Clang)
| 항목 | 버전                                     |
|------|----------------------------------------|
| 컴파일러 | Clang 20 (VS 2026 LLVM `clang-cl.exe`) |
| 빌드 시스템 | CMake 4.2 + Ninja                      |
| 테스트 프레임워크 | gtest / gmock                          |
| 커버리지 도구 | llvm-cov / llvm-profdata               |
| C++ 표준 | C++17                                  |

---

## cmake 경로

시스템 PATH에 cmake가 등록되어 있지 않으므로 VS 내장 cmake를 사용한다.

```
C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
```

이하 빌드 명령에서 `cmake`는 위 전체 경로를 가리킨다.

---

## 디렉터리 구조

```
ConsoleMVC/
├── src/
│   ├── main.cpp
│   └── mvc/                        # 이식 가능한 MVC 모듈 (이 폴더가 이식 단위)
│       ├── CMakeLists.txt          # static library 'mvc' 정의
│       ├── App.hpp / App.cpp       # Composition Root
│       ├── model/
│       │   ├── IModel.hpp
│       │   ├── TaskModel.hpp
│       │   └── TaskModel.cpp
│       ├── view/
│       │   ├── IView.hpp
│       │   ├── ConsoleView.hpp
│       │   └── ConsoleView.cpp
│       └── controller/
│           ├── IController.hpp
│           ├── TaskController.hpp
│           └── TaskController.cpp
├── lib/
│   ├── json/             # 자체 JSON 라이브러리
│   └── googletest-main/  # Google Test (테스트 빌드 전용)
├── build/                # CMake 빌드 출력 (gitignore 권장)
├── docs/
│   └── PLAN.md
├── CMakeLists.txt
└── BUILD.md
```

### mvc 모듈 이식 방법

`src/mvc/` 폴더를 대상 프로젝트의 `src/` 아래로 복사 후 CMakeLists.txt에 두 줄 추가:

```cmake
add_subdirectory(src/mvc)
target_link_libraries(YourTarget PRIVATE mvc)
```

이후 `#include "mvc/App.hpp"` 형태로 접근 가능 (include 경로는 mvc 라이브러리가 PUBLIC으로 전달).

---

## CMake 옵션

| 옵션 | 기본값 | 설명 |
|------|--------|------|
| `ENABLE_TESTS` | `OFF` | 테스트 빌드 활성화 (Clang 커버리지 플래그 + gtest 포함) |
| `JSON_BUILD_TESTS` | `OFF` | json 라이브러리 내부 테스트 빌드 (강제 비활성화) |

---

## 빌드 방법

### 일반 빌드 (MSVC)

```bat
:: configure
cmake -B build -G "Visual Studio 18 2026" -A x64

:: build (Release)
cmake --build build --config Release

:: build (Debug)
cmake --build build --config Debug
```

출력물: `build\Release\ConsoleMVC.exe`

> **알려진 경고**: `lib/json` 소스 파일에서 C4819 경고(코드 페이지 문자 관련)가 출력되나 빌드 결과에는 영향 없음.

---

### 테스트 빌드 (Clang + Ninja)

Clang 20이 PATH에 있는 환경(VS 2026 LLVM 설치)에서 실행한다.

```bat
:: configure
cmake -B build-test -G Ninja ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DENABLE_TESTS=ON

:: build
cmake --build build-test
```

---

### 커버리지 측정 (테스트 빌드 후)

```bat
:: 프로파일 데이터 병합
llvm-profdata merge -sparse default.profraw -o coverage.profdata

:: 커버리지 리포트 출력
llvm-cov report ./build-test/ConsoleMVC.exe -instr-profile=coverage.profdata

:: HTML 리포트 생성
llvm-cov show ./build-test/ConsoleMVC.exe -instr-profile=coverage.profdata ^
    -format=html -output-dir=coverage-report
```

---

## 컴파일 옵션 요약

| 구분 | 플래그 |
|------|--------|
| MSVC 경고 | `/W4 /utf-8` |
| Clang 경고 | `-Wall -Wextra` |
| Clang 커버리지 | `-fprofile-instr-generate -fcoverage-mapping` |

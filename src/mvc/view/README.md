# view — View 레이어

콘솔 I/O를 담당하는 레이어입니다. 비즈니스 로직을 포함하지 않습니다.

---

## 파일 목록

| 파일 | 역할 |
|------|------|
| `IView.hpp` | View 인터페이스 정의 |
| `ConsoleView.hpp/.cpp` | `IView`의 구체 구현 (표준 입출력) |

---

## IView 인터페이스

```cpp
class IView {
public:
    virtual void render(const std::vector<Item>& items) = 0;
    virtual void showMessage(const std::string& msg) = 0;
    virtual std::string prompt(const std::string& hint) = 0;
};
```

| 메서드 | 설명 |
|--------|------|
| `render` | 할 일 목록 전체를 출력 |
| `showMessage` | 단일 메시지(오류·결과)를 출력 |
| `prompt` | 힌트 문자열을 표시하고 사용자 입력을 반환 |

---

## ConsoleView (구체 구현)

`std::cout` / `std::cin`을 사용하는 순수 콘솔 View입니다.

---

## 커스터마이징 포인트

GUI나 다른 출력 방식으로 이식할 때 `IView`를 상속한 새 View를 작성하세요.  
Controller는 `IView`에만 의존하므로 View를 교체해도 Controller 수정이 필요 없습니다.

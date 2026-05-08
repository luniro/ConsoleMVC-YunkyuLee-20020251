# model — Model 레이어

할 일 목록의 데이터와 비즈니스 로직을 담당하는 레이어입니다.

---

## 파일 목록

| 파일 | 역할 |
|------|------|
| `IModel.hpp` | Model 인터페이스 + `Item` 구조체 정의 |
| `TaskModel.hpp/.cpp` | `IModel`의 구체 구현 (메모리 기반 할 일 목록) |

---

## Item 구조체

```cpp
struct Item {
    int id;
    std::string title;
    bool done;
};
```

---

## IModel 인터페이스

```cpp
class IModel {
public:
    virtual void addItem(const std::string& title) = 0;
    virtual void removeItem(int id) = 0;
    virtual void toggleItem(int id) = 0;
    virtual const std::vector<Item>& items() const = 0;
};
```

---

## TaskModel (구체 구현)

- `addItem` : `nextId_`를 자동 증가하며 새 항목 추가
- `removeItem` : id로 항목 삭제
- `toggleItem` : id로 `done` 상태 반전
- `items()` : 전체 목록 반환 (읽기 전용)

구현체를 교체하려면 `IModel`을 상속한 새 클래스를 작성하고 `App`에서 교체하면 됩니다.

---

## 커스터마이징 포인트

다른 도메인으로 이식할 때 `TaskModel`을 대체하는 새 Model을 작성하세요.  
Controller와 View는 `IModel`에만 의존하므로 교체해도 수정이 필요 없습니다.

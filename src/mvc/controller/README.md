# controller — Controller 레이어

사용자 입력을 처리하고 Model과 View를 조율하는 레이어입니다.

---

## 파일 목록

| 파일 | 역할 |
|------|------|
| `IController.hpp` | Controller 인터페이스 정의 |
| `TaskController.hpp/.cpp` | `IController`의 구체 구현 (입력 파싱 루프) |

---

## IController 인터페이스

```cpp
class IController {
public:
    virtual void run() = 0;   // 메인 루프 진입
    virtual void stop() = 0;  // 루프 종료
};
```

---

## TaskController (구체 구현)

생성자에서 `IModel&`와 `IView&`를 주입받습니다 (의존성 역전).

```cpp
TaskController(IModel& model, IView& view);
```

### 처리 명령

| 입력 | 동작 | 내부 핸들러 |
|------|------|-------------|
| `1` | 목록 보기 (`view_.render`) | — |
| `2` | 항목 추가 | `handleAdd()` |
| `3` | 완료 토글 | `handleToggle()` |
| `4` | 항목 삭제 | `handleRemove()` |
| `q` | 루프 종료 (`stop()`) | — |

### 루프 흐름

```
run()
 └─► view_.prompt("메뉴") → 입력 수신
       ├─ "1" → view_.render(model_.items())
       ├─ "2" → handleAdd()   → model_.addItem(...)
       ├─ "3" → handleToggle() → model_.toggleItem(id)
       ├─ "4" → handleRemove() → model_.removeItem(id)
       └─ "q" → stop()
```

---

## 커스터마이징 포인트

다른 명령 체계로 이식할 때 `IController`를 상속한 새 Controller를 작성하세요.  
Model과 View는 인터페이스를 통해서만 접근하므로 구현체를 자유롭게 조합할 수 있습니다.

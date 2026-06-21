# micro_C LLVM IR 생성 구현 명세

이 문서는 파서와 의미분석 이후, AST를 LLVM IR로 낮추기 위한 상세 구현 명세다. 저장소가 학습 모드이므로 완성 코드를 제공하지 않고, 어떤 정보를 준비해야 하고 어떤 순서로 구현하면 되는지 중심으로 정리한다.

## 1. 전제 조건

LLVM IR 생성은 파서가 어느 정도 완성된 뒤에 시작하는 것이 좋다.

최소 전제:

- `Program`에 최상위 선언 목록이 들어 있다.
- `Function`에 이름, 반환 타입, 매개변수, 본문 문장 목록이 들어 있다.
- `Return` 문과 기본 표현식이 AST로 만들어진다.
- 표현식은 연산자 우선순위가 반영된 트리 구조다.
- 변수 선언과 참조를 구분할 수 있다.

가능하면 IR 생성 전에 의미분석도 별도로 둔다.

의미분석에서 준비하면 좋은 정보:

- 각 이름이 어떤 선언을 가리키는지.
- 각 표현식의 타입.
- 각 표현식이 lvalue인지 rvalue인지.
- 함수 호출의 대상 함수와 인자 타입.
- `break`/`continue`가 유효한 반복문 안에 있는지.

처음에는 의미분석을 완전히 분리하지 않아도 된다. 하지만 IR 생성 중에 타입 검사와 이름 확인까지 모두 섞으면 복잡도가 빠르게 올라간다.

## 2. IR 생성 방식 선택

두 방식 중 하나를 고른다.

### A. 텍스트 LLVM IR 출력

AST를 순회하며 `.ll` 텍스트를 직접 만든다.

장점:

- LLVM 라이브러리 없이 시작 가능.
- IR 문법을 직접 익히기 좋다.
- 작은 예제를 빠르게 확인하기 좋다.

단점:

- 임시값 이름을 직접 관리해야 한다.
- 블록 라벨을 직접 관리해야 한다.
- 타입 문자열을 정확히 맞춰야 한다.
- 잘못된 IR을 만들기 쉽다.

### B. LLVM C++ API 사용

`LLVMContext`, `Module`, `IRBuilder`, `Type`, `Value`, `Function`, `BasicBlock` 같은 객체를 사용한다.

장점:

- 타입과 값이 객체로 연결된다.
- verifier로 IR 검증이 쉽다.
- 블록과 명령 생성이 체계적이다.

단점:

- 빌드 설정이 먼저 필요하다.
- LLVM API 학습량이 있다.
- 현재 저장소에는 아직 빌드 시스템이 없다.

학습 추천:

- 처음에는 텍스트 IR로 `return 1` 정도를 손으로 생성해 본다.
- 그 다음 AST에서 텍스트 IR을 만들며 LLVM의 구조를 익힌다.
- 이후 프로젝트 규모가 커지면 LLVM C++ API로 옮길지 결정한다.

이 문서의 명세는 두 방식 모두에 적용된다. 텍스트 IR이면 "문자열 생성", LLVM API면 "객체 생성"으로 해석하면 된다.

## 3. IR 생성기의 전체 책임

IR 생성기는 AST를 LLVM IR 단위로 낮춘다.

큰 흐름:

```text
Program
  -> module
Declaration
  -> function or global variable
Statement
  -> instruction and control flow
Expression
  -> value or address
```

IR 생성기의 책임:

- micro_C 타입을 LLVM 타입으로 바꾼다.
- 함수 시그니처를 만든다.
- 전역변수를 만든다.
- 함수 본문 basic block을 만든다.
- 지역변수 저장 공간을 만든다.
- 표현식 값을 계산한다.
- 대입과 변수 읽기를 위해 주소/값을 구분한다.
- `if`, `for`, `break`, `continue`의 제어 흐름을 만든다.
- 문자열 리터럴과 출력 함수를 처리한다.
- 생성된 IR이 기본 규칙을 만족하는지 확인한다.

IR 생성기가 하지 않아야 할 일:

- 파싱.
- 문법 오류 복구.
- 언어 규칙 전체 검사.
- 최적화.

## 4. 추천 파일/클래스 설계

현재 `Node.h`의 각 노드에는 `generate()->void`가 있다. 하지만 LLVM IR에서는 표현식이 값을 만들어야 하는 경우가 많다. 따라서 다음 중 하나를 선택해야 한다.

선택지 A:

- 각 노드의 `generate()`를 유지하되, 생성 결과를 전역/멤버 상태에 저장한다.

선택지 B:

- 별도 `IRGenerator` 클래스를 만들고, 노드 종류별 방문 함수를 둔다.

선택지 C:

- `Expression::generate()`는 값 결과를 반환하고, `Statement::generate()`는 반환값 없이 명령만 만든다.

학습 추천:

- 처음에는 별도 `IRGenerator` 개념을 두고 생각하는 것이 좋다.
- 실제 코드는 나중에 정하더라도, "생성 상태"를 노드 안에 흩뿌리지 않는 편이 이해하기 쉽다.

IR 생성기가 들고 있으면 좋은 상태:

- 현재 module.
- 현재 function.
- 현재 basic block 또는 insertion point.
- 현재 함수의 지역 변수 테이블.
- 전역 변수/함수 테이블.
- 문자열 리터럴 테이블.
- break 대상 블록 스택.
- continue 대상 블록 스택.
- 임시값 번호 생성기.
- 라벨 번호 생성기.

텍스트 IR 방식이면 추가로 필요하다.

- 전역 선언 출력 버퍼.
- 함수 본문 출력 버퍼.
- 현재 임시값 이름.
- 현재 블록 라벨 이름.

LLVM API 방식이면 다음 객체들이 대응된다.

- `LLVMContext`
- `Module`
- `IRBuilder`
- `Function`
- `BasicBlock`
- `Value`
- `Type`

## 5. micro_C 타입과 LLVM 타입 매핑

초기 타입 매핑 후보:

```text
Void       -> void
Int        -> i32
Char       -> i8
Boolean    -> i1
Pointer T  -> ptr
Array N T  -> [N x llvm(T)]
Function   -> function type
String     -> [N x i8] global constant + ptr
Null       -> null pointer
```

주의:

- LLVM 최신 버전은 opaque pointer를 사용하므로 포인터는 보통 `ptr`로 표현된다.
- `Int`를 `i32`로 할지 `i64`로 할지는 언어 설계 선택이다. 처음에는 `i32`가 예제가 많아 편하다.
- `Char`는 `i8`이 자연스럽다.
- 비교 결과는 LLVM에서는 `i1`이다.
- 언어 차원에서 boolean 타입을 노출하지 않더라도 조건식 내부에서는 `i1`이 필요하다.

타입 변환에서 필요한 질문:

- `FloatLiteral`을 지원할 것인가? 지원한다면 `double` 또는 `float` 중 무엇인가?
- `Void` 변수 선언을 허용하지 않을 것인가?
- 배열을 값으로 전달할 수 있는가?
- 함수 타입을 일급 값으로 다룰 것인가?

초기 범위 추천:

- `Int`, `Char`, `Void`, `Pointer`, `Array`만 먼저 처리한다.
- `FloatLiteral`, `Struct`, 함수 포인터는 뒤로 미룬다.

## 6. 값과 주소의 구분

LLVM IR 생성에서 가장 중요한 구분은 value와 address다.

value:

- 계산 결과.
- 예: 정수 상수, 덧셈 결과, 비교 결과.

address:

- 값을 저장하거나 읽을 수 있는 메모리 위치.
- 예: 지역변수의 `alloca` 결과, 전역변수 주소, 배열 원소 주소, 역참조 결과.

예시 개념:

```text
x       -> 문맥에 따라 주소 또는 값
x + 1   -> 값
x = 1   -> 왼쪽 x는 주소, 오른쪽 1은 값
&x      -> x의 주소가 값
*p      -> 문맥에 따라 주소 또는 값
a[i]    -> 문맥에 따라 주소 또는 값
```

권장 설계:

- 표현식에서 "값을 생성하는 함수"와 "주소를 생성하는 함수"를 분리해서 생각한다.

개념적 인터페이스:

```text
emitValue(expr)   -> rvalue
emitAddress(expr) -> lvalue address
```

이것은 그대로 구현하라는 뜻이 아니라 사고 모델이다.

예시:

- `emitValue(GetVariable x)`는 `emitAddress(x)` 후 `load`.
- `emitAddress(GetVariable x)`는 변수 테이블에서 주소를 찾음.
- `emitValue(Assigment)`는 왼쪽 주소를 구하고 오른쪽 값을 구해 `store`.
- `emitAddress(Arithmetic)`는 오류. 산술 결과는 저장 위치가 아니다.

이 구분이 없으면 대입, 주소 연산, 포인터 역참조, 배열 인덱스에서 거의 반드시 꼬인다.

## 7. Module 생성

`Program`은 LLVM module 하나로 내려간다.

처리 순서 추천:

1. 전역 함수 선언 정보를 먼저 등록한다.
2. 전역변수 선언 정보를 등록한다.
3. 함수 본문을 생성한다.
4. 문자열 리터럴 전역 상수를 추가한다.
5. 필요한 외부 함수 선언을 추가한다.

왜 함수 선언을 먼저 등록하는가?

- 함수 본문 안에서 뒤에 정의된 함수를 호출할 수 있기 때문이다.

예시:

```text
Int main() { return f(); }
Int f() { return 1; }
```

이 경우 `main`을 생성할 때 `f`의 시그니처를 이미 알고 있어야 한다.

## 8. 함수 생성

`Function` 노드는 LLVM function으로 내려간다.

필요한 정보:

- 반환 타입.
- 함수 이름.
- 매개변수 타입 목록.
- 매개변수 이름 목록.
- 본문 블록.

함수 생성 단계:

1. LLVM 함수 타입을 만든다.
2. 함수 심볼을 module에 등록한다.
3. 본문이 있으면 entry block을 만든다.
4. 매개변수를 지역 변수 테이블에 연결한다.
5. 본문 문장을 순서대로 생성한다.
6. 마지막 블록에 terminator가 없으면 기본 반환을 어떻게 할지 결정한다.

매개변수 처리 방식:

- LLVM 함수 인자는 SSA 값이다.
- 지역변수처럼 대입 가능하게 만들려면 entry block에 저장 공간을 만들고 인자 값을 store한다.

초기 단순화:

- 매개변수 대입을 허용하지 않는다면 바로 값으로 써도 된다.
- C 비슷한 언어를 목표로 한다면 매개변수도 지역변수처럼 주소를 갖게 하는 편이 일관된다.

점검 질문:

- `Void` 함수가 명시적 `return;` 없이 끝나면 자동 `ret void`를 넣을 것인가?
- `Int` 함수가 반환 없이 끝나면 오류로 볼 것인가, 기본값을 반환할 것인가?
- 이 검사는 의미분석에서 할 것인가, IR 생성에서 할 것인가?

학습 추천:

- 처음에는 명시적인 `return`이 있다고 가정한다.
- 이후 `Void` 함수 자동 반환을 추가한다.

## 9. 전역변수 생성

`GrobalVariable` 노드는 LLVM global variable로 내려간다.

필요한 정보:

- 변수 이름.
- LLVM 타입.
- 초기값.
- linkage 정책.

초기값 규칙:

- 초기화식이 없으면 0 초기화.
- 정수 리터럴 초기화부터 시작.
- 복잡한 표현식 초기화는 나중에 지원.

LLVM 전역변수 주의점:

- 전역 초기값은 컴파일 타임 상수여야 한다.
- 지역변수 초기화처럼 임의 명령을 실행할 수 없다.

점검 질문:

- `Int x = 1;`은 허용.
- `Int x = f();`는 전역 초기화에서 허용할 것인가?
- 배열 전역 초기화는 언제 지원할 것인가?

초기 추천:

- 전역변수는 초기화 없음 또는 정수/문자/null 상수만 허용한다.

## 10. 지역변수 생성

지역변수 노드가 추가되면 LLVM에서는 보통 `alloca`로 처리한다.

흐름:

```text
local variable declaration
  -> entry block에 alloca
  -> local symbol table에 이름 -> 주소 저장
  -> 초기화식이 있으면 값을 계산해서 store
```

왜 entry block에 alloca를 모으는가?

- LLVM 최적화 패스가 `alloca`를 SSA 레지스터로 승격하기 쉽다.
- 사람이 읽기에도 지역변수 저장 공간이 한곳에 모인다.

처음에는 현재 insertion point에 `alloca`를 둬도 학습은 가능하다. 하지만 장기적으로는 entry block 배치를 이해하는 것이 좋다.

점검 질문:

- 같은 스코프에서 같은 이름을 다시 선언하면 어떻게 할 것인가?
- 내부 블록에서 바깥 이름을 shadowing할 수 있는가?
- 스코프가 끝나면 심볼 테이블에서 이름을 제거하는가?

## 11. 리터럴 생성

### 정수 리터럴

`IntegerLiteral`은 LLVM 정수 상수로 내려간다.

필요한 정보:

- 값.
- 타입. 처음에는 `Int` 기준 `i32`로 처리 가능.

주의:

- 리터럴 값이 `i32` 범위를 넘으면 어떻게 할지 결정해야 한다.

### 문자 리터럴

현재 `Node.h`에는 별도 `CharLiteral` AST 노드가 없다.

선택지:

- `IntegerLiteral`로 낮춘다.
- 별도 `CharLiteral` 노드를 추가한다.

LLVM에서는 보통 `i8` 상수로 표현할 수 있다.

### 문자열 리터럴

문자열은 보통 module-level global constant로 만든다.

개념:

```text
"hello"
  -> private unnamed_addr constant [6 x i8] c"hello\00"
  -> 표현식 값은 그 시작 주소
```

필요한 상태:

- 문자열 리터럴 번호.
- 같은 문자열을 재사용할지 여부.
- escape 문자 출력 규칙.

처음에는 같은 문자열을 매번 새 전역 상수로 만들어도 된다.

### null 리터럴

`null`은 포인터 타입 문맥에서 null pointer로 내려간다.

주의:

- `null` 자체만으로는 정확한 포인터 대상 타입을 모를 수 있다.
- opaque pointer를 사용하면 `ptr null` 형태로 단순화된다.

## 12. 변수 참조 생성

`GetVariable`은 이름을 통해 변수나 함수를 찾는다.

변수 값이 필요할 때:

```text
주소 찾기 -> load -> 값 반환
```

변수 주소가 필요할 때:

```text
주소 찾기 -> 주소 반환
```

찾는 순서 후보:

1. 현재 지역 스코프.
2. 바깥 지역 스코프.
3. 전역변수.
4. 함수 이름.

주의:

- 함수 이름은 호출 대상 값으로는 쓸 수 있지만, 일반 변수 주소와는 다르다.
- 이름을 못 찾으면 의미분석 단계에서 이미 오류가 났어야 한다.

## 13. 대입 생성

`Assigment` 생성 흐름:

```text
lhs 주소 생성
rhs 값 생성
필요하면 타입 변환
store rhs -> lhs 주소
대입 표현식의 결과값을 무엇으로 할지 결정
```

결과값 선택:

- C처럼 저장한 값을 결과로 반환.
- 문장으로만 허용하고 결과를 사용하지 않음.

초기 추천:

- 표현식 구조는 유지하되, 대입 결과가 필요한 상황은 나중에 처리한다.

점검 질문:

- `x = y = 1`을 허용한다면 결과값이 필요하다.
- `if (x = 1)`을 허용한다면 대입 결과값이 조건으로 쓰인다.
- 학습 초기에는 이런 케이스를 제한해도 된다.

## 14. 산술 연산 생성

`Arithmetic` 노드는 `+`, `-`, `*`, `/`, `%`를 처리한다.

정수 연산 기준:

```text
+ -> add
- -> sub
* -> mul
/ -> sdiv 또는 udiv
% -> srem 또는 urem
```

처음에는 `Int`를 signed로 보고 `sdiv`, `srem`을 사용하는 것이 단순하다.

필요한 검사:

- 양쪽 피연산자 타입이 호환되는가?
- 포인터 산술을 허용할 것인가?
- `Char`는 연산 전에 `Int`로 확장할 것인가?

초기 추천:

- `Int`끼리 산술만 먼저 지원한다.
- `Char`, 포인터 산술, 실수 산술은 뒤로 미룬다.

## 15. 비교 연산 생성

`Relational` 노드는 비교 결과를 만든다.

정수 비교 후보:

```text
== -> icmp eq
!= -> icmp ne
<  -> icmp slt
>  -> icmp sgt
<= -> icmp sle
>= -> icmp sge
```

결과 타입:

```text
i1
```

주의:

- signed/unsigned 기준을 정해야 한다.
- 포인터 비교는 어떤 연산만 허용할지 정해야 한다.
- 비교 결과를 `Int`로 저장하려면 `zext i1 to i32` 같은 변환이 필요할 수 있다.

초기 추천:

- 조건문 내부에서는 `i1` 그대로 사용한다.
- 일반 표현식 값으로 쓸 때는 필요할 때만 `Int` 변환을 고려한다.

## 16. 논리 연산 생성

`And`, `Or`, `Unary(!)`를 처리한다.

두 방식이 있다.

### 단순 계산 방식

양쪽 값을 모두 계산하고 `and`/`or`를 사용한다.

장점:

- 구현이 쉽다.

단점:

- C의 short-circuit 동작과 다르다.
- `a != 0 && 10 / a > 1` 같은 코드에서 문제가 생길 수 있다.

### Short-circuit 방식

기본 블록을 나누어 필요한 쪽만 계산한다.

장점:

- C와 비슷한 동작.

단점:

- 제어 흐름과 phi 개념이 필요하다.

초기 추천:

- 조건문 안에서만 short-circuit을 먼저 고민한다.
- 일반 값으로 쓰이는 `&&`, `||`는 나중에 확장한다.

조건식 변환:

```text
Int value -> value != 0
Pointer value -> value != null
i1 value -> 그대로
```

이 변환을 별도 개념으로 두면 `if`, `for`, `!`에서 재사용하기 좋다.

## 17. 단항 연산 생성

### 논리 not

`!x`:

```text
x를 조건 i1로 변환
not 또는 icmp로 반전
```

### 산술 minus

`-x`:

```text
0 - x
```

### 주소 연산

`&x`:

```text
emitAddress(x)
```

주의:

- `&(x + 1)`은 주소를 만들 수 없다.
- 의미분석에서 lvalue인지 확인해야 한다.

### 역참조

`*p`:

- 값 문맥: `p` 값을 계산하고 그 주소에서 `load`.
- 주소 문맥: `p` 값을 계산하고 그 자체를 주소로 사용.

주의:

- `p`가 포인터 타입인지 확인해야 한다.

## 18. 함수 호출 생성

`Call` 노드 생성 흐름:

```text
호출 대상 함수 찾기
인자 표현식 값 생성
필요하면 타입 변환
call 명령 생성
반환값이 있으면 값 반환
```

점검 질문:

- 호출 대상은 `GetVariable` 함수 이름만 허용할 것인가?
- 함수 포인터 호출을 지원할 것인가?
- 인자 개수와 타입 검사는 어디에서 할 것인가?

초기 추천:

- 함수 이름 호출만 지원한다.
- 함수 포인터는 나중에 미룬다.

## 19. 배열 인덱스 생성

`GetElement`는 배열 또는 포인터에서 원소 주소를 만든다.

개념:

```text
a[i]
  -> a의 base 주소
  -> i 값
  -> 원소 주소 계산
  -> 값 문맥이면 load
```

LLVM에서는 `getelementptr` 개념이 필요하다.

배열과 포인터의 차이:

- 배열 변수 `a`는 `[N x T]` 타입 저장공간의 주소일 수 있다.
- 포인터 변수 `p`는 먼저 `load`해서 `ptr` 값을 얻은 뒤 인덱싱할 수 있다.

점검 질문:

- `a[i]`에서 `a`가 배열인지 포인터인지 어떻게 구분할 것인가?
- 인덱스는 `Int` 타입이어야 하는가?
- 범위 검사를 할 것인가?

초기 추천:

- 컴파일 타임 범위 검사는 하지 않는다.
- 인덱스 타입만 확인한다.

## 20. 구조체 멤버 접근 생성

현재 토큰과 AST에는 `Struct`, `.`, `->`가 있다.

필요한 추가 정보:

- 구조체 타입 정의.
- 멤버 이름 목록.
- 멤버 순서.
- 멤버 타입.
- 멤버 이름 -> 인덱스 매핑.

LLVM에서는 구조체 멤버 접근도 `getelementptr` 계열로 생각할 수 있다.

초기 추천:

- 구조체는 파서와 기본 IR이 안정된 뒤로 미룬다.
- `GetMember`, `GetMemberArrow` 노드는 AST 설계상 남겨 두되, codegen은 나중 단계로 둔다.

## 21. return 문 생성

`Return` 노드 생성 흐름:

```text
표현식이 있으면 값 생성
함수 반환 타입과 맞는지 확인
ret 생성
```

`Void` 반환:

```text
return;
  -> ret void
```

주의:

- `return` 이후 같은 블록에 명령을 계속 생성하면 안 된다.
- 현재 basic block이 이미 terminator를 가진 상태인지 확인해야 한다.

점검 질문:

- 반환 후 뒤 문장을 파서에서 허용할 것인가?
- 허용한다면 IR 생성에서 unreachable 블록을 만들 것인가, 의미분석에서 경고만 낼 것인가?

초기 추천:

- 반환 뒤 문장은 의미분석에서 나중에 확인한다.
- IR 생성에서는 terminator가 있는 블록에 추가 명령을 넣지 않는 원칙을 지킨다.

## 22. if 문 생성

`If` 노드는 조건 목록, 블록 목록, else 블록을 가진다.

기본 블록 구성:

```text
condition block
then block
else or next-condition block
merge block
```

단순 `if`:

```text
if (cond) { then }
```

흐름:

```text
cond 값을 i1로 변환
cond ? then_block : merge_block
then_block 생성
then 끝에서 merge로 branch
merge_block으로 이동
```

`if/else`:

```text
cond ? then_block : else_block
then_block -> merge
else_block -> merge
```

`else if`:

- 조건이 여러 개이므로 실패 시 다음 조건 블록으로 이동한다.
- 마지막 실패는 else 또는 merge로 이동한다.

주의:

- then이나 else 블록이 `return`으로 끝나면 merge로 branch를 추가하지 않는다.
- 모든 생성된 basic block의 끝에는 terminator가 필요하다.

## 23. for 문 생성

`For` 노드 구성:

- initialization
- condition
- expression
- block

기본 블록 후보:

```text
init
condition
body
step
exit
```

흐름:

```text
init 생성
condition으로 branch
condition 값 생성
condition ? body : exit
body 생성
body 끝에서 step으로 branch
step 생성
step 끝에서 condition으로 branch
exit으로 이동
```

`break`와 `continue`:

- `break`는 현재 반복문의 `exit` 블록으로 branch.
- `continue`는 현재 반복문의 `step` 블록으로 branch.

필요한 상태:

- break target stack.
- continue target stack.

왜 stack인가?

- 반복문이 중첩될 수 있기 때문이다.

주의:

- `break` 또는 `continue` 이후 현재 블록은 terminator를 가진다.
- 그 뒤 같은 블록에 명령을 추가하지 않도록 해야 한다.

## 24. print / printLine 생성

`Print` 노드는 내장 출력으로 처리할 수 있다.

구현 선택지:

### A. 외부 `printf` 호출로 낮추기

장점:

- LLVM 예제가 많다.
- 문자열 포맷으로 여러 타입 출력 가능.

단점:

- 가변 인자 함수 선언이 필요하다.
- 포맷 문자열을 만들어야 한다.

### B. 타입별 런타임 함수 만들기

예:

```text
printInt(i32)
printChar(i8)
printString(ptr)
```

장점:

- 호출 규칙이 단순하다.

단점:

- 별도 런타임 구현이 필요하다.

초기 추천:

- 학습 초반에는 `printf`보다 타입별 외부 함수라고 가정하는 편이 단순하다.
- 실제 실행까지 목표로 하면 나중에 C 런타임 함수와 연결 방식을 정한다.

점검 질문:

- `print(1, 2, 3)`은 공백 없이 출력할 것인가?
- `printLine`은 마지막에 newline만 추가할 것인가?
- 문자열과 문자의 출력 차이는 어떻게 둘 것인가?

## 25. 문자열 리터럴과 escape 출력

텍스트 IR에서 문자열은 escape 규칙이 중요하다.

필요한 처리:

- `\n`, `\t`, `\0` 같은 escape.
- 따옴표와 역슬래시.
- 문자열 끝의 null terminator.
- 배열 길이 계산.

점검 질문:

- Lexer가 이미 escape를 실제 문자로 바꾸고 있다. IR 출력 시 다시 LLVM escape 형태로 바꿔야 한다.
- 원본 문자열을 보존할지, 변환된 문자열만 저장할지 결정해야 한다.

초기 추천:

- `StringLiteral`에는 변환된 실제 문자열을 저장하고, IR 출력 단계에서 LLVM 문자열 escape로 다시 변환한다.

## 26. 기본 블록과 terminator 관리

LLVM basic block은 마지막에 terminator가 있어야 한다.

terminator 예:

- `ret`
- `br`
- `switch`
- `unreachable`

생성 중 확인할 것:

- 현재 블록이 이미 끝났는가?
- 끝난 블록에 명령을 추가하고 있지 않은가?
- `if`나 `for`에서 새로 만든 블록들이 모두 연결되는가?
- 함수 마지막 블록에 반환이 있는가?

개념적 보조 함수:

```text
hasTerminator(block)
ensureBranchTo(target)
setInsertionBlock(block)
```

이 이름들은 구현 지시가 아니라 필요한 사고 단위다.

## 27. 스코프와 심볼 테이블

IR 생성에서 최소한 이름 -> 주소/함수 매핑이 필요하다.

지역 스코프 구조:

```text
stack of maps
name -> local address/value info
```

전역 구조:

```text
global variables: name -> global address
functions: name -> function symbol
```

스코프 진입:

- 함수 시작.
- 블록 시작.

스코프 종료:

- 블록 끝.
- 함수 끝.

점검 질문:

- inner block에서 같은 이름을 다시 선언할 수 있는가?
- 매개변수와 지역변수 이름 충돌을 허용할 것인가?
- 전역변수와 지역변수 이름이 같으면 어느 쪽을 우선하는가?

초기 추천:

- 지역 스코프가 전역보다 우선한다.
- 같은 스코프의 중복 선언은 오류.

## 28. 타입 변환

처음에는 타입 변환을 최소화한다.

필요할 수 있는 변환:

- `Char` -> `Int`: sign extend 또는 zero extend.
- `Int` -> `Char`: truncate.
- `i1` -> `Int`: zero extend.
- `Int` -> 조건 `i1`: `value != 0`.
- pointer -> 조건 `i1`: `ptr != null`.

점검 질문:

- `Char`는 signed인가 unsigned인가?
- 암시적 변환을 허용할 것인가?
- 명시적 cast 문법이 없는데 변환을 얼마나 자동으로 할 것인가?

초기 추천:

- 산술은 `Int`끼리만 허용.
- 조건 변환만 허용.
- 나머지 변환은 의미분석 오류로 둔다.

## 29. 최소 구현 마일스톤

### IR-M0. 손으로 LLVM IR 관찰

목표:

- 아주 작은 C 코드를 `clang -S -emit-llvm`로 관찰한다.

관찰 대상:

```text
int main() { return 1; }
int main() { int x = 1; return x; }
int main() { return 1 + 2; }
```

확인:

- 함수 모양.
- `ret` 명령.
- `alloca`, `load`, `store`.
- 산술 명령.

### IR-M1. `return 1`

목표:

- `Int main() { return 1; }`를 IR로 낮춘다.

필요:

- module.
- function.
- entry block.
- integer constant.
- return.

아직 필요 없는 것:

- 지역변수.
- 심볼 테이블.
- 제어 흐름.
- 포인터.

### IR-M2. 산술 반환

목표:

```text
return 1 + 2 * 3;
```

필요:

- 산술 표현식 재귀 생성.
- 임시값 생성.
- 연산자별 IR 명령.

확인:

- AST 우선순위가 IR 순서에도 반영되는가?

### IR-M3. 지역변수와 대입

목표:

```text
Int x;
x = 1;
return x;
```

필요:

- 지역변수 노드.
- `alloca`.
- `store`.
- `load`.
- 이름 -> 주소 테이블.

핵심 질문:

- `x`를 읽을 때와 쓸 때의 codegen 경로가 다른가?

### IR-M4. 함수 매개변수

목표:

```text
Int id(Int x) { return x; }
```

필요:

- 함수 인자 등록.
- 매개변수 이름 연결.
- 필요하면 매개변수 alloca/store.

### IR-M5. 함수 호출

목표:

```text
Int one() { return 1; }
Int main() { return one(); }
```

필요:

- 함수 시그니처 선등록.
- call 생성.
- 반환값 사용.

### IR-M6. if

목표:

```text
if (x) { return 1; } else { return 0; }
```

필요:

- 조건 변환.
- then/else/merge 블록.
- terminator 확인.

### IR-M7. for, break, continue

목표:

```text
for (...) { ... }
```

필요:

- condition/body/step/exit 블록.
- break/continue 대상 스택.

### IR-M8. 포인터와 배열

목표:

```text
Int x;
Int *p;
p = &x;
*p = 3;
```

그리고:

```text
Int a[3];
a[0] = 1;
```

필요:

- 주소 생성.
- 역참조.
- getelementptr 개념.
- 배열/포인터 타입 구분.

### IR-M9. 문자열과 출력

목표:

```text
printLine("hello");
```

필요:

- 문자열 전역 상수.
- 출력 함수 선언 또는 runtime 호출.
- newline 정책.

## 30. 검증 방법

텍스트 IR을 만든다면 다음 도구를 사용할 수 있다.

- `llvm-as`: `.ll`을 bitcode로 조립하며 문법 확인.
- `lli`: IR 직접 실행.
- `clang`: IR을 실행 파일로 연결.
- `opt -verify`: IR 검증.

LLVM API를 쓴다면:

- module verifier를 호출한다.
- 생성된 IR을 파일로 출력해 사람이 읽는다.

학습 체크 방식:

1. 가장 작은 입력 하나를 정한다.
2. 예상 AST를 손으로 그린다.
3. 예상 IR 구조를 손으로 적는다.
4. 생성 결과와 비교한다.
5. `load`/`store`/`br`/`ret` 위치를 확인한다.

## 31. 흔한 실수 체크리스트

- 변수 값을 읽어야 하는데 주소를 그대로 사용했다.
- 대입 왼쪽에서 `load`를 해버렸다.
- `return` 뒤에 같은 블록에 명령을 추가했다.
- `if`의 then/else 블록 끝에 merge branch가 없다.
- 이미 `return`이 있는 블록에 branch를 또 붙였다.
- `for`의 `continue`가 condition으로 가야 하는지 step으로 가야 하는지 정하지 않았다.
- 전역변수 초기화에 런타임 계산식을 넣었다.
- 문자열 길이에 null terminator를 포함하지 않았다.
- 비교 결과 `i1`을 `i32`처럼 사용했다.
- 함수 선언 전에 호출을 생성하려 했다.
- 배열 주소와 첫 원소 주소를 혼동했다.

## 32. 파서 문서와의 연결

`PARSER_IMPLEMENTATION_SPEC.md`에서 다음 항목이 준비되어야 IR 생성이 쉬워진다.

- `Return` 문이 표현식을 가진다.
- 표현식 AST가 우선순위를 보존한다.
- 지역변수 선언 노드가 있다.
- `If`와 `For`의 구성요소가 명확히 나뉜다.
- `GetVariable`, `Assigment`, `AddressOf`, `Dereference`, `GetElement`가 AST에 남는다.
- 함수 호출 인자 목록이 보존된다.

파서가 애매하게 만든 정보는 IR 생성에서 더 크게 문제가 된다. IR 생성에서 막히면 먼저 AST에 정보가 충분한지 확인한다.

## 33. 다음 구현 질문

LLVM IR 생성을 시작하기 전에 답해 볼 질문:

```text
Int main() { return 1; }의 AST에서 LLVM function을 만들 때 필요한 정보는 정확히 무엇인가?
```

그 다음 질문:

```text
GetVariable("x")를 만났을 때 지금 필요한 것은 x의 값인가, x의 주소인가?
```

이 두 질문에 자연스럽게 답할 수 있으면 LLVM IR 생성의 첫 단계를 시작할 준비가 된 것이다.

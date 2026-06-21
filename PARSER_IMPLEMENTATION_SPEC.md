# micro_C 파서 구현 명세

이 문서는 `Parser.h`, `Parser.cpp`, `Node.h`를 이어서 구현할 때 참고할 상세 명세다. 저장소가 학습 모드이므로, 아래 내용은 그대로 옮겨 적는 완성 구현이 아니라 구현 순서, 판단 기준, 점검 질문을 제공하는 설계 문서다.

## 1. 파서의 목표

파서의 역할은 `Lexer`가 만든 `vector<Token>`을 읽어 `Program` AST로 바꾸는 것이다.

전체 흐름:

```text
vector<Token>
  -> Parser::parse()
  -> Program
  -> Declaration*
  -> Statement*
  -> Expression*
```

파서는 다음 책임을 가진다.

- 토큰 순서가 문법에 맞는지 확인한다.
- 선언, 문장, 표현식을 AST 노드로 만든다.
- 괄호, 세미콜론, 쉼표 같은 구분자를 소비한다.
- 연산자 우선순위에 맞게 표현식 트리를 만든다.
- 오류가 나면 "어떤 토큰이 필요했는지"를 알려준다.

파서가 아직 책임지지 않아도 되는 것:

- 변수나 함수가 실제로 선언되었는지 확인.
- 타입이 맞는지 확인.
- `break`가 반복문 안에 있는지 확인.
- LLVM IR 생성.

이것들은 의미분석 또는 코드 생성 단계의 책임이다. 다만 파서가 AST에 충분한 정보를 남겨야 다음 단계가 가능하다.

## 2. 현재 코드 기준으로 먼저 정리할 문제

현재 파서 주변에서 먼저 살펴볼 지점:

- `Parser`가 외부에서 `tokens`를 받는 방법이 보이지 않는다.
- `parse()`의 `default` 경로에서 토큰을 소비하지 않아 무한 루프 위험이 있다.
- `parseBlock()`이 아직 문장을 만들지 않고 반환도 없다.
- `parseDeclaration()`의 전역변수 초기화가 비어 있다.
- `parseExpressionStatement`, `parseVariable`, `parseFor`, `parseIf`, `parseReturn`, `parseContinue`, `parseBreak`가 선언만 되어 있다.
- `Node.h`에서 일부 타입/표현식 선언 순서가 C++ 컴파일에 영향을 줄 수 있다.

학습 순서상 가장 먼저 해야 할 일은 "가장 작은 프로그램 하나를 파싱할 수 있는 길"을 만드는 것이다.

권장 목표 입력:

```text
Int main() { return 1; }
```

이 입력을 처리하려면 최소한 다음 흐름이 필요하다.

```text
parse()
  -> parseDeclaration()
  -> parseType()
  -> parseParameter()
  -> parseBlock()
  -> parseReturn()
  -> parseExpression()
```

## 3. 입력과 파서 상태

파서는 최소한 다음 상태를 가진다.

- `tokens`: Lexer 결과.
- `current`: 현재 읽고 있는 토큰.

권장 불변식:

- `current`는 항상 아직 소비하지 않은 다음 토큰을 가리킨다.
- 어떤 parse 함수가 성공하면 자신이 맡은 문법 단위의 마지막 토큰까지 소비한다.
- 어떤 parse 함수가 실패하면 명확한 오류를 낸다.
- 반복문 안에서는 반드시 토큰을 소비하거나 오류를 내야 한다.

필요한 보조 동작:

- 현재 토큰 종류 확인.
- 현재 토큰이 특정 종류인지 확인하고 소비.
- 특정 종류면 소비하고 `true`, 아니면 그대로 두고 `false`.
- 오류 메시지에 현재 토큰과 기대 토큰을 포함.

현재 `skipCurrent`, `skipCurrent(Kind)`, `skipCurrentIf(Kind)`가 이 역할의 시작점이다.

## 4. 최상위 문법

최상위는 선언의 반복으로 본다.

개념 문법:

```text
program := declaration* EndOfToken
```

현재 토큰 정의 기준으로 최상위 선언은 `Int`, `Char`, `Void` 중 하나로 시작한다.

확인할 입력:

```text
Int x;
Char c;
Void f();
Int main() { return 1; }
```

점검 질문:

- `EndOfToken` 전까지 모든 토큰이 소비되는가?
- 선언이 아닌 토큰이 최상위에 오면 조용히 무시하는가, 오류를 내는가?
- 오류를 낸다면 현재 토큰 문자열이 메시지에 보이는가?

학습 힌트:

- 최상위에서 모르는 토큰을 무시하면 나중에 버그가 숨는다.
- 처음에는 "선언이 필요하다"는 오류를 내는 편이 디버깅하기 쉽다.

## 5. 선언 파싱

선언은 크게 함수와 전역변수로 나뉜다.

개념 문법:

```text
declaration := type declarator function_tail_or_variable_tail
```

현재 코드의 방향:

- `parseType(name)`이 타입과 이름을 함께 읽는다.
- 그 다음 토큰이 `(`이면 함수.
- 아니면 전역변수.

함수 선언/정의:

```text
function := type name "(" parameters? ")" (";" | block)
```

전역변수 선언:

```text
global_variable := type name ("=" expression)? ";"
```

파서가 만들어야 할 AST:

- 함수면 `Function`
  - `type`: 반환 타입.
  - `name`: 함수 이름.
  - `parameters`: 매개변수 타입/이름 목록.
  - `block`: 함수 본문이 있으면 문장 목록.
- 전역변수면 `GrobalVariable`
  - `type`: 변수 타입.
  - `name`: 변수 이름.
  - `expression`: 초기화식이 있으면 표현식.

점검 질문:

- 함수 선언 `Int f();`와 함수 정의 `Int f() { ... }`를 구분하는가?
- 전역변수 `Int x;`와 `Int x = 1;`을 구분하는가?
- 초기화식이 없는 경우 `expression`은 어떤 값으로 둘 것인가?
- `Void x;` 같은 선언을 파서에서 막을 것인가, 의미분석에서 막을 것인가?

추천:

- 문법 모양만 파서에서 처리하고, 타입으로 가능한지 여부는 의미분석에서 검사한다.

## 6. 타입과 선언자 파싱

현재 `parseType`/`parseType_reg`는 C 스타일 선언자를 목표로 한다.

지원하려는 선언 예시:

```text
Int x;
Int *p;
Int a[3];
Int *a[3];
Int (*p)[3];
```

타입 AST의 의도:

```text
Type(Int)
Pointer(sub = Type(Int))
Array(sub = Type(Int), length = 3)
```

중요한 점:

- `*`는 이름 앞에 붙지만 의미상 "포인터 타입"을 만든다.
- `[]`는 이름 뒤에 붙지만 의미상 "배열 타입"을 만든다.
- 괄호는 `*`와 `[]`의 결합 순서를 바꾼다.

예상 해석:

```text
Int *a[3]
```

의미:

```text
a is array[3] of pointer to Int
```

```text
Int (*p)[3]
```

의미:

```text
p is pointer to array[3] of Int
```

현재 코드의 스택 방식은 이 차이를 표현하려는 시도로 보인다. 구현을 이어갈 때는 각 선언을 손으로 그려 보며 스택 push/pop 순서가 원하는 타입 트리를 만드는지 확인한다.

점검 질문:

- `name`이 반드시 채워지는가?
- 배열 길이는 정수 리터럴만 허용하는가?
- 배열 길이 0 또는 음수는 어디에서 검사할 것인가?
- 매개변수에서 배열 타입을 그대로 둘 것인가, 포인터처럼 다룰 것인가?

## 7. 매개변수 파싱

개념 문법:

```text
parameters := parameter ("," parameter)*
parameter  := type declarator
```

빈 매개변수 목록:

```text
()
```

매개변수 예시:

```text
(Int x)
(Int x, Char c)
(Int *p, Int n)
```

현재 `parseParameter()`는 `RightParen`을 만날 때까지 타입을 읽고 쉼표를 처리한다.

점검 질문:

- 빈 목록 `()`을 처리하는가?
- 마지막 매개변수 뒤에 쉼표가 있는 경우 허용할 것인가?
- `Void` 하나만 있는 매개변수 목록을 특별 취급할 것인가?
- 매개변수 이름이 없는 선언을 허용할 것인가?

처음에는 단순하게 가도 된다.

- 빈 목록은 허용.
- 모든 매개변수는 이름 필수.
- trailing comma는 불허.
- `Void` 매개변수 특별 규칙은 나중에 고려.

## 8. 블록 파싱

개념 문법:

```text
block := "{" statement* "}"
```

블록은 `vector<Statement*>`를 만든다.

중요한 책임:

- `{`를 소비한다.
- `}`가 나올 때까지 문장을 하나씩 읽는다.
- `}`를 소비한다.
- 빈 블록도 허용한다.

반복문에서 반드시 확인할 것:

- 문장 하나를 파싱할 때마다 `current`가 전진하는가?
- 문장 파싱에 실패하면 즉시 오류가 나는가?
- `EndOfToken`을 만났는데 `}`가 없으면 오류를 내는가?

지원할 문장 종류의 권장 순서:

1. `return`
2. 표현식 문장
3. 지역변수 선언
4. `if`
5. `for`
6. `break`, `continue`
7. `print`, `printLine`

처음부터 모든 문장을 넣으려 하지 말고, `return` 하나부터 연결하는 것이 좋다.

## 9. 문장 파싱

문장 선택 기준:

```text
Return      -> return statement
If          -> if statement
For         -> for statement
Break       -> break statement
Continue    -> continue statement
Print       -> print statement
PrintLine   -> printLine statement
Int/Char/Void -> local variable declaration
default     -> expression statement
```

### return 문

개념 문법:

```text
return_statement := "return" expression? ";"
```

점검 질문:

- `Void` 함수에서 `return;`을 허용할 것인가?
- `Int` 함수에서 `return;`은 어디에서 오류 처리할 것인가?
- 파서는 반환 타입을 모르므로, 표현식 유무만 AST에 남기고 타입 검사는 의미분석에서 하는 편이 자연스럽다.

### break / continue 문

개념 문법:

```text
break_statement    := "break" ";"
continue_statement := "continue" ";"
```

파서는 노드를 만들기만 한다. 반복문 안인지 여부는 의미분석에서 검사한다.

### 표현식 문

개념 문법:

```text
expression_statement := expression ";"
```

예시:

```text
x = 1;
f(x);
a[i] = 3;
```

표현식 문장은 결과값을 사용하지 않는 표현식이다.

### 지역변수 선언

현재 `Node.h`에는 지역변수 전용 `Statement` 노드가 없다. 선택지가 있다.

- 별도 `LocalVariable` 문장 노드를 추가한다.
- 전역변수 노드와 비슷한 구조를 재사용하지 않고, 문장용 선언 노드를 둔다.
- 초기에는 지역변수를 미루고 함수 매개변수와 전역변수만으로 진행한다.

학습 추천:

- `return 1;`까지 먼저 끝낸 뒤 지역변수 노드를 설계한다.
- 지역변수는 LLVM IR에서 `alloca`로 내려갈 가능성이 크므로, 타입/이름/초기화식을 담는 문장 노드가 있으면 편하다.

### if 문

개념 문법 후보:

```text
if_statement :=
  "if" "(" expression ")" block
  ("else" "if" "(" expression ")" block)*
  ("else" block)?
```

현재 `If` 노드는 다음 구조를 가진다.

- `conditions`: 조건식 목록.
- `blocks`: 각 조건이 참일 때 실행할 블록 목록.
- `elseBlock`: 마지막 else 블록.

이 구조는 `if / else if / else`를 담기 좋다.

점검 질문:

- `else if`를 하나의 `If` 노드에 누적할 것인가?
- 아니면 `else` 블록 안에 또 다른 `If` 문을 넣을 것인가?
- 현재 `Node.h` 구조를 보면 누적 방식이 의도에 가깝다.

### for 문

개념 문법 후보:

```text
for_statement :=
  "for" "(" init? ";" condition? ";" step? ")" block
```

현재 `For` 노드:

- `initialization`: `Statement*`
- `condition`: `Expression*`
- `expression`: `Expression*`
- `block`: 본문

점검 질문:

- 초기화 자리에 변수 선언을 허용할 것인가?
- 조건 생략은 무한 반복으로 볼 것인가?
- step 생략을 허용할 것인가?
- `initialization`이 `Statement*`면 세미콜론 처리를 어디에서 할 것인가?

학습 추천:

- 처음에는 세 구성요소를 모두 요구하는 단순한 `for`만 처리한다.
- 그 뒤 하나씩 생략 가능하게 넓힌다.

### print / printLine 문

개념 문법 후보:

```text
print_statement     := "print" "(" arguments? ")" ";"
printLine_statement := "printLine" "(" arguments? ")" ";"
arguments           := expression ("," expression)*
```

현재 `Print` 노드:

- `lineFeed`: 줄바꿈 여부.
- `arguments`: 출력할 표현식 목록.

파서는 `print`와 `printLine`을 같은 노드로 만들고 `lineFeed`만 다르게 둘 수 있다.

## 10. 표현식 파싱 전체 구조

표현식은 우선순위별로 나눠 읽는 것이 가장 안정적이다.

권장 단계:

```text
assignment
logical_or
logical_and
equality
relational
additive
multiplicative
unary
postfix
primary
```

핵심 규칙:

- 낮은 우선순위 함수가 높은 우선순위 함수를 호출한다.
- 왼쪽 결합 연산자는 반복으로 묶는다.
- 대입은 보통 오른쪽 결합이다.
- 단항 연산자는 자기보다 높은 우선순위인 unary 또는 postfix를 다시 읽는다.

이 문서는 전체 코드를 쓰지 않지만, 생각 흐름은 다음과 같다.

```text
parseAdditive:
  왼쪽 = parseMultiplicative
  while 현재 토큰이 + 또는 -:
    연산자를 기억하고 소비
    오른쪽 = parseMultiplicative
    왼쪽 = Arithmetic(연산자, 왼쪽, 오른쪽)
  왼쪽 반환
```

이 정도 형태를 각 우선순위 단계에 맞게 적용한다.

## 11. Primary 표현식

Primary는 표현식의 가장 작은 단위다.

후보:

```text
primary :=
    IntegerLiteral
  | FloatLiteral
  | StringLiteral
  | CharLiteral
  | NullLiteral
  | Identifier
  | "(" expression ")"
```

생성 노드:

- 정수 -> `IntegerLiteral`
- 실수 -> `FloatLiteral`
- 문자열 -> `StringLiteral`
- null -> `NullLiteral`
- 식별자 -> `GetVariable`
- 괄호 -> 내부 표현식을 그대로 사용

점검 질문:

- `CharLiteral`에 대응하는 AST 노드가 현재 있는가?
- `BooleanLiteral` 노드는 있지만 `true`, `false` 토큰은 없다. 언어에 boolean 리터럴을 넣을 것인가?
- 문자열 리터럴은 표현식 타입을 어떻게 둘 것인가?

## 12. Postfix 표현식

Postfix는 primary 뒤에 붙는 연산이다.

후보:

```text
postfix :=
  primary
  ("(" arguments? ")"
  | "[" expression "]"
  | "." Identifier
  | "->" Identifier)*
```

생성 노드:

- 함수 호출 -> `Call`
- 배열 인덱스 -> `GetElement`
- 구조체 멤버 -> `GetMember`
- 포인터 멤버 -> `GetMemberArrow`

예시:

```text
f(x)
a[i]
s.member
p->member
f(x)[0]
```

점검 질문:

- 호출 대상은 꼭 변수 이름이어야 하는가, 아니면 임의 표현식도 허용할 것인가?
- `a[i][j]`처럼 postfix가 연속되는 경우를 처리하는가?
- 멤버 접근에서 식별자가 없으면 명확한 오류를 내는가?

## 13. Unary 표현식

후보:

```text
unary :=
    "!" unary
  | "-" unary
  | "&" unary
  | "*" unary
  | postfix
```

생성 노드:

- `!`, `-` -> `Unary`
- `&` -> `AddressOf`
- `*` -> `Dereference`

점검 질문:

- `*`는 토큰 이름이 `Asterisk`다. 곱셈과 역참조를 문맥으로 구분해야 한다.
- `&`는 토큰 이름이 `Ampersand`다. 주소 연산과 나중의 bitwise-and를 구분할지 결정해야 한다.
- unary에서 `-1`을 하나의 리터럴로 만들지, `Unary(-, IntegerLiteral(1))`로 만들지 결정한다.

## 14. 이항 연산

현재 AST 노드:

- `Or`
- `And`
- `Relational`
- `Arithmetic`

연산자 매핑 후보:

```text
||            -> Or
&&            -> And
== != < > <= >= -> Relational
+ - * / %     -> Arithmetic
```

점검 질문:

- `==`와 `<`를 모두 `Relational`에 넣을 것인가?
- `&&`, `||`를 단순 이항 노드로 두면 short-circuit 처리는 코드 생성에서 어떻게 할 것인가?
- 비교 결과 타입은 의미분석에서 `Boolean`으로 볼 것인가, `Int`로 볼 것인가?

## 15. 대입 표현식

개념 문법:

```text
assignment := logical_or ("=" assignment)?
```

현재 AST 노드:

- `Assigment`
  - `lhs`
  - `rhs`

중요:

- 파서는 왼쪽이 대입 가능한지 아직 모를 수 있다.
- 의미분석에서 `lhs`가 lvalue인지 확인한다.

예시:

```text
x = 1
a[i] = x + 2
*p = 3
```

점검 질문:

- `x = y = 1`을 허용할 것인가?
- 허용한다면 대입은 오른쪽 결합이어야 한다.
- 대입 표현식 자체의 결과값을 허용할 것인가?

처음에는 대입문이 표현식 문장으로만 쓰인다고 생각해도 된다. 하지만 AST 구조상 표현식으로 두면 나중에 확장하기 쉽다.

## 16. 오류 처리 명세

좋은 오류 메시지는 구현 속도를 올린다.

최소 메시지 구성:

- 기대한 토큰 종류.
- 실제 토큰 종류.
- 실제 토큰 문자열.
- 가능하면 현재 위치.

현재 `Token`에는 위치 정보가 없으므로 처음에는 종류와 문자열만 사용한다.

예시 메시지 형태:

```text
expected ';', got Identifier("x")
```

파서에서 조심할 상황:

- `)`가 필요한데 `;`가 나온 경우.
- `}`가 필요한데 `EndOfToken`이 나온 경우.
- 표현식이 필요한데 연산자가 나온 경우.
- 선언에서 이름이 빠진 경우.

오류 처리 원칙:

- 학습 초기에는 복구를 시도하지 말고 즉시 실패해도 된다.
- 조용히 무시하지 않는다.
- 실패 지점이 어디인지 알 수 있게 한다.

## 17. AST와 파서 사이의 계약

파서가 AST에 남겨야 할 정보:

- 선언 이름.
- 선언 타입.
- 함수 매개변수 목록.
- 블록 내부 문장 순서.
- 각 표현식의 연산자 종류.
- 리터럴 원본 값 또는 변환된 값.
- 변수 참조 이름.

파서가 AST에 남기지 않아도 되는 정보:

- 변수의 실제 선언 위치.
- 표현식의 최종 타입.
- LLVM 값 이름.
- 최적화 정보.

나중에 의미분석에서 추가할 수 있는 정보:

- `Expression`별 타입.
- `Expression`별 lvalue/rvalue 여부.
- `GetVariable`이 가리키는 선언.
- 함수 호출이 가리키는 함수 선언.

## 18. 구현 순서 제안

권장 순서:

1. 파서가 토큰 배열을 받을 수 있게 한다.
2. `parse()`가 최상위 선언을 끝까지 읽거나 오류를 내게 한다.
3. `parseBlock()`이 빈 블록을 처리하게 한다.
4. `return` 문 하나를 처리한다.
5. primary 표현식 중 정수 리터럴만 처리한다.
6. 목표 입력 `Int main() { return 1; }`을 AST로 만든다.
7. 식별자와 괄호 표현식을 추가한다.
8. 산술 표현식을 추가한다.
9. 대입 표현식과 표현식 문장을 추가한다.
10. 지역변수 선언 노드를 설계한다.
11. `if`, `for`, `break`, `continue`를 추가한다.
12. 호출, 인덱스, 포인터, 멤버 접근을 추가한다.
13. `print`, `printLine`을 추가한다.

각 단계마다 확인할 것:

- 새로 지원한 입력 하나가 통과하는가?
- 의도적으로 틀린 입력 하나가 오류를 내는가?
- 무한 루프 없이 종료하는가?

## 19. 단계별 확인 입력

### 선언

```text
Int x;
Int *p;
Int a[3];
Int main();
```

확인:

- 전역변수와 함수 선언이 구분되는가?
- 포인터/배열 타입 구조가 기대와 같은가?

### 함수 본문

```text
Int main() { }
Int main() { return 1; }
```

확인:

- 빈 블록이 된다.
- `Return` 문이 블록에 들어간다.

### 표현식

```text
1 + 2 * 3
(1 + 2) * 3
x = y + 1
```

확인:

- 곱셈이 덧셈보다 깊은 AST 노드가 된다.
- 괄호가 우선순위를 바꾼다.
- 대입의 왼쪽과 오른쪽이 나뉜다.

### 제어문

```text
if (x) { return 1; } else { return 0; }
for (i = 0; i < n; i = i + 1) { print(i); }
```

확인:

- 조건식과 블록이 분리된다.
- `for`의 init/condition/step/body가 각각 AST에 들어간다.

## 20. 다음 단계로 넘어가기 전 체크리스트

LLVM IR로 넘어가기 전에 파서 쪽에서 최소한 확인할 것:

- `Program` 안에 최상위 선언들이 순서대로 들어간다.
- 함수 선언과 함수 정의가 구분된다.
- 함수 본문은 문장 목록을 가진다.
- `return` 문이 표현식을 가질 수 있다.
- 표현식 AST가 연산자 우선순위를 보존한다.
- 변수 참조, 호출, 대입, 배열 인덱스가 AST로 표현된다.
- 파서가 모르는 토큰을 조용히 넘기지 않는다.
- 오류 상황에서 무한 루프가 나지 않는다.

마지막 질문:

```text
지금 Parser.cpp에서 Int main() { return 1; }을 목표로 할 때, 가장 먼저 current가 멈추는 지점은 어디일까?
```

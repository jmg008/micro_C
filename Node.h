#pragma once
#include <vector>
#include <string>
#include "Token.h"

using std::vector;
using std::string;

struct Program {
    vector<struct Function*> functions;
};

struct Statement {
    virtual ~Statement() = default;
    virtual auto generate()->void = 0;
};

// 멤버를 만들기 vs Type에 이름 추가
// Struct의 Name도 필요하니 name 추가로 결정

// struct Type {
//     Kind type;
//     // int pointerDepth;
//     // bool isArray;
//     vector<bool> pointerAndArray; // 0:pointer, 1:array ex) int* arr[10] -> <0, 1> / int (*arr)[10] -> <1,0>
//     int memLength; // 전체 메모리 크기
//     string name; // type == struct || type == member 변수
//     // Type* member;
// };

struct Type {
    Kind type;
};

struct Pointer: Type {
    Type* sub;
};

struct Array: Type {
    Type* sub;
    int length; // 배열의 길이
};

struct Expression {
    virtual ~Expression() = default;
    virtual auto generate()->void = 0;
};

struct Function: Statement {
    string name;
    vector<vector<Kind, string>> parameters;
    vector<Statement*> block;
    auto generate()->void;
};

struct Variable: Statement {
    Type type; // <Pointer, Pointer, Int>
    string name;
    Expression* expression;
    auto generate()->void;
};

struct Return: Statement {
    Expression* expression;
    auto generate()->void;
};

struct For: Statement {
    Statement* initialization;
    Expression* condition;
    Expression* expression;
    vector<Statement*> block;
    auto generate()->void;
};

struct Break: Statement {
    auto generate()->void;
};

struct Continue: Statement {
    auto generate()->void;
};

struct If: Statement {
    vector<Expression*> conditions;
    vector<vector<Statement*>> blocks;
    vector<Statement*> elseBlock;
    auto generate()->void;
};

struct Print: Statement {
  bool lineFeed = false;
  vector<Expression*> arguments;
  auto generate()->void;
};

struct ExpressionStatement: Statement {
  Expression* expression;
  auto generate()->void;
};

struct Or: Expression {
  Expression* lhs;
  Expression* rhs;
  auto generate()->void;
};

struct And: Expression {
  Expression* lhs;
  Expression* rhs;
  auto generate()->void;
};

struct Relational: Expression {
  Kind kind;
  Expression* lhs;
  Expression* rhs;
  auto generate()->void;
};

struct Arithmetic: Expression {
  Kind kind;
  Expression* lhs;
  Expression* rhs;
  auto generate()->void;
};

struct Unary: Expression {
  Kind kind;
  Expression* sub;
  auto generate()->void;
};

struct Assigment: Expression {
    // Kind kind; // 할당식의 타입 -> 생성 단계로 이동
    Expression* lhs; // 이후에 검증
    Expression* rhs;
    auto generate()->void;
};

struct Call: Expression {
  Expression* sub;
  vector<Expression*> arguments;
  auto generate()->void;
};

// 포인터 연산
struct AddressOf: Expression {
    Expression* sub;
    auto generate() -> void;
};

struct Dereference: Expression {
    Expression* sub;
    auto generate() -> void;
};

struct GetElement: Expression {
  Expression* sub;
  Expression* index;
  auto generate()->void;
};

struct GetVariable: Expression {
  string name;
  auto generate()->void;
};

// 구조체 접근
struct GetMember: Expression {
    Expression* object;
    string name;
    auto generate()->void;
};

struct GetMemberArrow: Expression {
    Expression* object;
    string name;
    auto generate()->void;
};

struct NullLiteral: Expression {
  auto generate()->void;
};

struct BooleanLiteral: Expression {
  bool value = false;
  auto generate()->void;
};

struct IntegerLiteral: Expression {
  long long value = 0;
  auto generate()->void;
};

struct FloatLiteral: Expression {
  double value = 0.0;
  auto generate()->void;
};

struct StringLiteral: Expression {
  string value;
  auto generate()->void;
};
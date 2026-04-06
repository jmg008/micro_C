#include <string>
#include <iostream>

using std::string;
using std::ostream;

enum class Kind {
    Unknown, EndOfToken,

    NullLiteral,
    IntLiteral, FloatLiteral, StringLiteral, CharLiteral,
    Identifier,

    Int, Char, Void,
    Struct,

    Function, Return,
    For, Break, Continue,
    If, Else,
    Print, PrintLine,

    LogicalAnd, LogicalOr, LogicalNot,
    Assignment,
    Add, Subtract,
    Asterisk, Divide, Modulo,
    Ampersand,
    Equal, NotEqual,
    LessThan, GreaterThan,
    LessOrEqual, GreaterOrEqual,

    Dot,
    Arrow,

    Comma, Colon, Semicolon,
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    LeftBraket, RightBraket,
};

auto toKind(string)->Kind;
auto toString(Kind)->string;

struct Token {
    Kind kind = Kind::Unknown;
    string string;
};

auto operator<<(ostream&, Token&)->ostream&;
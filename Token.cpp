#include <map>
#include <iomanip>
#include "Token.h"

using std::map;
using std::setw;
using std::left;

static map<string, Kind> stringToKind = {
    {"#Unknown",    Kind::Unknown},
    {"#EndOfToken", Kind::EndOfToken},
    
    {"null",        Kind::NullLiteral},
    {"#Number",     Kind::NullLiteral},
    {"#String",     Kind::StringLiteral},
    {"#Char",       Kind::CharLiteral},
    {"#Identifier", Kind::Identifier},

    {"Int",         Kind::Int},
    {"Char",        Kind::Char},
    {"Void",        Kind::Void},
    {"Struct",      Kind::Struct},

    {"function",    Kind::Function},
    {"return",      Kind::Return},
    {"for",         Kind::For},
    {"break",       Kind::Break},
    {"continue",    Kind::Continue},
    {"if",          Kind::If},
    {"else",        Kind::Else},
    {"print",       Kind::Print},
    {"printLine",   Kind::PrintLine},

    {"&&",          Kind::LogicalAnd},
    {"||",          Kind::LogicalOr},
    {"!",           Kind::LogicalNot},

    {"=",           Kind::Assignment},

    {"+",           Kind::Add},
    {"-",           Kind::Subtract},
    {"*",           Kind::Asterisk},
    {"/",           Kind::Divide},
    {"%",           Kind::Modulo},
    {"&",           Kind::Ampersand},

    {"==",          Kind::Equal},
    {"!=",          Kind::NotEqual},
    {"<",           Kind::LessThan},
    {">",           Kind::GreaterThan},
    {"<=",          Kind::LessOrEqual},
    {">=",          Kind::GreaterOrEqual},

    {".",           Kind::Dot},
    {"->",          Kind::Arrow},

    {",",           Kind::Comma},
    {":",           Kind::Colon},
    {";",           Kind::Semicolon},
    {"(",           Kind::LeftParen},
    {")",           Kind::RightParen},
    {"{",           Kind::LeftBrace},
    {"}",           Kind::RightBrace},
    {"[",           Kind::LeftBraket},
    {"]",           Kind::RightBraket},
};

static auto KindtoString = [] {
    map<Kind, string> result;
    for (auto& [key, value] : stringToKind)
        result[value] = key;
    return result;
}();

auto toKind(string string)->Kind {
    if (stringToKind.count(string))
        return stringToKind.at(string);
    return Kind::Unknown;
}

auto toString(Kind type)->string {
    if (KindtoString.count(type))
        return KindtoString.at(type);
    return "";
}

auto operator<<(ostream& stream, Token& token)->ostream& {
    return stream << setw(12) << left << toString(token.kind) << token.string;
}
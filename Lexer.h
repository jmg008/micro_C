#include <vector>
#include <string>
#include "Token.h"

using std::string;
using std::vector;

enum class CharType {
    Unknown,
    WhiteSpace,
    NumberLiteral,
    StringLiteral,
    CharLiteral,
    IdentifierAndKeyword,
    OperatorAndPunctuator
};

auto scanNumberLiteral(string::iterator&)->Token;
auto scanStringLiteral(string::iterator&)->Token;
auto scanCharLiteral(string::iterator&)->Token;
auto scanIdentifierAndKeyword(string::iterator&)->Token;
auto scanOperatorAndPunctuator(string::iterator&)->Token;
auto getCharType(char)->CharType;
auto isCharType(char, CharType)->bool;
auto convertToEscape(char)->char;

auto scan(string)->vector<Token>;
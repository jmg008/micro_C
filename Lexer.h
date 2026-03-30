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
    IdentifierAndKeyword,
    OperatorAndPunctuator
};

auto scanNumberLiteral(string::iterator&)->Token;
auto scanStringLiteral(string::iterator&)->Token;
auto scanIdentifierAndKeyword(string::iterator&)->Token;
auto scanOperatorAndPunctuator(string::iterator&)->Token;
auto getCharType(char)->CharType;
auto isChartype(char, CharType)->bool;

auto scan(string)->vector<Token>;
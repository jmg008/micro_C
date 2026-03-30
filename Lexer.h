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

auto scanNumberLiteral()->Token;
auto scanStringLiteral()->Token;
auto scanIdentifierAndKeyword()->Token;
auto scanOperatorAndPunctuator()->Token;
auto getCharType(char)->CharType;
auto isChartype(char, CharType)->bool;

static string::iterator current;

auto scan(string)->vector<Token>;
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

class Lexer
{
private:
    static string::iterator current;
    auto scanNumberLiteral()->Token;
    auto scanStringLiteral()->Token;
    auto scanCharLiteral()->Token;
    auto scanIdentifierAndKeyword()->Token;
    auto scanOperatorAndPunctuator()->Token;
public:
    auto scan(string)->vector<Token>;
};


auto getCharType(char)->CharType;
auto isCharType(char, CharType)->bool;
auto convertToEscape(char)->char;
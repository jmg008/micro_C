#include <iostream>
#include <string>
#include "Token.h"
#include "Lexer.h"

using namespace std;

auto scan(string sourceCode)->vector<Token> {
    vector<Token> result;
    sourceCode += '\0';
    current = sourceCode.begin();
    while (*current != '\0') {
        switch (getCharType(*current)) {
        case CharType::WhiteSpace: {
            current += 1;
            break;
        }
        case CharType::NumberLiteral: {
            result.push_back(scanNumberLiteral());
            break;
        }
        case CharType::StringLiteral: {
            result.push_back(scanStringLiteral());
            break;
        }
        case CharType::IdentifierAndKeyword: {
            result.push_back(scanIdentifierAndKeyword());
            break;
        }
        case CharType::OperatorAndPunctuator: {
            result.push_back(scanOperatorAndPunctuator());
            break;
        }
        default:
            cout << *current << " 사용할 수 없는 문자.";
            exit(1);
        }
    }
}

auto scanNumberLiteral()->Token {
    string string;
    while (isChartype(*current, CharType::NumberLiteral))
        string += *current++;
    if (*current == '.') {
        string += *current++;
        while (isChartype(*current, CharType::NumberLiteral))
            string += *current++;
    }
    return Token{Kind::NumberLiteral, string};
}

auto scanStringLiteral()->Token {
    string string;
    while (isChartype(*current, CharType::StringLiteral))
        string += *current++;
    if (*current != '\'') {
        cout << "문자열이 안 끝남.";
        exit(1);
    }
    current++;
    return Token{Kind::StringLiteral, string};
}

auto scanIdentifierAndKeyowrd()->Token {
    string string;
    while (isChartype(*current, CharType::IdentifierAndKeyword))
        string += *current++;
    while (string.empty() == false && toKind(string) == Kind::Unknown) {
        string.pop_back();
        current--;
    }
    if (string.empty()) {
        cout << *current << " 사용할 수 없는 문자.";
        exit(1);
    }
    return Token{toKind(string), string};
}

auto getCharType(char c)->CharType {
    if (' ' == c || '\t' == c || '\r' == c || '\n' == c) {
        return CharType::WhiteSpace;
    }
    if ('0' <= c && c <= '9') {
        return CharType::NumberLiteral;
    }
    if (c == '\'') {
        return CharType::StringLiteral;
    }
    if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
        return CharType::IdentifierAndKeyword;
    }
}
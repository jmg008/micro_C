#include <iostream>
#include <string>
#include "Token.h"
#include "Lexer.h"

using namespace std;

auto scan(string sourceCode)->vector<Token> {
    vector<Token> result;
    sourceCode += '\0';
    string::iterator current = sourceCode.begin();
    while (*current != '\0') {
        switch (getCharType(*current)) {
        case CharType::WhiteSpace: {
            current += 1;
            break;
        }
        case CharType::NumberLiteral: {
            result.push_back(scanNumberLiteral(current));
            break;
        }
        case CharType::StringLiteral: {
            result.push_back(scanStringLiteral(current));
            break;
        }
        case CharType::IdentifierAndKeyword: {
            result.push_back(scanIdentifierAndKeyword(current));
            break;
        }
        case CharType::OperatorAndPunctuator: {
            result.push_back(scanOperatorAndPunctuator(current));
            break;
        }
        default:
            cout << *current << " 사용할 수 없는 문자.";
            exit(1);
        }
    }
    result.push_back({Kind::EndOfToken});
    return result;
}

auto scanNumberLiteral(string::iterator& current)->Token {
    string lexeme;
    current++;
    while (isChartype(*current, CharType::NumberLiteral))
        lexeme += *current++;
    if (*current == '.') {
        lexeme += *current++;
        while (isChartype(*current, CharType::NumberLiteral))
            lexeme += *current++;
    }
    return Token{Kind::NumberLiteral, lexeme};
}

auto scanStringLiteral(string::iterator& current)->Token {
    string lexeme;
    while (isChartype(*current, CharType::StringLiteral))
        lexeme += *current++;
    if (*current != '\'') {
        cout << "문자열이 안 끝남.";
        exit(1);
    }
    current++;
    return Token{Kind::StringLiteral, lexeme};
}

auto scanIdentifierAndKeyowrd(string::iterator& current)->Token {
    string lexeme;
    while (isChartype(*current, CharType::IdentifierAndKeyword))
        lexeme += *current++;
    while (lexeme.empty() == false && toKind(lexeme) == Kind::Unknown) {
        lexeme.pop_back();
        current--;
    }
    if (lexeme.empty()) {
        cout << *current << " 사용할 수 없는 문자.";
        exit(1);
    }
    return Token{toKind(lexeme), lexeme};
}

auto scanOperatorAndPunctuator(string::iterator& current)->Token {
    string lexeme;
    while (isCharType(*current, CharType::OperatorAndPunctuator)) {
        lexeme += *current++;
    }
    while (lexeme.empty() == false && toKind(lexeme) == Kind::Unknown) {
        lexeme.pop_back();
        current--;
    }
    if (lexeme.empty()) {
        cout << *current << " 불가용한 문지";
        exit(1);
    }
    return Token{toKind(lexeme), lexeme};
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
    if (33 <= c && c <= 47 && c != '\'' ||
        58 <= c && c <= 64 ||
        91 <= c && c <= 96 ||
        123 <= c && c <= 126) {
        return CharType::OperatorAndPunctuator;
    }
}

auto isCharType(char c, CharType type)->bool {
    switch (type) {
        case CharType::NumberLiteral: {
            return '0' <= c && c <= '9';
        }
        case CharType::StringLiteral: {
            return 32 <= c && c <= 126 && c != '\'';
        }
        case CharType::IdentifierAndKeyword: {
            return '0' <= c && c <= '9' ||
                   'a' <= c && c <= 'z' ||
                   'A' <= c && c <= 'Z';
        }
        case CharType::OperatorAndPunctuator: {
            return 33 <= c && c <= 47 ||
                   58 <= c && c <= 64 ||
                   91 <= c && c <= 96 ||
                   123 <= c && c <= 126;
        }
        default: {
            return false;
        }
    }
}
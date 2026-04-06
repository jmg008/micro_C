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
        case CharType::CharLiteral: {
            result.push_back(scanCharLiteral(current));
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
    Kind kind = Kind::IntLiteral;
    while (isCharType(*current, CharType::NumberLiteral)) {
        lexeme += *current++;
    }
    if (*current == '.') {
        kind = Kind::FloatLiteral;
        lexeme += *current++;
        while (isCharType(*current, CharType::NumberLiteral))
            lexeme += *current++;
    }
    return Token{kind, lexeme};
}

auto scanStringLiteral(string::iterator& current)->Token {
    string lexeme;
    current++;
    while (isCharType(*current, CharType::StringLiteral)) {
        if (*current == '\\') {
            current++;
            if (isCharType(*current, CharType::StringLiteral) || *current == '\'' || *current == '\"') {
                lexeme += convertToEscape(*current++);
            }
        } else {
            lexeme += *current++;
        }
    }
    if (*current != '\"') {
        cout << "문자열이 안 끝남.";
        exit(1);
    }
    current++;
    return Token{Kind::StringLiteral, lexeme};
}

auto scanCharLiteral(string::iterator& current)->Token {
    string lexeme;
    current++;
    if (isCharType(*current, CharType::CharLiteral)) {
        if (*current == '\\') {
            current++;
            if (isCharType(*current, CharType::CharLiteral) || *current == '\'' || *current == '\"') {
                lexeme += convertToEscape(*current++);
            }
        } else {
            lexeme += *current++;
        }
    }
    if (*current != '\'') {
        cout << "문자가 안 끝남.";
        exit(1);
    }
    current++;
    return Token{Kind::CharLiteral, lexeme};
}

auto scanIdentifierAndKeyword(string::iterator& current)->Token {
    string lexeme;
    while (isCharType(*current, CharType::IdentifierAndKeyword)) {
        lexeme += *current++;
    }
    auto kind = toKind(lexeme);
    if (kind == Kind::Unknown) {
        kind = Kind::Identifier;
    }
    return Token{kind, lexeme};
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
    if (c == '\"') {
        return CharType::StringLiteral;
    }
    if (c == '\'') {
        return CharType::CharLiteral;
    }
    if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_') {
        return CharType::IdentifierAndKeyword;
    }
    if (33 <= c && c <= 47 && c != '\'' ||
        58 <= c && c <= 64 ||
        91 <= c && c <= 96 ||
        123 <= c && c <= 126) {
        return CharType::OperatorAndPunctuator;
    }
    return CharType::Unknown;
}

auto isCharType(char c, CharType type)->bool {
    switch (type) {
        case CharType::NumberLiteral: {
            return '0' <= c && c <= '9';
        }
        case CharType::StringLiteral: {
            return 32 <= c && c <= 126 && c != '\'' && c != '\"';
        }
        case CharType::CharLiteral: {
            return 32 <= c && c <= 126 && c != '\'' && c != '\"';
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

char convertToEscape(char c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        case 'v': return '\v';
        case 'f': return '\f';
        case 'a': return '\a';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0': return '\0';
        default:  return c; // 바꿀 게 없으면 그대로 반환
    }
}
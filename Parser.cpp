#include <set>
#include <iostream>
#include <vector>
#include <stack>
#include <stdlib.h>
#include "Parser.h"
#include "Token.h"
#include "Node.h"

using namespace std;

auto Parser::parse()->Program* {
    auto result = new Program();
    current = tokens.begin();
    while (current->kind != Kind::EndOfToken) {
        switch (current->kind) {
        case Kind::Int:
        case Kind::Char:
        case Kind::Void:
            result->declarations.push_back(parseDeclaration());
            break;
        default:
            break;
        }
    }

    return result;
}

auto Parser::parseDeclaration()->Declaration* {
    string name;
    auto type = parseType(name);

    if (current->kind == Kind::LeftParen) {
        auto res = new Function();
        res->name = name;
        res->type = type;
        skipCurrent();
        res->parameters = parseParameter();

        if (current->kind == Kind::LeftBrace) {
            res->block = parseBlock();
        } else {
            skipCurrent(Kind::Semicolon);
        }
        return res;
    } else {
        auto res = new GrobalVariable();
        res->name = name;
        res->type = type;
        if (current->kind == Kind::Assignment) {
            // 구현예정
        } else {
            skipCurrent(Kind::Semicolon);
        }
        return res;
    }
}

auto Parser::parseType(string& name)->Type* {
    auto res = new Type{current->kind};
    skipCurrent();

    auto pType = parseType_reg(name);
    
    while (!pType.empty()) {
        int cur = pType.top();
        pType.pop();
        if (cur == -1) {
            auto tmp = new Pointer();
            tmp->sub = res;
            res = tmp;
        } else {
            auto tmp = new Array();
            tmp->sub = res;
            tmp->length = cur;
            res = tmp;
        }
    }
    
    return res;
}

auto Parser::parseType_reg(string& name)->stack<int> {
    stack<int> res;
    int pointer = 0;
    while (skipCurrentIf(Kind::Asterisk)) {
        pointer++;
    }
    
    if (current->kind == Kind::LeftParen) {
        skipCurrent();
        res = parseType_reg(name);
        skipCurrent(Kind::RightParen);
    } else if (current->kind == Kind::Identifier) {
        name = current->string;
        skipCurrent();
    }

    while (skipCurrentIf(Kind::LeftBracket)) {
        int num = atoi(current->string.c_str());
        skipCurrent(Kind::IntegerLiteral);
        skipCurrent(Kind::RightBracket);
        res.push(num);
    }
    for (int i=0;i<pointer;i++) {
        res.push(-1);
    }

    return res;
}

auto Parser::parseParameter()->vector<pair<Type*, string>> {
    vector<pair<Type*, string>> res;

    while (!(current->kind == Kind::RightParen)) {
        string name;
        auto pType = parseType(name);
        res.emplace_back();
        res.back().first = pType;
        res.back().second = name;
        if (!skipCurrentIf(Kind::Comma)) {
            if (current->kind != Kind::RightParen) {
                throw ",가 필요함";
            }
        }
    }
    skipCurrent();

    return res;
}

auto Parser::parseBlock()->vector<Statement*> {
    vector<Statement*> res;
    skipCurrent(Kind::LeftBrace);
    while (current->kind != Kind::RightBrace) {
        switch (current->kind) {
        default: 
        }
    }
}

auto Parser::skipCurrent()->void {
    current++;
}

auto Parser::skipCurrent(Kind kind)->void {
    if (current->kind != kind) {
        cout << toString(kind) << " 토큰이 필요합니다.";
        exit(1);
    }
    current++;
}
auto Parser::skipCurrentIf(Kind kind)->bool {
    if (current->kind != kind) {
        return false;
    }
    current++;
    return true;
}
#include <set>
#include <iostream>
#include <vector>
#include "Parser.h"
#include "Token.h"
#include "Node.h"

using namespace std;

auto Parser::parse()->Program* {
    auto result = new Program();
    vector<Token>::iterator current = tokens.begin();
    while (current->kind != Kind::EndOfToken) {
        switch (current->kind) {
        case Kind::Int:
        case Kind::Char:
        case Kind::Void:
            parseDeclaration();
            break;
        default:
            break;
        }
    }
}

auto Parser::parseDeclaration()->Type* {
    auto result = new Type();
    
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
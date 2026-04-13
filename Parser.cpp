#include <set>
#include <iostream>
#include <vector>
#include "Token.h"
#include "Node.h"

using std::vector;

auto parse(vector<Token> tokens)->Program* {
    auto result = new Program();
    vector<Token>::iterator current = tokens.begin();
    while (current->kind != Kind::EndOfToken) {
        switch (current->kind) {
        case Kind::Function: {
            parseFunction(current);
        }
        default:
            break;
        }
    }
}

auto parseFunction(vector<Token>::iterator& current)->Function* {
    auto result = new Function();

    return result;
}


auto skipCurrent(vector<Token>::iterator& current)->void {
    current++;
}
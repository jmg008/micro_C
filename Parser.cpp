#include <set>
#include <iostream>
#include <vector>
#include "Parser.h"
#include "Token.h"
#include "Node.h"

using std::vector;

auto Parser::parse()->Program* {
    auto result = new Program();
    vector<Token>::iterator current = tokens.begin();
    while (current->kind != Kind::EndOfToken) {
        switch (current->kind) {
        
        default:
            break;
        }
    }
}


auto Parser::skipCurrent()->void {
    current++;
}
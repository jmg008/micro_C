#include <vector>
#include <stack>
#include "Token.h"
#include "Node.h"

using namespace std;

class Parser
{
private:
    vector<Token> tokens;
    vector<Token>::iterator current;

    auto parseBlock()->vector<Statement*>;
    auto parseDeclaration()->Declaration*;
    auto parseType(string&)->Type*;
    auto parseType_reg(string&)->stack<int>;
    auto parseParameter()->vector<pair<Type*, string>>;
    auto parseExpressionStatement()->Expression*;
    auto parseVariable();
    auto parseFor()->For*;
    auto parseIf()->If*;
    auto parseReturn()->Return*;
    auto parseContinue()->Continue*;
    auto parseBreak()->Break*;

    auto skipCurrent()->void;
    auto skipCurrent(Kind)->void;
    auto skipCurrentIf(Kind)->bool;
public:
    auto parse()->Program*;
};
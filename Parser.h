#include <vector>
#include "token.h"
#include "Node.h"

using std::vector;

auto parseFunction(vector<Token>::iterator&)->Function*;
auto parseBlock(vector<Token>::iterator&)->vector<Statement*>;
auto parseVariable(vector<Token>::iterator&)->Variable*;
auto parseFor(vector<Token>::iterator&)->For*;
auto parseIf(vector<Token>::iterator&)->If*;
auto parsePrint(vector<Token>::iterator&)->Print*;
auto parseReturn(vector<Token>::iterator&)->Return*;
auto parseBreak(vector<Token>::iterator&)->Break*;
auto parseContinue(vector<Token>::iterator&)->Continue*;
auto parseExpressionStatement(vector<Token>::iterator&)->ExpressionStatement*;
auto parseExpression(vector<Token>::iterator&)->Expression*;
auto parseAssigment(vector<Token>::iterator&)->Expression*;
auto parseOr(vector<Token>::iterator&)->Expression*;
auto parseAnd(vector<Token>::iterator&)->Expression*;
auto parseRelational(vector<Token>::iterator&)->Expression*;
auto parseArithmetic1(vector<Token>::iterator&)->Expression*;
auto parseArithmetic2(vector<Token>::iterator&)->Expression*;
auto parseUnary(vector<Token>::iterator&)->Expression*;
auto parseOperand(vector<Token>::iterator&)->Expression*;
auto parseNullLiteral(vector<Token>::iterator&)->Expression*;
auto parseBooleanLiteral(vector<Token>::iterator&)->Expression*;
auto parseIntLiteral(vector<Token>::iterator&)->Expression*;
auto parseFloatLiteral(vector<Token>::iterator&)->Expression*;
auto parseStringLiteral(vector<Token>::iterator&)->Expression*;
auto parseCharLiteral(vector<Token>::iterator&)->Expression*;
auto parseArrayLiteral(vector<Token>::iterator&)->Expression*;

auto skipCurrent(vector<Token>::iterator&)->void;
auto skipCurrent(vector<Token>::iterator&, Kind)->void;
auto skipCurrentIf(vector<Token>::iterator&, Kind)->bool;

auto parse(vector<Token>)->Program*;
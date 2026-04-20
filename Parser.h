#include <vector>
#include "Token.h"
#include "Node.h"

using std::vector;

class Parser
{
private:
    vector<Token> tokens;
    vector<Token>::iterator current;

    auto parseDefinition()->void;

    auto skipCurrent()->void;
    auto skipCurrent(Kind)->void;
    auto skipCurrentIf(Kind)->bool;
public:
    auto parse()->Program*;
};
#include "syntax_rules.h"

namespace ParseTree {

    namespace Rules {
        Rule TERM;
        Rule EXPRESSION;
    }

    void CreateRules() {
        using namespace Rules;
    
        TERM << TokenType::KEYWORD << TokenType::VARIABLE_IDENTIFIER;

        EXPRESSION << &TERM << TokenType::ASSIGNMENT_OPERATOR << &TERM;
    }
}
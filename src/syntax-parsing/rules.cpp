#include "syntax_rules.h"

namespace ParseTree {

    namespace Rules {
        Rule TERM;
        Rule EXPRESSION;
    }

    void CreateRules() {
        using namespace Rules;
    
        TERM << TokenType::KEYWORD << D_SBST << TokenType::VARIABLE_IDENTIFIER << D_OR << TokenType::ASSIGNMENT_OPERATOR << D_SBED << D_OPST << TokenType::BRACKET << D_OPED;

        EXPRESSION << &TERM << TokenType::ASSIGNMENT_OPERATOR << &TERM;
    }
}
#include "syntax_rules.h"

namespace ParseTree {

    namespace Rules {
        Rule TERM;
        Rule EXPRESSION;
    }

    void CreateRules() {
    
        Rule* testRule = CreateRule(0);
        *testRule << D_SBST << D_SBED;
    }
}
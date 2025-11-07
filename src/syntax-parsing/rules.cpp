#include "syntax_rules.h"

// macros for increased readability
#define T TokenType
#define OR << D_OR <<

namespace ParseTree {

    namespace Rules {
        Rule TOP_STATEMENT_SEQUENCE("Top-Level Statements");
        Rule SCOPE_STATEMENT_SEQUENCE("Scope-Level Statements");
        Rule STATEMENT("Statement");
        Rule SCOPE("Scope");
        Rule STATEMENT_TERMINATOR("Statement Terminator (;)");
        Rule CONTROL_SEQUENCE("Control Sequence");        
        Rule TYPE_IDENTIFIER("Type Identifier");
        Rule FUNCTION_DECLARATION("Function Declaration");
        Rule VARIABLE_DECLARATION("Variable Declaration");
        Rule ASSIGNMENT("Assignment Operation");
        Rule ENUMERATION_EXPRESSIONS("Expressions"); 
        Rule ENUMERATION_TYPED_IDENTIFIERS("Typed Identifiers");
        Rule FUNCITON_CALL("Function Call");
        Rule EXPRESSION("Expression");
        Rule TERM("Term");

        Rule SCOPE_END("Scope End");
    }

    void CreateRules() {
        using namespace Rules;

        /* 
        Order is important here. The parser exits and moves up
        the syntax tree immediately if it succesfully parses a
        rule. Thus, make sure to try and match the longest possible
        definition of a rule first
        
        */

        TOP_STATEMENT_SEQUENCE << &STATEMENT << D_SBST << T::END_OF_FILE OR &TOP_STATEMENT_SEQUENCE << D_SBED;
        SCOPE_STATEMENT_SEQUENCE << &STATEMENT << D_OPST << &SCOPE_STATEMENT_SEQUENCE << D_OPED;

        STATEMENT << D_SBST << &CONTROL_SEQUENCE OR &VARIABLE_DECLARATION OR &ASSIGNMENT OR &EXPRESSION << D_SBED << &STATEMENT_TERMINATOR;
        SCOPE << T::BRACKET_CURLY_LEFT << &SCOPE_STATEMENT_SEQUENCE << T::BRACKET_CURLY_RIGHT;
        STATEMENT_TERMINATOR << D_SBST << T::STATEMENT_TERMINATOR << D_SBED;
        
        CONTROL_SEQUENCE << D_SBST << T::KEYWORD_IF OR T::KEYWORD_WHILE << D_SBED << D_RSUC << D_SBST << T::BRACKET_NORMAL_LEFT << &EXPRESSION << T::BRACKET_NORMAL_RIGHT << D_SBED << &SCOPE;

        ENUMERATION_EXPRESSIONS << D_SBST << &EXPRESSION << D_SBED << D_OPST << T::ELEMENT_SEPARATOR << &ENUMERATION_EXPRESSIONS << D_OPED;
        ENUMERATION_TYPED_IDENTIFIERS << D_SBST << &TYPE_IDENTIFIER << T::IDENTIFIER << D_SBED << D_OPST << T::ELEMENT_SEPARATOR << &ENUMERATION_TYPED_IDENTIFIERS << D_OPED;

        EXPRESSION << &TERM << D_OPST << D_SBST << T::RELATIONAL_OPERATOR OR T::BINARY_OPERATOR << D_SBED << &EXPRESSION << D_OPED;
        TERM << D_SBST << T::IDENTIFIER OR T::LITERAL_FLOAT OR T::LITERAL_INTEGER OR &FUNCITON_CALL OR T::BRACKET_NORMAL_LEFT << &EXPRESSION << T::BRACKET_NORMAL_RIGHT << D_SBED;

        TYPE_IDENTIFIER << T::IDENTIFIER << D_OPST << T::BRACKET_NORMAL_LEFT << &ENUMERATION_EXPRESSIONS << T::BRACKET_NORMAL_RIGHT << D_OPED;

        FUNCITON_CALL << T::IDENTIFIER << T::BRACKET_NORMAL_LEFT << &ENUMERATION_EXPRESSIONS << T::BRACKET_NORMAL_RIGHT;
        FUNCTION_DECLARATION << T::IDENTIFIER << T::IDENTIFIER << T::BRACKET_NORMAL_LEFT << &ENUMERATION_TYPED_IDENTIFIERS << &SCOPE; 
        
        VARIABLE_DECLARATION << &TYPE_IDENTIFIER << D_SBST << &ASSIGNMENT OR T::IDENTIFIER << D_SBED; 
        ASSIGNMENT << T::IDENTIFIER << T::ASSIGNMENT_OPERATOR << &EXPRESSION;


        // add recovery rules
        STATEMENT.AddRecoveryRule(&STATEMENT_TERMINATOR, 99999); // goto the end of the definition
        TOP_STATEMENT_SEQUENCE.AddRecoveryRule(&STATEMENT_TERMINATOR, 0); // try to parse the first statement again
        TOP_STATEMENT_SEQUENCE.throwSyntaxErrors = false;
        TOP_STATEMENT_SEQUENCE.requireTotalSuccess = true;

        SCOPE_END << T::BRACKET_CURLY_LEFT;
        SCOPE_STATEMENT_SEQUENCE.AddRecoveryRule(&STATEMENT_TERMINATOR, 0);
        SCOPE_STATEMENT_SEQUENCE.AddRecoveryRule(&SCOPE_END, 99999);
    }
}
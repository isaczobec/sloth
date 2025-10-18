#include "syntax_rules.h"

// macros for increased readability
#define T TokenType
#define OR << D_OR <<

namespace ParseTree {

    namespace Rules {
        Rule STATEMENT;
        Rule SCOPE;
        Rule CONTROL_SEQUENCE;        
        Rule TYPE_IDENTIFIER;
        Rule FUNCTION_DECLARATION;
        Rule VARIABLE_DECLARATION;
        Rule ASSIGNMENT;
        Rule ENUMERATION_EXPRESSIONS; 
        Rule ENUMERATION_TYPED_IDENTIFIERS;
        Rule FUNCITON_CALL;
        Rule EXPRESSION;
        Rule TERM;
    }

    void CreateRules() {
        using namespace Rules;

        /* 
        Order is important here. The parser exits and moves up
        the syntax tree immediately if it succesfully parses a
        rule. Thus, make sure to try and match the longest possible
        definition of a rule first
        
        */

        STATEMENT << D_SBST << &CONTROL_SEQUENCE OR &VARIABLE_DECLARATION OR &ASSIGNMENT OR &EXPRESSION << D_SBED << T::STATEMENT_TERMINATOR << D_OPST << &STATEMENT OR &SCOPE << D_OPED;
        SCOPE << T::BRACKET_CURLY_LEFT << &STATEMENT << T::BRACKET_CURLY_RIGHT;
        
        CONTROL_SEQUENCE << D_SBST << T::KEYWORD_IF OR T::KEYWORD_WHILE << D_SBED << &EXPRESSION << &SCOPE;

        ENUMERATION_EXPRESSIONS << D_SBST << &EXPRESSION << D_SBED << D_OPST << T::ELEMENT_SEPARATOR << &ENUMERATION_EXPRESSIONS << D_OPED;
        ENUMERATION_TYPED_IDENTIFIERS << D_SBST << &TYPE_IDENTIFIER << T::IDENTIFIER << D_SBED << D_OPST << T::ELEMENT_SEPARATOR << &ENUMERATION_TYPED_IDENTIFIERS << D_OPED;

        EXPRESSION << &TERM << D_OPST << D_SBST << T::RELATIONAL_OPERATOR OR T::BINARY_OPERATOR << D_SBED << &EXPRESSION << D_OPED;
        TERM << D_SBST << T::IDENTIFIER OR T::LITERAL_FLOAT OR T::LITERAL_INTEGER OR &FUNCITON_CALL OR T::BRACKET_NORMAL_LEFT << &EXPRESSION << T::BRACKET_NORMAL_RIGHT << D_SBED;

        TYPE_IDENTIFIER << T::IDENTIFIER << D_OPST << T::BRACKET_NORMAL_LEFT << &ENUMERATION_EXPRESSIONS << T::BRACKET_NORMAL_RIGHT << D_OPED;

        FUNCITON_CALL << T::IDENTIFIER << T::BRACKET_NORMAL_LEFT << &ENUMERATION_EXPRESSIONS << T::BRACKET_NORMAL_RIGHT;
        FUNCTION_DECLARATION << T::IDENTIFIER << T::IDENTIFIER << T::BRACKET_NORMAL_LEFT << &ENUMERATION_TYPED_IDENTIFIERS << &SCOPE; 
        
        VARIABLE_DECLARATION << &TYPE_IDENTIFIER << D_SBST << T::IDENTIFIER OR &ASSIGNMENT << D_SBED; 
        ASSIGNMENT << T::IDENTIFIER << T::ASSIGNMENT_OPERATOR << &EXPRESSION;
    }
}
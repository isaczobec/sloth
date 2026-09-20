#include "syntax_rules.h"

// macros for increased readability
#define T TokenType
#define OR << D_OR <<

namespace ParseTree {

    namespace Rules {

        // program structure
        Rule TOP_STATEMENT_SEQUENCE("Top-Level Statements");
        Rule TOP_STATEMENT("Top-Level Statement");
        Rule SCOPE_STATEMENT_SEQUENCE("Scope-Level Statements");
        Rule STATEMENT("Statement");
        Rule SCOPE("Scope");
        Rule STATEMENT_TERMINATOR("Statement Terminator (;)");
        Rule SCOPE_END("Scope End (})");

        // control flow
        Rule CONTROL_SEQUENCE("Control Sequence");
        Rule CONDITION("Condition");
        Rule ELSE_CLAUSE("Else Clause");
        Rule RETURN_STATEMENT("Return Statement");

        // types
        Rule TYPE("Type");
        Rule TYPE_CORE("Type");
        Rule NAMED_TYPE("Named Type");
        Rule GENERIC_ARGUMENTS("Generic Arguments");
        Rule TYPE_ENUMERATION("Type List");
        Rule PARAMETER("Parameter");
        Rule PARAMETER_ENUMERATION("Parameter List");
        Rule GENERIC_PARAMETERS("Generic Parameters");
        Rule GENERIC_PARAMETER("Generic Parameter");
        Rule GENERIC_CONSTRAINT("Generic Parameter Constraint");
        Rule GENERIC_PARAMETER_ENUMERATION("Generic Parameter List");

        // declarations
        Rule DECLARATION("Declaration");
        Rule DECLARATION_NAME("Declared Name");
        Rule DECLARATION_TAIL("Declaration Body");
        Rule CLASS_DECLARATION("Class Declaration");
        Rule INTERFACE_DECLARATION("Interface Declaration");
        Rule CLASS_BODY("Class Body");
        Rule ADT_VARIANT("Algebraic Data Type Variant");
        Rule ADT_VARIANT_ENUMERATION("Algebraic Data Type Variants");
        Rule MEMBER_BLOCK("Member Block");
        Rule MEMBER_SEQUENCE("Member Declarations");
        Rule MEMBER("Member Declaration");

        // definition clauses (pattern matching)
        Rule FUNCTION_CLAUSE("Function Definition Clause");
        Rule INFIX_CLAUSE("Infix Definition Clause");
        Rule PATTERN("Pattern");
        Rule PATTERN_ENUMERATION("Pattern List");
        Rule DEFINITION_BODY("Definition Body");
        Rule GUARDED_EXPRESSION("Guarded Expression");
        Rule GUARD_CLAUSE("Guard Clause");

        // statements
        Rule ASSIGNMENT("Assignment Operation");
        Rule SUBSTITUTION("Substitution Operation");
        Rule DESIGNATOR("Assignment Target");

        // expressions
        Rule EXPRESSION("Expression");
        Rule OPERATOR("Operator");
        Rule TERM("Term");
        Rule PRIMARY("Primary Expression");
        Rule POSTFIX("Postfix");
        Rule POSTFIX_CHAIN("Postfix");
        Rule ENUMERATION_EXPRESSIONS("Expressions");
        Rule CONSTRUCTION_LITERAL("Construction Literal");
    }

    void CreateRules() {
        using namespace Rules;

        /*
        Order is important here. The parser exits and moves up
        the syntax tree immediately if it succesfully parses a
        rule. Thus, make sure to try and match the longest possible
        definition of a rule first.

        There is also no support for left recursion: a rule may only
        refer to itself after it has consumed at least one token.
        Every repetition below is therefore written as right recursion
        inside an optional group.
        */

        // =====================================================================
        // program structure
        // =====================================================================

        TOP_STATEMENT_SEQUENCE << &TOP_STATEMENT << D_SBST << T::END_OF_FILE OR &TOP_STATEMENT_SEQUENCE << D_SBED;

        /* Everything that may appear at file scope. Declarations are tried before
           bare statements so that "int a := 4;" is read as a declaration rather than
           as an expression followed by junk. */
        TOP_STATEMENT << D_SBST
            << &CLASS_DECLARATION
            OR &INTERFACE_DECLARATION
            OR &DECLARATION
            OR &FUNCTION_CLAUSE
            OR &INFIX_CLAUSE
            OR &STATEMENT
            << D_SBED;

        SCOPE_STATEMENT_SEQUENCE << &STATEMENT << D_OPST << &SCOPE_STATEMENT_SEQUENCE << D_OPED;

        /* The statement terminator is part of each alternative rather than being
           appended to all of them, because the block bodied forms (control sequences,
           and declarations whose body is a scope) do not require one. */
        STATEMENT << D_SBST
            << &CONTROL_SEQUENCE
            OR &DECLARATION
            OR &FUNCTION_CLAUSE
            OR &INFIX_CLAUSE
            OR &RETURN_STATEMENT << &STATEMENT_TERMINATOR
            OR &SUBSTITUTION << &STATEMENT_TERMINATOR
            OR &ASSIGNMENT << &STATEMENT_TERMINATOR
            OR &EXPRESSION << &STATEMENT_TERMINATOR
            << D_SBED;

        /* The scope body is optional so that "{}" parses, which is needed for empty
           ADT variants such as "Nothing {}". Once the opening brace has been matched
           we are committed, so D_RSUC turns any later failure into a real syntax error
           instead of a silent backtrack. */
        SCOPE << T::BRACKET_CURLY_LEFT << D_RSUC << D_OPST << &SCOPE_STATEMENT_SEQUENCE << D_OPED << T::BRACKET_CURLY_RIGHT;

        STATEMENT_TERMINATOR << D_SBST << T::STATEMENT_TERMINATOR << D_SBED;
        SCOPE_END << T::BRACKET_CURLY_RIGHT;

        // =====================================================================
        // control flow
        // =====================================================================

        CONTROL_SEQUENCE << D_SBST << T::KEYWORD_IF OR T::KEYWORD_WHILE << D_SBED << D_RSUC
            << T::BRACKET_NORMAL_LEFT << &CONDITION << T::BRACKET_NORMAL_RIGHT << &SCOPE
            << D_OPST << &ELSE_CLAUSE << D_OPED
            << D_OPST << T::STATEMENT_TERMINATOR << D_OPED;

        /* "if (Node next)" both tests that next is the Node variant and binds it at
           that type inside the scope, so a condition may be a subtype test as well as
           an ordinary expression. The subtype test is tried first because it is longer. */
        CONDITION << D_SBST << &TYPE << T::IDENTIFIER OR &EXPRESSION << D_SBED;

        ELSE_CLAUSE << T::KEYWORD_ELSE << D_RSUC << D_SBST << &CONTROL_SEQUENCE OR &SCOPE << D_SBED;

        RETURN_STATEMENT << T::KEYWORD_RETURN << D_OPST << &EXPRESSION << D_OPED;

        // =====================================================================
        // types
        // =====================================================================

        TYPE << &TYPE_CORE << D_OPST << T::REFERENCE_OPERATOR << D_OPED;

        /* The function type alternative recurses into TYPE on its right hand side,
           which is what makes '->' right associative:
           "(int a) -> (int b) -> (int c)" is "(int a) -> ((int b) -> (int c))".
           The tuple type alternative has to come second, since it is a prefix of it. */
        TYPE_CORE << D_SBST
            << T::BRACKET_NORMAL_LEFT << &PARAMETER_ENUMERATION << T::BRACKET_NORMAL_RIGHT << T::ARROW << &TYPE
            OR T::BRACKET_NORMAL_LEFT << &PARAMETER_ENUMERATION << T::BRACKET_NORMAL_RIGHT
            OR T::KEYWORD_VOID
            OR &NAMED_TYPE
            << D_SBED;

        NAMED_TYPE << T::IDENTIFIER << D_OPST << &GENERIC_ARGUMENTS << D_OPED;
        GENERIC_ARGUMENTS << T::BRACKET_ANGLE_LEFT << &TYPE_ENUMERATION << T::BRACKET_ANGLE_RIGHT;
        TYPE_ENUMERATION << &TYPE << D_OPST << T::ELEMENT_SEPARATOR << &TYPE_ENUMERATION << D_OPED;

        /* Parameter names are optional, so that "(int, float) -> float" and
           "(int u, float v) -> float" are both accepted, and so that the same rule
           can describe a tuple type. */
        PARAMETER << &TYPE << D_OPST << T::IDENTIFIER << D_OPED;
        PARAMETER_ENUMERATION << &PARAMETER << D_OPST << T::ELEMENT_SEPARATOR << &PARAMETER_ENUMERATION << D_OPED;

        GENERIC_PARAMETERS << T::BRACKET_ANGLE_LEFT << &GENERIC_PARAMETER_ENUMERATION << T::BRACKET_ANGLE_RIGHT;
        GENERIC_PARAMETER_ENUMERATION << &GENERIC_PARAMETER << D_OPST << T::ELEMENT_SEPARATOR << &GENERIC_PARAMETER_ENUMERATION << D_OPED;
        GENERIC_PARAMETER << T::IDENTIFIER << D_OPST << T::TYPE_CONSTRAINT << &GENERIC_CONSTRAINT << D_OPED;

        /* A single constraint needs no brackets, several are grouped in square brackets:
               <T : Comparable<T>>
               <T : [Comparable<T>, Equatable<T>], U>
           Without the brackets the comma would be ambiguous, since it is also what
           separates one generic parameter from the next. */
        GENERIC_CONSTRAINT << D_SBST
            << T::BRACKET_SQUARE_LEFT << &TYPE_ENUMERATION << T::BRACKET_SQUARE_RIGHT
            OR &TYPE
            << D_SBED;

        // =====================================================================
        // declarations
        // =====================================================================

        /* One rule covers all of the equivalent declaration forms:
               int a := 4;
               int b;
               (int, float) -> float f1(u, v) := u + v;
               float f1_2(int u, float v) := u + v;
               (int u, float v) -> float f1_3 := u + v;
               (int, int) -> int +++;
               impure (BinaryTree&, T) -> BinaryTree& insert;
        */
        DECLARATION
            << D_OPST << &GENERIC_PARAMETERS << D_OPED
            << D_OPST << T::KEYWORD_PRIVATE << D_OPED
            << D_OPST << T::KEYWORD_IMPURE << D_OPED
            << &TYPE << &DECLARATION_NAME
            << D_OPST << T::BRACKET_NORMAL_LEFT << &PATTERN_ENUMERATION << T::BRACKET_NORMAL_RIGHT << D_OPED
            << &DECLARATION_TAIL;

        /* A declared name is either an identifier or an operator symbol, which is how
           a user defined infix operator such as "+++" gets declared. */
        DECLARATION_NAME << D_SBST << T::IDENTIFIER OR T::BINARY_OPERATOR OR T::RELATIONAL_OPERATOR << D_SBED;

        /* ':=' fixes the definition, '=' declares a rebindable variable, a bare scope is
           the class member function form, and a lone ';' leaves the value indeterminate. */
        DECLARATION_TAIL << D_SBST
            << T::DEFINITION_OPERATOR << &DEFINITION_BODY
            OR T::ASSIGNMENT_OPERATOR << &DEFINITION_BODY
            OR &SCOPE << D_OPST << T::STATEMENT_TERMINATOR << D_OPED
            OR T::STATEMENT_TERMINATOR
            << D_SBED;

        CLASS_DECLARATION
            << D_OPST << &GENERIC_PARAMETERS << D_OPED
            << D_OPST << T::KEYWORD_PRIVATE << D_OPED
            << T::KEYWORD_CLASS << D_RSUC << T::IDENTIFIER
            << D_OPST << T::DEFINITION_OPERATOR << D_OPED
            << &CLASS_BODY
            << D_OPST << T::STATEMENT_TERMINATOR << D_OPED;

        INTERFACE_DECLARATION
            << D_OPST << &GENERIC_PARAMETERS << D_OPED
            << T::KEYWORD_INTERFACE << D_RSUC << T::IDENTIFIER
            << D_OPST << T::DEFINITION_OPERATOR << D_OPED
            << &MEMBER_BLOCK
            << D_OPST << T::STATEMENT_TERMINATOR << D_OPED;

        /* A class body is either a plain member block or a list of ADT variants.
           The two are told apart by their first token: a variant list starts with the
           variant name, a plain body starts with '{'. */
        CLASS_BODY << D_SBST << &ADT_VARIANT_ENUMERATION OR &MEMBER_BLOCK << D_SBED;
        ADT_VARIANT_ENUMERATION << &ADT_VARIANT << D_OPST << T::ALTERNATIVE_SEPARATOR << &ADT_VARIANT_ENUMERATION << D_OPED;
        ADT_VARIANT << T::IDENTIFIER << &MEMBER_BLOCK;

        MEMBER_BLOCK << T::BRACKET_CURLY_LEFT << D_RSUC << D_OPST << &MEMBER_SEQUENCE << D_OPED << T::BRACKET_CURLY_RIGHT;
        MEMBER_SEQUENCE << &MEMBER << D_OPST << &MEMBER_SEQUENCE << D_OPED;
        MEMBER << D_SBST << &CLASS_DECLARATION OR &INTERFACE_DECLARATION OR &DECLARATION << D_SBED;

        // =====================================================================
        // definition clauses (pattern matching)
        // =====================================================================

        /* A function that was declared earlier is defined by one or more clauses,
           which are matched against left to right:
               f3(5, 4) := 8;
               f3(u, v) := u == v ? 2.0 | u == v+1 ? 3.0;
               g(First a) := { return a.b; };
        */
        FUNCTION_CLAUSE << &DECLARATION_NAME << T::BRACKET_NORMAL_LEFT << &PATTERN_ENUMERATION << T::BRACKET_NORMAL_RIGHT
                        << T::DEFINITION_OPERATOR << &DEFINITION_BODY;

        /* The same, for an infix operator: "a +++ b := (a + 1) +++ (b - 1);" */
        INFIX_CLAUSE << &PATTERN << &DECLARATION_NAME << &PATTERN << T::DEFINITION_OPERATOR << &DEFINITION_BODY;

        /* A pattern binds a name, optionally narrowing it to an ADT variant or a type
           ("Node n"), or matches a literal value ("f3(5, 4)"). */
        PATTERN << D_SBST
            << &TYPE << T::IDENTIFIER
            OR T::IDENTIFIER
            OR T::LITERAL_FLOAT
            OR T::LITERAL_INTEGER
            OR T::LITERAL_BOOL
            << D_SBED;
        PATTERN_ENUMERATION << &PATTERN << D_OPST << T::ELEMENT_SEPARATOR << &PATTERN_ENUMERATION << D_OPED;

        DEFINITION_BODY << D_SBST
            << &SCOPE << D_OPST << T::STATEMENT_TERMINATOR << D_OPED
            OR &GUARDED_EXPRESSION << &STATEMENT_TERMINATOR
            OR &EXPRESSION << &STATEMENT_TERMINATOR
            << D_SBED;

        GUARDED_EXPRESSION << &GUARD_CLAUSE << D_OPST << T::ALTERNATIVE_SEPARATOR << &GUARDED_EXPRESSION << D_OPED;
        GUARD_CLAUSE << &EXPRESSION << T::GUARD_OPERATOR << &EXPRESSION;

        // =====================================================================
        // statements
        // =====================================================================

        DESIGNATOR << T::IDENTIFIER << D_OPST << T::MEMBER_ACCESS << &DESIGNATOR << D_OPED;
        ASSIGNMENT << &DESIGNATOR << T::ASSIGNMENT_OPERATOR << &EXPRESSION;

        /* "c << b = 1;" substitutes b = 1 into the expression tree of the
           indeterminate value c, without changing b itself. */
        SUBSTITUTION << &DESIGNATOR << T::SUBSTITUTION_OPERATOR << &DESIGNATOR << T::ASSIGNMENT_OPERATOR << &EXPRESSION;

        // =====================================================================
        // expressions
        // =====================================================================

        EXPRESSION << &TERM << D_OPST << &OPERATOR << &EXPRESSION << D_OPED;

        /* '<' and '>' are lexed as angle brackets, so they have to be listed here to
           remain usable as the less-than and greater-than operators. */
        OPERATOR << D_SBST
            << T::BINARY_OPERATOR
            OR T::RELATIONAL_OPERATOR
            OR T::BRACKET_ANGLE_LEFT
            OR T::BRACKET_ANGLE_RIGHT
            << D_SBED;

        TERM << &PRIMARY << D_OPST << &POSTFIX_CHAIN << D_OPED;
        POSTFIX_CHAIN << &POSTFIX << D_OPST << &POSTFIX_CHAIN << D_OPED;

        /* An explicit generic argument list is only accepted when it is immediately
           followed by a call, as in "genericFunction<SomeType>(val)". Without that
           restriction "a < b > c" would be parsed as a generic instantiation of a. */
        POSTFIX << D_SBST
            << T::MEMBER_ACCESS << T::IDENTIFIER
            OR &GENERIC_ARGUMENTS << T::BRACKET_NORMAL_LEFT << D_OPST << &ENUMERATION_EXPRESSIONS << D_OPED << T::BRACKET_NORMAL_RIGHT
            OR T::BRACKET_NORMAL_LEFT << D_OPST << &ENUMERATION_EXPRESSIONS << D_OPED << T::BRACKET_NORMAL_RIGHT
            << D_SBED;

        PRIMARY << D_SBST
            << &CONSTRUCTION_LITERAL
            OR T::BRACKET_NORMAL_LEFT << &ENUMERATION_EXPRESSIONS << T::BRACKET_NORMAL_RIGHT
            OR T::LITERAL_FLOAT
            OR T::LITERAL_INTEGER
            OR T::LITERAL_BOOL
            OR T::IDENTIFIER
            << D_SBED;

        /* Also covers tuple literals such as "(d, c)". */
        ENUMERATION_EXPRESSIONS << &EXPRESSION << D_OPST << T::ELEMENT_SEPARATOR << &ENUMERATION_EXPRESSIONS << D_OPED;

        /* "(Example) {a = 1;}" and "(BinaryTree Node) { value = t; }". The optional
           identifier names which ADT variant is being constructed. The body reuses
           SCOPE, since the sketch allows arbitrary statements inside it. */
        CONSTRUCTION_LITERAL << T::BRACKET_NORMAL_LEFT << &TYPE << D_OPST << T::IDENTIFIER << D_OPED
                             << T::BRACKET_NORMAL_RIGHT << &SCOPE;

        // =====================================================================
        // error recovery
        // =====================================================================

        /*
        Recovery rules are only attached to rules that are "committed" by the time they
        can fail, i.e. rules that are never tried speculatively as one alternative among
        several. Attaching one to a speculative rule would make the parser skip tokens
        looking for a recovery point during ordinary backtracking.

        The second argument is the definition component index to jump to once the
        recovery rule has been parsed. An index past the end of the definition means
        "stop here and return what has been parsed so far".
        */
        constexpr size_t GOTO_END_OF_DEFINITION = 99999;

        // a broken statement is abandoned at its terminating ';'
        STATEMENT.AddRecoveryRule(&STATEMENT_TERMINATOR, GOTO_END_OF_DEFINITION);

        // at file scope, skip to the next ';' and try to parse a whole new statement
        TOP_STATEMENT_SEQUENCE.AddRecoveryRule(&STATEMENT_TERMINATOR, 0);
        TOP_STATEMENT_SEQUENCE.throwSyntaxErrors = true;
        TOP_STATEMENT_SEQUENCE.requireTotalSuccess = true;

        // the same inside a scope and inside a class or interface body
        SCOPE_STATEMENT_SEQUENCE.AddRecoveryRule(&STATEMENT_TERMINATOR, 0);
        MEMBER_SEQUENCE.AddRecoveryRule(&STATEMENT_TERMINATOR, 0);

        /* If the statements inside a block cannot be recovered at a ';', give up on the
           block as a whole and resynchronise on its closing '}'. Both of these rules are
           committed by D_RSUC once their opening brace has been consumed, so a failure
           here is always a genuine syntax error. */
        SCOPE.AddRecoveryRule(&SCOPE_END, GOTO_END_OF_DEFINITION);
        MEMBER_BLOCK.AddRecoveryRule(&SCOPE_END, GOTO_END_OF_DEFINITION);
    }
}

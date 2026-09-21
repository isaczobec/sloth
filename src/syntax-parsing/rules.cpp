#include "syntax_rules.h"

// macros for increased readability
#define T TokenType

/*
Grouping macros. These expand to the raw definition directives, so the grammar below
reads as nested groups instead of as matching pairs of start and end markers.

    ONE_OF(a OR b)     a required group, most often a choice between alternatives.
                       Expands to: D_SBST << a << D_OR << b << D_SBED
    OPTIONAL(a << b)   a group that may be left out entirely.
                       Expands to: D_OPST << a << b << D_OPED
    a OR b             separates the alternatives inside a group.
    a COMMIT b         once a has matched, the rest of the enclosing group must match
                       too: a failure after this point is reported as a syntax error
                       instead of making the parser backtrack. Expands to: a << D_RSUC << b

ONE_OF and OPTIONAL are used like any other component (`<< ONE_OF(...) <<`), while OR
and COMMIT are infix and already contain their own `<<` on both sides.
*/
#define ONE_OF(...)   D_SBST << __VA_ARGS__ << D_SBED
#define OPTIONAL(...) D_OPST << __VA_ARGS__ << D_OPED
#define OR            << D_OR <<
#define COMMIT        << D_RSUC <<

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

        TOP_STATEMENT_SEQUENCE << &TOP_STATEMENT << ONE_OF(T::END_OF_FILE OR &TOP_STATEMENT_SEQUENCE);

        /* Everything that may appear at file scope. Declarations are tried before
           bare statements so that "int a := 4;" is read as a declaration rather than
           as an expression followed by junk. */
        TOP_STATEMENT << ONE_OF(
               &CLASS_DECLARATION
            OR &INTERFACE_DECLARATION
            OR &DECLARATION
            OR &FUNCTION_CLAUSE
            OR &INFIX_CLAUSE
            OR &STATEMENT
        );

        SCOPE_STATEMENT_SEQUENCE << &STATEMENT << OPTIONAL(&SCOPE_STATEMENT_SEQUENCE);

        /* The statement terminator is part of each alternative rather than being
           appended to all of them, because the block bodied forms (control sequences,
           and declarations whose body is a scope) do not require one. */
        STATEMENT << ONE_OF(
               &CONTROL_SEQUENCE
            OR &DECLARATION
            OR &FUNCTION_CLAUSE
            OR &INFIX_CLAUSE
            OR &RETURN_STATEMENT << &STATEMENT_TERMINATOR
            OR &SUBSTITUTION << &STATEMENT_TERMINATOR
            OR &ASSIGNMENT << &STATEMENT_TERMINATOR
            OR &EXPRESSION << &STATEMENT_TERMINATOR
        );

        /* The scope body is optional so that "{}" parses, which is needed for empty
           ADT variants such as "Nothing {}". Once the opening brace has been matched
           we are committed, so any later failure is a real syntax error instead of a
           silent backtrack. */
        SCOPE << T::BRACKET_CURLY_LEFT COMMIT OPTIONAL(&SCOPE_STATEMENT_SEQUENCE) << T::BRACKET_CURLY_RIGHT;

        STATEMENT_TERMINATOR << ONE_OF(T::STATEMENT_TERMINATOR);
        SCOPE_END << T::BRACKET_CURLY_RIGHT;

        // =====================================================================
        // control flow
        // =====================================================================

        CONTROL_SEQUENCE << ONE_OF(T::KEYWORD_IF OR T::KEYWORD_WHILE)
            COMMIT T::BRACKET_NORMAL_LEFT << &CONDITION << T::BRACKET_NORMAL_RIGHT << &SCOPE
            << OPTIONAL(&ELSE_CLAUSE)
            << OPTIONAL(T::STATEMENT_TERMINATOR);

        /* "if (Node next)" both tests that next is the Node variant and binds it at
           that type inside the scope, so a condition may be a subtype test as well as
           an ordinary expression. The subtype test is tried first because it is longer. */
        CONDITION << ONE_OF(&TYPE << T::IDENTIFIER OR &EXPRESSION);

        ELSE_CLAUSE << T::KEYWORD_ELSE COMMIT ONE_OF(&CONTROL_SEQUENCE OR &SCOPE);

        RETURN_STATEMENT << T::KEYWORD_RETURN << OPTIONAL(&EXPRESSION);

        // =====================================================================
        // types
        // =====================================================================

        TYPE << &TYPE_CORE << OPTIONAL(T::REFERENCE_OPERATOR);

        /* The function type alternative recurses into TYPE on its right hand side,
           which is what makes '->' right associative:
           "(int a) -> (int b) -> (int c)" is "(int a) -> ((int b) -> (int c))".
           The tuple type alternative has to come second, since it is a prefix of it. */
        TYPE_CORE << ONE_OF(
               T::BRACKET_NORMAL_LEFT << &PARAMETER_ENUMERATION << T::BRACKET_NORMAL_RIGHT << T::ARROW << &TYPE
            OR T::BRACKET_NORMAL_LEFT << &PARAMETER_ENUMERATION << T::BRACKET_NORMAL_RIGHT
            OR T::KEYWORD_VOID
            OR &NAMED_TYPE
        );

        NAMED_TYPE << T::IDENTIFIER << OPTIONAL(&GENERIC_ARGUMENTS);
        GENERIC_ARGUMENTS << T::BRACKET_ANGLE_LEFT << &TYPE_ENUMERATION << T::BRACKET_ANGLE_RIGHT;
        TYPE_ENUMERATION << &TYPE << OPTIONAL(T::ELEMENT_SEPARATOR << &TYPE_ENUMERATION);

        /* Parameter names are optional, so that "(int, float) -> float" and
           "(int u, float v) -> float" are both accepted, and so that the same rule
           can describe a tuple type. */
        PARAMETER << &TYPE << OPTIONAL(T::IDENTIFIER);
        PARAMETER_ENUMERATION << &PARAMETER << OPTIONAL(T::ELEMENT_SEPARATOR << &PARAMETER_ENUMERATION);

        GENERIC_PARAMETERS << T::BRACKET_ANGLE_LEFT << &GENERIC_PARAMETER_ENUMERATION << T::BRACKET_ANGLE_RIGHT;
        GENERIC_PARAMETER_ENUMERATION << &GENERIC_PARAMETER << OPTIONAL(T::ELEMENT_SEPARATOR << &GENERIC_PARAMETER_ENUMERATION);
        GENERIC_PARAMETER << T::IDENTIFIER << OPTIONAL(T::TYPE_CONSTRAINT << &GENERIC_CONSTRAINT);

        /* A single constraint needs no brackets, several are grouped in square brackets:
               <T : Comparable<T>>
               <T : [Comparable<T>, Equatable<T>], U>
           Without the brackets the comma would be ambiguous, since it is also what
           separates one generic parameter from the next. */
        GENERIC_CONSTRAINT << ONE_OF(
               T::BRACKET_SQUARE_LEFT << &TYPE_ENUMERATION << T::BRACKET_SQUARE_RIGHT
            OR &TYPE
        );

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
            << OPTIONAL(&GENERIC_PARAMETERS)
            << OPTIONAL(T::KEYWORD_PRIVATE)
            << OPTIONAL(T::KEYWORD_IMPURE)
            << &TYPE << &DECLARATION_NAME
            << OPTIONAL(T::BRACKET_NORMAL_LEFT << &PATTERN_ENUMERATION << T::BRACKET_NORMAL_RIGHT)
            << &DECLARATION_TAIL;

        /* A declared name is either an identifier or an operator symbol, which is how
           a user defined infix operator such as "+++" gets declared. */
        DECLARATION_NAME << ONE_OF(T::IDENTIFIER OR T::BINARY_OPERATOR OR T::RELATIONAL_OPERATOR);

        /* ':=' fixes the definition, '=' declares a rebindable variable, a bare scope is
           the class member function form, and a lone ';' leaves the value indeterminate. */
        DECLARATION_TAIL << ONE_OF(
               T::DEFINITION_OPERATOR << &DEFINITION_BODY
            OR T::ASSIGNMENT_OPERATOR << &DEFINITION_BODY
            OR &SCOPE << OPTIONAL(T::STATEMENT_TERMINATOR)
            OR T::STATEMENT_TERMINATOR
        );

        CLASS_DECLARATION
            << OPTIONAL(&GENERIC_PARAMETERS)
            << OPTIONAL(T::KEYWORD_PRIVATE)
            << T::KEYWORD_CLASS COMMIT T::IDENTIFIER
            << OPTIONAL(T::DEFINITION_OPERATOR)
            << &CLASS_BODY
            << OPTIONAL(T::STATEMENT_TERMINATOR);

        INTERFACE_DECLARATION
            << OPTIONAL(&GENERIC_PARAMETERS)
            << T::KEYWORD_INTERFACE COMMIT T::IDENTIFIER
            << OPTIONAL(T::DEFINITION_OPERATOR)
            << &MEMBER_BLOCK
            << OPTIONAL(T::STATEMENT_TERMINATOR);

        /* A class body is either a plain member block or a list of ADT variants.
           The two are told apart by their first token: a variant list starts with the
           variant name, a plain body starts with '{'. */
        CLASS_BODY << ONE_OF(&ADT_VARIANT_ENUMERATION OR &MEMBER_BLOCK);
        ADT_VARIANT_ENUMERATION << &ADT_VARIANT << OPTIONAL(T::ALTERNATIVE_SEPARATOR << &ADT_VARIANT_ENUMERATION);
        ADT_VARIANT << T::IDENTIFIER << &MEMBER_BLOCK;

        MEMBER_BLOCK << T::BRACKET_CURLY_LEFT COMMIT OPTIONAL(&MEMBER_SEQUENCE) << T::BRACKET_CURLY_RIGHT;
        MEMBER_SEQUENCE << &MEMBER << OPTIONAL(&MEMBER_SEQUENCE);
        MEMBER << ONE_OF(&CLASS_DECLARATION OR &INTERFACE_DECLARATION OR &DECLARATION);

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
        PATTERN << ONE_OF(
               &TYPE << T::IDENTIFIER
            OR T::IDENTIFIER
            OR T::LITERAL_FLOAT
            OR T::LITERAL_INTEGER
            OR T::LITERAL_BOOL
        );
        PATTERN_ENUMERATION << &PATTERN << OPTIONAL(T::ELEMENT_SEPARATOR << &PATTERN_ENUMERATION);

        DEFINITION_BODY << ONE_OF(
               &SCOPE << OPTIONAL(T::STATEMENT_TERMINATOR)
            OR &GUARDED_EXPRESSION << &STATEMENT_TERMINATOR
            OR &EXPRESSION << &STATEMENT_TERMINATOR
        );

        GUARDED_EXPRESSION << &GUARD_CLAUSE << OPTIONAL(T::ALTERNATIVE_SEPARATOR << &GUARDED_EXPRESSION);
        GUARD_CLAUSE << &EXPRESSION << T::GUARD_OPERATOR << &EXPRESSION;

        // =====================================================================
        // statements
        // =====================================================================

        DESIGNATOR << T::IDENTIFIER << OPTIONAL(T::MEMBER_ACCESS << &DESIGNATOR);
        ASSIGNMENT << &DESIGNATOR << T::ASSIGNMENT_OPERATOR << &EXPRESSION;

        /* "c << b = 1;" substitutes b = 1 into the expression tree of the
           indeterminate value c, without changing b itself. */
        SUBSTITUTION << &DESIGNATOR << T::SUBSTITUTION_OPERATOR << &DESIGNATOR << T::ASSIGNMENT_OPERATOR << &EXPRESSION;

        // =====================================================================
        // expressions
        // =====================================================================

        EXPRESSION << &TERM << OPTIONAL(&OPERATOR << &EXPRESSION);

        /* '<' and '>' are lexed as angle brackets, so they have to be listed here to
           remain usable as the less-than and greater-than operators. */
        OPERATOR << ONE_OF(
               T::BINARY_OPERATOR
            OR T::RELATIONAL_OPERATOR
            OR T::BRACKET_ANGLE_LEFT
            OR T::BRACKET_ANGLE_RIGHT
        );

        TERM << &PRIMARY << OPTIONAL(&POSTFIX_CHAIN);
        POSTFIX_CHAIN << &POSTFIX << OPTIONAL(&POSTFIX_CHAIN);

        /* An explicit generic argument list is only accepted when it is immediately
           followed by a call, as in "genericFunction<SomeType>(val)". Without that
           restriction "a < b > c" would be parsed as a generic instantiation of a. */
        POSTFIX << ONE_OF(
               T::MEMBER_ACCESS << T::IDENTIFIER
            OR &GENERIC_ARGUMENTS << T::BRACKET_NORMAL_LEFT << OPTIONAL(&ENUMERATION_EXPRESSIONS) << T::BRACKET_NORMAL_RIGHT
            OR T::BRACKET_NORMAL_LEFT << OPTIONAL(&ENUMERATION_EXPRESSIONS) << T::BRACKET_NORMAL_RIGHT
        );

        PRIMARY << ONE_OF(
               &CONSTRUCTION_LITERAL
            OR T::BRACKET_NORMAL_LEFT << &ENUMERATION_EXPRESSIONS << T::BRACKET_NORMAL_RIGHT
            OR T::LITERAL_FLOAT
            OR T::LITERAL_INTEGER
            OR T::LITERAL_BOOL
            OR T::IDENTIFIER
        );

        /* Also covers tuple literals such as "(d, c)". */
        ENUMERATION_EXPRESSIONS << &EXPRESSION << OPTIONAL(T::ELEMENT_SEPARATOR << &ENUMERATION_EXPRESSIONS);

        /* "(Example) {a = 1;}" and "(BinaryTree Node) { value = t; }". The optional
           identifier names which ADT variant is being constructed. The body reuses
           SCOPE, since the sketch allows arbitrary statements inside it. */
        CONSTRUCTION_LITERAL << T::BRACKET_NORMAL_LEFT << &TYPE << OPTIONAL(T::IDENTIFIER)
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
           block as a whole and resynchronise on its closing '}'. Both of these rules
           COMMIT once their opening brace has been consumed, so a failure
           here is always a genuine syntax error. */
        SCOPE.AddRecoveryRule(&SCOPE_END, GOTO_END_OF_DEFINITION);
        MEMBER_BLOCK.AddRecoveryRule(&SCOPE_END, GOTO_END_OF_DEFINITION);
    }
}

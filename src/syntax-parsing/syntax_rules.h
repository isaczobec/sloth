#pragma once
#include "../tokenization/tokenizer.h"
#include <vector>
#include <cstdint>
using namespace Tokenization;

namespace ParseTree {

    struct Rule;

    enum class DefinitionDirective {
        NONE,
        SUBDEFINITION_START,
        SUBDEFINITION_END,
        OR,
        OPTIONAL_START,
        OPTIONAL_END,
        REQUIRED_SUCCESS
    };

    // expressions for increased readability
    inline constexpr DefinitionDirective D_SBST   = DefinitionDirective::SUBDEFINITION_START;
    inline constexpr DefinitionDirective D_SBED   = DefinitionDirective::SUBDEFINITION_END;
    inline constexpr DefinitionDirective D_OR     = DefinitionDirective::OR;
    inline constexpr DefinitionDirective D_OPST   = DefinitionDirective::OPTIONAL_START;     // `DefinitionComponent`s between this directive and `D_OPED can optionally be included in the current rule.
    inline constexpr DefinitionDirective D_OPED   = DefinitionDirective::OPTIONAL_END;

    /* subdefenitions following immediately after this directive must be 
       successfully parsed, if not an error will be thrown and the parser 
       will attempt to find a recovery point. */
    inline constexpr DefinitionDirective D_RSUC   = DefinitionDirective::REQUIRED_SUCCESS;   


    
    inline constexpr TokenType RULECOMPONENT_NO_TOKEN = TokenType::NONE;
    inline constexpr Rule* RULECOMPONENT_NO_RULE = NULL;
    inline constexpr DefinitionDirective RULECOMPONENT_NO_DIRECTIVE = DefinitionDirective::NONE;

    /* The total amount of rules that exist*/
    inline constexpr int RULE_AMOUNT = 44;
    
    /* The amount of `DefinitionComponent`s a definition by default gets allocated space for. */
    inline constexpr int INITIAL_DEFINITION_COMPONENT_CAPACITY = 32;

    struct DefinitionComponent {
        Rule* rule = RULECOMPONENT_NO_RULE;
        TokenType token = RULECOMPONENT_NO_TOKEN;
        DefinitionDirective directive = RULECOMPONENT_NO_DIRECTIVE;

        DefinitionComponent(Rule* rule);
        DefinitionComponent(TokenType token);
        DefinitionComponent(DefinitionDirective directive);
    };

    
    struct Rule {

        std::string name;
        std::vector<DefinitionComponent> definition;

        // A set of pairs of rules to try to parse to find a recovery point for this definition, 
        // and which `DefinitionComponent` index of the definition to jump to if the recovery point is found.
        // If the `size_t` is greater or equal to the amount of definition components, simply finish parsing 
        // the current node and return it.  
        std::vector<std::pair<Rule*, size_t>> recoveryRules;

        bool throwSyntaxErrors;
        bool requireTotalSuccess; // require that the entire rule has to be parsed correctly upon entering into it
        
        Rule(std::string name);
        
        bool AllowRecover();
        void AddRecoveryRule(Rule* rule, size_t gotoDefinitonIndex);

        // functions to add a definition component, and << operator overload to increase readability
        Rule& AddRuleComponent(Rule* rule);
        Rule& AddRuleComponent(TokenType token);
        Rule& AddRuleComponent(DefinitionDirective directive);

        inline Rule& operator<<(Rule* rule)                    {return AddRuleComponent(rule);}
        inline Rule& operator<<(TokenType token)               {return AddRuleComponent(token);}
        inline Rule& operator<<(DefinitionDirective directive) {return AddRuleComponent(directive);}
    };  

    enum class NodeChildType {Token, Node};
    struct NodeChildInfo {
        NodeChildType type; // the type of this child
        size_t dcidx;       // which definition component index this child was parsed at
        NodeChildInfo(NodeChildType type, size_t dcidx);
    };

    enum class ParseNodeStatus {
        OK,
        ERROR,
        ERROR_RECOVERED
    };
    constexpr ControlFlow::CompilationError* PARSETREENODE_NO_ERROR = nullptr;
    struct ParseTreeNode {

        /* The rule that this node has parsed.*/
        Rule* rule;

        /* The children rules of this node, if any.*/
        std::vector<ParseTreeNode*> children;
        /* the tokens that this rule has parsed, if any.*/
        std::vector<Token> tokens; // TODO: inneffective to copy the tokens from the existing token vector given from the tokenizer? Safe to just use pointers to the original tokens instead?

        /* a vector info for the children of the node. */
        std::vector<NodeChildInfo> childrenInfo; 
        
        /* The parent of this node.*/
        ParseTreeNode* parent;
        
        ControlFlow::CompilationError* error;
        ParseNodeStatus status;

        ParseTreeNode(ParseTreeNode* parent, Rule* rule);
        ~ParseTreeNode();

        bool HadError();
    };

    enum class ParseErrorType {
        TOKEN,
        RULE
    };

    namespace Rules {

        // ---- program structure ----
        extern Rule TOP_STATEMENT_SEQUENCE;
        extern Rule TOP_STATEMENT;
        extern Rule SCOPE_STATEMENT_SEQUENCE;
        extern Rule STATEMENT;
        extern Rule SCOPE;
        extern Rule STATEMENT_TERMINATOR;
        extern Rule SCOPE_END;

        // ---- control flow ----
        extern Rule CONTROL_SEQUENCE;
        extern Rule CONDITION;   // either an expression or an ADT subtype test, e.g. "Node next"
        extern Rule ELSE_CLAUSE;
        extern Rule RETURN_STATEMENT;

        // ---- types ----
        extern Rule TYPE;                 // TYPE_CORE with an optional trailing '&'
        extern Rule TYPE_CORE;            // function type, tuple type, void, or a named type
        extern Rule NAMED_TYPE;           // an identifier with optional generic arguments
        extern Rule GENERIC_ARGUMENTS;    // "<int, float>"
        extern Rule TYPE_ENUMERATION;     // "int, float"
        extern Rule PARAMETER;            // a type with an optional name, e.g. "int" or "int a"
        extern Rule PARAMETER_ENUMERATION;
        extern Rule GENERIC_PARAMETERS;             // "<T : [Comparable<T>, Equatable<T>], U>"
        extern Rule GENERIC_PARAMETER;
        extern Rule GENERIC_CONSTRAINT;             // one interface, or several inside '[...]'
        extern Rule GENERIC_PARAMETER_ENUMERATION;

        // ---- declarations ----
        extern Rule DECLARATION;          // "[generics] [impure] TYPE name [(params)] tail"
        extern Rule DECLARATION_NAME;     // an identifier or a user defined operator symbol
        extern Rule DECLARATION_TAIL;     // ":= body", "= body", a bare scope, or just ';'
        extern Rule CLASS_DECLARATION;
        extern Rule INTERFACE_DECLARATION;
        extern Rule CLASS_BODY;           // either a member block or a list of ADT variants
        extern Rule ADT_VARIANT;
        extern Rule ADT_VARIANT_ENUMERATION;
        extern Rule MEMBER_BLOCK;
        extern Rule MEMBER_SEQUENCE;
        extern Rule MEMBER;

        // ---- definition clauses (pattern matching) ----
        extern Rule FUNCTION_CLAUSE;      // "f3(5, 4) := 8;"
        extern Rule INFIX_CLAUSE;         // "a +++ 0 := a;"
        extern Rule PATTERN;
        extern Rule PATTERN_ENUMERATION;
        extern Rule DEFINITION_BODY;      // a scope, a guarded expression, or a plain expression
        extern Rule GUARDED_EXPRESSION;   // "u == v ? 2.0 | u == v+1 ? 3.0"
        extern Rule GUARD_CLAUSE;

        // ---- statements ----
        extern Rule ASSIGNMENT;
        extern Rule SUBSTITUTION;         // "c << b = 1;"
        extern Rule DESIGNATOR;           // an assignable target, e.g. "n.right"

        // ---- expressions ----
        extern Rule EXPRESSION;
        extern Rule OPERATOR;             // any infix operator, including '<' and '>'
        extern Rule TERM;
        extern Rule PRIMARY;
        extern Rule POSTFIX;              // ".member", "(args)" or "<T>(args)"
        extern Rule POSTFIX_CHAIN;
        extern Rule ENUMERATION_EXPRESSIONS;
        extern Rule CONSTRUCTION_LITERAL; // "(Example) {a = 1;}"

    }

    /* Populate all rules created in the ParseTree::Rules namespace. */
    void CreateRules();

    class ParseTreeBuilder {

        private:

        public:
        ParseTreeBuilder();
        ~ParseTreeBuilder();

        /* 
        Attempt to parse the stream of tokens according to the given rule.
        Returns `NULL` if the stream of tokens did not adhere to the syntax.
        */
        ParseTreeNode* ParseNode(Rule* rule, std::vector<Token>& tokens, int& tokenPtr, ControlFlow::ControlFlowHandler& flowHandler);
    };
}
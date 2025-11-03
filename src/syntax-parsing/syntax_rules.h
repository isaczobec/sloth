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
    inline constexpr int RULE_AMOUNT = 32;
    
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
        std::vector<DefinitionComponent> definition;
        
        Rule();

        // functions to add a definition component, and << operator overload to increase readability
        Rule& AddRuleComponent(Rule* rule);
        Rule& AddRuleComponent(TokenType token);
        Rule& AddRuleComponent(DefinitionDirective directive);

        inline Rule& operator<<(Rule* rule)                    {return AddRuleComponent(rule);}
        inline Rule& operator<<(TokenType token)               {return AddRuleComponent(token);}
        inline Rule& operator<<(DefinitionDirective directive) {return AddRuleComponent(directive);}
    };  

    constexpr ControlFlow::CompilationError* PARSETREENODE_NO_ERROR = nullptr;
    struct ParseTreeNode {

        /* The children rules of this node, if any.*/
        std::vector<ParseTreeNode*> children;
        /* The parent of this node.*/
        ParseTreeNode* parent;
        
        /* the tokens that this rule has parsed, if any.*/
        std::vector<Token> tokens; // TODO: inneffective to copy the tokens from the existing token vector given from the tokenizer? Safe to just use pointers to the original tokens instead?
        /* The rule that this node has parsed.*/
        Rule* rule;

        ControlFlow::CompilationError* error;

        ParseTreeNode(ParseTreeNode* parent, Rule* rule);
        ~ParseTreeNode();

        bool HadError();
    };

    enum class ParseErrorType {
        TOKEN,
        RULE
    };

    namespace Rules {
        
        extern Rule STATEMENT;
        extern Rule SCOPE;
        extern Rule STATEMENT_TERMINATOR;

        extern Rule CONTROL_SEQUENCE;
        
        extern Rule TYPE_IDENTIFIER;
        extern Rule FUNCTION_DECLARATION;
        extern Rule VARIABLE_DECLARATION;
        extern Rule ASSIGNMENT;
        
        extern Rule ENUMERATION_EXPRESSIONS; // an enumeration of elements/expressions, e.g. "potato, 2, matrix, matmul(potato, matrix)". used e.g. for calling functions.
        extern Rule ENUMERATION_TYPED_IDENTIFIERS; // an enumeration of typed identifiers, e.g. "Matrix a, Scalar b"

        extern Rule FUNCITON_CALL;
        extern Rule EXPRESSION;
        extern Rule TERM;

    }

    /* Populate all rules created in the ParseTree::Rules namespace. */
    void CreateRules();

    class ParseTreeBuilder {

        private:
        /*
        a list of rules that allows recovery when parsed correctly
        */ 
        std::vector<Rule*> recoveryRules;

        public:
        ParseTreeBuilder();
        ~ParseTreeBuilder();

        /* 
        Attempt to parse the stream of tokens according to the given rule.
        Returns `NULL` if the stream of tokens did not adhere to the syntax.
        */
        ParseTreeNode* ParseNode(Rule* rule, std::vector<Token>& tokens, int& tokenPtr, bool recover, ControlFlow::ControlFlowHandler& flowHandler);
    };
}
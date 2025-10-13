#pragma once
#include "tokenizer.h"
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
        OPTIONAL_END
    };

    // expressions for increased readability
    inline constexpr DefinitionDirective D_SBST = DefinitionDirective::SUBDEFINITION_START;
    inline constexpr DefinitionDirective D_SBED = DefinitionDirective::SUBDEFINITION_END;
    inline constexpr DefinitionDirective D_OR   = DefinitionDirective::OR;
    /* `DefinitionComponent`s between this directive and `D_OPED can optionally be included in the current rule.*/
    inline constexpr DefinitionDirective D_OPST   = DefinitionDirective::OPTIONAL_START;
    inline constexpr DefinitionDirective D_OPED   = DefinitionDirective::OPTIONAL_END;

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

    struct ParseTreeNode {

        /* The children rules of this node, if any.*/
        std::vector<ParseTreeNode*> children;
        /* The parent of this node.*/
        ParseTreeNode* parent;
        
        /* the tokens that this rule has parsed, if any.*/
        std::vector<Token> tokens; // TODO: inneffective to copy the tokens from the existing token vector given from the tokenizer? Safe to just use pointers to the original tokens instead?
        /* The rule that this node has parsed.*/
        Rule* rule;

        ParseTreeNode(ParseTreeNode* parent);
        ~ParseTreeNode();
    };

    namespace Rules {
        extern Rule TERM;
        extern Rule EXPRESSION;
    }

    /* Populate all rules created in the ParseTree::Rules namespace. */
    void CreateRules();

    class ParseTreeBuilder {

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
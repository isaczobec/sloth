#pragma once
#include "tokenizer.h"
#include <vector>
#include <cstdint>
using namespace Tokenization;

namespace ParseTree {

    inline constexpr TokenType RULECOMPONENT_NO_TOKEN = TokenType::NONE;
    inline constexpr Rule* RULECOMPONENT_NO_RULE = NULL;

    inline constexpr int RULE_AMOUNT = 2;
    
    /* The amount of `DefinitionComponent`s a definition by default gets allocated space for. */
    inline constexpr int INITIAL_DEFINITION_COMPONENT_CAPACITY = 4;

    struct DefinitionComponent {
        Rule* rule = RULECOMPONENT_NO_RULE;
        TokenType token = RULECOMPONENT_NO_TOKEN;

        DefinitionComponent(Rule* rule);
        DefinitionComponent(TokenType token);
    };

    struct Rule {
        private:
        std::vector<std::vector<DefinitionComponent>> possibleDefinitions;
        
        void CreateDefinitionsUpTo(size_t definitionIndex);

        public:
        Rule(size_t definitionAmount);
        Rule();

        void AddRuleComponent(size_t definitionIndex, Rule* rule);
        void AddRuleComponent(size_t definitionIndex, TokenType token);
    };  

    class ParseTreeBuilder {
        private:
        Rule rules[RULE_AMOUNT];

        void CreateRules();

        public:
        ParseTreeBuilder();
        ~ParseTreeBuilder();

        Rule* CreateRule(size_t ruleIndex, size_t definitionAmount);
        Rule* GetRule(size_t ruleIndex);


    };
}
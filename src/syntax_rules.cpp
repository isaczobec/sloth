#include "syntax_rules.h"
#include <stdexcept>

namespace ParseTree {

    DefinitionComponent::DefinitionComponent(Rule* rule) {
        this->rule = rule;
    }
    DefinitionComponent::DefinitionComponent(TokenType token) {
        this->token = token;
    }

    void Rule::CreateDefinitionsUpTo(size_t definitionIndex) {
        while(possibleDefinitions.size() - 1 > definitionIndex) {
            possibleDefinitions.emplace_back();
            possibleDefinitions.back().reserve(INITIAL_DEFINITION_COMPONENT_CAPACITY);
        }
    }
    
    Rule::Rule() {}
    Rule::Rule(size_t definitionAmount) {
        CreateDefinitionsUpTo(definitionAmount-1);
    }
        
    void Rule::AddRuleComponent(size_t definitionIndex, Rule* rule) {
        CreateDefinitionsUpTo(definitionIndex);
        possibleDefinitions[definitionIndex].emplace_back(rule);
    }   

    void Rule::AddRuleComponent(size_t definitionIndex, TokenType token) {
        CreateDefinitionsUpTo(definitionIndex);
        possibleDefinitions[definitionIndex].emplace_back(token);
    }   

    Rule* ParseTreeBuilder::CreateRule(size_t ruleIndex, size_t definitionAmount) {
        if (ruleIndex < 0 || ruleIndex >= RULE_AMOUNT) {
            throw std::logic_error("Rule index was out of bounds.");
            return;
        }
        rules[ruleIndex] = Rule(definitionAmount);
    }

    Rule* ParseTreeBuilder::GetRule(size_t ruleIndex) {
        if (ruleIndex < 0 || ruleIndex >= RULE_AMOUNT) {
            throw std::logic_error("Rule index was out of bounds.");
            return;
        }
        return &rules[ruleIndex];
    }

    ParseTreeBuilder::ParseTreeBuilder() {
        CreateRules();
    }

    void ParseTreeBuilder::CreateRules() {

        Rule* testRule = CreateRule(0, 2);
        testRule->AddRuleComponent(0, TokenType::BRACKET);
        testRule->AddRuleComponent(0, TokenType::BRACKET);
    }


}
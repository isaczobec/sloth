#include "syntax_rules.h"

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
}
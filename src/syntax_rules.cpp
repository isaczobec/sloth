#include "syntax_rules.h"
#include <stdexcept>

namespace ParseTree {

    DefinitionComponent::DefinitionComponent(Rule* rule) {
        this->rule = rule;
    }
    DefinitionComponent::DefinitionComponent(TokenType token) {
        this->token = token;
    }
    DefinitionComponent::DefinitionComponent(DefinitionDirective directive) {
        this->directive = directive;
    }
    
    Rule::Rule() {
        definition.reserve(INITIAL_DEFINITION_COMPONENT_CAPACITY);
    }
        
    Rule& Rule::AddRuleComponent(Rule* rule) {
        definition.emplace_back(rule);
        return *this;
    }   

    Rule& Rule::AddRuleComponent(TokenType token) {
        definition.emplace_back(token);
        return *this;
    }   

    Rule& Rule::AddRuleComponent(DefinitionDirective directive) {
        definition.emplace_back(directive);
        return *this;
    }   

    ParseTreeNode::ParseTreeNode(ParseTreeNode* parent) {
        this->parent = parent;
    }

    ParseTreeNode::~ParseTreeNode() {
        // recursively delete children nodes
        for (ParseTreeNode* child : children) {
            delete child;
        }
    }

    ParseTreeBuilder::ParseTreeBuilder() {
        CreateRules();
    }

    ParseTreeBuilder::~ParseTreeBuilder() {}

    ParseTreeNode* ParseTreeBuilder::ParseNode(Rule* rule, std::vector<Token>& tokens, int& tokenPtr, ControlFlow::ControlFlowHandler& flowHandler) {

        ParseTreeNode* node = new ParseTreeNode(NULL);

        for (DefinitionComponent dc : (*rule).definition) {

            // divide token/directive/subrule cases
            if (dc.rule != RULECOMPONENT_NO_RULE) {

                // try to parse the child node, add it to the tree if it exists
                ParseTreeNode* childNode = ParseNode(dc.rule, tokens, tokenPtr, flowHandler);   
                if (childNode != NULL) {
                    childNode->parent = node;
                    node->children.push_back(childNode);
                } else {
                    // the child node did not exist, delete created nodes and return `NULL` (the parsing failed)
                    delete node;
                    return NULL;
                }
                
            } else if (dc.directive != RULECOMPONENT_NO_DIRECTIVE) {

            } else if (dc.token != RULECOMPONENT_NO_TOKEN) {
                if (tokens[tokenPtr].type == dc.token) {
                    node->tokens.push_back(tokens[tokenPtr]);
                    tokenPtr += 1;
                } else {
                    delete node;
                    return nullptr;
                }  
            }
        }

        return node;
    
    }
}
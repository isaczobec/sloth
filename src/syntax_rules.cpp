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

        /* Indexes into the token stream, of where we should return to if parsing an alternative for a subdefinition fails*/
        std::vector<size_t> subDefinitionReturnStack;
        std::vector<bool> subDefIsOptionalStack; // if the subdefinitions in `subDefinitionReturnStack` are optional
        
        /*
        helper function to go forward upon failed parsing to the next
        possible definition. After this is called dcidx will be pointing
        at a D_OPED or D_OR directive if one was found, and in that case
        return true, otherwise return false.
        */  
        auto gotoNextParsePoint = [&subDefIsOptionalStack, &subDefinitionReturnStack](int& dcidx, Rule* rule) {
            // look forward and try to find an `OR` directive or optional subsdefinition ender.
            while (dcidx < (*rule).definition.size()) {
                dcidx++;

                // remove any subdefinitions we pass
                if ((*rule).definition[dcidx].directive == D_SBED) {
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back();

                } else if ((*rule).definition[dcidx].directive == D_OPED) {
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back();
                    return true;
                    
                } else if ((*rule).definition[dcidx].directive == D_OR) {
                    return true;
                }
            }
            return false;
        };
        
        bool failed = false; 
        for (int dcidx = 0; dcidx < (*rule).definition.size(); ++dcidx) {
            DefinitionComponent& dc = (*rule).definition[dcidx];

            // divide token/directive/subrule cases
            if (dc.rule != RULECOMPONENT_NO_RULE) {

                // try to parse the child node, add it to the tree if it exists
                ParseTreeNode* childNode = ParseNode(dc.rule, tokens, tokenPtr, flowHandler);   
                if (childNode != NULL) {
                    childNode->parent = node;
                    node->children.push_back(childNode);
                } else {
                    // the child node did adhere to the syntax, delete created nodes and return `NULL` (the parsing failed)

                    // look forward and try to find an `OR` directive or optional subsdefinition ender.
                    // if we found an OR directive or end of optional subdefinition and do not need to exit
                    if (!gotoNextParsePoint(dcidx, rule)) {
                        failed = true;
                        break;
                    }
                }
                
            } else if (dc.directive != RULECOMPONENT_NO_DIRECTIVE) {

                // if there is a subdefinition starting, push the current token ptr to the return stack
                if (dc.directive == D_SBST) {
                    subDefinitionReturnStack.push_back(tokenPtr);
                    subDefIsOptionalStack.push_back(false);
                    continue;
                }
                
                
                // if we made it to the end of a subdefinition, pop the top return index
                else if (dc.directive == D_SBED) {
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back(); // TODO: check if it is not optional, otherwise rules are wrongly defined
                    continue;
                }
                
                // same but for optional substrings
                if (dc.directive == D_OPST) {
                    subDefinitionReturnStack.push_back(tokenPtr);
                    subDefIsOptionalStack.push_back(true);
                    continue;
                }
                else if (dc.directive == D_OPED) {
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back(); // TODO: check if it is not optional, otherwise rules are wrongly defined
                    continue;
                }

                /*
                if we have reached an `OR` separator without failing, we can continue
                until the next subdefinition or finnish if we are not in a subdefinition
                */ 
                else if (dc.directive == D_OR) {

                    if (subDefinitionReturnStack.size() == 0) {
                        // if we are not in a subdefinition, the parsing is finnished
                        break;

                    } else {
                        // we are in a subdefinition, pop the return index and go forward
                        // until the end of the subdefinition
                        subDefinitionReturnStack.pop_back();
                        subDefIsOptionalStack.pop_back();
                        while ((*rule).definition[dcidx].directive != D_SBED && (*rule).definition[dcidx].directive != D_OPED) {
                            dcidx++;

                            // if we do not find a substring ender, throw an error
                            if (dcidx >= (*rule).definition.size()) {
                                throw std::logic_error("Could not find the end of the current subdefinition. Make sure the Rule has a DefinitionDirective::SUBDEFINITION_END or DefinitionDirective::OPTIONAL_END.");
                                return nullptr;
                            }
                        }
                    }
                }

            } else if (dc.token != RULECOMPONENT_NO_TOKEN) {

                // if there are too few tokens left in the stream
                if (tokens.size() <= tokenPtr) {
                    
                    // if we are inside an optional inclusion, it is fine that there are no more tokens left
                    if (!(subDefinitionReturnStack.size() >= 1 && subDefIsOptionalStack[0])) {
                        failed = true;
                        return nullptr;
                    }
                } 
                else if (tokens[tokenPtr].type == dc.token) {
                    node->tokens.push_back(tokens[tokenPtr]);
                    tokenPtr += 1;
                } else {

                    if (!gotoNextParsePoint(dcidx, rule)) {
                        failed = true;
                        break;
                    }
                }  
            }
        }

        if (failed) {
            delete node;
            return nullptr;
        }

        return node;
    
    }
}
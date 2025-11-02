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

    ParseTreeNode::ParseTreeNode(ParseTreeNode* parent, Rule* rule) {
        this->parent = parent;
        this->rule = rule;
        this->error = PARSETREENODE_NO_ERROR;
    }

    ParseTreeNode::~ParseTreeNode() {
        // recursively delete children nodes
        for (ParseTreeNode* child : children) {
            delete child;
        }
    }

    bool ParseTreeNode::HadError() {
        return (error != PARSETREENODE_NO_ERROR);
    }

    ParseTreeBuilder::ParseTreeBuilder() {
        CreateRules();

        // assign which rules are recovery rules
        recoveryRules.push_back(&Rules::STATEMENT_TERMINATOR);
    }

    ParseTreeBuilder::~ParseTreeBuilder() {}

    ParseTreeNode* ParseTreeBuilder::ParseNode(
        Rule* rule, std::vector<Token>& tokens, int& tokenPtr, 
        bool recover, ControlFlow::ControlFlowHandler& flowHandler
    ) {

        ParseTreeNode* node = new ParseTreeNode(NULL, rule);

        // TODO: all the stacks used throughout this functions could be more cleanly organized

        /* Indexes into the token stream, of where we should return to if parsing an alternative for a subdefinition fails*/
        int initialTokenPtr = tokenPtr;
        std::vector<size_t> subDefinitionReturnStack;
        std::vector<bool> subDefIsOptionalStack; // if the subdefinitions in `subDefinitionReturnStack` are optional
        std::vector<size_t> subDefTokenStartStack; // A stack keeping track of where subdefinition tokens start, so they can be popped if parsing a subdefinition fails
        std::vector<size_t> subDefChildStartStack; // A stack keeping track of where subdefinition children start, so they can be popped if parsing a subdefinition fails

        bool isInRequiredSuccessSubDefinition = false;
        size_t requiredSuccesStartLevel = 0; // which subdefinition level the required success began at

        const auto popChildren = [&subDefTokenStartStack, &subDefChildStartStack, node](bool popStacks = true, bool deletechildren = true) {
            if (subDefTokenStartStack.empty() || subDefChildStartStack.empty()) return;

            if (deletechildren) {
                // only delete while we have more children than the recorded start index
                while (node->children.size() > subDefChildStartStack.back()) {
                    delete node->children.back();
                    node->children.pop_back();
                }
                // only pop tokens that were added after the recorded start index
                while (node->tokens.size() > subDefTokenStartStack.back()) {
                    node->tokens.pop_back();
                }
            }

            if (popStacks) {
                subDefChildStartStack.pop_back();
                subDefTokenStartStack.pop_back();
            }
        };

        /*
        helper function to go forward upon failed parsing to the next
        possible definition. After this is called dcidx will be pointing
        at a D_OPED or D_OR directive if one was found, and in that case
        return true, otherwise return false.
        */  
        const auto gotoNextParsePoint = [&subDefIsOptionalStack, &subDefinitionReturnStack, &popChildren, &tokenPtr](int& dcidx, Rule* rule) {
            // look forward and try to find an `OR` directive or optional subsdefinition ender.

            // we must find an OPED or OR on the same or lower level to be able to exit to it
            int scopeLevelInitial = subDefinitionReturnStack.size() - 1;
            int scopeLevel = scopeLevelInitial;

            while (dcidx < (*rule).definition.size()) {
                dcidx++;

                if ((*rule).definition[dcidx].directive == D_SBST || (*rule).definition[dcidx].directive == D_OPST) {
                    scopeLevel++;
                }

                // remove any subdefinitions we pass
                if ((*rule).definition[dcidx].directive == D_SBED && scopeLevel <= scopeLevelInitial) {
                    scopeLevel--;
                    tokenPtr = subDefinitionReturnStack.back();
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back();
                    popChildren();
                    
                } else if ((*rule).definition[dcidx].directive == D_OPED && scopeLevel <= scopeLevelInitial) {
                    tokenPtr = subDefinitionReturnStack.back();
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back();
                    popChildren();
                    if (scopeLevel <= scopeLevelInitial) {
                        return true;
                    } else {
                        scopeLevel--;
                    }
                    
                } else if ((*rule).definition[dcidx].directive == D_OR && scopeLevel <= scopeLevelInitial) {
                    if (scopeLevel == scopeLevelInitial) {
                        tokenPtr = subDefinitionReturnStack.back();
                        popChildren(false, true); // only delete children, do not pop stacks
                    }
                    return true;
                }
            }
            return false;
        };

        bool failed = false; 

        const auto handleParseError = [&] (
            DefinitionComponent& dc, Token* currentToken, ParseErrorType errorType,
            bool recover = false, bool raiseError = false, int& dcidx
        ) {

            // construct error message string

            if (raiseError) {
                std::string errorMessage;
                if (errorType == ParseErrorType::TOKEN) {
                    errorMessage += "Expected '";
                    errorMessage.append(std::to_string((long int)dc.token)); // TODO: Replace this with a readable token name
                    errorMessage += "', got ";
                    errorMessage.append(std::to_string((long int)currentToken->type)); // TODO: Replace this with a readable token name
                } 
                else if (errorType == ParseErrorType::RULE) {
                    errorMessage += "Invalid '";
                    errorMessage.append(std::to_string((long int)dc.rule)); // TODO: Replace this with a readable rule name
                }
                
                ControlFlow::CompilationError* error = flowHandler.Error(
                    ControlFlow::CompilationErrorSeverity::ERROR,
                    ControlFlow::ERRCODE_SYNTAX_ERROR,
                    errorMessage,
                    currentToken->sourceString
                );
                node->error = error;
            }

            // attempt to move forward and parse a recovery rule
            if (recover) {
                while (tokenPtr < tokens.size()) {
                    for (Rule* recoveryRule : recoveryRules) {

                        ParseTreeNode* recoveryNode = ParseNode(recoveryRule, tokens, tokenPtr, false, flowHandler);
                        if (recoveryNode == NULL) {
                            tokenPtr += 1;
                        } else {
                            return true;
                        }
                    }
                }
                return false; // if no recovery token could be parsed
            }
          
            return false; // return false if recovery was not specified
        };

        
        for (int dcidx = 0; dcidx < (*rule).definition.size(); ++dcidx) {
            DefinitionComponent& dc = (*rule).definition[dcidx];

            // divide token/directive/subrule cases
            if (dc.rule != RULECOMPONENT_NO_RULE) {

                // try to parse the child node, add it to the tree if it exists
                ParseTreeNode* childNode = ParseNode(dc.rule, tokens, tokenPtr, false, flowHandler);   
                if (childNode != NULL) {

                    // check if there was an error in the child node
                    if (childNode->HadError())  {
                        if (recover) {
                            handleParseError(dc, &tokens[tokenPtr], ParseErrorType::RULE, true, false, dcidx);
                        } else {
                            node->error = childNode->error;
                            childNode->parent = node;
                            node->children.push_back(childNode);
                        }
                    } else {

                        // nomrally push back the child node
                        childNode->parent = node;
                        node->children.push_back(childNode);
                    }

                } else {
                    // the child node did adhere to the syntax, delete created nodes and return `NULL` (the parsing failed)
                    // look forward and try to find an `OR` directive or optional subsdefinition ender.
                    // if we found an OR directive or end of optional subdefinition and do not need to exit
                    if (!gotoNextParsePoint(dcidx, rule)) {
                        // if we are in a required subdefinition, raise an error when parsing fails
                        if (isInRequiredSuccessSubDefinition) {
                            handleParseError(dc, &tokens[tokenPtr], ParseErrorType::RULE, false, true, dcidx);
                            return node;
                        }
                            
                        failed = true;
                        break;
                    }
                }
                
            } else if (dc.directive != RULECOMPONENT_NO_DIRECTIVE) {

                // if there is a subdefinition starting, push the current token ptr to the return stack
                if (dc.directive == D_SBST) {
                    subDefinitionReturnStack.push_back(tokenPtr);
                    subDefIsOptionalStack.push_back(false);
                    subDefChildStartStack.push_back(node->children.size());
                    subDefTokenStartStack.push_back(node->tokens.size());

                    continue;
                }
                
                
                // if we made it to the end of a subdefinition, pop the top return index
                else if (dc.directive == D_SBED) {
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back(); // TODO: check if it is not optional, otherwise rules are wrongly defined
                    popChildren(true, false);
                    
                    // Reset required success flag when we successfully exit the required subdefinition
                    if (isInRequiredSuccessSubDefinition && subDefinitionReturnStack.size() < requiredSuccesStartLevel) {
                        isInRequiredSuccessSubDefinition = false;
                        requiredSuccesStartLevel = 0;
                    }
                    continue;
                }
                
                // same but for optional substrings
                if (dc.directive == D_OPST) {
                    // record current positions as the start of the optional group
                    subDefinitionReturnStack.push_back(tokenPtr);
                    subDefIsOptionalStack.push_back(true);
                    subDefChildStartStack.push_back(node->children.size());
                    subDefTokenStartStack.push_back(node->tokens.size());
                    continue;
                }
                else if (dc.directive == D_OPED) {
                    subDefinitionReturnStack.pop_back();
                    subDefIsOptionalStack.pop_back(); // TODO: check if it is not optional, otherwise rules are wrongly defined
                    popChildren(true, false);
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
                        popChildren(true, false);

                        while ((*rule).definition[dcidx].directive != D_SBED && (*rule).definition[dcidx].directive != D_OPED) {
                            dcidx++;

                            // if we do not find a substring ender, throw an error
                            if (dcidx >= (*rule).definition.size()) {
                                throw std::logic_error("Could not find the end of the current subdefinition. Make sure the Rule has a DefinitionDirective::SUBDEFINITION_END or DefinitionDirective::OPTIONAL_END.");
                                return NULL;
                            }
                        }
                    }
                }

                else if (dc.directive == D_RSUC) {
                    // note where we were upon entering a required success subdefinition
                    requiredSuccesStartLevel = subDefinitionReturnStack.size();
                    isInRequiredSuccessSubDefinition = true;
                }

            } else if (dc.token != RULECOMPONENT_NO_TOKEN) {

                // if there are too few tokens left in the stream
                if (tokens.size() <= tokenPtr) {

                    // if we are inside an optional inclusion, it is fine that there are no more tokens left
                    if (!(subDefinitionReturnStack.size() >= 1 && subDefIsOptionalStack[0])) {
                        failed = true;
                        break;
                    }
                } 
                else if (tokens[tokenPtr].type == dc.token) {
                    node->tokens.push_back(tokens[tokenPtr]);
                    tokenPtr += 1;
                } else {

                    if (!gotoNextParsePoint(dcidx, rule)) {
                        if (isInRequiredSuccessSubDefinition) {
                            handleParseError(dc, &tokens[tokenPtr], ParseErrorType::TOKEN, false, true, dcidx);
                            return node;
                        }

                        failed = true;
                        break;
                    }
                }  
            }
        }

        // if we have used all definition components and parsed nothing, we have failed
        if (node->tokens.empty() && node->children.empty()) {
            failed = true;
        }

        if (failed) {
            delete node;
            return NULL;
            tokenPtr = initialTokenPtr; // reset tokenPtr
        }

        return node;
    
    }
}
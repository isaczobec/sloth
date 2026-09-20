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
    
    Rule::Rule(std::string name) {
        this->name = name;
        definition.reserve(INITIAL_DEFINITION_COMPONENT_CAPACITY);
        throwSyntaxErrors = true;
        requireTotalSuccess = false;
    }

    bool Rule::AllowRecover() {
        return !recoveryRules.empty();
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

    void Rule::AddRecoveryRule(Rule* rule, size_t gotoDefinitonIndex) {
        recoveryRules.push_back({rule, gotoDefinitonIndex});
    }

    NodeChildInfo::NodeChildInfo(NodeChildType type, size_t dcidx) {
        this->type = type;
        this->dcidx = dcidx;
    }

    ParseTreeNode::ParseTreeNode(ParseTreeNode* parent, Rule* rule) {
        this->parent = parent;
        this->rule = rule;
        this->error = PARSETREENODE_NO_ERROR;
        status = ParseNodeStatus::OK;
    }

    ParseTreeNode::~ParseTreeNode() {
        // recursively delete children nodes
        for (ParseTreeNode* child : children) {
            delete child;
        }
    }

    bool ParseTreeNode::HadError() {
        return (status == ParseNodeStatus::ERROR);
    }

    ParseTreeBuilder::ParseTreeBuilder() {
        CreateRules();
    }

    ParseTreeBuilder::~ParseTreeBuilder() {}

    ParseTreeNode* ParseTreeBuilder::ParseNode(
        Rule* rule, std::vector<Token>& tokens, int& tokenPtr, 
        ControlFlow::ControlFlowHandler& flowHandler
    ) {

        ParseTreeNode* node = new ParseTreeNode(NULL, rule);

        // TODO: all the stacks used throughout this functions could be more cleanly organized

        /* Indexes into the token stream, of where we should return to if parsing an alternative for a subdefinition fails*/
        int initialTokenPtr = tokenPtr;
        std::vector<size_t> subDefinitionReturnStack;
        std::vector<bool> subDefIsOptionalStack; // if the subdefinitions in `subDefinitionReturnStack` are optional
        /* A stack keeping track of how much of `node->childrenInfo` had been filled in when
           each subdefinition was entered, so that everything parsed inside a subdefinition
           can be discarded if that subdefinition turns out not to match.

           This indexes `childrenInfo` rather than `children` or `tokens` on purpose:
           `childrenInfo` is the only list that records both kinds of child in parse order,
           and unwinding it is what tells us which of the other two lists to pop from. */
        std::vector<size_t> subDefChildStartStack;

        bool isInRequiredSuccessSubDefinition = false;
        size_t requiredSuccesStartLevel = 0; // which subdefinition level the required success began at

        const auto popChildren = [&subDefChildStartStack, node](bool popStacks = true, bool deletechildren = true) {
            if (subDefChildStartStack.empty()) return;

            if (deletechildren) {
                while (node->childrenInfo.size() > subDefChildStartStack.back()) {
                    if (node->childrenInfo.back().type == NodeChildType::Node) {
                        delete node->children.back();
                        node->children.pop_back();
                    } else {
                        node->tokens.pop_back();
                    }
                    node->childrenInfo.pop_back();
                }
            }

            if (popStacks) {
                subDefChildStartStack.pop_back();
            }
        };

        /*
        helper function to go forward upon failed parsing to the next
        possible definition. After this is called dcidx will be pointing
        at a D_OPED or D_OR directive if one was found, and in that case
        return true, otherwise return false.
        */  
        const auto gotoNextParsePoint = [&](int& dcidx, Rule* rule) {
            /*
            Scan forward from a failed component for a point where parsing can resume:
            either an alternative separator, or the end of an optional group, belonging
            to a subdefinition that we are actually inside of. Sets `dcidx` to that
            directive and returns true, or returns false if there is no such point.

            `depth` counts how many nested groups we have scanned *past* without ever
            having entered them. Only a directive at depth 0 belongs to the group that is
            live at the top of `subDefinitionReturnStack`, and only such a directive is a
            real resume point. Without this counter, the terminator of an unrelated
            sibling group further along the definition gets mistaken for our own.
            */
            int depth = 0;

            for (int i = dcidx + 1; i < (int)(*rule).definition.size(); ++i) {
                dcidx = i;
                const DefinitionDirective directive = (*rule).definition[dcidx].directive;

                if (directive == D_SBST || directive == D_OPST) {
                    depth++;
                }

                else if (directive == D_SBED || directive == D_OPED) {

                    // a nested group that was scanned past rather than entered
                    if (depth > 0) {
                        depth--;
                        continue;
                    }

                    // this terminates the group we are currently inside of, so rewind
                    // everything that group had consumed
                    if (subDefinitionReturnStack.empty()) return false;
                    tokenPtr = subDefinitionReturnStack.back();
                    subDefinitionReturnStack.pop_back();
                    if (!subDefIsOptionalStack.empty()) subDefIsOptionalStack.pop_back();
                    popChildren();

                    // leaving an optional group out entirely is a valid parse, so we are
                    // done. a required group gives us nothing, so keep looking for a
                    // resume point in the group that encloses it.
                    if (directive == D_OPED) return true;
                }

                else if (directive == D_OR && depth == 0) {
                    if (subDefinitionReturnStack.empty()) return false;

                    // rewind to the start of the group and try the next alternative.
                    // the group itself stays open, so its stack entries are left alone.
                    tokenPtr = subDefinitionReturnStack.back();
                    popChildren(false, true);
                    return true;
                }
            }
            return false;
        };

        bool failed = false; 

        /* The token an error message should point at. Error recovery can walk the token
           pointer all the way to the end of the stream, so this clamps to the last token
           (which is always the end of file token) instead of reading out of range. */
        const auto errorToken = [&tokens, &tokenPtr]() -> Token* {
            if (tokens.empty()) return nullptr;
            if (tokenPtr >= (int)tokens.size()) return &tokens.back();
            if (tokenPtr < 0) return &tokens.front();
            return &tokens[tokenPtr];
        };

        const auto handleParseError = [&] (
            DefinitionComponent& dc, Token* currentToken, ParseErrorType errorType,
            bool recover = false, bool raiseError = false, int& dcidx
        ) {

            // construct error message string

            if (raiseError && rule->throwSyntaxErrors && currentToken != nullptr) {
                std::string errorMessage;
                if (errorType == ParseErrorType::TOKEN) {
                    errorMessage += "Expected '";
                    errorMessage.append(std::to_string((long int)dc.token)); // TODO: Replace this with a readable token name
                    errorMessage += "', got ";
                    errorMessage.append(std::to_string((long int)currentToken->type)); // TODO: Replace this with a readable token name
                } 
                else if (errorType == ParseErrorType::RULE) {
                    errorMessage += "Invalid ";
                    errorMessage.append(rule->name); // TODO: Replace this with a readable rule name
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
                    for (std::pair<Rule*, size_t> rec : rule->recoveryRules) {

                        ParseTreeNode* recoveryNode = ParseNode(rec.first, tokens, tokenPtr, flowHandler);
                        if (recoveryNode != NULL) {
                            // recovery token succesfully parsed

                            // delete all the children after the specefied index
                            while (!node->childrenInfo.empty() && node->childrenInfo.back().dcidx >= rec.second) {
                                if (node->childrenInfo.back().type == NodeChildType::Node) {
                                    delete node->children.back();
                                    node->children.pop_back();
                                } else {
                                    node->tokens.pop_back();
                                }
                                node->childrenInfo.pop_back();
                            }

                            dcidx = rec.second;
                            node->status = ParseNodeStatus::ERROR_RECOVERED;
                            
                            return true;
                        }
                    }
                    tokenPtr += 1; // did not parse a recovery rule here, step forward 
                }
                node->status = ParseNodeStatus::ERROR;
                return false; // if no recovery token could be parsed
            }
            
            node->status = ParseNodeStatus::ERROR;
            return false; // return false if recovery was not specified
        };

        
        for (int dcidx = 0; dcidx < (*rule).definition.size(); ++dcidx) {
            DefinitionComponent& dc = (*rule).definition[dcidx];

            // divide token/directive/subrule cases
            if (dc.rule != RULECOMPONENT_NO_RULE) {

                // try to parse the child node, add it to the tree if it exists
                ParseTreeNode* childNode = ParseNode(dc.rule, tokens, tokenPtr, flowHandler);   
                if (childNode != NULL) {

                    // check if there was an error in the child node
                    if (childNode->HadError())  {
                        node->error = childNode->error;
                        if (rule->AllowRecover()) {
                            handleParseError(dc, errorToken(), ParseErrorType::RULE, true, false, dcidx);
                        } else {
                            childNode->parent = node;
                            node->children.push_back(childNode);
                            node->childrenInfo.emplace_back(NodeChildType::Node, dcidx);
                            node->status = ParseNodeStatus::ERROR;
                        }
                    } else {
                        
                        // nomrally push back the child node
                        childNode->parent = node;
                        node->children.push_back(childNode);
                        node->childrenInfo.emplace_back(NodeChildType::Node, dcidx);
                    }

                } else {
                    // the child node did adhere to the syntax, delete created nodes and return `NULL` (the parsing failed)
                    // look forward and try to find an `OR` directive or optional subsdefinition ender.
                    // if we found an OR directive or end of optional subdefinition and do not need to exit
                    if (!gotoNextParsePoint(dcidx, rule)) {
                        // if we are in a required subdefinition, raise an error when parsing fails
                        if (isInRequiredSuccessSubDefinition || rule->requireTotalSuccess) {
                            if (handleParseError(dc, errorToken(), ParseErrorType::RULE, rule->AllowRecover(), true, dcidx)) {
                                continue;
                            } else {
                                return node;
                            }
                        }
                            
                        failed = true;
                        break;
                    }
                }
                
            } else if (dc.directive != RULECOMPONENT_NO_DIRECTIVE) {

                // if there is a subdefinition starting, push the current token ptr to the return stack
                if (dc.directive == D_SBST) {

                    // std::cout << "size is " << subDefIsOptionalStack.size() << std::endl;

                    subDefinitionReturnStack.push_back(tokenPtr);
                    subDefIsOptionalStack.push_back(false);
                    subDefChildStartStack.push_back(node->childrenInfo.size());

                    continue;
                }
                
                
                // if we made it to the end of a subdefinition, pop the top return index
                else if (dc.directive == D_SBED) {
                    if (!subDefinitionReturnStack.empty()) subDefinitionReturnStack.pop_back();
                    if (!subDefIsOptionalStack.empty()) subDefIsOptionalStack.pop_back(); // TODO: check if it is not optional, otherwise rules are wrongly defined
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
                    subDefChildStartStack.push_back(node->childrenInfo.size());
                    continue;
                }
                else if (dc.directive == D_OPED) {
                    if (!subDefinitionReturnStack.empty()) subDefinitionReturnStack.pop_back();
                    if (!subDefIsOptionalStack.empty()) subDefIsOptionalStack.pop_back(); // TODO: check if it is not optional, otherwise rules are wrongly defined
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
                        if (!subDefinitionReturnStack.empty()) subDefinitionReturnStack.pop_back();
                        if (!subDefIsOptionalStack.empty()) subDefIsOptionalStack.pop_back();
                        popChildren(true, false);

                        /* Skip the alternatives we no longer need to try. Nested groups
                           have to be counted, otherwise a nested group's terminator inside
                           a later alternative is mistaken for the end of this one. */
                        int nesting = 0;
                        while (true) {
                            const DefinitionDirective d = (*rule).definition[dcidx].directive;
                            if (d == D_SBST || d == D_OPST) {
                                nesting++;
                            } else if (d == D_SBED || d == D_OPED) {
                                if (nesting == 0) break;
                                nesting--;
                            }

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

                    // if we are inside an optional inclusion, it is fine that there are no more tokens left.
                    // the innermost group is the one we are currently in, so check the back of the
                    // stack rather than the front.
                    if (!(!subDefIsOptionalStack.empty() && subDefIsOptionalStack.back())) {
                        failed = true;
                        break;
                    }
                } 
                else if (tokens[tokenPtr].type == dc.token) {
                    node->tokens.push_back(tokens[tokenPtr]);
                    node->childrenInfo.emplace_back(NodeChildType::Token, dcidx);
                    tokenPtr += 1;
                } else {

                    if (!gotoNextParsePoint(dcidx, rule)) {
                        if (isInRequiredSuccessSubDefinition || rule->requireTotalSuccess) {
                            if (handleParseError(dc, errorToken(), ParseErrorType::RULE, rule->AllowRecover(), true, dcidx)) {
                                continue;
                            } else {
                                return node;
                            }
                        }

                        failed = true;
                        break;
                    }
                }  
            }
        }

        // if we have used all definition components and parsed nothing, we have failed
        if (node->tokens.empty() && node->children.empty() && !(node->status == ParseNodeStatus::ERROR_RECOVERED)) {
            failed = true;
        }

        if (failed) {
            // rewind the token pointer before giving up, so that the caller can cleanly
            // try its next alternative. this has to happen *before* the return, and
            // `tokenPtr` is a reference into the caller's state, so leaving it advanced
            // would make every alternative after a partially matched one start in the
            // wrong place.
            tokenPtr = initialTokenPtr;
            delete node;
            return NULL;
        }

        return node;
    
    }
}
#include "tree_traverser.h"

using namespace CodeGeneration;

PfStack::PIteratorPair PfStack::GetHandlers() {
    if (startIndicies.empty()) {
        return {pfs.begin(), pfs.end()};
    }
    size_t startIndex = startIndicies.back();
    return {pfs.begin() + startIndex, pfs.end()};
}

void PfStack::Pop() {
    pfs.pop_back();

    // remove from startindicies if we are popping a
    // handler with overridePrev set
    if (!startIndicies.empty() && startIndicies.back() == pfs.size()) {
        startIndicies.pop_back();
    }
}

void PfStack::Push(NodeHandler&& handler, bool overridePrev) {
    pfs.push_back(std::move(handler));

    // if overriding, add a start index to the stack pointing at the 
    // recently added function
    if (overridePrev) {
        startIndicies.push_back(pfs.size() - 1);
    }  
}

TreeTraverser::TreeTraverser() :
    pfMap() {}

TreeTraverser::~TreeTraverser() {}

void TreeTraverser::TraverseNode(ParseTreeNode& node, ControlFlowHandler& cf) {

    // create a vector of flags of which rules have had handlers added
    std::vector<Rule*> currentRulesAddedFlags;
    auto prevRulesAddedFlags = rulesAddedFlags;
    rulesAddedFlags = &currentRulesAddedFlags;

    auto it = pfMap.find(node.rule);
    if (it != pfMap.end()) { // if the handler existed

        // get all the handlers and run them
        PfStack pfs = it->second;
        auto [first, last] = pfs.GetHandlers();
        for (auto it = first; it != last; ++it) {
            
            // run the handler
            (*it)(node, *this, cf);
        }
    }

    // process all children
    for (ParseTreeNode* child : node.children) {
        TraverseNode(*child, cf);
    }

    // TODO: add possibility to run a lambda after all 
    // children have been processed

    // pop handlers from where they have been added
    for (Rule* rule : currentRulesAddedFlags) {
        pfMap[rule].Pop();
    }

    // point back to the rules added flags of the parent node
    rulesAddedFlags = prevRulesAddedFlags;
}

void TreeTraverser::AddHandler(Rule* rule, bool overridePrev, NodeHandler pf) {
 
    // record that a handler has been added by this node
    if (rulesAddedFlags != NULL) rulesAddedFlags->push_back(rule);
    pfMap[rule].Push(std::move(pf), overridePrev);
}
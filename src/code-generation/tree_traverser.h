#pragma once
#include "../syntax-parsing/syntax_rules.h"
#include "../flow-handler/control_flow_handler.h"

using namespace ParseTree;
using namespace ControlFlow;

namespace CodeGeneration
{
    class TreeTraverser;
    typedef std::function<void(ParseTreeNode&, TreeTraverser&, ControlFlowHandler&)> NodeHandler;
    const NodeHandler NONE = [](...) {}; // Node Handler that does nothing

    struct PfStack {
        private:
        std::vector<NodeHandler> pfs; // the Node handlers

        /** Stack with indicies to start the handling at.
         * When parsing a node begins, begin at the index
         * at the top of the stack and move up in indicies. */ 
        std::vector<size_t> startIndicies; 

        public:
        using PIteratorPair = std::pair<std::vector<NodeHandler>::iterator, 
                              std::vector<NodeHandler>::iterator>;

        PIteratorPair GetHandlers();
        void Pop();
        void Push(NodeHandler&& handler, bool overridePrev);
    };

    class TreeTraverser {

        public: 
        TreeTraverser();
        ~TreeTraverser();

        void TraverseNode(ParseTreeNode& node, ControlFlowHandler&);

        /** Adds a handler function for a rule. Until the current node
         * has been processed completely, this handler will be applied 
         * to all its children. 
         * 
         * `overridePrev` will cause previous handlers not to be called
         * at all until the current node has been processed completely.
         */
        void AddHandler(Rule* rule, NodeHandler pf, bool overridePrev);

        private:
        /**A map from rules to the stack of rules used to process them.*/
        std::map<Rule*, PfStack> pfMap;

        /**A pointer to a vector with for which rules the handlers have
         * been modified by the current handler. Used to pop those handlers
         * after the node has been processed completely.
         */
        std::vector<Rule*>* rulesAddedFlags;
    };
} 

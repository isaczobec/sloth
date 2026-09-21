#include "symbol_table.h"

using namespace CodeGeneration;

TreeTraverser CodeGeneration::symbolTableGenerator;

void CodeGeneration::InitSymbolTableGenerator() {
    symbolTableGenerator = TreeTraverser();
    CodeGeneration::symbolTableGenerator.AddHandler(
        &Rules::TOP_STATEMENT, false,
        [](ParseTreeNode& node, TreeTraverser& traverser, ControlFlowHandler& cf) {
            std::cout << "HELLO FREOM" << node.rule->name << std::endl;
        }
    );
}
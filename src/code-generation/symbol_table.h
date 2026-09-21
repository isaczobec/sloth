#pragma once
#include "../syntax-parsing/syntax_rules.h"
#include "../flow-handler/control_flow_handler.h"
#include "tree_traverser.h"

namespace CodeGeneration {
    extern TreeTraverser symbolTableGenerator;

    void InitSymbolTableGenerator();
}
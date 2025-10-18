#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <cstdio>
#include <vector>
#include "tokenization/tokenizer.h"
#include "flow-handler/control_flow_handler.h"

namespace sloth {
    int x;
}

int main(int argc, char** argv) {

    ControlFlow::ControlFlowHandler cf;
    cf.Compile(argv[1]);

    return 0;
}
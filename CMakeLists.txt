cmake_minimum_required(VERSION 3.28.3)

set(SRC 
    "src/main.cpp" 
    "src/token_data_parsers.cpp" 
    "src/token_regexes.cpp" 
    "src/tokenizer.cpp" 
    "src/control_flow_handler.cpp" 
    "src/syntax_rules.cpp"
)

project(sloth)

add_executable(out ${SRC})
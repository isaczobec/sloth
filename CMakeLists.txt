cmake_minimum_required(VERSION 3.28.3)

set(SRC 
    "src/main.cpp" 
    "src/tokenization/token_data_parsers.cpp" 
    "src/tokenization/token_regexes.cpp" 
    "src/tokenization/tokenizer.cpp" 
    "src/flow-handler/control_flow_handler.cpp" 
    "src/syntax-parsing/syntax_rules.cpp"
    "src/syntax-parsing/rules.cpp"
    "src/file-reading/file_reader.cpp"
)

project(sloth)

add_compile_options(-g)

add_executable(out ${SRC})
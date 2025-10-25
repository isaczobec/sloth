#include "tokenizer.h"

namespace Tokenization {
    
    /* The regexes used to match each token type.*/
    const std::vector<std::pair<TokenType, std::regex>> tokenRegexes = {
        
        {TokenType::ASSIGNMENT_OPERATOR,    std::regex(R"(=)")},
        {TokenType::RELATIONAL_OPERATOR,    std::regex(R"(==|>=|<=|>|<)")},
        {TokenType::BINARY_OPERATOR,        std::regex(R"(\+|\*)")},
        
        {TokenType::LITERAL_FLOAT,          std::regex(R"(\b\d*\.\d+\b)")},
        {TokenType::LITERAL_INTEGER,        std::regex(R"(\b\d+\b)")},
        
        {TokenType::BRACKET_CURLY_LEFT,     std::regex(R"(\{)")},
        {TokenType::BRACKET_CURLY_RIGHT,    std::regex(R"(\})")},
        {TokenType::BRACKET_NORMAL_LEFT,    std::regex(R"(\()")},
        {TokenType::BRACKET_NORMAL_RIGHT,   std::regex(R"(\))")},
        
        {TokenType::STATEMENT_TERMINATOR,   std::regex(R"(;)")},
        {TokenType::ELEMENT_SEPARATOR,      std::regex(R"(,)")},
        
        {TokenType::KEYWORD_IF,             std::regex(R"(\bif\b)")},
        {TokenType::KEYWORD_ELSE,           std::regex(R"(\belse\b)")},
        {TokenType::KEYWORD_WHILE,          std::regex(R"(\bwhile\b)")}, 

        {TokenType::IDENTIFIER,             std::regex(R"(\b[A-Za-z][A-Za-z0-9]*\b)")},
    };
}
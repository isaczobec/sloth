#include "tokenizer.h"
using namespace Tokenization;

/* The regexes used to match each token type.*/
const const std::map<TokenType, std::regex> tokenRegexes = {
    {TokenType::BRACKET,                std::regex(R"(\(|\)|\[\]|\{\})")},
    {TokenType::KEYWORD,                std::regex(R"(mat|var)")},
    {TokenType::VARIABLE_IDENTIFIER,    std::regex("[A-Za-z][A-Za-z0-9]+")}
};
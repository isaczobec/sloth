#include "tokenizer.h"

namespace Tokenization {
    
    /* The regexes used to match each token type.*/
    const std::map<TokenType, std::regex> tokenRegexes = {
        {TokenType::BRACKET,                std::regex(R"(\(|\)|\[|\]|\{|\})")},
        {TokenType::KEYWORD,                std::regex(R"(\b(mat|var)\b)")},
        {TokenType::VARIABLE_IDENTIFIER,    std::regex(R"(\b[A-Za-z][A-Za-z0-9]+\b)")}
    };
}
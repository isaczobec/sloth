#pragma once
#include "control_flow_handler.h"
#include <string>
#include <vector>
#include <cstdint>
#include <regex>
#include <map>
#include <unordered_map>
#include <string_view>

#define TOKEN_DATA_DEFAULT_CAPACITY_BYTES 16384 
#define TOKEN_DATA_REALLOCATION_LIMIT     8192  // TODO: Make it actually reallocate if this limit is reached

namespace Tokenization {

    enum class TokenType {
        NONE,
        BRACKET,
        KEYWORD,
        VARIABLE_IDENTIFIER,
        ASSIGNMENT_OPERATOR
    };
    
    struct Token {
        TokenType type;
        size_t dataSizeBytes;
        void* tokenData = NULL;
        
        Token(TokenType type, size_t dataSizeBytes, void* tokenData);
    };
    
    typedef void (*TokenDataParser)(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler);
    extern const std::unordered_map<TokenType, TokenDataParser> tokenDataParserMap;
    
    /* The regexes used to match each token type.*/
    extern const std::map<TokenType, std::regex> tokenRegexes;

    class Tokenizer {
        private:
        uint8_t* tokenData;
        std::vector<Token> tokens;

        /* The current size of all stored token data, in bytes*/
        size_t tokenDataSizeBytes;

        public:
        Tokenizer();
        ~Tokenizer();

        void Tokenize(std::string& fileString, ControlFlow::ControlFlowHandler& flowHandler);
        std::vector<Token>& GetTokens();
    };
    
}
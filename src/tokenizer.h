#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <regex>
#include <map>
#include <string_view>

#define TOKEN_DATA_DEFAULT_CAPACITY_BYTES 16384
#define TOKEN_DATA_REALLOCATION_LIMIT     8192

namespace Tokenization {

    enum class TokenType {
        BRACKET,
        KEYWORD,
        VARIABLE_IDENTIFIER,
    };

    /* The regexes used to match each token type.*/
    extern const std::map<TokenType, std::regex> tokenRegexes;
    
    struct Token {
        TokenType type;
        size_t dataSizeBytes;
        void* tokenData;
    };
    
    typedef void (*TokenDataParser)(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes);
    extern const std::map<TokenType, TokenDataParser> tokenParserMap;

    class Tokenizer {
        private:
        uint8_t* tokenData;
        std::vector<Token> tokens;

        /* The current size of all stored token data, in bytes*/
        size_t tokenDataSizeBytes;

        public:
        Tokenizer();
        ~Tokenizer();

        void Tokenize(std::string& fileString);
    };
    
}
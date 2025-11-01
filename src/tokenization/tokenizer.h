#pragma once
#include "../flow-handler/control_flow_handler.h"
#include "../file-reading/file_reader.h"
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
        NONE,    // tokentype used to indicate in a `RuleComponent` that it is not a token
        UNKNOWN, // token that is added to the stream upon trying to parse an unknown token

        BRACKET_CURLY_LEFT,
        BRACKET_CURLY_RIGHT,
        BRACKET_NORMAL_LEFT,
        BRACKET_NORMAL_RIGHT,
        
        IDENTIFIER,
        
        ASSIGNMENT_OPERATOR,
        RELATIONAL_OPERATOR,
        BINARY_OPERATOR,

        STATEMENT_TERMINATOR,
        
        LITERAL_INTEGER,
        LITERAL_FLOAT,

        KEYWORD_IF,
        KEYWORD_ELSE,
        KEYWORD_WHILE,

        ELEMENT_SEPARATOR
    };
    
    struct Token {
        TokenType type;
        size_t dataSizeBytes;
        void* tokenData = NULL;

        FileReader::SourceString sourceString;

        Token(TokenType type, size_t dataSizeBytes, void* tokenData, size_t sourceFileIndex, size_t lineNumber, size_t startIndex, size_t length);
        Token(); // Creates a token with `TokenType::UNKNOWN`
    };

    
    typedef void (*TokenDataParser)(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler);
    extern const std::unordered_map<TokenType, TokenDataParser> tokenDataParserMap;
    
    /* The regexes used to match each token type.*/
    extern const std::vector<std::pair<TokenType, std::regex>> tokenRegexes;

    class Tokenizer {
        private:
        uint8_t* tokenData;
        std::vector<Token> tokens;

        /* The current size of all stored token data, in bytes*/
        size_t tokenDataSizeBytes;

        public:
        Tokenizer();
        ~Tokenizer();

        void Tokenize(FileReader::FileStream* fileStream, ControlFlow::ControlFlowHandler& flowHandler);
        std::vector<Token>& GetTokens();
    };
    
}
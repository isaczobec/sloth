#include "tokenizer.h"
#include <string_view>
#include <cctype>

using namespace Tokenization;

Tokenizer::Tokenizer() {
    tokenData = new uint8_t[TOKEN_DATA_DEFAULT_CAPACITY_BYTES];
    tokenDataSizeBytes = 0;
}

void Tokenizer::Tokenize(std::string& fileString) {
    
    // create a string view and a pointer to the current character
    size_t s_ptr = 0;
    std::string_view s(fileString);

    size_t tokenDataPtr = 0;


    while (s_ptr < s.length()) {

        // skip whitespaces
        if (std::isspace(s.at(s_ptr))) {
            ++s_ptr;
            continue;
        }

        std::cmatch tokenMatch;
        for (std::pair<TokenType, std::regex> tokenRegexPair : tokenRegexes) {
            
            // try to search for each token pattern in the remaining input
            std::string_view subString = s.substr(s_ptr, s.length() - s_ptr);
            bool foundMatch = std::regex_search(subString.begin(), subString.end(), tokenMatch, tokenRegexPair.second);

            if (foundMatch) {
                
                // find and call the function to parse the token data for the token type
                size_t tokenDataSizeBytes;
                tokenParserMap.at(tokenRegexPair.first)(tokenMatch.str(), (void*)(tokenData+tokenDataPtr), &tokenDataSizeBytes);

                // construct and emplace the token, advance the data pointer
                tokens.emplace_back(tokenRegexPair.first, tokenDataSizeBytes, (void*)(tokenData+tokenDataPtr));
                tokenDataPtr += tokenDataSizeBytes;

                s_ptr += tokenMatch.length();
                
            }

        }
        
    }
}

Tokenizer::~Tokenizer() {
    delete[] tokenData;
}
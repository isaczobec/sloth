#include "tokenizer.h"
#include <string_view>
#include <cctype>

using namespace Tokenization;

Token::Token(TokenType type, size_t dataSizeBytes, void* tokenData, size_t sourceFileIndex, size_t lineNumber, size_t startIndex, size_t length) 
    : sourceString(sourceFileIndex, lineNumber, startIndex, length)
{
    this->type = type; 
    this->dataSizeBytes = dataSizeBytes; 
    this->tokenData = tokenData; 
}

Tokenizer::Tokenizer() {
    tokenData = new uint8_t[TOKEN_DATA_DEFAULT_CAPACITY_BYTES];
    tokenDataSizeBytes = 0;
}

void Tokenizer::Tokenize(FileReader::FileStream* fileStream, ControlFlow::ControlFlowHandler& flowHandler) {
    
    // create a string view and a pointer to the current character
    size_t s_ptr = 0;
    std::string_view s(fileStream->stream);
    size_t currentLine = 1;

    size_t tokenDataPtr = 0;

    // a step for beginning the token matching
    flowHandler.NewStep(true);
    flowHandler.CompleteStep();

    while (s_ptr < s.length()) {

        // skip whitespaces
        if (std::isspace(s.at(s_ptr))) {
            if (s.at(s_ptr) == '\n') {
                ++currentLine;
            }
            ++s_ptr;
            continue;
        }
        
        // search the relevant substring
        std::string_view subString = s.substr(s_ptr, s.length() - s_ptr);
        std::cmatch tokenMatch;

        bool found = false;
        for (std::pair<TokenType, std::regex> tokenRegexPair : tokenRegexes) {
            
            // try to search for each token pattern in the remaining input
            bool foundMatch = std::regex_search(subString.begin(), subString.end(), tokenMatch, tokenRegexPair.second);

            if (foundMatch && tokenMatch.position() == 0) 
            {
                // begin token data parsing step
                flowHandler.NewStep(); // down is true
                
                // find and call the function to parse the token data for the token type if it exists
                size_t tokenDataSizeBytes = 0;
                auto parserFunction = tokenDataParserMap.find(tokenRegexPair.first);
                if (parserFunction != tokenDataParserMap.end()) {
                    parserFunction->second(tokenMatch.str(), (void*)(tokenData+tokenDataPtr), &tokenDataSizeBytes, flowHandler);
                } else {
                    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE); // no data to attempt to parse; automatically complete the step
                }

                // construct and emplace the token, advance the data pointer
                tokens.emplace_back(tokenRegexPair.first, tokenDataSizeBytes, (void*)(tokenData+tokenDataPtr), fileStream->fileIndex, currentLine, s_ptr, tokenMatch.length());
                tokenDataPtr += tokenDataSizeBytes; // since `tokenDataSizeBytes` is initialized to 0, the data pointer does not advance if no parser was found

                s_ptr += tokenMatch.length();
                found = true;
                break;
            } 
        }
            if (found == false) {
            // if no token could be parsed, throw error
            flowHandler.Error(ControlFlow::CompilationErrorSeverity::ERROR, ControlFlow::ERRCODE_UNKNOWN_TOKEN, "Invalid token", 
                FileReader::SourceString(fileStream->fileIndex, currentLine, s_ptr, 1)
            );
            flowHandler.CompleteStep(ControlFlow::STATUSCODE_ERROR_EXIT);
            return;
        }
    }

    // a step for finalizing the token parsing.
    // currently just used to go up in the step tree.
    flowHandler.NewStep();
    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE, true);

    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE);
}

std::vector<Token>& Tokenizer::GetTokens() {
    return tokens;
}

Tokenizer::~Tokenizer() {
    delete[] tokenData;
}
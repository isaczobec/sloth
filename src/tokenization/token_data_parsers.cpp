#include "tokenizer.h"
#include "keywords.h"
#include "../flow-handler/control_flow_handler.h"
#include <cstdint>
#include <cstring>

void BracketDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler) 
{
    // simply write the bracket  
    char bracketChar = matchedString.at(0);
    *(char*)dataWriteDestination = bracketChar;
    *dataSizeBytes = sizeof(char);

    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE);
}

void KeywordDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler) 
{
    // get the appropriate keyword id
    *(size_t*)dataWriteDestination = keywordIdMap.at(matchedString);
    *dataSizeBytes = sizeof(size_t);

    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE);
}

void MemCpyMatchedStringParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler) 
{
    // get the appropriate keyword id
    std::memcpy(dataWriteDestination, matchedString.data(), matchedString.size());
    *dataSizeBytes = matchedString.size() * sizeof(char);

    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE);
}

namespace Tokenization {
    
    const std::unordered_map<TokenType, TokenDataParser> tokenDataParserMap = {
        {TokenType::IDENTIFIER,          &MemCpyMatchedStringParser},
        {TokenType::BINARY_OPERATOR,     &MemCpyMatchedStringParser},
        {TokenType::RELATIONAL_OPERATOR, &MemCpyMatchedStringParser},
        {TokenType::LITERAL_FLOAT,       &MemCpyMatchedStringParser}, // TODO: Change theese two to maybe parse the string to floats etc?
        {TokenType::LITERAL_INTEGER,     &MemCpyMatchedStringParser},
    };
};

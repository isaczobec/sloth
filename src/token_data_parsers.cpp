#include "tokenizer.h"
#include "keywords.h"
#include "control_flow_handler.h"
#include <cstdint>
#include <cstring>

void BracketDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler) 
{
    // simply write the bracket  
    char bracketChar = matchedString.at(0);
    *(char*)dataWriteDestination = bracketChar;
    *dataSizeBytes = sizeof(char);

    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE, true);
}

void KeywordDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler) 
{
    // get the appropriate keyword id
    *(size_t*)dataWriteDestination = keywordIdMap.at(matchedString);
    *dataSizeBytes = sizeof(size_t);

    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE, true);
}

void VariableIdentifierDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes, ControlFlow::ControlFlowHandler& flowHandler) 
{
    // get the appropriate keyword id
    std::memcpy(dataWriteDestination, matchedString.data(), matchedString.size());
    *dataSizeBytes = matchedString.size() * sizeof(char);

    flowHandler.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE, true);
}

namespace Tokenization {
    
    const std::unordered_map<TokenType, TokenDataParser> tokenDataParserMap = {
        {TokenType::BRACKET,             &BracketDataParser},
        {TokenType::KEYWORD,             &KeywordDataParser},
        {TokenType::VARIABLE_IDENTIFIER, &VariableIdentifierDataParser},
    };
};

#include "tokenizer.h"
#include "keywords.h"
#include <cstdint>
using namespace Tokenization;

void BracketDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes) 
{
    // simply write the bracket  
    char bracketChar = matchedString.at(0);
    *(char*)dataWriteDestination = bracketChar;
    *dataSizeBytes = sizeof(char);
}

void KeywordDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes) 
{
    // get the appropriate keyword id
    *(size_t*)dataWriteDestination = keywordIdMap.at(matchedString);
    *dataSizeBytes = sizeof(size_t);
}

void VariableIdentifierDataParser(std::string_view matchedString, void* dataWriteDestination, size_t* dataSizeBytes) 
{
    // get the appropriate keyword id
    *(char*)dataWriteDestination = *matchedString.data();
    *dataSizeBytes = matchedString.size() * sizeof(char);
}

const std::map<TokenType, TokenDataParser> tokenParserMap = {
    {TokenType::BRACKET,             &BracketDataParser},
    {TokenType::KEYWORD,             &KeywordDataParser},
    {TokenType::VARIABLE_IDENTIFIER, &VariableIdentifierDataParser},
};
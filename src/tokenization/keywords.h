#pragma once
#include <cstdint>
#include <unordered_map>
#include <string_view>
#include <string>

inline constexpr std::string_view KEYWORD_MAT = "mat";
inline constexpr uint8_t KEYWORD_MAT_ID = 0;

inline constexpr std::string_view KEYWORD_VAR = "var";
inline constexpr uint8_t KEYWORD_VAR_ID = 1;

inline const std::unordered_map<std::string_view, uint8_t> keywordIdMap = {
    {KEYWORD_MAT, KEYWORD_MAT_ID}, 
    {KEYWORD_VAR, KEYWORD_VAR_ID}
};
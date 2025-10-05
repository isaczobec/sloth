#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <cstdio>
#include "tokenizer.h"

namespace sloth {
    int x;
}

int main() {

    std::FILE* file = std::fopen("./test","r");

    std::string s;
    int c;
    while ((c = std::fgetc(file)) != EOF) {
        s.append(1, (char)c);
    }
    
    std::regex r("s.");

    std::regex_iterator iter = std::sregex_iterator(s.begin(), s.end(), r);
    std::regex_iterator end = std::sregex_iterator();

    for(std::regex_iterator i_match = iter; i_match != end; ++i_match){
        std::smatch match = *i_match;
        std::string match_string = match.str();
        std::cout << match_string << std::endl;

    } 

    std::fclose(file);

}
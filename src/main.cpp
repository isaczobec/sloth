#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <cstdio>
#include <vector>
#include "tokenizer.h"

namespace sloth {
    int x;
}

int main() {

    std::FILE* file = std::fopen("test","r");

    std::string s;
    int c;
    while ((c = std::fgetc(file)) != EOF) {
        s.append(1, (char)c);
    }
    
    Tokenization::Tokenizer t;

    t.Tokenize(s);

    for (Tokenization::Token token : *t.GetTokens()) {
        std::cout << (int)token.type << std::endl;
    }
    
    std::fclose(file);

}
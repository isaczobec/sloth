#include "tokenizer.h"

namespace Tokenization {

    /* 
    The regexes used to match each token type.

    ORDER IS SIGNIFICANT. The tokenizer walks this list in order and takes the first
    entry that matches at the current position, so every token that is a prefix of a
    longer token must be listed *after* that longer token. The groups below are ordered
    to respect the following prefix relations:

        "//" and "/*" before "/"    (a comment must win over division)
        ":=" before ":"
        "<<" before "<"
        "==", "!=", ">=", "<=" before "=", "<" and ">"
        "->" before "-"
        keywords before IDENTIFIER  (otherwise "if" tokenizes as an identifier)
        LITERAL_FLOAT before LITERAL_INTEGER and before MEMBER_ACCESS
    */
    const std::vector<std::pair<TokenType, std::regex>> tokenRegexes = {

        // comments are matched like any other token, but the tokenizer drops them
        // instead of emitting them into the stream
        {TokenType::COMMENT,                std::regex(R"(//[^\n]*|/\*[\s\S]*?\*/)")},

        // multi character operators, before any of their single character prefixes
        {TokenType::DEFINITION_OPERATOR,    std::regex(R"(:=)")},
        {TokenType::SUBSTITUTION_OPERATOR,  std::regex(R"(<<)")},
        {TokenType::RELATIONAL_OPERATOR,    std::regex(R"(==|!=|>=|<=)")},
        {TokenType::ARROW,                  std::regex(R"(->)")},

        // single character operators
        {TokenType::ASSIGNMENT_OPERATOR,    std::regex(R"(=)")},
        {TokenType::TYPE_CONSTRAINT,        std::regex(R"(:)")},
        {TokenType::REFERENCE_OPERATOR,     std::regex(R"(&)")},
        {TokenType::GUARD_OPERATOR,         std::regex(R"(\?)")},
        {TokenType::ALTERNATIVE_SEPARATOR,  std::regex(R"(\|)")},

        /* A run of operator characters is matched as a single token so that user defined
           infix operators such as "+++" tokenize as one operator rather than three. */
        {TokenType::BINARY_OPERATOR,        std::regex(R"([+\-*/%]+)")},

        // literals. the float pattern must be tried first, otherwise "1.5" would
        // tokenize as INTEGER(1) MEMBER_ACCESS(.) INTEGER(5)
        {TokenType::LITERAL_FLOAT,          std::regex(R"(\b\d*\.\d+\b)")},
        {TokenType::LITERAL_INTEGER,        std::regex(R"(\b\d+\b)")},

        {TokenType::MEMBER_ACCESS,          std::regex(R"(\.)")},

        // brackets
        {TokenType::BRACKET_CURLY_LEFT,     std::regex(R"(\{)")},
        {TokenType::BRACKET_CURLY_RIGHT,    std::regex(R"(\})")},
        {TokenType::BRACKET_NORMAL_LEFT,    std::regex(R"(\()")},
        {TokenType::BRACKET_NORMAL_RIGHT,   std::regex(R"(\))")},
        {TokenType::BRACKET_ANGLE_LEFT,     std::regex(R"(<)")},
        {TokenType::BRACKET_ANGLE_RIGHT,    std::regex(R"(>)")},

        {TokenType::STATEMENT_TERMINATOR,   std::regex(R"(;)")},
        {TokenType::ELEMENT_SEPARATOR,      std::regex(R"(,)")},

        // keywords, all of which must precede IDENTIFIER
        {TokenType::LITERAL_BOOL,           std::regex(R"(\btrue\b|\bfalse\b)")},
        {TokenType::KEYWORD_IF,             std::regex(R"(\bif\b)")},
        {TokenType::KEYWORD_ELSE,           std::regex(R"(\belse\b)")},
        {TokenType::KEYWORD_WHILE,          std::regex(R"(\bwhile\b)")},
        {TokenType::KEYWORD_RETURN,         std::regex(R"(\breturn\b)")},
        {TokenType::KEYWORD_CLASS,          std::regex(R"(\bclass\b)")},
        {TokenType::KEYWORD_INTERFACE,      std::regex(R"(\binterface\b)")},
        {TokenType::KEYWORD_IMPURE,         std::regex(R"(\bimpure\b)")},
        {TokenType::KEYWORD_PRIVATE,        std::regex(R"(\bprivate\b)")},
        {TokenType::KEYWORD_VOID,           std::regex(R"(\bvoid\b)")},

        {TokenType::IDENTIFIER,             std::regex(R"(\b[A-Za-z_][A-Za-z0-9_]*\b)")},
    };
}

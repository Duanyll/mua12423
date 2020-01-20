#pragma once
#include "ast_parser.h"
#include "context.h"
#include "lexer.h"

namespace mua {
class repl {
    lexer::string_lexer lex;
    runtime_context context;

   public:
    inline void reset() { context.reset(); }
    inline void eval(const std::string& str) { 
        ast_parser p(lex(str));

    }
};
}  // namespace mua
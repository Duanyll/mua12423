#pragma once

#include <vector>
#include "tokens.h"

namespace mua {
namespace lexer {
using namespace tokens;
typedef std::vector<tokens::token*> token_array;

class lexer {
   private:
    enum possible_token_type {
        UNKNOWN,
        NAME,
        DEC,
        HEX,
        SCI,
        SYMBOL,
        COMMENT,
        STRING
    };
    inline static bool is_symbol_first_char(char ch) {
        for (int i = 0; i < tokens::symbol_count; i++) {
            if (ch == tokens::symbols[i][0]) {
                return true;
            }
        }
        return false;
    }

   public:
    inline lexer() {}
    inline ~lexer() {}
    token_array operator()(const std::string&);
};

#ifdef _DEBUG
void test_lexer();
#endif

}  // namespace lexer
}  // namespace mua
#pragma once

#include <vector>
#include "tokens.h"

namespace mua {
namespace lexer {
typedef std::vector<token*> token_array;

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
        for (auto& i : symbols) {
            if (ch == i[0]) {
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
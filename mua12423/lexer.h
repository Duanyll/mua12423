#pragma once

#include <vector>

#include "tokens.h"

namespace mua {
namespace lexer {
typedef std::vector<token*> token_array;

class string_lexer {
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

    token* get_token(const std::string& buffer, possible_token_type buffer_type,
                     bool need_comment);

   public:
    inline string_lexer() {}
    inline ~string_lexer() {}
    token_array operator()(const std::string&, bool = true);
};

void test_lexer();
}  // namespace lexer
}  // namespace mua
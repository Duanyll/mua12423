#pragma once
#include <iostream>

#include "ast_parser.h"
#include "context.h"
#include "lexer.h"

namespace mua {
class repl {
   protected:
    lexer::string_lexer lex;
    rt_context context;

   public:
    inline void reset() { context.reset(); }
    inline void eval(const std::string& str) {
        ast_parser p(lex(str));
        size_t end_pos;
        auto stat = p.parse_stat(0, end_pos);
        stat->eval(&context);
    }

    inline virtual void run() {
        std::clog << "Running the Mua REPL at default mode." << std::endl;
        // std::clog << "Terminate the input with ^Z." << std::endl;
        std::clog << "Exit the REPL by typing '.exit' (whithout quotes)"
                  << std::endl;
        while (true) {
            std::string str;
            std::getline(std::cin, str, '\n');
            if (str.empty()) continue;
            if (str == ".exit") break;
            eval(str + '\n');
        }
    }
};

class solution_uva12421 : public repl {
   public:
    inline void run() { test_lexer(); }
};

class solution_uva12422 : public repl {
   public:
    inline void run() { std::string str;
        while (std::getline(std::cin, str)) {
            if (str.empty()) {
                context.reset();
            } else {
                eval(str + '\n');
            }
        }
    }
};
}  // namespace mua
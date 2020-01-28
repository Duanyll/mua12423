#pragma once
#include <iostream>

#include "ast_parser.h"
#include "lexer.h"
#include "runtime.h"

namespace mua {
class repl {
   protected:
    lexer::string_lexer lex;
    runtime rt;

   public:
    inline void reset() { rt.reset(); }
    inline void eval(const std::string& str) {
        ast_parser p(lex(str, false));
        size_t end_pos;
        auto stat = p.parse_inner_block(0, end_pos);
        stat->eval(&rt);
    }

    inline virtual void run() {
        std::clog << "Running the Mua REPL at default mode." << std::endl;
        // std::clog << "Terminate the input with ^Z." << std::endl;
        std::clog << "Exit the REPL by typing '.exit' (whithout quotes)"
                  << std::endl;
        while (true) {
            std::string str, x;
            do {
                std::getline(std::cin, x, '\n');
                str.append(x);
                str.append("\n");
            } while (!x.empty());
            if (str == ".exit\n") break;
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
    inline void run() {
        std::string str;
        while (std::getline(std::cin, str)) {
            if (str.empty()) {
                rt.reset();
            } else {
                eval(str + '\n');
            }
        }
    }
};
}  // namespace mua
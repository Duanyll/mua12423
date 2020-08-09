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
    inline virtual void eval(const std::string& str) {
        ast_parser p(lex(str, false));
        size_t end_pos;
        auto stat = p.parse_inner_block(0, end_pos);
        if (stat->ch.size() == 1 &&
            std::dynamic_pointer_cast<expr_statement>(stat->ch[0])) {
            auto exp =
                std::dynamic_pointer_cast<expr_statement>(stat->ch[0])->exp;
            auto res = exp->eval(&rt);
            if (res->get_typeid() != NIL) {
                std::clog << "< ";
                libiary_functions::print(res);
            }
            delete res;
        } else {
            stat->eval(&rt);
        }
        for (auto& i : p.input) delete i;
    }

    inline virtual void run() {
        std::clog << "Running the Mua REPL at default mode." << std::endl;
        // std::clog << "Terminate the input with ^Z." << std::endl;
        std::clog << "Exit the REPL by typing '.exit' (whithout quotes)"
                  << std::endl;
        while (true) {
            std::string str, x;
            do {
                std::clog << ">>> ";
                std::getline(std::cin, x, '\n');
                str.append(x);
                str.append("\n");
            } while (!x.empty());
            if (str == ".exit\n\n") break;
            std::clog << std::endl;
            eval(str);
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

class solution_uva12423 : public repl {
   public:
    inline virtual void eval(const std::string& str) {
        ast_parser p(lex(str, false));
        size_t end_pos;
        auto stat = p.parse_inner_block(0, end_pos);
        stat->eval(&rt);
        for (auto& i : p.input) delete i;
    }

    inline void run() {
        int T = 0;
        std::string str;
        while (true) {
            std::string x;
            if (!std::getline(std::cin, x, '\n')) {
                eval(str);
                std::cout << std::endl;
                return;
            }
            if (x.find_first_of("-- PROGRAM") == 0 || x.find_first_of("--PROGRAM")) {
                eval(str);
                std::cout << std::endl;
                str.clear();
                rt.reset();
                T++;
                std::cout << "Program " << T << ":" << std::endl;
            }
            str.append(x);
            str.append("\n");
        }
    }
};
}  // namespace mua
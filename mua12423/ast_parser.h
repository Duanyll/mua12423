#pragma once
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "context.h"
#include "lexer.h"
#include "operator.h"
#include "utils.h"
using namespace mua::ast;
using namespace mua::lexer;

namespace mua {
namespace parser {
class ast_parser {
    lexer::token_array input;

    struct term {
        bool is_opr;
        pexpr val_expr;
        std::string val_opr;

        inline term(pexpr val) : is_opr(false), val_expr(val) {}
        inline term(std::string opr) : is_opr(true), val_opr(opr) {}
    };

    bool is_operator(const token& x);
    bool is_expr_start(const token& x);
    // 这个 token 是不是 expr 的元素
    bool is_expr_end(const token& x);
    bool is_unop(const std::string& x);

    std::list<std::unordered_map<std::string, runtime::local_var_id>> frames;
    runtime::local_var_id id_begin = 0;

   public:
    inline ast_parser(lexer::token_array in) : input(in) { push_frame(); }
    pexpr parse_expr(size_t start_pos, size_t& end_pos);
    std::vector<pexpr> parse_param_list(size_t start_pos, size_t& end_pos);
    std::shared_ptr<table_constant> parse_table(size_t start_pos,
                                                size_t& end_pos);

    void declare_local_var(const std::string& name);
    pexpr get_var_reference(const std::string& name);
    void push_frame();
    void pop_frame();
};
}  // namespace parser
}  // namespace mua
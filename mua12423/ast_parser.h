#pragma once
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "operator.h"
#include "runtime.h"
#include "statement.h"
#include "utils.h"

namespace mua {
using namespace mua::ast;
using namespace mua::lexer;
class ast_parser {
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

    typedef std::unordered_map<std::string, local_var_id> scope;
    struct frame {
        std::list<scope> scopes;  // 函数中第一个 scope 总是参数
        std::unordered_set<local_var_id> captures;
    };
    std::list<frame> context;
    local_var_id id_begin = 0;

    bool find_next(const std::string& name, size_t& cur);

    pstat parse_block(size_t start_pos, size_t& end_pos);
    pstat parse_if(size_t start_pos, size_t& end_pos);
    pstat parse_for(size_t start_pos, size_t& end_pos);
    pstat parse_while(size_t start_pos, size_t& end_pos);
    pstat parse_repeat(size_t start_pos, size_t& end_pos);
    pstat parse_local(size_t start_pos, size_t& end_pos);

    std::pair<pexpr, plexpr> parse_function(size_t start_pos, size_t& end_pos);

   public:
    lexer::token_array input;
    inline ast_parser(lexer::token_array in) : input(in) {
        context.push_back(frame());
        context.back().scopes.push_back(scope());
    }

    std::shared_ptr<block_statement> parse_inner_block(size_t start_pos,
                                                       size_t& end_pos);
    pstat parse_stat(size_t start_pos, size_t& end_pos);

    pexpr parse_expr(size_t start_pos, size_t& end_pos);
    plexpr parse_lexpr(size_t start_pos, size_t& end_pos);
    std::vector<pexpr> parse_param_list(size_t start_pos, size_t& end_pos);
    std::shared_ptr<table_constant> parse_table(size_t start_pos,
                                                size_t& end_pos);

    local_var_id declare_local_var(const std::string& name);
    pexpr use_var(const std::string& name);
};

#ifdef _DEBUG
void test_expr(const std::string& str, const object* res);
#endif  // _DEBUG
}  // namespace mua
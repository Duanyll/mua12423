#pragma once
#include <unordered_map>
#include <vector>

#include "ast.h"

namespace mua {
namespace ast {

class statement : public ast_base {
   public:
    virtual void eval(runtime_context* context) = 0;
};

typedef std::shared_ptr<statement> pstat;

class expr_statement : public statement {
   public:
    pexpr exp;
    inline expr_statement(pexpr x) : exp(x) {}
    inline virtual void eval(runtime_context* context) {
        auto res = exp->eval(context);
        delete res;
    }
};

class assign_statement : public statement {
   public:
    plexpr lexp;
    pexpr rexp;
    inline assign_statement(plexpr l, pexpr r) : lexp(l), rexp(r) {}
    inline virtual void eval(runtime_context* context) {
        auto val = rexp->eval(context);
        lexp->set_value(context, val);
        delete val;
    }
};

class block_statement : public statement {
   public:
    std::vector<pstat> ch;
    enum { none, break_stat, return_stat } last_stat;
    pexpr return_value;
    virtual void eval(runtime_context* context);
};

class if_statement : public statement {
   public:
    std::vector<pexpr> conditions;
    std::vector<pstat> stats;
    pstat else_stat;
    virtual void eval(runtime_context* context);
};

class while_statement : public statement {
   public:
    pexpr condition;
    pstat ch;
    virtual void eval(runtime_context* context);
};

class repeat_statement : public statement {
   public:
    pexpr condition;
    pstat ch;
    virtual void eval(runtime_context* context);
};

class for_statement : public statement {
   public:
    local_var_id loop_var;
    pexpr begin, step, end;
    pstat ch;
    virtual void eval(runtime_context* context);
};

class range_based_for : public statement {
   public:
    local_var_id loop_var;
    pexpr range;
    pstat ch;
    virtual void eval(runtime_context* context);
};

class ast_function : public types::function {
   public:
    std::unordered_map<local_var_id, storage_id> captures;
    std::vector<local_var_id> param_name;
    pstat ch;
    virtual object* invoke(runtime_context* context,
                           std::vector<const object*> params);
    runtime_context* binded_context;
    ~ast_function();
};

class function_declaration : public statement {
   public:
    std::unordered_set<local_var_id> captures;
    plexpr func_name;
    std::vector<local_var_id> param_name;
    pstat ch;
    virtual void eval(runtime_context* context);
};

class varible_declaration : public statement {
   public:
    local_var_id vid;
    virtual void eval(runtime_context* context);
};

}  // namespace ast
}  // namespace mua
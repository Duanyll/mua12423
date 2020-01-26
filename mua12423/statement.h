#pragma once
#include <unordered_map>
#include <vector>

#include "ast.h"

namespace mua {
namespace ast {

class statement : public ast_base {
   public:
    virtual void eval(rt_context* context) = 0;
};

typedef std::shared_ptr<statement> pstat;

class expr_statement : public statement {
   public:
    pexpr exp;
    inline expr_statement(pexpr x) : exp(x) {}
    virtual void eval(rt_context* context);
};

class assign_statement : public statement {
   public:
    plexpr lexp;
    pexpr rexp;
    inline assign_statement(plexpr l, pexpr r) : lexp(l), rexp(r) {}
    virtual void eval(rt_context* context);
};

class block_statement : public statement {
   public:
    std::vector<pstat> ch;
    enum { none, break_stat, return_stat } last_stat = none;
    pexpr return_value;
    virtual void eval(rt_context* context);
};

bool is_control_flow_true(pexpr x, rt_context* context);

class if_statement : public statement {
   public:
    std::vector<pexpr> conditions;
    std::vector<pstat> stats;
    pstat else_stat;
    virtual void eval(rt_context* context);
};

class while_statement : public statement {
   public:
    pexpr condition;
    pstat ch;
    virtual void eval(rt_context* context);
};

class repeat_statement : public statement {
   public:
    pexpr condition;
    pstat ch;
    virtual void eval(rt_context* context);
};

class for_statement : public statement {
   public:
    local_var_id loop_var;
    pexpr begin, step, end;
    pstat ch;
    virtual void eval(rt_context* context);
};

class generic_for_statement : public statement {
   public:
    local_var_id loop_var;
    pexpr range;
    pstat ch;
    virtual void eval(rt_context* context);
};

class ast_function : public types::function {
   public:
    std::unordered_map<local_var_id, storage_id> captures;
    std::vector<local_var_id> param_name;
    pstat ch;
    rt_context* binded_context;
    ~ast_function();

    virtual object* invoke(rt_context* context,
                           std::vector<const object*> params) const;
};

class lambda_expression : public expr {
   public:
    std::unordered_set<local_var_id> captures;
    std::vector<local_var_id> param_name;
    pstat ch;
    virtual object* eval(rt_context* context);
};

class varible_declaration : public statement {
   public:
    local_var_id vid;
    virtual void eval(rt_context* context);
};

}  // namespace ast
}  // namespace mua
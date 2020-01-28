#pragma once
#include <unordered_map>
#include <vector>

#include "ast.h"

namespace mua {
namespace ast {
class statement : public ast_base {
   public:
    virtual void eval(runtime* rt) = 0;
};

typedef std::shared_ptr<statement> pstat;

class expr_statement : public statement {
   public:
    pexpr exp;
    inline expr_statement(pexpr x) : exp(x) {}
    virtual void eval(runtime* rt);
};

class assign_statement : public statement {
   public:
    plexpr lexp;
    pexpr rexp;
    inline assign_statement(plexpr l, pexpr r) : lexp(l), rexp(r) {}
    virtual void eval(runtime* rt);
};

class block_statement : public statement {
   public:
    std::vector<pstat> ch;
    enum { none, break_stat, return_stat } last_stat = none;
    pexpr return_value;
    virtual void eval(runtime* rt);
};

bool is_control_flow_true(pexpr x, runtime* rt);

class if_statement : public statement {
   public:
    std::vector<pexpr> conditions;
    std::vector<pstat> stats;
    pstat else_stat;
    virtual void eval(runtime* rt);
};

class while_statement : public statement {
   public:
    pexpr condition;
    pstat ch;
    virtual void eval(runtime* rt);
};

class repeat_statement : public statement {
   public:
    pexpr condition;
    pstat ch;
    virtual void eval(runtime* rt);
};

class for_statement : public statement {
   public:
    local_var_id loop_var;
    pexpr begin, step, end;
    pstat ch;
    virtual void eval(runtime* rt);
};

class generic_for_statement : public statement {
   public:
    local_var_id loop_var;
    pexpr range;
    pstat ch;
    virtual void eval(runtime* rt);
};

class ast_function : public types::function {
   public:
    std::unordered_map<local_var_id, storage_id> captures;
    std::vector<local_var_id> param_name;
    pstat ch;
    runtime* binded_context;
    ~ast_function();

    virtual object* invoke(runtime* rt,
                           std::vector<const object*> params) const;
};

class lambda_expression : public expr {
   public:
    std::unordered_set<local_var_id> captures;
    std::vector<local_var_id> param_name;
    pstat ch;
    virtual object* eval(runtime* rt);
};

class varible_declaration : public statement {
   public:
    local_var_id vid;
    pexpr val;
    bool is_local_function = false;
    virtual void eval(runtime* rt);
};
}  // namespace ast
}  // namespace mua
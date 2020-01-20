#pragma once
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

}
}
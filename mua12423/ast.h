#pragma once
#include "context.h"
#include "types.h"
#include "utils.h"
#include <memory>
#include <vector>
using namespace mua::types;
using namespace mua::runtime;

namespace mua {
namespace ast {
class ast_base {
   public:
    virtual ~ast_base() = 0;
};

class expr : public ast_base {
   public:
    inline virtual ~expr() {}
    virtual object* eval(runtime_context* context) = 0;
};

class lexpr : virtual public expr {
   public:
    inline virtual ~lexpr() {}
    virtual void set_value(runtime_context* context, const object* val) = 0;
};

typedef std::shared_ptr<expr> pexpr;
typedef std::shared_ptr<lexpr> plexpr;

class simple_constant : public expr {
   public:
    object* value;
    inline simple_constant(const object* val) : value(val->clone()) {}
    inline virtual object* eval(runtime_context* context) {
        return value->clone();
    }
    ~simple_constant() { delete value; }
};

class table_constant : public expr {
   public:
    inline virtual object* eval(runtime_context* context) {
        return new table_pointer(new table(), true);
    }
};

class global_varible : virtual public expr, virtual public lexpr {
   public:
    std::string name;
    inline global_varible(const std::string& n) : name(n) {}
    inline object* eval(runtime_context* context) {
        return context->get_global_varible(name);
    }
    inline void set_value(runtime_context* context, const object* val) {
        context->set_global_varible(name, val);
    }
};

class local_varible : virtual public expr, virtual public lexpr {
   public:
    std::string name;
    local_var_id id;
    inline local_varible(local_var_id id, const std::string& n = "") : id(id), name(n) {}
    inline object* eval(runtime_context* context) {
        return context->get_local_varible(id);
    }
    inline void set_value(runtime_context* context, const object* val) {
        context->set_local_varible(id, val);
    }
};

class member_access : virtual public expr, virtual public lexpr {
   public:
    pexpr obj;
    pexpr member_name;
    inline member_access(pexpr obj, pexpr member)
        : obj(obj), member_name(member) {}
    object* eval(runtime_context* context);
    void set_value(runtime_context* context, const object* val);
};

class functional_call : public expr {
   public:
    pexpr func;
    std::vector<pexpr> params;
    inline functional_call(pexpr a, std::vector<pexpr> b)
        : func(a), params(b) {}
    object* eval(runtime_context* context);
};

class statement : public ast_base {
   public:
    virtual void eval(runtime_context* context) = 0;
};

class expr_statement : public statement {
   public:
    pexpr exp; 
    inline virtual void eval(runtime_context* context) {
        auto res = exp->eval(context);
        delete res;
    }
};

class assign_statement : public statement {
   public:
    plexpr lexp;
    pexpr rexp;
    inline virtual void eval(runtime_context* context) {
        auto val = rexp->eval(context);
        lexp->set_value(context, val);
        delete val;
    }
};

}  // namespace ast
}  // namespace mua
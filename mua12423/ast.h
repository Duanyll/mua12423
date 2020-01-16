#pragma once
#include "context.h"
#include "types.h"
#include <memory>
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

typedef std::shared_ptr<expr> pexpr;

class lexpr : virtual public expr {
   public:
    inline virtual ~lexpr() {}
    virtual void set_value(runtime_context* context, const object* val) = 0;
};

class simple_constant : public expr {
    object* value;

   public:
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
    std::string name;

   public:
    inline global_varible(std::string n) : name(n) {}
    inline object* eval(runtime_context* context) {
        return context->get_global_varible(name);
    }
    inline void set_value(runtime_context* context, const object* val) {
        context->set_global_varible(name, val);
    }
};

class local_varible : virtual public expr, virtual public lexpr {
    std::string name;
    local_var_id id;

   public:
    inline local_varible(local_var_id id, std::string n = "") : id(id), name(n) {}
    inline object* eval(runtime_context* context) {
        return context->get_local_varible(id);
    }
    inline void set_value(runtime_context* context, const object* val) {
        context->set_local_varible(id, val);
    }
};

class member_access : virtual public expr, virtual public lexpr {
    pexpr obj;
    pexpr member_name;

   public:
    inline member_access(pexpr obj, pexpr member)
        : obj(obj), member_name(member) {}
    inline object* eval(runtime_context* context) {
        auto u = obj->eval(context);
        if (u->get_typeid() == TABLE) {
            auto t = static_cast<table_pointer*>(u)->ptr;
            auto key = member_name->eval(context);
            auto result = t->get_copy(key);
            delete u;
            delete t;
            return result;
        } else {
            delete u;
            return new nil();
        }
    }
    inline void set_value(runtime_context* context, const object* val) {
        auto u = obj->eval(context);
        if (u->get_typeid() == TABLE) {
            auto t = static_cast<table_pointer*>(u)->ptr;
            auto key = member_name->eval(context);
            t->set_copy(key, val);
            delete u;
            delete t;
            return;
        } else {
            delete u;
            return;
        }
    }
};

}  // namespace ast
}  // namespace mua
#pragma once
#include <memory>
#include <vector>

#include "runtime.h"
#include "types.h"
#include "utils.h"

namespace mua {
namespace ast {
using namespace mua::types;
class ast_base {
   public:
    inline virtual ~ast_base() {}
};

class expr : public ast_base {
   public:
    inline virtual ~expr() {}
    virtual object* eval(runtime* rt) = 0;
};

class lexpr : virtual public expr {
   public:
    inline virtual ~lexpr() {}
    virtual void set_value(runtime* rt, const object* val) = 0;
};

typedef std::shared_ptr<expr> pexpr;
typedef std::shared_ptr<lexpr> plexpr;

class simple_constant : public expr {
   public:
    object* value;
    inline simple_constant(const object* val) : value(val->clone()) {}
    inline virtual object* eval(runtime* rt) { return value->clone(); }
    ~simple_constant() { delete value; }
};

class table_constant : public expr {
   public:
    inline table_constant() {}
    inline virtual object* eval(runtime* rt) {
        return new table_pointer(new table(), true);
    }
};

class global_varible : virtual public expr, virtual public lexpr {
   public:
    std::string name;
    inline global_varible(const std::string& n) : name(n) {}
    inline object* eval(runtime* rt) { return rt->get_global_varible(name); }
    inline void set_value(runtime* rt, const object* val) {
        rt->set_global_varible(name, val);
    }
};

class local_varible : virtual public expr, virtual public lexpr {
   public:
    std::string name;
    local_var_id id;
    inline local_varible(local_var_id id, const std::string& n = "")
        : id(id), name(n) {}
    inline object* eval(runtime* rt) { return rt->get_local_varible(id); }
    inline void set_value(runtime* rt, const object* val) {
        rt->set_local_varible(id, val);
    }
};

class member_access : virtual public expr, virtual public lexpr {
   public:
    pexpr obj;
    pexpr member_name;
    inline member_access(pexpr obj, pexpr member)
        : obj(obj), member_name(member) {}
    object* eval(runtime* rt);
    void set_value(runtime* rt, const object* val);
};

class functional_call : public expr {
   public:
    pexpr func;
    std::vector<pexpr> params;
    inline functional_call(pexpr a, std::vector<pexpr> b)
        : func(a), params(b) {}
    object* eval(runtime* rt);
};
}  // namespace ast
}  // namespace mua
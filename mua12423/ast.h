#pragma once
#include "types.h"
#include "context.h"
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
}
}